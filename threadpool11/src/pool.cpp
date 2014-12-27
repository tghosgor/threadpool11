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

#include "threadpool11/pool.hpp"

#include <algorithm>

namespace threadpool11 {

Pool::Pool(std::size_t worker_count)
    : m_workerCount(0), m_activeWorkerCount(0), m_workQueue(0), m_workQueueSize(0) {
  spawnWorkers(worker_count);
}

Pool::~Pool() { joinAll(); }

void Pool::waitAll() {
  std::unique_lock<std::mutex> lock(notify_all_finished_signal_mtx);
  if (m_activeWorkerCount > 0) {
    m_notifyAllFinishedSignal.wait(lock, [this]() { return m_areAllReallyFinished; });
    m_areAllReallyFinished = false;
  }
}

void Pool::joinAll() { decWorkerCountBy(std::numeric_limits<size_t>::max(), Method::SYNC); }

size_t Pool::getWorkerCount() const { return m_workerCount.load(); }

void Pool::setWorkerCount(std::size_t n, Method method) {
  if (getWorkerCount() < n)
    incWorkerCountBy(n - getWorkerCount());
  else
    decWorkerCountBy(getWorkerCount() - n, method);
}

size_t Pool::getWorkQueueSize() const { return m_workQueueSize.load(); }

size_t Pool::getActiveWorkerCount() const { return m_activeWorkerCount.load(); }

size_t Pool::getInactiveWorkerCount() const { return m_workerCount.load() - m_activeWorkerCount.load(); }

void Pool::incWorkerCountBy(std::size_t n) { spawnWorkers(n); }

void Pool::decWorkerCountBy(size_t n, Method method) {
  n = std::min(n, getWorkerCount());
  if (method == Method::SYNC) {
    std::vector<std::future<void>> futures;
    futures.reserve(n);
    while (n-- > 0)
      futures.emplace_back(postWork<void>([]() {}, Work::Type::TERMINAL));
    for (auto& it : futures)
      it.get();
  } else {
    while (n-- > 0)
      postWork<void>([]() {}, Work::Type::TERMINAL);
  }
}

void Pool::spawnWorkers(std::size_t n) {
  //'OR' makes sure the case where one of the expressions is zero, is valid.
  assert(static_cast<size_t>(m_workerCount + n) > n || static_cast<size_t>(m_workerCount + n) > m_workerCount);
  while (n-- > 0) {
    new Worker(*this); //! Worker class takes care of its de-allocation itself after here
    ++m_workerCount;
  }
}
}
