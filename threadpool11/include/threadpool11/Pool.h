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
  std::future<T> postWork(std::function<T()> callable, Work::Type const type = Work::Type::STD, Work::Prio const priority = Work::Prio::DEFERRED);

  /*!
   * This function joins all the threads in the thread pool as fast as possible.
   * All the posted works are NOT GUARANTEED to be finished before the worker threads
   * are destroyed and this function returns.
   *
   * However, ongoing works in the threads in the pool are guaranteed
   * to finish before that threads are terminated.
   *
   * Properties: NOT thread-safe.
   *
   */
  threadpool11_EXPORT
  void joinAll();

  /*!
   * \brief Pool::getWorkerCount
   * not thread-safe.
   * \return The number of worker threads.
   */
  threadpool11_EXPORT
  size_t getWorkerCount() const;

  /*!
   * \brief setWorkerCount
   * \param n
   * Sets the number of workers to 'n'
   */
  threadpool11_EXPORT
  void setWorkerCount(size_t const& n);

  /*!
   * This function requires a mutex lock so you should call it
   * wisely if you performance is a life matter to you.
   */
  threadpool11_EXPORT
  size_t getWorkQueueSize() const;

  /*!
   * Increases the number of threads in the pool by n.
   */
  threadpool11_EXPORT
  void increaseWorkerCountBy(size_t const& n);

  /*!
   * Tries to decrease the number of threads in the pool by n.
   * Setting 'n' higher than the number of workers has no effect.
   *
   * WARNING: This function is async. It will return before the threads are joined. It will just post
   * 'n' requests for termination. This means that if you call this function multiple times before
   * those requests are served, worker termination requests will pile up. It can even kill the newly
   * created workers if all workers are removed before all requests are processed.
   */
  threadpool11_EXPORT
  void decreaseWorkerCountBy(size_t n = std::numeric_limits<size_t>::max());

private:
  Pool(Pool&&);
  Pool(Pool const&);
  Pool& operator=(Pool&&);
  Pool& operator=(Pool const&);

  std::deque<Worker> workers;

  mutable std::mutex workSignalMutex;
  std::condition_variable workSignal;

  boost::lockfree::queue<Work::Callable*> workQueue;
  std::atomic<size_t> workQueueSize;

  void spawnWorkers(size_t n);
};

template<typename T>
threadpool11_EXPORT
inline std::future<T> Pool::postWork(std::function<T()> callable, Work::Type const type, Work::Prio const prio)
{
  std::promise<T> promise;
  auto future = promise.get_future();

  /* TODO: how to avoid copy of callable into this lambda and the ones below? In a decent way... */
  /* evil move hack */
  auto move_callable = make_move_on_copy(std::move(callable));
  /* evil move hack */
  auto move_promise = make_move_on_copy(std::move(promise));

  std::unique_lock<std::mutex> workSignalLock(workSignalMutex);
  ++workQueueSize;
  workQueue.push(new Work::Callable([move_callable, move_promise, type]() mutable { move_promise.Value().set_value((move_callable.Value())()); return type; }));
  workSignal.notify_one();

  return future;
}

template<>
threadpool11_EXPORT
inline std::future<void> Pool::postWork(std::function<void()> callable, Work::Type const type, Work::Prio const prio)
{
  std::promise<void> promise;
  auto future = promise.get_future();

  /* evil move hack */
  auto move_callable = make_move_on_copy(std::move(callable));
  /* evil move hack */
  auto move_promise = make_move_on_copy(std::move(promise));

  std::unique_lock<std::mutex> workSignalLock(workSignalMutex);
  ++workQueueSize;
  workQueue.push(new Work::Callable([move_callable, move_promise, type]() mutable { (move_callable.Value())(); move_promise.Value().set_value(); return type; }));
  workSignal.notify_one();

  return future;
}

#undef threadpool11_EXPORT
#undef threadpool11_EXPORTING
}
