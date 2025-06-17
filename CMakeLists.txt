cmake_minimum_required(VERSION 3.10)
project(embedded_metrics_uds)

find_package(prometheus-cpp CONFIG REQUIRED COMPONENTS pull)
find_package(Threads REQUIRED)

add_executable(embedded_metrics_uds
    src/main.cpp
    src/uds_socket.cpp
)
target_include_directories(embedded_metrics_uds PRIVATE include)
target_link_libraries(embedded_metrics_uds prometheus-cpp::pull Threads::Threads)
target_compile_features(embedded_metrics_uds PRIVATE cxx_std_17)
