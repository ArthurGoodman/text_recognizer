set(IMG_GENERATOR generate)

file(GLOB_RECURSE SOURCES *.cpp)
add_executable(${IMG_GENERATOR} ${SOURCES})

target_include_directories(${IMG_GENERATOR}
    PRIVATE ${PROJECT_SOURCE_DIR}/CImg)

find_package(X11)
find_package(Threads)

if(X11_FOUND)
target_link_libraries(${IMG_GENERATOR} ${X11_X11_LIB})
endif()

if(THREADS_FOUND)
target_link_libraries(${IMG_GENERATOR} ${CMAKE_THREAD_LIBS_INIT})
endif()
