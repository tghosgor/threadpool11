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
