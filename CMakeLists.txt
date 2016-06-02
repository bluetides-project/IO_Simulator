###########################################################################################
# Simple cmake build setup for IOSimulator
#  Usage:
#    1) Create out-of-source build directory.
#    2) Execute cmake, pointing to top-level source.
#    3) Run make.
#      ex) > cd ~
#          > git clone git@github.com:bluetides-project/IO_Simulator.git
#          > mkdir ~/iosim-build
#          > cd ~/iosim-build
#          > cmake ~/IO_Simulator
#          > make
#          > ll -l src/
#            total 9044
#            drwxr-xr-x 3 dadams bw_staff    4096 Jun  2 10:18 CMakeFiles
#            -rw-r--r-- 1 dadams bw_staff     992 Jun  2 10:16 cmake_install.cmake
#            -rwxr-xr-x 1 dadams bw_staff 9238407 Jun  2 10:18 iosim
#            -rw-r--r-- 1 dadams bw_staff    5525 Jun  2 10:16 Makefile
#
###########################################################################################
project(IO_Simulator C)

cmake_minimum_required(VERSION 3.1)

set (FIND_LIBRARY_USE_LIB64_PATHS TRUE)


###########################################################################################
# setup some global variables
###########################################################################################
set (IOSIM_SRC_SRCDIR                ${CMAKE_CURRENT_SOURCE_DIR}/src)
set (IOSIM_SRC_BINDIR                ${CMAKE_CURRENT_BINARY_DIR}/src)
set (BIGFILE_SRC_SRCDIR              ${CMAKE_CURRENT_SOURCE_DIR}/bigfile)
set (BIGFILE_SRC_BINDIR              ${CMAKE_CURRENT_BINARY_DIR}/bigfile)

set (BIGFILE_TARGET     bigfile)
set (IOSIM_TARGET       iosim)

###########################################################################################
# add src directories
###########################################################################################
add_subdirectory(bigfile)
add_subdirectory(src)

