INCLUDE_DIRECTORIES(${threadpool11_SOURCE_DIR}/threadpool11/include)
LINK_DIRECTORIES(${threadpool11_BINARY_DIR}/threadpool11/bin)

ADD_EXECUTABLE(threadpool11_demo src/main.cpp)

TARGET_LINK_LIBRARIES(threadpool11_demo threadpool11)
