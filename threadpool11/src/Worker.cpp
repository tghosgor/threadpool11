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

#include "threadpool11/Pool.h"
#include "threadpool11/Worker.h"

namespace threadpool11
{

Worker::Worker(Pool* const& pool) :
  pool(pool),
  work(nullptr),
  isWorkReallyPosted(false),
  isReallyInitialized(false),
  terminate(false),
  thread(std::bind(&Worker::execute, this))
{
  //std::cout << std::this_thread::get_id() << " Worker created" << std::endl;
}

inline bool Worker::operator==(Worker const& other) const
{
  return thread.get_id() == other.thread.get_id();
}

inline bool Worker::operator==(const Worker* other) const
{
  return operator==(*other);
}

void Worker::setWork(WorkType work)
{
  this->work = std::move(work);
  std::lock_guard<std::mutex> lock_guard(activatorMutex);
  isWorkReallyPosted = true;
  activator.notify_one();
}

void Worker::execute()
{
  {
    std::unique_lock<std::mutex> initLock(this->initMutex);
    std::unique_lock<std::mutex> lock(activatorMutex);
    isReallyInitialized = true;
    initializer.notify_one();
    initLock.unlock();
    activator.wait(lock, [this](){ return isWorkReallyPosted; });
  }

  while(!terminate)
  {
    std::unique_lock<std::mutex> lock(activatorMutex);
    isWorkReallyPosted = false;
    WORK:
    work();
    {
      std::lock_guard<std::mutex> lock(pool->enqueuedWorkMutex);
      if(pool->enqueuedWork.size() > 0)
      {
        work = std::move(pool->enqueuedWork.front());
        pool->enqueuedWork.pop_front();
        goto WORK;
      }
    }

    {
      std::lock(pool->activeWorkerContMutex, pool->inactiveWorkerContMutex);
      auto end = iterator;
      ++end;
      pool->inactiveWorkers.splice(pool->inactiveWorkers.end(), pool->activeWorkers, iterator, end);
      iterator = --pool->inactiveWorkers.end();

      pool->activeWorkerContMutex.unlock();
      pool->inactiveWorkerContMutex.unlock();
    }

    activator.wait(lock, [this](){ return isWorkReallyPosted; });
  }
}

}
