file(GLOB_RECURSE ENGINE ./ *.cpp)
add_library(engine ${ENGINE})
target_compile_definitions(engine PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE GLM_FORCE_RADIANS GLM_FORCE_LEFT_HANDED)
target_compile_definitions(engine PUBLIC WITH_DEBUG_INFO)

add_compile_options(/wd4828)
add_compile_options(/wd4819)