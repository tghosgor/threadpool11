/**
 * threadpool11
 * Copyright (C) 2013, 2014, 2015, 2016, 2017, 2018, 2019  Tolga HOSGOR
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "threadpool11/pool.hpp"

#include <algorithm>
#include <future>
#include <vector>

namespace threadpool11 {

const pool::no_future_t pool::no_future_tag;

pool::pool(size_type worker_count)
    : worker_count_{0}
    , work_queue_{0}
    , work_queue_size_{0} {
  increase_worker_count(worker_count);
}

pool::~pool() { join_all(); }

void pool::join_all() { decrease_worker_count(std::numeric_limits<size_type>::max(), method_t::SYNC); }

void pool::set_worker_count(size_type n, method_t method) {
  if (get_worker_count() < n) {
    increase_worker_count(n - get_worker_count());
  } else {
    decrease_worker_count(get_worker_count() - n, method);
  }
}

void pool::increase_worker_count(size_type n) {
  worker_count_ += n;

  while (n-- > 0) {
    std::thread thread{std::bind(&pool::worker_main, this)};
    thread.detach();
  }
}

void pool::decrease_worker_count(size_type n, method_t method) {
  std::vector<std::future<void>> futures;
  n = std::min(n, get_worker_count());

  worker_count_ -= n;

  if (method == method_t::SYNC) {
    futures.reserve(n);
  }

  while (n > 0) {
    --n;

    if (method == method_t::SYNC) {
      futures.emplace_back(post_work<void>(work_t::type_t::TERMINAL, []() {}));
    } else {
      post_work<void>(work_t::type_t::TERMINAL, []() {}, no_future_tag);
    }
  }

  for (auto& future : futures) {
    future.get();
  }
}

}
