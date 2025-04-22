#include <asio.hpp>
#include <iostream>
#include <string>

using asio::ip::tcp;

int main(int argc, char *argv[]) {
  if (argc < 3) {
    std::cerr << "Usage: kv-cli <GET|SET|DEL> <key> [value]\n";
    return 1;
  }

  std::string cmd = argv[1];
  std::string key = argv[2];
  std::string val;

  if (cmd == "SET") {
    if (argc < 4) {
      std::cerr << "Usage: kv-cli SET <key> <value>\n";
      return 1;
    }
    val = argv[3];
  }

  // build request line
  std::string req = cmd + " " + key;
  if (cmd == "SET")
    req += " " + val;
  req += "\n";

  try {
    asio::io_context io_ctx;
    tcp::socket sock(io_ctx);
    sock.connect({asio::ip::make_address("127.0.0.1"), 12345});

    asio::write(sock, asio::buffer(req));

    asio::streambuf resp_buf;
    asio::read_until(sock, resp_buf, "\n");
    std::istream is(&resp_buf);

    std::string line;
    std::getline(is, line);
    std::cout << line << "\n";
  } catch (std::exception &e) {
    std::cerr << "CLI error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}