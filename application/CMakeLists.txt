file(GLOB_RECURSE APP ./ *.cpp)
add_library(app ${APP})
add_compile_options(/wd4819)
add_compile_options(/wd4828)
target_compile_definitions(app PUBLIC WITH_DEBUG_INFO)

# 添加 SFML 包含目录
target_include_directories(app PRIVATE ../thirdParty/SFML/include)

# 链接 SFML 库
target_link_libraries(app PRIVATE sfml-graphics)