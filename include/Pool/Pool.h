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
#include <vector>
#include <deque>
#include <list>
#include <mutex>
#include <memory>
#include <condition_variable>

#include "Worker/Worker.h"

#ifdef threadpool11_EXPORT
	#define threadpool11_DLLIE __declspec(dllexport)
#else
	#define threadpool11_DLLIE __declspec(dllimport)
#endif

namespace threadpool11
{
	class Pool
	{
		friend class Worker;

	public:
		typedef std::list<Worker*> WorkerListType;
		typedef unsigned int WorkerCountType;

	private:
		Pool& operator=(Pool&);
		Pool& operator=(Pool&&);

		std::vector<std::unique_ptr<Worker>> workers;

		mutable std::mutex workerContainerMutex;
		WorkerListType activeWorkers;
		WorkerListType inactiveWorkers;
		
		mutable std::mutex enqueuedWorkMutex;
		std::deque<decltype(Worker::work)> enqueuedWork;

		mutable std::mutex notifyAllFinishedMutex;
		std::condition_variable notifyAllFinished;
		
		void spawnWorkers(WorkerCountType const& n);

	public:
		threadpool11_DLLIE Pool(WorkerCountType const& workerCount = std::thread::hardware_concurrency());
		//Worker& operator[](unsigned int i);

		threadpool11_DLLIE void postWork(Worker::WorkType const& work);
		threadpool11_DLLIE void waitAll();
		threadpool11_DLLIE void joinAll();

		threadpool11_DLLIE WorkerCountType getActiveWorkerCount() const;
		threadpool11_DLLIE WorkerCountType getInactiveWorkerCount() const;
		
		threadpool11_DLLIE void increaseWorkerCountBy(WorkerCountType const& n);
		threadpool11_DLLIE WorkerCountType decreaseWorkerCountBy(WorkerCountType n);
	};

#undef threadpool11_DLLIE
}
