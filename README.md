![threadpool11](https://raw2.github.com/metherealone/threadpool11/misc/img/logo.png)
threadpool11
==========

### A fast, lock-free, cross-platform thread pool implementation using C++11 features.

You can find the **dead simple API** documentation on header comments.

This project was initially developed in just a few hours of free time as I could not find a simple lightweight thread pooling library for my needs.

I will be glad to hear about the suggestions/ideas you have about the project, via the [issue reporting section](https://github.com/tghosgor/threadpool11/issues).

All non '_-dev_' branches are safe to use but prefer the latest version.

##Dependencies
###boost::lockfree
You don't need to have anything external but the project uses `boost::lockfree`. If you have Boost and CMake var `Boost_FOUND` is `true` it will use your boost. If not. there is one in the repo and it will be used.
