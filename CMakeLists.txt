
cmake_minimum_required(VERSION 3.5)

project(Chat)
#设置C++17标准
set(CMAKE_CXX_STANDARD 11)
# 强制使用 C++17，不可回退，如果编译器不支持 C++ 17，直接报错
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fPIC")
endif()

#设置Debug模式
set(CMAKE_BUILD_TYPE Debug)

#设置可执行文件的生成目录
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

#搜索公共头文件
include_directories(${CMAKE_SOURCE_DIR}/include)

#搜索动态库所在路径
link_directories(
    /usr/local/lib/ 
    /usr/lib64/ 
    /usr/lib64/mysql/
)

#添加子目录
add_subdirectory(Server)
add_subdirectory(Client)
