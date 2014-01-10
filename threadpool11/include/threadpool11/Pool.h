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
#include <list>
#include <memory>
#include <mutex>

#include "threadpool11/Worker.h"

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
	typedef size_t WorkerCountType;

private:
	Pool(Pool&&);
	Pool(Pool const&);
	Pool& operator=(Pool&&);
	Pool& operator=(Pool const&);

	std::deque<Worker> workers;
	WorkerCountType activeWorkerCount;

	mutable std::mutex workerContainerMutex;
		
	mutable std::mutex enqueuedWorkMutex;
	std::deque<decltype(Worker::work)> enqueuedWork;

	mutable std::mutex notifyAllFinishedMutex;
	std::condition_variable notifyAllFinished;
	bool areAllReallyFinished;
	
	//std::atomic<WorkerCountType> workCallCounter;
		
	void spawnWorkers(WorkerCountType const& n);

public:
  /**
   * Constructs a thread pool of workerCount threads. Threads are immediately initialized and put to
   * wait for a signal which will be sent by Pool::postWork.
   * 
   * If you leave it empty, the number of threads will be defaulted to the number of threads that can
   * be run concurrently on the machine which the program is running on.
   */
	threadpool11_EXPORT Pool(WorkerCountType const& workerCount = std::thread::hardware_concurrency());
	
  /**
   * If there are no threads left (i.e. you called Pool::joinAll(); prior to this function)
   * all the works you post gets enqueued. If you spawn new threads in future, they will be executed then.
   */
	threadpool11_EXPORT void postWork(Worker::WorkType work);
  
  /**
   * This function suspends the calling thread until all posted works are finished and, therefore, all worker
   * threads are free. It guarantees you that before the function returns, all queued works are finished.
   */
	threadpool11_EXPORT void waitAll();
  
  /**
   * This function joins all the threads in the thread pool as fast as possible.
   * All the posted works are NOT GUARANTEED to be finished before the worker threads are destroyed and
   * this function returns. Enqueued works are wiped out.
   * 
   * However, ongoing works in the threads in the pool are guaranteed to finish before that threads are terminated.
   */
	threadpool11_EXPORT void joinAll();
	
  /**
   * This function requires a mutex lock so you should call it wisely if you performance is a life matter to you.
   */
  threadpool11_EXPORT WorkerCountType getWorkerCount() const;
  
  /**
   * This function requires a mutex lock so you should call it wisely if you performance is a life matter to you.
   */
	threadpool11_EXPORT WorkerCountType getWorkQueueCount() const;
  
  /**
   * This function requires a mutex lock so you should call it wisely if you performance is a life matter to you.
   */
	threadpool11_EXPORT WorkerCountType getActiveWorkerCount() const;
  
  /**
   * This function requires a mutex lock so you should call it wisely if you performance is a life matter to you.
   */
	threadpool11_EXPORT WorkerCountType getInactiveWorkerCount() const;
  
  /**
   * Increases the number of threads in the pool by n.
   */
	threadpool11_EXPORT void increaseWorkerCountBy(WorkerCountType const& n);
  
  /**
   * Tries to decrease the number of threads in the pool by **n*. Setting **n* higher than the
   * number of workers has no bad effect. If there are no active threads and you want to destroy all the threads
   * Pool::decreaseWorkerCountBy(std::numeric_limits<Pool::WorkerCountType>::max());
   * will do.
   * 
   * This function is not an optimal implementation because it linearly looks for workers from the end of the list
   * until it sees an active Worker. If the last workers in the list are working but others are inactive,
   * it does not get past the inactive ones.
  */
	threadpool11_EXPORT WorkerCountType decreaseWorkerCountBy(WorkerCountType const& n);
};

#undef threadpool11_EXPORT
#undef threadpool11_EXPORTING
}
