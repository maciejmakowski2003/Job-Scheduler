#include "CLI.h"
#include <cstdlib>
#include <iostream>

int main(int argc, char *argv[]) {
  std::size_t numThreads = 4;

  if (argc == 2) {
    int n = std::atoi(argv[1]);
    if (n <= 0) {
      std::cerr << "Usage: scheduler-cli [num_threads]\n"
                << "  num_threads must be a positive integer (default: 4)\n";
      return 1;
    }
    numThreads = static_cast<std::size_t>(n);
  } else if (argc > 2) {
    std::cerr << "Usage: scheduler-cli [num_threads]\n";
    return 1;
  }

  CLI cli(numThreads);
  cli.run();
  return 0;
}
