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


//#include <iostream>


#include <thread>
#include <functional>
#include <mutex>
#include <list>
#include <condition_variable>

namespace threadpool11
{
	class Pool;

	class Worker
	{
		friend class Pool;

	public:
		typedef std::function<void()> WorkType;

	private:
		Worker& operator=(Worker&& other);
		Worker(Worker&& other);
		//Worker& operator=(Worker const&& other);
		//Worker& operator=(Worker&& other);

		Pool* const pool;
		
		std::mutex initMutex;
		bool init;
		std::condition_variable initialized;

		std::list<Worker*>::iterator poolIterator;

		WorkType work;

		std::mutex workPostedMutex;
		std::condition_variable workPosted;

		bool terminate;

		/**
		* This should always stay at bottom so that it is called at the most end.
		*/
		std::thread thread;

	private:
		//this is here for inlining purposes
		void setWork(WorkType const& work)
		{	
			std::lock_guard<std::mutex> lock(workPostedMutex);
			this->work = std::move(work);
			workPosted.notify_one();
		}
		void execute();

	public:
		Worker(Pool* const& pool);

		bool operator==(Worker const& other) const;
		bool operator==(const Worker* other) const;
	};
}
