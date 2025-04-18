#include <asio.hpp>
#include <iostream>
#include <string>

using asio::ip::tcp;

auto main(int argc, char *argv[]) -> int {
  try {
    unsigned short port = 12345;
    if (argc > 1)
      port = static_cast<unsigned short>(std::stoi(argv[1]));

    asio::io_context io_ctx;
    tcp::acceptor acceptor(io_ctx, tcp::endpoint(tcp::v4(), port));

    std::cout << "Echo server listening on port " << port << "\n";

    for (;;) {
      tcp::socket sock(io_ctx);
      acceptor.accept(sock);

      // Read until newline
      asio::streambuf buf;
      asio::read_until(sock, buf, "\n");

      // Convert to string
      std::istream is(&buf);
      std::string line;
      std::getline(is, line);

      // Echo back (add newline)
      line += "\n";
      asio::write(sock, asio::buffer(line));

      // socket closes on destruction
    }
  } catch (std::exception &e) {
    std::cerr << "Server error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}