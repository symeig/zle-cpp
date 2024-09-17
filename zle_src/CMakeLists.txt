cmake_minimum_required(VERSION 3.10)
project(zle_project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MPLAPACK_DIR "/home/docker/MPLAPACK")
set(MPLAPACK_INCLUDE_DIR "${MPLAPACK_DIR}/include")
set(MPLAPACK_LIB_DIR "${MPLAPACK_DIR}/lib")

find_package(Threads REQUIRED)
include_directories(${MPLAPACK_INCLUDE_DIR})
link_directories(${MPLAPACK_LIB_DIR})

# Build zle as a shared library
add_library(zle SHARED zle.cpp)
target_link_libraries(zle 
    mpblas_mpfr 
    mplapack_mpfr 
    mpfr 
    mpc 
    gmp 
    gmpxx
    Threads::Threads
)

# Install targets
install(TARGETS zle
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin)
install(FILES zle.h DESTINATION include)

# Generate and install pkg-config file
configure_file(zle.pc.in zle.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/zle.pc DESTINATION lib/pkgconfig)