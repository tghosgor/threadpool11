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
#include "threadpool11/worker.hpp"

#include <boost/lockfree/queue.hpp>

namespace threadpool11 {

Worker::Worker(Pool& pool)
    : thread(std::bind(&Worker::execute, this, std::ref(pool))) {
  // std::cout << std::this_thread::get_id() << " Worker created" << std::endl;
  thread.detach();
}

void Worker::execute(Pool& pool) {
  const std::unique_ptr<Worker> self(this); //! auto de-allocation when thread is terminated

  while (true) {
    Work::Callable* work_;
    // std::cout << "\tThread " << std::this_thread::get_id() << " awaken." << std::endl;
    while (pool.m_workQueue->pop(work_)) {
      const std::unique_ptr<Work::Callable> work(work_);
      --pool.m_workQueueSize;
      ++pool.m_activeWorkerCount;
      // std::cout << "\tThread " << std::this_thread::get_id() << " worked." << std::endl;
      if ((*work)() == Work::Type::TERMINAL) {
        --pool.m_activeWorkerCount;
        --pool.m_workerCount;
        return;
      }

      --pool.m_activeWorkerCount;
    }

    if (pool.m_activeWorkerCount == 0) {
      std::unique_lock<std::mutex> notifyAllFinishedLock(pool.notify_all_finished_signal_mtx);
      pool.m_areAllReallyFinished = true;
      pool.m_notifyAllFinishedSignal.notify_all();
    }

    // std::cout << "\tThread " << std::this_thread::get_id() << " will sleep." << std::endl;
    std::unique_lock<std::mutex> workSignalLock(pool.m_workSignalMutex);
    pool.m_workSignal.wait(workSignalLock, [&pool]() { return (pool.m_workQueueSize.load()); });
  }
}
}
