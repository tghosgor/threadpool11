/*!
 * threadpool11
 * Copyright (C) 2013, 2014, 2015, 2016, 2017, 2028  Tolga HOSGOR
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

#include "threadpool11/pool.hpp"
#include "threadpool11/worker.hpp"

#include <boost/lockfree/queue.hpp>

namespace threadpool11 {

Worker::Worker(Pool& pool)
    : thread_(std::bind(&Worker::execute, this, std::ref(pool))) {
  thread_.detach();
}

void Worker::execute(Pool& pool) {
  const std::unique_ptr<Worker> self(this); //! auto de-allocation when thread is terminated

  while (true) {
    Work* work_ptr;
    std::unique_ptr<Work> work;
    Work::Type work_type = Work::Type::STD;

    while (pool.work_queue_->pop(work_ptr)) {
      work.reset(work_ptr);
      work_type = work->type();

      --pool.work_queue_size_;

      if (work_type == Work::Type::TERMINAL)
        break;

      (*work)();
      work.reset(nullptr);
    }

    std::unique_lock<std::mutex> work_signal_lock(pool.work_signal_mutex_);

    if (work_type == Work::Type::TERMINAL) {
      --pool.worker_count_;
    }

    if (--pool.active_worker_count_ == 0) {
      if (pool.work_queue_size_ == 0) {
        std::unique_lock<std::mutex> notify_all_finished_lock(pool.notify_all_finished_mutex_);

        pool.are_all_really_finished_ = true;
        pool.notify_all_finished_signal_.notify_all();
      }
    }

    if (work_type == Work::Type::TERMINAL) {
      work_signal_lock.unlock();
      (*work)();
      return;
    }

    pool.work_signal_.wait(work_signal_lock, [&pool]() { return (pool.work_queue_size_ > 0); });

    ++pool.active_worker_count_;
  }
}

}
