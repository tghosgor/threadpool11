#pragma once

#include <functional>

namespace threadpool11 {

class work {
public:
  using callable_t = std::function<void()>;

  enum class type_t {
    STANDARD,
    TERMINAL,
  };

public:
  work(type_t type, callable_t callable)
    : type_{std::move(type)}
    , callable_{std::move(callable)} {
  }

  work(const work&) = delete;
  work(work&&) = default;

  work& operator=(const work&) = delete;
  work& operator=(work&&) = default;

  type_t type() const { return type_; }

  void operator()() const { callable_(); }

private:
  type_t type_;
  callable_t callable_;
};

}

