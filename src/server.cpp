#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <asio.hpp>
#include <csignal>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "lock_free_hash_map.hpp"

using asio::ip::tcp;
using kv::LockFreeHashMap;

static LockFreeHashMap<std::string, std::string> KV;

// Handle a single client session
void session(tcp::socket sock) {
  try {
    // Enforce a 5s read timeout on the underlying socket
    struct timeval tv {
      .tv_sec = 5, .tv_usec = 0
    };
    ::setsockopt(sock.native_handle(), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    asio::streambuf buf;
    asio::read_until(sock, buf, "\n");

    std::istream is(&buf);
    std::string line;
    std::getline(is, line);

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
      std::string value;
      std::getline(iss, value);
      if (!value.empty() && value.front() == ' ') value.erase(0, 1);
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

    auto response = oss.str() + "\n";
    asio::write(sock, asio::buffer(response));
  } catch (const std::exception& e) {
    std::cerr << "Session error: " << e.what() << '\n';
  }
}

int main(int argc, char* argv[]) {
  unsigned short port = 12345;
  if (argc > 1) port = static_cast<unsigned short>(std::stoi(argv[1]));

  asio::io_context io_ctx;

  // Setup acceptor
  tcp::endpoint ep(tcp::v4(), port);
  tcp::acceptor acceptor(io_ctx);
  acceptor.open(ep.protocol());
  acceptor.set_option(asio::socket_base::reuse_address(true));
  acceptor.bind(ep);
  acceptor.listen();

  // Handle shutdown signals
  asio::signal_set signals(io_ctx, SIGINT, SIGTERM);
  signals.async_wait([&](auto /*ec*/, auto /*sig*/) {
    std::cout << "Signal received, shutting down..." << '\n';
    acceptor.close();
    io_ctx.stop();
  });

  std::cout << "KV server listening on port " << port << '\n';

  // Async accept loop
  std::function<void()> do_accept;
  do_accept = [&]() {
    acceptor.async_accept([&](std::error_code ec, tcp::socket sock) {
      if (!ec) {
        asio::post(io_ctx, [s = std::move(sock)]() mutable { session(std::move(s)); });
      }
      if (acceptor.is_open()) do_accept();
    });
  };
  do_accept();

  // Run on a thread pool
  unsigned int thread_count = std::max(1u, std::thread::hardware_concurrency());
  std::vector<std::thread> pool;
  pool.reserve(thread_count);
  for (unsigned int i = 0; i < thread_count; ++i) {
    pool.emplace_back([&]() { io_ctx.run(); });
  }

  // Wait for threads to finish
  for (auto& t : pool) t.join();

  std::cout << "KV server shut down cleanly." << '\n';
  return 0;
}