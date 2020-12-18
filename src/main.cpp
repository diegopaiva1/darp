/**
 * @file    main.cpp
 * @author  Diego Paiva
 * @date    24/09/2019
 */

#include "instance.hpp"
#include "algorithms/reactive_grasp.hpp"

#include <iostream> // std::cerr, std::cout

int main(const int argc, const char* argv[])
{
  const int min_args = 2, max_args = 3, args_given = argc - 1;

  if (args_given < min_args || args_given > max_args) {
    std::cerr << "Usage: " << argv[0] << " <instance> <threads> <save_file (optional)>\n";
    return EXIT_FAILURE;
  }

  inst.init(argv[1]);

  Run run = algorithms::reactive_grasp(
    1000, 50, {0.10, 0.20, 0.30, 0.40, 0.50, 0.60, 0.70, 0.80, 0.90, 1.0}, std::stoi(argv[2])
  );

  std::cout << run.to_string();

  if (argv[3])
    run.persist(argv[3]);

  return EXIT_SUCCESS;
}
