#################
# metal library #
#################
PROJECT(metal)

SET(metal_LIB_SRCS
        metal.cpp
)

SET(metal_LIB_HDRS
        metal.h
)

# moc, if needed
KDE3_AUTOMOC(${metal_LIB_SRCS})

ADD_LIBRARY(metal SHARED ${metal_LIB_SRCS} ${metal_LIB_HDRS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(metal simlib)

SET_TARGET_PROPERTIES(metal PROPERTIES PREFIX "")

# install target
INSTALL(TARGETS metal LIBRARY DESTINATION ${SIM_PLUGIN_DIR}/styles RUNTIME  DESTINATION ${SIM_PLUGIN_DIR}/styles)

IF(WIN32)
    SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/plugins/styles)
ENDIF(WIN32)
