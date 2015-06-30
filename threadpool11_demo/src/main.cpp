/*!
Copyright (c) 2013, 2014, 2015 Tolga HOŞGÖR
All rights reserved.

This file is part of threadpool11.

    threadpool11 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    threadpool11 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with threadpool11.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "threadpool11/threadpool11.hpp"

#include <algorithm>
#include <array>
#include <cstdio>
#include <iostream>
#include <mutex>
#include <thread>

namespace {

std::mutex coutMutex;

void test1Func() {
  coutMutex.lock();
  // heavy job that takes 1 second
  std::cout << "Waiting thread id: " << std::this_thread::get_id() << std::endl;
  coutMutex.unlock();
  std::this_thread::sleep_for(std::chrono::seconds(1));
}

void test2Func() { volatile int i = std::min(5, rand()); }

std::atomic<uint64_t> test3Var(0);
void test3Func() {
  volatile uint16_t var = 0;
  while (++var < (std::numeric_limits<uint16_t>::max()))
    ;
  ++test3Var;
}

std::size_t factorial(std::size_t i) {
  i = std::max(1ul, i);

  std::size_t res = i;

  while (i > 2) {
    res *= --i;
  }

  return res;
}

} // NS

int main(int argc, char* argv[]) {
  threadpool11::Pool pool;

  std::cout << "Your machine's hardware concurrency is " << std::thread::hardware_concurrency() << std::endl
            << std::endl;

  /**
  * Demo #1.
  */
  {
    std::cout
        << "This demo is about showing how parallelization increases the performance. "
        << "In this test case, work is simulated by making the threads inactive by sleeping for 1 second. "
        << "It will seem like all jobs complete in ~1 second even in a single core machine. "
        << "However, in real life cases, work would keep CPU busy so you would not get any real benefit "
           "using "
        << "thread numbers that are higher than your machine's hardware concurrency (threads that are "
           "executed "
        << "concurrently) except in some cases like doing file IO." << std::endl << std::endl;
    {
      std::cout << "Demo 1\n";
      std::cout << "Executing 5 test1Func() WITHOUT posting to thread pool:\n";
      auto begin = std::chrono::high_resolution_clock::now();
      /*test1Func();
      test1Func();
      test1Func();
      test1Func();
      test1Func();*/
      auto end = std::chrono::high_resolution_clock::now();
      std::cout << "\texecution took "
                << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()
                << " milliseconds.\n\n";
    }

    {
      std::cout << "Executing 5 test1Func() WITH posting to thread pool:" << std::endl;
      std::vector<std::future<void>> futures;
      auto begin = std::chrono::high_resolution_clock::now();
      futures.emplace_back(pool.postWork<void>(test1Func));
      futures.emplace_back(pool.postWork<void>(test1Func));
      futures.emplace_back(pool.postWork<void>(test1Func));
      futures.emplace_back(pool.postWork<void>(test1Func));
      futures.emplace_back(pool.postWork<void>(test1Func));
      for (auto& it : futures)
        it.get();
      auto end = std::chrono::high_resolution_clock::now();
      std::cout << "\tDemo 1 took "
                << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()
                << " milliseconds." << std::endl << std::endl;
    }
  }

  /*!
    Helper function tests
  */
  {
    std::cout << "Increasing worker count by 5." << std::endl;
    pool.incWorkerCountBy(5);
    std::cout << "Current worker count is " << pool.getWorkerCount() << ". Async setting worker count to "
              << std::thread::hardware_concurrency() << "... " << std::flush;
    pool.setWorkerCount(std::thread::hardware_concurrency());
    std::cout << "The new worker count is (may still be 13) " << pool.getWorkerCount() << "." << std::endl;
    std::cout << "Waiting 1 second..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "The new worker count is " << pool.getWorkerCount() << "." << std::endl;
    std::cout << "Increasing worker count by 5." << std::endl;
    pool.incWorkerCountBy(5);
    std::cout << "Current worker count is " << pool.getWorkerCount() << ". Sync setting worker count to "
              << std::thread::hardware_concurrency() << "... " << std::flush;
    pool.setWorkerCount(std::thread::hardware_concurrency(), threadpool11::Pool::Method::SYNC);
    std::cout << "The new worker count is " << pool.getWorkerCount() << "." << std::endl << std::endl;
  }

  /**
  * Demo #2
  */
  {
    std::cout << "Demo 2" << std::endl;
    std::cout << "Posting 1.000.000 jobs." << std::endl;

    std::vector<std::future<void>> futures;
    auto begin = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000000; ++i) {
      futures.emplace_back(pool.postWork<void>(test2Func));
    }
    auto end = std::chrono::high_resolution_clock::now();
    for (auto& it : futures)
      it.get();
    auto end2 = std::chrono::high_resolution_clock::now();

    std::cout << "Demo 2 took " << std::chrono::duration_cast<std::chrono::milliseconds>(end2 - begin).count()
              << " milliseconds. (Posting: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()
              << " ms, getting: " << std::chrono::duration_cast<std::chrono::milliseconds>(end2 - end).count()
              << " ms)" << std::endl << std::endl;
  }

  /**
  * Demo #3
  * You should always capture by value or use appropriate mutexes for reference access.
  */
  {
    std::cout << "Demo 3" << std::endl;
    std::cout << "Testing work queue flow." << std::endl;
#define th11_demo_iterations 30000
    // pool.increaseWorkerCountBy(th11_demo_iterations - 2);
    std::array<std::future<void>, th11_demo_iterations> futures;
    auto begin = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < th11_demo_iterations; ++i)
      futures[i] = pool.postWork<void>([=]() { test3Func(); });
    for (int i = 0; i < th11_demo_iterations; ++i)
      futures[i].get();
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Variable is: " << test3Var << ", expected: " << th11_demo_iterations << std::endl;
    std::cout << "Demo 3 took " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()
              << " milliseconds." << std::endl << std::endl;
  }

  /**
  * Demo #4
  * Using futures.
  */
  {
    std::cout << "Demo 4\n";
    std::cout
        << "WARNING: This test's output may be distorted because no synchronization on std::cout is done.\n";
    std::array<std::future<float>, 20> futures;

    auto begin = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 20; i++) {
      futures[i] = pool.postWork<float>([=]() {
        std::cout << "\tExecuted pow(" << i << ", 2) by thread id " << std::this_thread::get_id()
                  << std::endl;
        return pow(i, 2);
      });
    }

    for (int i = 0; i < 20; i++) {
      std::cout << "\tfuture[" << i << "] value: " << futures[i].get() << std::endl;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Demo 4 took " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()
              << " milliseconds." << std::endl << std::endl;
  }
  
  /**
   * Demo #5
   * For performance test purposes.
   */
   
  const constexpr auto iter = 300000ul;
  std::array<std::size_t, iter> a;

  {
    pool.setWorkerCount(std::thread::hardware_concurrency());
    
    std::array<std::future<std::size_t>, iter> futures;

    const auto begin = std::chrono::high_resolution_clock::now();

    for (auto i = 0u; i < iter; ++i) {
      futures[i] = pool.postWork<std::size_t>([i]() { return factorial(i); });
    }

    for (auto i = 0u; i < iter; ++i) {
      a[i] = futures[i].get();
    }

    const auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Demo 5 took "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()
              << " milliseconds." << std::endl << std::endl;
  }

  /**
  * Test case for Issue #1 (fixed): Pool::postWork waiting forever, due to posting work before all threads in
  * pool
  * are properly initialized and waiting.
  */
  /*{
    while(true)
    {
      std::cout << "Loop begin" << std::endl;

      std::vector<std::future<void>> futures;
      futures.emplace_back(pool.postWork<void>([]{std::cout << std::this_thread::get_id() << " heyyo1" <<
  std::endl;}));
      futures.emplace_back(pool.postWork<void>([]{std::cout << std::this_thread::get_id() << " heyyo2" <<
  std::endl;}));
      futures.emplace_back(pool.postWork<void>([]{std::cout << std::this_thread::get_id() << " heyyo3" <<
  std::endl;}));
      futures.emplace_back(pool.postWork<void>([]{std::cout << std::this_thread::get_id() << " heyyo4" <<
  std::endl;}));
      futures.emplace_back(pool.postWork<void>([]{std::cout << std::this_thread::get_id() << " heyyo5" <<
  std::endl;}));
      futures.emplace_back(pool.postWork<void>([]{std::cout << std::this_thread::get_id() << " heyyo6" <<
  std::endl;}));
      futures.emplace_back(pool.postWork<void>([]{std::cout << std::this_thread::get_id() << " heyyo7" <<
  std::endl;}));
      futures.emplace_back(pool.postWork<void>([]{std::cout << std::this_thread::get_id() << " heyyo8" <<
  std::endl;}));

      //pool[0].setWork([]{});
      std::cout << "waiting 1" << std::endl;
      for(auto& it : futures)
        it.get();
      std::cout << "wait 1 end" << std::endl;
      futures.clear();

      futures.emplace_back(pool.postWork<void>([]{std::cout << std::this_thread::get_id() << " 2heyyo1" <<
  std::endl;}));
      futures.emplace_back(pool.postWork<void>([]{std::cout << std::this_thread::get_id() << " 2heyyo2" <<
  std::endl;}));
      futures.emplace_back(pool.postWork<void>([]{std::cout << std::this_thread::get_id() << " 2heyyo3" <<
  std::endl;}));
      futures.emplace_back(pool.postWork<void>([]{std::cout << std::this_thread::get_id() << " 2heyyo4" <<
  std::endl;}));

      std::cout << "waiting 2" << std::endl;
      for(auto& it : futures)
        it.get();
      std::cout << "wait 2 end" << std::endl;
      futures.clear();

      std::cout << "Worker count: " << pool.getWorkerCount() << std::endl;

      std::cout << "Increasing worker count by 4." << std::endl;
      pool.incWorkerCountBy(4);

      std::cout << "Worker count: " << pool.getWorkerCount() << std::endl;

      std::cout << "Decreasing thread count by 4." << std::endl;
      pool.decWorkerCountBy(4);

      std::cout << "Worker count: " << pool.getWorkerCount() << std::endl;

      std::cout << std::endl << std::endl << std::endl << std::endl << std::endl << std::endl;

      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
  }*/

  std::cout << std::endl << std::endl;

  std::cout << "Demos completed." << std::endl;

  // std::cin.get();
  return 0;
}
