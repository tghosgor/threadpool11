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
