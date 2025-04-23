// A lock‑free, fixed‑size hash map with epoch‑based reclamation.

#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <utility>
#include <vector>

namespace kv {

/**
 * @brief Lock-free hash map with fixed bucket count and epoch-based GC.
 * @tparam K Key type (must be hashable and comparable).
 * @tparam V Value type (copyable/movable).
 * @tparam N Number of buckets (power-of-two).
 */
template <typename K, typename V, std::size_t N = 1 << 20>
class LockFreeHashMap {
 public:
  LockFreeHashMap() : global_epoch_(0) {
    for (auto& b : buckets_) {
      b.store(nullptr, std::memory_order_relaxed);
    }
  }

  ~LockFreeHashMap() {
    // Clean up live nodes
    for (auto& b : buckets_) {
      Node* node = b.load(std::memory_order_relaxed);
      while (node) {
        Node* next = node->next.load(std::memory_order_relaxed);
        delete node;
        node = next;
      }
    }
    // Reclaim retired nodes
    for (Node* r : retired_) delete r;
  }

  /**
   * @brief Insert or replace a key/value.
   */
  bool put(const K& key, V&& value) {
    const size_t idx = bucket_index(key);
    Node* new_node = new Node{key, std::move(value), nullptr, 0};
    Node* head = buckets_[idx].load(std::memory_order_acquire);
    do {
      new_node->next.store(head, std::memory_order_relaxed);
    } while (!buckets_[idx].compare_exchange_weak(head, new_node, std::memory_order_release,
                                                  std::memory_order_acquire));
    return true;
  }

  /**
   * @brief Get a copy of the value for a key.
   */
  std::optional<V> get(const K& key) const {
    const size_t idx = bucket_index(key);
    Node* node = buckets_[idx].load(std::memory_order_acquire);
    while (node) {
      if (node->key == key) return node->value;
      node = node->next.load(std::memory_order_acquire);
    }
    return std::nullopt;
  }

  /**
   * @brief Erase an entry if present.
   * @return true if erased, false if not found.
   */
  bool erase(const K& key) {
    const size_t idx = bucket_index(key);
    Node* prev = nullptr;
    Node* curr = buckets_[idx].load(std::memory_order_acquire);
    while (curr) {
      Node* next = curr->next.load(std::memory_order_acquire);
      if (curr->key == key) {
        if (prev) {
          // Unlink curr
          if (!prev->next.compare_exchange_strong(curr, next, std::memory_order_acq_rel,
                                                  std::memory_order_acquire)) {
            return erase(key);  // retry
          }
        } else {
          // Removing head
          if (!buckets_[idx].compare_exchange_strong(curr, next, std::memory_order_acq_rel,
                                                     std::memory_order_acquire)) {
            return erase(key);  // retry
          }
        }
        retire(curr);
        return true;
      }
      prev = curr;
      curr = next;
    }
    return false;
  }

 private:
  struct Node {
    K key;
    V value;
    std::atomic<Node*> next;
    uint64_t retire_epoch;
  };

  static constexpr size_t bucket_count = N;
  std::array<std::atomic<Node*>, bucket_count> buckets_;
  std::atomic<uint64_t> global_epoch_;
  std::vector<Node*> retired_;

  size_t bucket_index(const K& key) const noexcept {
    return std::hash<K>{}(key) & (bucket_count - 1);
  }

  /**
   * @brief Mark a node as retired and periodically reclaim.
   */
  void retire(Node* node) {
    node->retire_epoch = global_epoch_.fetch_add(1, std::memory_order_relaxed);
    retired_.push_back(node);
    if (retired_.size() > threshold_) {
      advance_epoch();
      reclaim_nodes();
    }
  }

  /**
   * @brief Advance the epoch counter.
   */
  void advance_epoch() noexcept { global_epoch_.fetch_add(1, std::memory_order_relaxed); }

  /**
   * @brief Free nodes retired two epochs ago.
   */
  void reclaim_nodes() {
    uint64_t safe_epoch = global_epoch_.load(std::memory_order_relaxed) - 2;
    auto it = retired_.begin();
    while (it != retired_.end()) {
      if ((*it)->retire_epoch < safe_epoch) {
        delete *it;
        it = retired_.erase(it);
      } else {
        ++it;
      }
    }
  }

  static constexpr size_t threshold_ = 64;
};

}  // namespace kv