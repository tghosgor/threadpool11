/*
Copyright (c) 2013, Tolga HOŞGÖR
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.
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

#include "threadpool11/Worker.h"
#include "threadpool11/Helper.hpp"

#ifdef WIN32
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
  typedef unsigned int WorkerCountType;

private:
  Pool(Pool&&);
  Pool(Pool const&);
  Pool& operator=(Pool&&);
  Pool& operator=(Pool const&);

  std::list<Worker> activeWorkers;
  mutable std::mutex activeWorkerContMutex;

  std::list<Worker> inactiveWorkers;
  mutable std::mutex inactiveWorkerContMutex;

  mutable std::mutex enqueuedWorkMutex;
  std::deque<decltype(Worker::work)> enqueuedWork;

  void spawnWorkers(WorkerCountType n);

public:
  threadpool11_EXPORT Pool(WorkerCountType const& workerCount = std::thread::hardware_concurrency());

  /**
   * Posts a work to the pool for getting processed.
   *
   * If there are no threads left (i.e. you called Pool::joinAll(); prior to
   * this function) all the works you post gets enqueued. If you spawn new threads in
   * future, they will be executed then.
   */
  template<typename T>
  threadpool11_EXPORT
  std::future<T> postWork(std::function<T()> callable);

  /**
   * This function joins all the threads in the thread pool as fast as possible.
   * All the posted works are NOT GUARANTEED to be finished before the worker threads
   * are destroyed and this function returns. Enqueued works are wiped out.
   *
   * However, ongoing works in the threads in the pool are guaranteed
   * to finish before that threads are terminated.
   */
  threadpool11_EXPORT void joinAll();

  /**
   * This function requires a mutex lock so you should call it
   *  wisely if you performance is a life matter to you.
   */
  threadpool11_EXPORT WorkerCountType getWorkQueueCount() const;

  /**
   * This function requires a mutex lock so you should call it
   *  wisely if you performance is a life matter to you.
   */
  threadpool11_EXPORT WorkerCountType getActiveWorkerCount() const;

  /**
   * This function requires a mutex lock so you should call it
   *  wisely if you performance is a life matter to you.
   */
  threadpool11_EXPORT WorkerCountType getInactiveWorkerCount() const;

  /**
   * Increases the number of threads in the pool by n.
   */
  threadpool11_EXPORT void increaseWorkerCountBy(WorkerCountType const& n);

  /**
   * Tries to decrease the number of threads in the pool by **n*.
   * Setting **n* higher than the number of workers has no bad effect.
   * If there are no active threads and you want to destroy all the threads
   * Pool::decreaseWorkerCountBy(std::numeric_limits<Pool::WorkerCountType>::max());
   * will do.
   *
   * This function is not an optimal implementation because it
   * linearly looks for workers from the end of the list until it
   * sees an active Worker. If the last workers in the list are working
   * but others are inactive, it does not get past the inactive ones.
   */
  threadpool11_EXPORT WorkerCountType decreaseWorkerCountBy(WorkerCountType n);
};

template<typename T>
threadpool11_EXPORT inline
std::future<T> Pool::postWork(std::function<T()> callable)
{
  std::promise<T> promise;
  auto future = promise.get_future();

  /* TODO: how to avoid copy of callable into this lambda and the ones below? In a decent way... */
  /* evil move hack */
  auto move_callable = make_move_on_copy(std::move(callable));
  /* evil move hack */
  auto move_promise = make_move_on_copy(std::move(promise));

  std::lock_guard<std::mutex> enqueueLock(enqueuedWorkMutex);
  std::lock_guard<std::mutex> inactiveWorkersLock(inactiveWorkerContMutex);

  if(inactiveWorkers.size() > 0)
  {
    std::lock_guard<std::mutex> activeWorkersLock(activeWorkerContMutex);
    activeWorkers.splice(activeWorkers.end(), inactiveWorkers, --inactiveWorkers.end(), inactiveWorkers.end());
      //TODO: if the element is inserted at the ::end() no need to look it up again and can use std::forward_list
    auto workerIterator = --activeWorkers.end();
    workerIterator->iterator = workerIterator;
    workerIterator->setWork([move_callable, move_promise]() mutable { move_promise.Value().set_value((move_callable.Value())()); });
    return future;
  }

  enqueuedWork.emplace_back([move_callable, move_promise]() mutable { move_promise.Value().set_value((move_callable.Value())()); });
  return future;
}

template<>
threadpool11_EXPORT inline
std::future<void> Pool::postWork(std::function<void()> callable)
{
  std::promise<void> promise;
  auto future = promise.get_future();

  /* TODO: how to avoid copy of callable into this lambda and the ones below? In a decent way... */
  /* evil move hack */
  auto move_callable = make_move_on_copy(std::move(callable));
  /* evil move hack */
  auto move_promise = make_move_on_copy(std::move(promise));

  std::lock_guard<std::mutex> enqueueLock(enqueuedWorkMutex);
  std::lock_guard<std::mutex> inactiveWorkersLock(inactiveWorkerContMutex);

  if(inactiveWorkers.size() > 0)
  {
    std::lock_guard<std::mutex> activeWorkersLock(activeWorkerContMutex);
    activeWorkers.splice(activeWorkers.end(), inactiveWorkers, --inactiveWorkers.end(), inactiveWorkers.end());
      //TODO: if the element is inserted at the ::end() no need to look it up again and can use std::forward_list
    auto workerIterator = --activeWorkers.end();
    workerIterator->iterator = workerIterator;
    workerIterator->setWork([move_callable, move_promise]() mutable { (move_callable.Value())(); move_promise.Value().set_value(); });
    return future;
  }

  enqueuedWork.emplace_back([move_callable, move_promise]() mutable { (move_callable.Value())(); move_promise.Value().set_value(); });
  return future;
}

#undef threadpool11_EXPORT
#undef threadpool11_EXPORTING
}
