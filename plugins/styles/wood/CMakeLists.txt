################
# wood library #
################
PROJECT(wood)

SET(wood_LIB_SRCS
        wood.cpp
)

SET(wood_LIB_HDRS
        wood.h
)

# moc, if needed
KDE3_AUTOMOC(${wood_LIB_SRCS})

ADD_LIBRARY(wood SHARED ${wood_LIB_SRCS} ${wood_LIB_HDRS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(wood simlib)

SET_TARGET_PROPERTIES(wood PROPERTIES PREFIX "")

# install target
INSTALL(TARGETS wood LIBRARY DESTINATION ${SIM_PLUGIN_DIR}/styles RUNTIME  DESTINATION ${SIM_PLUGIN_DIR}/styles)

IF(WIN32)
    SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/plugins/styles)
ENDIF(WIN32)
