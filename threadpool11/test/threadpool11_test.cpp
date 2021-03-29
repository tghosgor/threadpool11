#include <threadpool11/pool.hpp>

#include <gtest/gtest.h>

#include <utility>

using pool = threadpool11::pool;
using size_type = threadpool11::pool::size_type;

TEST(pool, ctor_dtor_joinall) {
  constexpr size_type count = 5000;
  for (size_type i = 0; i < count; ++i) {
    pool p;
  }
}

TEST(pool, post_work) {
  constexpr size_type count = 150000;
  std::vector<std::future<size_type>> values;
  values.reserve(count);
  pool p;
  for (size_type i = 0; i < count; ++i) {
    auto&& future = p.post_work<size_type>([i]() -> size_type { return i + 1; });
    values.emplace_back(std::move(future));
  }
  for (size_type i = 0; i < count; ++i) {
    ASSERT_EQ(i + 1, values[i].get());
  }
}

TEST(pool, post_work_no_future) {
  constexpr size_type count = 150000;
  std::vector<size_type> values(count, 0);
  {
    pool p;
    for (size_type i = 0; i < count; ++i) {
      p.post_work<void>([&values, i]() { values[i] = i + 1; });
    }
  }
  for (size_type i = 0; i < count; ++i) {
    ASSERT_EQ(i+1, values[i]);
  }
}

