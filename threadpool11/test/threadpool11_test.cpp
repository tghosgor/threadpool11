/**
 * threadpool11
 * Copyright (C) 2013, 2014, 2015, 2016, 2017, 2018, 2019 Tolga HOSGOR
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

