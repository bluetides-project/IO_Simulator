
project(IO_Simulator_bigfile C)


set (CMAKE_INCLUDE_CURRENT_DIR ON)

set ( BIGFILE_SRCS 
        ${CMAKE_CURRENT_SOURCE_DIR}/bigfile.c
        ${CMAKE_CURRENT_SOURCE_DIR}/bigfile-mpi.c
    )

add_library( ${BIGFILE_TARGET} ${BIGFILE_SRCS} )
