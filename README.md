![threadpool11](https://raw2.github.com/metherealone/threadpool11/misc/img/logo.png)
threadpool11
==========

### A fast, cross-platform thread pool implementation using C++11 features.

You can find the *VERY EASY* API documentation on header annotations.

This project has been initially developed in just a few hours of free time as I could not find a simple lightweight thread pooling library for my needs.

This thread-pool implementation assumes that workload takes a considerable time to process. It is not tuned towards fast work posting (on the other hand, it takes as low as 2 seconds to post 1 million _work_ s to the pool on an _x86-64 x2 3.4 GHz_, so it is pretty fast enough. Yet the test case is in a _for_ loop, a real case would relatively be more performant due to less mutex interceptions).

I will be glad to hear about the suggestions/ideas you have about the project, via the [issue reporting section](https://github.com/tghosgor/threadpool11/issues).

All non '_-dev_' branches should be safe to use but prefer the latest version.

###For more information, head on to the [wiki](https://github.com/tghosgor/threadpool11/wiki/). There is also a demo application in repository.
