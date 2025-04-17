#include <iostream>
#include <fmt/core.h>
#include <spdlog/spdlog.h>

int main(){
    fmt::print("Hello from fmt!\n");
    spdlog::info("Hello from spdlog!\n");
    return 0;
}