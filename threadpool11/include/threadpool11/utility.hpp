/*!
 * threadpool11
 * Copyright (C) 2013, 2014, 2015, 2016, 2017, 2028  Tolga HOSGOR
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

#pragma once

#include <functional>

namespace threadpool11 {

class Work {
public:
  enum class Type { STD, TERMINAL };
  enum class Prio { DEFERRED, IMMIDIATE };

  using Callable = std::function<void()>;

public:
  Work(Callable callable, Type type)
      : type_{std::move(type)}
      , callable_{std::move(callable)} {
  }

  void operator()() {
    callable_();
  }

  Type type() const { return type_; }

private:
  Type type_;
  Callable callable_;
};

template <typename T>
class move_on_copy {
public:
  move_on_copy(T&& aValue)
      : value_(std::move(aValue)) {}
  move_on_copy(const move_on_copy& other)
      : value_(std::move(other.value_)) {}

  move_on_copy& operator=(move_on_copy&& aValue) = delete;      // not needed here
  move_on_copy& operator=(const move_on_copy& aValue) = delete; // not needed here

  T& value() { return value_; }
  const T& value() const { return value_; }

private:
  mutable T value_;
};

template <typename T>
inline move_on_copy<T> make_move_on_copy(T&& aValue) {
  return move_on_copy<T>(std::move(aValue));
}
}
