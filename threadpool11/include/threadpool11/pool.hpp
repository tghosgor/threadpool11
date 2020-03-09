/**
 * threadpool11
 * Copyright (C) 2013, 2014, 2015, 2016, 2017, 2018, 2019  Tolga HOSGOR
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#pragma once

#include "work.hpp"

#include <boost/lockfree/queue.hpp>

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>

#if defined(WIN32) && defined(threadpool11_DLL)
#ifdef threadpool11_EXPORTING
#define threadpool11_EXPORT __declspec(dllexport)
#else
#define threadpool11_EXPORT __declspec(dllimport)
#endif
#else
#define threadpool11_EXPORT
#endif

namespace threadpool11 {

class pool {
public:
  enum class method_t {
    SYNC,
    ASYNC,
  };
  template <class T>
  using callable_t = std::function<T()>;
  using size_type = std::size_t;

private:
  using work_t = work;
  using queue_t = boost::lockfree::queue<work_t*>;
  class no_future_t { friend class pool; no_future_t() {} };

public:
  threadpool11_EXPORT pool(size_type worker_count = std::thread::hardware_concurrency() / 2);

  ~pool();

  /**
   * \brief Posts a work to the pool for getting processed.
   *
   * if there are no threads left (i.e. you called pool::join_all(); prior to
   * this function) all the works you post gets enqueued. if you spawn new threads in
   * the future, they will be executed then.
   *
   * properties: thread-safe.
   */
  template <class T>
  threadpool11_EXPORT std::future<T> post_work(callable_t<T> callable) {
    return post_work(work_t::type_t::STANDARD, std::move(callable));
  }

  /**
   * Same as post_work(callable_t<T>) except does not have the overhead of futures.
   */
  template <class T>
  threadpool11_EXPORT void post_work(callable_t<T> callable, no_future_t) {
    return post_work(work_t::type_t::STANDARD, std::move(callable), no_future_tag);
  }

  /**
   * \brief join_all Joins the worker threads.
   *
   * This function joins all the threads in the thread pool as fast as possible.
   * All the posted works are NOT GUARANTEED to be finished before the worker threads
   * are destroyed and this function returns.
   *
   * However, ongoing works in the threads in the pool are guaranteed
   * to finish before that threads are terminated.
   *
   * Properties: NOT thread-safe.
   */
  threadpool11_EXPORT void join_all();

  /**
   * \brief get_worker_count
   *
   * \return The number of worker threads.
   *
   * Properties: NOT thread-safe.
   */
  threadpool11_EXPORT size_type get_worker_count() const { return worker_count_; }

  /**
   * \brief set_worker_count
   * \param n The number to set worker count to.
   * \param method The method to use for when the thread count is being decreased.
   *
   * method_t::ASYNC: It will return before the threads are joined. It will just post
   *  'n' requests for termination. This means that if you call this function multiple times,
   *  worker termination requests will pile up. It can even kill the newly
   *  created workers if all workers are removed before all requests are processed.
   *
   * method_t::SYNC: It won't return until the specified number of workers are actually destroyed.
   *  There still may be a few milliseconds delay before value returned by pool::get_worker_count is updated.
   *  But it will be more accurate compared to ASYNC one.
   *
   * Properties: NOT thread-safe.
   */
  threadpool11_EXPORT void set_worker_count(size_type n, method_t method = method_t::ASYNC);

  /**
   * \brief get_work_queue_size
   *
   * \return The number of work items that has not been acquired by workers.
   *
   * Properties: thread-safe.
   */
  threadpool11_EXPORT size_type get_work_queue_size() const { return work_queue_size_.load(std::memory_order_relaxed); }

  /**
   * \brief increase_worker_count Increases the number of threads in the pool by n.
   *
   * Properties: NOT thread-safe.
   */
  threadpool11_EXPORT void increase_worker_count(size_type n);

  /**
   * \brief decrease_worker_count Tries to decrease the number of threads in the pool by n.
   *
   * Setting 'n' higher than the number of workers has no effect.
   * Calling without arguments asynchronously terminates all workers.
   *
   * \warning This function behaves different based on second parameter.
   *
   * method_t::ASYNC: It will return before the threads are joined. It will just post
   *  'n' requests for termination. This means that if you call this function multiple times,
   *  worker termination requests will pile up. It can even kill the newly
   *  created workers if all workers are removed before all requests are processed.
   *
   * method_t::SYNC: It won't return until the specified number of workers are actually destroyed.
   *  There still may be a few milliseconds delay before value returned by pool::get_worker_count is updated.
   *  But it will be more accurate compared to ASYNC one.
   *
   * Properties: NOT thread-safe.
   */
  threadpool11_EXPORT void decrease_worker_count(size_type n = std::numeric_limits<size_type>::max(),
                                                 method_t method = method_t::ASYNC);

