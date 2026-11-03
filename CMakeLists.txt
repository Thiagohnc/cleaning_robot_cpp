cmake_minimum_required(VERSION 3.16)
project(cleaning_robot_cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(cleaning_robot_lib
    env.cpp model.cpp policy.cpp dp.cpp qlearning.cpp animate.cpp sprites.cpp
)
target_include_directories(cleaning_robot_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(cleaning_robot main.cpp)
target_link_libraries(cleaning_robot PRIVATE cleaning_robot_lib)
