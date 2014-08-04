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

#include "threadpool11/Pool.h"
#include "threadpool11/Worker.h"

namespace threadpool11
{

Worker::Worker(Pool* const& pool)
  : thread(std::bind(&Worker::execute, this, pool))
{
  //std::cout << std::this_thread::get_id() << " Worker created" << std::endl;
  thread.detach();
}

void Worker::execute(Pool* const& pool)
{
  const std::unique_ptr<Worker> self(this); //! auto de-allocation when thread is terminated

  while(true)
  {
    Work::Callable* work_;
    //std::cout << "\tThread " << std::this_thread::get_id() << " awaken." << std::endl;
    while(pool->work_queue.pop(work_))
    {
      const std::unique_ptr<Work::Callable> work(work_);
      --pool->work_queue_size;
      ++pool->active_worker_count;
      //std::cout << "\tThread " << std::this_thread::get_id() << " worked." << std::endl;
      if((*work)() == Work::Type::TERMINAL)
      {
        --pool->active_worker_count;
        --pool->worker_count;
        return;
      }
      
      --pool->active_worker_count;
    }
    
    if(pool->active_worker_count == 0)
	{
		std::unique_lock<std::mutex> notifyAllFinishedLock(pool->notify_all_finished_signal_mtx);
		pool->are_all_really_finished = true;
		pool->notify_all_finished_signal.notify_all();
	}

    //std::cout << "\tThread " << std::this_thread::get_id() << " will sleep." << std::endl;
    std::unique_lock<std::mutex> workSignalLock(pool->work_signal_mtx);
    pool->work_signal.wait(workSignalLock, [&pool](){ return (pool->work_queue_size.load()); });
  }
}

}
