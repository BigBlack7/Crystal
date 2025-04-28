file(GLOB_RECURSE APP ./ *.cpp)
add_library(app ${APP})
add_compile_options(/wd4819)
add_compile_options(/wd4828)