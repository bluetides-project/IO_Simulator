
project(IO_Simulator_bigfile C)


set (CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories (${BIGFILE_SRC_SRCDIR})

set ( IOSIM_SRCS 
        ${CMAKE_CURRENT_SOURCE_DIR}/iosim.c
    )

add_executable( ${IOSIM_TARGET} ${IOSIM_SRCS} )

target_link_libraries(${IOSIM_TARGET} ${BIGFILE_TARGET})
