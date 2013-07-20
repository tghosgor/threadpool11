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

#include "threadpool11/threadpool11.h"

namespace threadpool11
{
	
Worker::Worker(Pool* const& pool) :
	pool(pool),
	init(0),
	status(Status::DEACTIVE),
	work(nullptr),
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

void Worker::setWork(WorkType& work)
{
	++pool->activeWorkerCount;
	status = Status::ACTIVE;
	std::lock_guard<std::mutex> lock_guard(activatorMutex);
	this->work = std::move(work);
	activator.notify_one();
}

void Worker::execute()
{
	//std::cout << std::this_thread::get_id() << " Execute called" << std::endl;
	{
		std::unique_lock<std::mutex> initLock(this->initMutex);
		std::unique_lock<std::mutex> lock(activatorMutex);
		init = 1;
		initializer.notify_one();
		initLock.unlock();
		activator.wait(lock);
	}

	while(!terminate)
	{
		std::unique_lock<std::mutex> lock(activatorMutex);
		//std::cout << "thread started" << std::endl;
		WORK:
		//std::cout << "work started 2" << std::endl;
		work();
		//std::cout << std::this_thread::get_id() <<  "-" << workPosted.native_handle()
		//	<< " Work called" << std::endl;
		{
			std::lock_guard<std::mutex> lock(pool->enqueuedWorkMutex);
			if(pool->enqueuedWork.size() > 0)
			{
				work = std::move(pool->enqueuedWork.front());
				pool->enqueuedWork.pop_front();
			//	std::cout << pool->enqueuedWork.size() << " got work from enqueue, going back to work" << std::endl;
				goto WORK;
			}
		}
		
		pool->notifyAllFinishedMutex.lock();
		
		pool->workerContainerMutex.lock();
		
		--pool->activeWorkerCount;
		status = Status::DEACTIVE;
		
		//	std::cout << "notify all finished" << std::endl;
		if(pool->activeWorkerCount)
			pool->notifyAllFinished.notify_all();
		
		pool->workerContainerMutex.unlock();
		
		pool->notifyAllFinishedMutex.unlock();
		
		//std::cout << pool->activeWorkers.size() << " work finished" << std::endl;
		//no need for this anymore, can't set terminate = true when thread is busy. - needs testing
		//if(terminate)
		//	break;
		activator.wait(lock);
	}
		//std::cout << "terminating" << std::endl;
}

}
