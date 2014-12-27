#include "threadpool11/threadpool11.hpp"

#include <iostream>
#include <chrono>
#include <cstdlib>

//#define TEST_1
#define TEST_2
//#define TEST_0

namespace {

int recursiveWork(threadpool11::Pool& pool, int depth) {
  if (depth <= 0)
    return 0;
  auto result = pool.postWork<int>([]() { return 1; });
  recursiveWork(pool, --depth);
  return result.get();
}

} // NS

int main(int argc, char* argv[]) {
  try {
    int threads = 8;
    int recursion = 100;
    int initial_jobs = 100000;
    auto t0 = std::chrono::high_resolution_clock::now();
#ifdef TEST_2
    threadpool11::Pool pool(threads);
#endif // TEST_2
    for (int i = 0; i < initial_jobs; i++) {
#ifdef TEST_0
      threadpool11::Pool pool(threads);
// recursiveWork(pool, recursion);
#elif defined(TEST_1)
      threadpool11::Pool pool(threads);
      recursiveWork(pool, recursion);
      pool.joinAll();
#elif defined(TEST_2)
      recursiveWork(pool, recursion);
#endif
    }
#ifdef TEST_2
    pool.joinAll();
#endif // TEST_2
    std::cout << "finish" << std::endl;
    auto t1 = std::chrono::high_resolution_clock::now();
    std::cout << "threads: " << threads << " depth: " << recursion << std::endl;
    std::cout << "ns/job: "
              << std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count() /
                     ((recursion + 1) * initial_jobs) << std::endl;
  } catch (...) {
    std::cout << "exception" << std::endl;
  }
  return 0;
}
