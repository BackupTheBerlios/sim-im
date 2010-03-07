IF(BUILD_DROPPED)
IF(WIN32)
    ###################
    # windock library #
    ###################
    SET(windock_SRCS
            windock.cpp
    )

    SET(windock_HDRS
            windock.h
    )

    # some needed include dirs
    INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/plugins/ontop)

    REMOVE_DEFINITIONS(-DQT3_SUPPORT)
    REMOVE_DEFINITIONS(-DQT_3SUPPORT_LIB)
    REMOVE_DEFINITIONS(-DQT3_SUPPORT_WARNINGS)
    SIM_ADD_PLUGIN(windock)
ELSE(WIN32)
    MESSAGE(STATUS "Windock plugin can only be used on windows")
ENDIF(WIN32)
ENDIF(BUILD_DROPPED)
