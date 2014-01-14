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
    while(pool->workQueue.pop(work_))
    {
      std::unique_ptr<Work::Callable> work(work_);
      --pool->workQueueSize;
      //std::cout << "\tThread " << std::this_thread::get_id() << " worked." << std::endl;
      if((*work)() == Work::Type::TERMINAL)
        return;
    }

    //std::cout << "\tThread " << std::this_thread::get_id() << " will sleep." << std::endl;
    std::unique_lock<std::mutex> workSignalLock(pool->workSignalMutex);
    pool->workSignal.wait(workSignalLock, [this](){ return (pool->workQueueSize.load()); });
  }
}

}
