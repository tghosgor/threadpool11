/*!
Copyright (c) 2013, 2014, Tolga HOŞGÖR
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

#pragma once

#include <iostream>

#include <cassert>

#include <atomic>
#include <condition_variable>
#include <deque>
#include <future>
#include <list>
#include <forward_list>
#include <memory>
#include <mutex>

#include <boost/lockfree/queue.hpp>

#include "threadpool11/Worker.h"
#include "threadpool11/Helper.hpp"

#if defined(WIN32) && defined(threadpool11_DLL)
  #ifdef threadpool11_EXPORTING
    #define threadpool11_EXPORT __declspec(dllexport)
  #else
    #define threadpool11_EXPORT __declspec(dllimport)
  #endif
#else
  #define threadpool11_EXPORT
#endif

namespace threadpool11
{

class Pool
{
friend class Worker;

public:
  enum class Method
  {
    SYNC,
    ASYNC
  };

private:
  Pool(Pool&&);
  Pool(Pool const&);
  Pool& operator=(Pool&&);
  Pool& operator=(Pool const&);

  std::atomic<size_t> worker_count;

  mutable std::mutex work_signal_mtx;
  //bool work_signal_prot; //! wake up protection // <- work_queue_size is used instead of this
  std::condition_variable work_signal;

  boost::lockfree::queue<Work::Callable*> work_queue;
  std::atomic<size_t> work_queue_size;

  void spawnWorkers(size_t n);

  /*!
   * \brief executor
   * This is run by different threads to do necessary operations for queue processing.
   */
  void executor(std::unique_ptr<std::thread> self);

public:
  threadpool11_EXPORT
  Pool(size_t const& workerCount = std::thread::hardware_concurrency());
  ~Pool();

  /*!
   * Posts a work to the pool for getting processed.
   *
   * If there are no threads left (i.e. you called Pool::joinAll(); prior to
   * this function) all the works you post gets enqueued. If you spawn new threads in
   * future, they will be executed then.
   */
  template<typename T>
  threadpool11_EXPORT
  std::future<T> postWork(std::function<T()> callable, Work::Type const type = Work::Type::STD);

  /*!
   * This function joins all the threads in the thread pool as fast as possible.
   * All the posted works are NOT GUARANTEED to be finished before the worker threads
   * are destroyed and this function returns.
   *
   * However, ongoing works in the threads in the pool are guaranteed
   * to finish before that threads are terminated.
   *
   * Properties: NOT thread-safe.
   */
  threadpool11_EXPORT
  void joinAll();

  /*!
   * \brief Pool::getWorkerCount
   * not thread-safe.
   * \return The number of worker threads.
   */
  threadpool11_EXPORT
  decltype(worker_count.load()) getWorkerCount() const;

  /*!
   * \brief setWorkerCount
   * \param n
   *
   * WARNING: This function behaves different based on second parameter. (Only if decreasing)
   *
   * Method::ASYNC: It will return before the threads are joined. It will just post
   *  'n' requests for termination. This means that if you call this function multiple times,
   *  worker termination requests will pile up. It can even kill the newly
   *  created workers if all workers are removed before all requests are processed.
   *
   * Method::SYNC: It won't return until the specified number of workers are actually destroyed.
   *  There still may be a few milliseconds delay before value returned by Pool::getWorkerCount is updated.
   *  But it will be more accurate compared to ASYNC one.
   */
  threadpool11_EXPORT
  void setWorkerCount(decltype(worker_count.load()) const& n, Method const& method = Method::ASYNC);

  /*!
   * \brief getWorkQueueSize
   * \return The number of work items that has not been acquired by workers.
   */
  threadpool11_EXPORT
  decltype(work_queue_size.load()) getWorkQueueSize() const;

  /*!
   * Increases the number of threads in the pool by n.
   */
  threadpool11_EXPORT
  void incWorkerCountBy(decltype(worker_count.load()) const& n);

  /*!
   * Tries to decrease the number of threads in the pool by n.
   * Setting 'n' higher than the number of workers has no effect.
   *
   * WARNING: This function behaves different based on second parameter.
   *
   * Method::ASYNC: It will return before the threads are joined. It will just post
   *  'n' requests for termination. This means that if you call this function multiple times,
   *  worker termination requests will pile up. It can even kill the newly
   *  created workers if all workers are removed before all requests are processed.
   *
   * Method::SYNC: It won't return until the specified number of workers are actually destroyed.
   *  There still may be a few milliseconds delay before value returned by Pool::getWorkerCount is updated.
   *  But it will be more accurate compared to ASYNC one.
   */
  threadpool11_EXPORT
  void decWorkerCountBy(decltype(worker_count.load()) n = std::numeric_limits<decltype(worker_count.load())>::max(), Method const& method = Method::ASYNC);
};

template<typename T>
threadpool11_EXPORT
inline std::future<T> Pool::postWork(std::function<T()> callable, Work::Type const type)
{
  std::promise<T> promise;
  auto future = promise.get_future();

  /* TODO: how to avoid copy of callable into this lambda and the ones below? In a decent way... */
  /* evil move hack */
  auto move_callable = make_move_on_copy(std::move(callable));
  /* evil move hack */
  auto move_promise = make_move_on_copy(std::move(promise));

  std::unique_lock<std::mutex> workSignalLock(work_signal_mtx);
  ++work_queue_size;
  work_queue.push(new Work::Callable([move_callable, move_promise, type]() mutable {
    move_promise.Value().set_value( (move_callable.Value()) () );
    return type;
  }));
  work_signal.notify_one();

  return future;
}

template<>
threadpool11_EXPORT
inline std::future<void> Pool::postWork(std::function<void()> callable, Work::Type const type)
{
  std::promise<void> promise;
  auto future = promise.get_future();

  /* evil move hack */
  auto move_callable = make_move_on_copy(std::move(callable));
  /* evil move hack */
  auto move_promise = make_move_on_copy(std::move(promise));

  std::unique_lock<std::mutex> workSignalLock(work_signal_mtx);
  ++work_queue_size;
  work_queue.push(new Work::Callable([move_callable, move_promise, type]() mutable {
    (move_callable.Value()) ();
    move_promise.Value().set_value();
    return type;
  }));
  work_signal.notify_one();

  return future;
}

#undef threadpool11_EXPORT
#undef threadpool11_EXPORTING
}
