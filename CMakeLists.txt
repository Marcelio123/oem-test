cmake_minimum_required(VERSION 3.10)
project(GXTest)
set(CMAKE_CXX_STANDARD 11)

# Path to your extracted Daheng SDK
set(GX_SDK_PATH "/home/alfabeta/oemtest/Galaxy_camera")

include_directories(${GX_SDK_PATH}/inc)
link_directories(${GX_SDK_PATH}/lib/x86_64)

add_executable(gx_test src/main.cpp)
target_link_libraries(gx_test gxiapi pthread)