private:
  using mutex_t = std::mutex;
  using cv_t = std::condition_variable;

private:
  pool(pool&&) = delete;
  pool(pool const&) = delete;
  pool& operator=(pool&&) = delete;
  pool& operator=(pool const&) = delete;

  template <class T>
  threadpool11_EXPORT std::future<T> post_work(work_t::type_t type, callable_t<T> callable);

  template <class T>
  threadpool11_EXPORT void post_work(work_t::type_t type, callable_t<T> callable, no_future_t);

  template <class T>
  static void call_helper(callable_t<T> callable, std::shared_ptr<std::promise<T>> promise);

  template <class T>
  static void call_helper(callable_t<T> callable);

  void push(std::unique_ptr<work_t> work);

  void worker_main();

public:
  static const no_future_t no_future_tag;

private:
  size_type worker_count_;

  mutable mutex_t work_signal_mutex_;
  cv_t work_signal_;

  queue_t work_queue_;
  std::atomic<size_type> work_queue_size_;
};

template <class T>
inline void pool::call_helper(callable_t<T> callable, std::shared_ptr<std::promise<T>> promise) {
  auto&& val = callable();
  promise->set_value(std::move(val));
}

template <>
inline void pool::call_helper<void>(callable_t<void> callable, std::shared_ptr<std::promise<void>> promise) {
  callable();
  promise->set_value();
}

template <class T>
inline void pool::call_helper(callable_t<T> callable) {
  callable();
}

template <class T>
threadpool11_EXPORT inline std::future<T> pool::post_work(work_t::type_t type, callable_t<T> callable) {
  auto promise = std::make_shared<std::promise<T>>();
  auto future = promise->get_future();
  std::function<void()> func = std::bind(
    static_cast<void(*)(callable_t<T>, std::shared_ptr<std::promise<T>>)>(&pool::call_helper<T>),
    std::move(callable),
    std::move(promise));

  std::unique_ptr<work_t> work{new work_t{std::move(type), std::move(func)}};

  push(std::move(work));

  return future;
}

template <class T>
threadpool11_EXPORT inline void pool::post_work(work_t::type_t type, callable_t<T> callable, no_future_t) {
  std::function<void()> func = std::bind(
    static_cast<void(*)(callable_t<T>)>(&pool::call_helper<T>),
    std::move(callable));

  std::unique_ptr<work_t> work{new work_t{std::move(type), std::move(func)}};

  push(std::move(work));
}

inline void pool::push(std::unique_ptr<work_t> work) {
  work_queue_.push(work.release());

  {
    std::lock_guard<mutex_t> work_signal_lock(work_signal_mutex_);
    work_queue_size_.fetch_add(1, std::memory_order_relaxed);
  }
  work_signal_.notify_one();
}

inline void pool::worker_main() {
  while (true) {
    work_t* work_ptr;

    while (work_queue_.pop(work_ptr)) {
      const std::unique_ptr<work_t> work(work_ptr);

      work_queue_size_.fetch_sub(1, std::memory_order_relaxed);

      (*work)();

      if (work->type() == work_t::type_t::TERMINAL) {
        return;
      }
    }

    std::unique_lock<mutex_t> work_signal_lock(work_signal_mutex_);
    work_signal_.wait(work_signal_lock, [this]() { return work_queue_size_.load(std::memory_order_relaxed) > 0; });
  }
}

#undef threadpool11_EXPORT
#undef threadpool11_EXPORTING
}
