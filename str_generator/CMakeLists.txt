set(STR_GENERATOR str_generator)

file(GLOB_RECURSE SOURCES *.cpp)
add_executable(${STR_GENERATOR} ${SOURCES})

target_include_directories(${STR_GENERATOR}
    PRIVATE ${CMAKE_SOURCE_DIR}/CImg)
