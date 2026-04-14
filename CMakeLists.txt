cmake_minimum_required(VERSION 3.10)
project(IKP_Z7)

set(CMAKE_CXX_STANDARD 17)

include_directories(${PROJECT_SOURCE_DIR}/src)

add_executable(broker
    src/main.cpp

    src/network/broker.cpp
    src/network/socket_utils.cpp

    src/model/topic.cpp
    src/model/subscriber.cpp

    src/utils/hash_map.cpp
    src/utils/subscriber_list.cpp

    src/threading/task.cpp
    src/threading/task_queue.cpp
    src/threading/thread_pool.cpp
)

target_link_libraries(broker ws2_32)
target_link_options(broker PRIVATE -mconsole)

add_executable(admin_client
    src/client/admin_client.cpp
    src/network/socket_utils.cpp
)

target_link_libraries(admin_client ws2_32)
target_link_options(admin_client PRIVATE -mconsole)

add_executable(subscriber_client
    src/client/subscriber_client.cpp
    src/network/socket_utils.cpp
)

target_link_libraries(subscriber_client ws2_32)
target_link_options(subscriber_client PRIVATE -mconsole)

add_executable(publisher_client
    src/client/publisher_client.cpp
    src/network/socket_utils.cpp
)

target_link_libraries(publisher_client ws2_32)
target_link_options(publisher_client PRIVATE -mconsole)

add_executable(stress_test_1
    src/stress_test_1_scalability.cpp
    src/network/socket_utils.cpp
)
target_link_libraries(stress_test_1 ws2_32)
target_link_options(stress_test_1 PRIVATE -mconsole)

