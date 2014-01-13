#pragma once

//http://marcoarena.wordpress.com/2012/11/01/learn-how-to-capture-by-move/

namespace threadpool11
{

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
