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

#pragma once

namespace threadpool11
{

namespace Work
{
  enum class Type
  {
    STD,
    TERMINAL
  };

  enum class Prio
  {
    DEFERRED,
    IMMIDIATE
  };

  typedef std::function<Work::Type()> Callable;
}

template<typename T>
struct move_on_copy
{
   move_on_copy(T&& aValue) : value(std::move(aValue)) {}
   move_on_copy(const move_on_copy& other) : value(std::move(other.value)) {}
 
   T& Value()
   {
      return value;
   }
 
   const T& Value() const
   {
      return value;
   }
 
private:
   mutable T value;
   move_on_copy& operator=(move_on_copy&& aValue) = delete; // not needed here
   move_on_copy& operator=(const move_on_copy& aValue) = delete; // not needed here
};

template<typename T> inline
move_on_copy<T> make_move_on_copy(T&& aValue)
{
   return move_on_copy<T>(std::move(aValue));
}

}
