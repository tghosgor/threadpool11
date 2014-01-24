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

Worker::Worker(Pool* const& pool) :
  pool(pool),
  work(nullptr),
  thread(std::bind(&Worker::execute, this))
{
  //std::cout << std::this_thread::get_id() << " Worker created" << std::endl;
}

void Worker::execute()
{
  while(true)
  {
    Work::Callable* work_;
    //std::cout << "\tThread " << std::this_thread::get_id() << " awaken." << std::endl;
    while(pool->work_queue.pop(work_))
    {
      std::unique_ptr<Work::Callable> work(work_);
      --pool->work_queue_size;
      //std::cout << "\tThread " << std::this_thread::get_id() << " worked." << std::endl;
      if((*work)() == Work::Type::TERMINAL)
        return;
    }

    //std::cout << "\tThread " << std::this_thread::get_id() << " will sleep." << std::endl;
    std::unique_lock<std::mutex> workSignalLock(pool->work_signal_mutex);
    pool->work_signal.wait(workSignalLock, [this](){ return (pool->work_queue_size.load()); });
  }
}

}
