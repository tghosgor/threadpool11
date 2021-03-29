#include "threadpool11/threadpool11.hpp"

#include <iostream>
#include <vector>

std::size_t factorial(std::size_t i) {
  i = std::max(1ul, i);

  std::size_t res = i;

  while (i > 2) {
    res *= --i;
  }

  return res;
}

int main(int argc, char* argv[]) {
  std::cout << "Your machine's hardware concurrency is " << std::thread::hardware_concurrency() << std::endl
            << std::endl;

  const constexpr auto iter = 900000ul;

  {
    threadpool11::pool pool;

    std::vector<std::size_t> a;
    a.reserve(iter);

    std::vector<std::future<std::size_t>> futures;
    futures.reserve(iter);

    const auto begin = std::chrono::high_resolution_clock::now();

    for (auto i = 0u; i < iter; ++i) {
      futures.emplace_back(pool.post_work<std::size_t>([i]() { return factorial(i % 100000); }));
    }

    for (auto i = 0u; i < iter; ++i) {
      a.emplace_back(futures[i].get());
    }

    const auto end = std::chrono::high_resolution_clock::now();
    std::cout << "threadpool11 execution took "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()
              << " milliseconds." << std::endl << std::endl;
  }

  {
    threadpool11::pool pool;

    std::vector<std::size_t> a;
    a.reserve(iter);

    const auto begin = std::chrono::high_resolution_clock::now();

    for (auto i = 0u; i < iter; ++i) {
      pool.post_work<std::size_t>([i]() { return factorial(i % 100000); }, threadpool11::pool::no_future_tag);
    }
    pool.join_all();

    const auto end = std::chrono::high_resolution_clock::now();
    std::cout << "threadpool11 (no future) execution took "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()
              << " milliseconds." << std::endl << std::endl;
  }

  {
    std::vector<std::size_t> a;
    a.reserve(iter);

    const auto begin = std::chrono::high_resolution_clock::now();

#pragma omp parallel for
    for (auto i = 0u; i < iter; ++i) {
      a.emplace_back(factorial(i % 100000));
    }

    const auto end = std::chrono::high_resolution_clock::now();
    std::cout << "openmp execution took "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()
              << " milliseconds." << std::endl << std::endl;
  }

  {
    std::vector<std::size_t> a;
    a.reserve(iter);

    const auto begin = std::chrono::high_resolution_clock::now();

#pragma omp parallel for schedule(dynamic, 1)
    for (auto i = 0u; i < iter; ++i) {
      a.emplace_back(factorial(i % 100000));
    }

    const auto end = std::chrono::high_resolution_clock::now();
    std::cout << "openmp execution (dynamic schedule) took "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()
              << " milliseconds." << std::endl << std::endl;
  }

  return 0;
}
