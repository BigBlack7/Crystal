cmake_minimum_required(VERSION 3.12)
project(Crystal)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_BUILD_TYPE Release)
include_directories(
	SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/include
	SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/SFML/include
)

# 设置并行编译线程数，这里使用处理器核心数
include(ProcessorCount)
ProcessorCount(N)
if(N EQUAL 0)
	set(N 24)
endif()
set(CMAKE_BUILD_PARALLEL_LEVEL ${N})

add_subdirectory(application)
add_subdirectory(core)
add_subdirectory(thirdParty/SFML)

aux_source_directory(. SRCS)

add_executable(PBRT ${SRCS})

target_link_libraries(PBRT app engine)

add_compile_options(/wd4828)
add_compile_options(/wd4819)