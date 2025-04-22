#include "lock_free_hash_map.hpp"
#include <asio.hpp>
#include <iostream>
#include <sstream>
#include <string>

using asio::ip::tcp;
using kv::LockFreeHashMap;

// our global, in‑memory store
static LockFreeHashMap<std::string, std::string> KV;

// Parse a single request line and produce a response
std::string handle_request(const std::string &line) {
  std::istringstream iss(line);
  std::string cmd, key;
  iss >> cmd >> key;

  std::ostringstream oss;
  if (cmd == "GET") {
    auto v = KV.get(key);
    if (v)
      oss << "+VALUE " << *v;
    else
      oss << "-ERR NotFound";
  } else if (cmd == "SET") {
    // everything after “SET key” is the value
    std::string value;
    std::getline(iss, value);
    if (!value.empty() && value.front() == ' ')
      value.erase(0, 1);
    KV.put(key, std::move(value));
    oss << "+OK";
  } else if (cmd == "DEL") {
    if (KV.erase(key))
      oss << "+OK";
    else
      oss << "-ERR NotFound";
  } else {
    oss << "-ERR UnknownCommand";
  }
  return oss.str();
}

int main(int argc, char *argv[]) {
  unsigned short port = 12345;
  if (argc > 1)
    port = static_cast<unsigned short>(std::stoi(argv[1]));

  asio::io_context io_ctx;
  tcp::acceptor acceptor(io_ctx, tcp::endpoint(tcp::v4(), port));
  std::cout << "KV server listening on port " << port << "\n";

  // simple thread‐pool: each session gets its own worker thread
  const auto max_threads = std::max(1u, std::thread::hardware_concurrency());

  while (true) {
    tcp::socket sock(io_ctx);
    acceptor.accept(sock);

    // hand off each connection to its own thread
    std::thread([s = std::move(sock)]() mutable {
      try {
        asio::streambuf buf;
        asio::read_until(s, buf, "\n");

        std::istream is(&buf);
        std::string line;
        std::getline(is, line);

        auto resp = handle_request(line) + "\n";
        asio::write(s, asio::buffer(resp));
      } catch (std::exception &e) {
        std::cerr << "Session error: " << e.what() << "\n";
      }
    }).detach();
  }

  return 0;
}