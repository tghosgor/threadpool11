/*!
Copyright (c) 2013, 2014, Tolga HOŞGÖR
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
*/

#include <algorithm>

#include "threadpool11/Pool.h"

namespace threadpool11
{

Pool::Pool(size_t const& workerCount) :
  workQueueSize(0)
{
  spawnWorkers(workerCount);
}

Pool::~Pool()
{
  joinAll();
}

void Pool::joinAll()
{
  for(size_t i = 0; i < workers.size(); ++i)
  {
    postWork<void>([]() { }, Work::Type::TERMINAL);
  }
  for(auto& it : workers)
    if(it.thread.joinable())
      it.thread.join();
}

size_t Pool::getWorkerCount() const
{
  return workers.size();
}

void Pool::setWorkerCount(size_t const& n)
{
  if(getWorkerCount() < n)
    increaseWorkerCountBy(n - getWorkerCount());
  else
    decreaseWorkerCountBy(getWorkerCount() - n);
}

size_t Pool::getWorkQueueSize() const
{
  return workQueueSize.load();
}

void Pool::increaseWorkerCountBy(size_t const& n)
{
  spawnWorkers(n);
}

void Pool::decreaseWorkerCountBy(size_t n)
{
  n = std::min(n, getWorkerCount());
  while(n-- > 0)
    postWork<void>([]() { }, Work::Type::TERMINAL);
}

void Pool::spawnWorkers(size_t n)
{
  //'OR' makes sure the case where one of the expressions is zero, is valid.
  assert(static_cast<size_t>(inactiveWorkers.size() + n) > n || static_cast<size_t>(inactiveWorkers.size() + n) > inactiveWorkers.size());
  while(n-- > 0)
    workers.emplace_back(this);
}

}
