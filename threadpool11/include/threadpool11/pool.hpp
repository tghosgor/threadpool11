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

#pragma once

#include "worker.hpp"
#include "utility.hpp"

#include <atomic>
#include <cassert>
#include <condition_variable>
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

namespace boost {
namespace parameter {
struct void_;
}
namespace lockfree {
template <typename T,
          class A0,
          class A1,
          class A2>
class queue;
}
}

namespace threadpool11 {

class Pool {
  friend class Worker;

public:
  enum class Method { SYNC, ASYNC };

public:
  threadpool11_EXPORT Pool(std::size_t m_workerCount = std::thread::hardware_concurrency());
  ~Pool();

  /*!
   * Posts a work to the pool for getting processed.
   *
   * If there are no threads left (i.e. you called Pool::joinAll(); prior to
   * this function) all the works you post gets enqueued. If you spawn new threads in
   * future, they will be executed then.
   *
   * Properties: thread-safe.
   */
  template <typename T>
  threadpool11_EXPORT std::future<T> postWork(std::function<T()> callable, Work::Type type = Work::Type::STD);
  // TODO: convert 'type' above to const& when MSVC fixes that bug.

  /**
   * This function suspends the calling thread until all posted works are finished and, therefore, all worker
   * threads are free. It guarantees you that before the function returns, all queued works are finished.
   */
  threadpool11_EXPORT void waitAll();

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
  threadpool11_EXPORT void joinAll();

  /*!
   * \brief Pool::getWorkerCount
   *
   * Properties: thread-safe.
   *
   * \return The number of worker threads.
   */
  threadpool11_EXPORT size_t getWorkerCount() const;

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
  threadpool11_EXPORT void setWorkerCount(std::size_t n, Method method = Method::ASYNC);

  /*!
   * \brief getWorkQueueSize
   *
   * Properties: thread-safe.
   *
   * \return The number of work items that has not been acquired by workers.
   */
  threadpool11_EXPORT size_t getWorkQueueSize() const;

  /**
   * This function requires a mutex lock so you should call it wisely if you performance is a life matter to
   * you.
   */
  threadpool11_EXPORT size_t getActiveWorkerCount() const;

  /**
   * This function requires a mutex lock so you should call it wisely if you performance is a life matter to
   * you.
   */
  threadpool11_EXPORT size_t getInactiveWorkerCount() const;

  /*!
   * Increases the number of threads in the pool by n.
   *
   * Properties: thread-safe.
   */
  threadpool11_EXPORT void incWorkerCountBy(std::size_t n);

  /*!
   * Tries to decrease the number of threads in the pool by n.
   * Setting 'n' higher than the number of workers has no effect.
   * Calling without arguments asynchronously terminates all workers.
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
   *
   * Properties: thread-safe.
   */
  threadpool11_EXPORT void decWorkerCountBy(std::size_t n = std::numeric_limits<size_t>::max(),
                                            Method method = Method::ASYNC);

private:
  Pool(Pool&&) = delete;
  Pool(Pool const&) = delete;
  Pool& operator=(Pool&&) = delete;
  Pool& operator=(Pool const&) = delete;

  void spawnWorkers(std::size_t n);

  /*!
   * \brief executor
   * This is run by different threads to do necessary operations for queue processing.
   */
  void executor(std::unique_ptr<std::thread> self);

  /**
   * @brief push Internal usage.
   * @param workFunc
   */
  void push(Work::Callable* workFunc);

private:
  std::atomic<size_t> m_workerCount;
  std::atomic<size_t> m_activeWorkerCount;

  mutable std::mutex notify_all_finished_signal_mtx;
  std::condition_variable m_notifyAllFinishedSignal;
  bool m_areAllReallyFinished;

  mutable std::mutex m_workSignalMutex;
  // bool work_signal_prot; //! wake up protection // <- work_queue_size is used instead of this
  std::condition_variable m_workSignal;

  std::unique_ptr<
      boost::lockfree::
          queue<Work::Callable*, boost::parameter::void_, boost::parameter::void_, boost::parameter::void_>>
      m_workQueue;
  std::atomic<size_t> m_workQueueSize;
};

template <typename T>
threadpool11_EXPORT inline std::future<T> Pool::postWork(std::function<T()> callable, Work::Type type) {
  std::promise<T> promise;
  auto future = promise.get_future();

  /* TODO: how to avoid copy of callable into this lambda and the ones below? In a decent way... */
  /* evil move hack */
  auto move_callable = make_move_on_copy(std::move(callable));
  /* evil move hack */
  auto move_promise = make_move_on_copy(std::move(promise));

  auto workFunc = new Work::Callable([move_callable, move_promise, type]() mutable {
    move_promise.Value().set_value((move_callable.Value())());
    return type;
  });

  push(workFunc);

  return future;
}

template <>
threadpool11_EXPORT inline std::future<void> Pool::postWork(std::function<void()> callable,
                                                            Work::Type const type) {
  std::promise<void> promise;
  auto future = promise.get_future();

  /* evil move hack */
  auto move_callable = make_move_on_copy(std::move(callable));
  /* evil move hack */
  auto move_promise = make_move_on_copy(std::move(promise));

  auto workFunc = new Work::Callable([move_callable, move_promise, type]() mutable {
    (move_callable.Value())();
    move_promise.Value().set_value();
    return type;
  });

  push(workFunc);

  return future;
}

#undef threadpool11_EXPORT
#undef threadpool11_EXPORTING
}
