
cmake_minimum_required(VERSION 3.5)

project(Client)
#设置C++17标准
set(CMAKE_CXX_STANDARD 11)
# 强制使用 C++17，不可回退，如果编译器不支持 C++ 17，直接报错
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
#设置搜索头文件搜索路径
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
#搜索源文件
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRC)


#生成可执行文件
add_executable(Client ${SRC})

#链接库文件
target_link_libraries(Client 
    muduo_net muduo_base muduo_http muduo_inspect 
    mysqlclient hiredis pthread
)


