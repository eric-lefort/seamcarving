project(seamcarving)
set(ROOT_DIR ${CMAKE_SOURCE_DIR})
add_executable(seamcarving src/seamcarving.c src/c_img.c)
target_compile_definitions(seamcarving PUBLIC ROOT_DIR="${ROOT_DIR}")
target_include_directories(seamcarving PUBLIC "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(seamcarving m)