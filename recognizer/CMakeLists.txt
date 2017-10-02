set(RECOGNIZER recognize)

file(GLOB_RECURSE SOURCES *.cpp)
add_executable(${RECOGNIZER} ${SOURCES})

target_include_directories(${RECOGNIZER}
    PRIVATE ${PROJECT_SOURCE_DIR}/CImg)

find_package(X11)
find_package(Threads)

if(X11_FOUND)
    target_link_libraries(${RECOGNIZER} ${X11_X11_LIB})
endif()

if(THREADS_FOUND)
    target_link_libraries(${RECOGNIZER} ${CMAKE_THREAD_LIBS_INIT})
endif()

target_link_libraries(${RECOGNIZER} label)
