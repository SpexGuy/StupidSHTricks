cmake_minimum_required(VERSION 3.4)
project(shview)

message("Source " "${CMAKE_SOURCE_DIR}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(INCLUDE "${CMAKE_SOURCE_DIR}/include")

if (APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo")
elseif(WIN32)
    find_package(OpenGL REQUIRED)
    include_directories(${OPENGL_INCLUDE_DIRS})
else()
	find_package(OpenGL REQUIRED)
	include_directories(${OPENGL_INCLUDE_DIRS})
endif()

add_definitions(-DGLEW_STATIC -D_USE_MATH_DEFINES)
if (WIN32)
    add_definitions(-DWINDOWS -DPERF) # perf doesn't work anywhere other than windows yet
elseif(APPLE)
    add_definitions(-DAPPLE)
else()
    add_definitions(-DLINUX)
endif()

include_directories(${INCLUDE})

set(SOURCE_FILES main.cpp gl_includes.h Perf.h Perf.cpp stb_image_impl.cpp)
add_executable(shview ${SOURCE_FILES})

if (APPLE)
    set(LIB "${CMAKE_SOURCE_DIR}/lib/osx")
    link_directories(${LIB})
    target_link_libraries(shview ${LIB}/libGLEW.a)
    target_link_libraries(shview ${LIB}/libglfw3.a)
elseif (WIN32)
    set(LIB "${CMAKE_SOURCE_DIR}/lib/windows")
    link_directories(${LIB})
    target_link_libraries(shview ${LIB}/libglew32.a)
    target_link_libraries(shview ${LIB}/libglfw3.a)
    target_link_libraries(shview ${OPENGL_LIBRARIES})
    target_link_libraries(shview -static-libgcc -static-libstdc++)
else ()
    set(LIB "${CMAKE_SOURCE_DIR}/lib/linux64")
    link_directories(${LIB})
    target_link_libraries(shview ${LIB}/libGLEW.a)
    target_link_libraries(shview ${LIB}/libglfw3.a)
    target_link_libraries(shview ${OPENGL_LIBRARIES})
    target_link_libraries(shview -lGL -lX11 -lXi -lXrandr -lXxf86vm -lXinerama -lXcursor -lrt -ldl -lm -lpthread)
endif()

file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR}/)
