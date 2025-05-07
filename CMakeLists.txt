cmake_minimum_required(VERSION 3.12)
project(Crystal)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_BUILD_TYPE Release)
include_directories(
	SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/include
)

add_subdirectory(application)
add_subdirectory(myEngine)

aux_source_directory(. SRCS)

add_executable(PBRT ${SRCS})

target_link_libraries(PBRT app engine)

add_compile_options(/wd4828)
add_compile_options(/wd4819)