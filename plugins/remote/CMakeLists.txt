##################
# remote library #
##################
IF(BUILD_DROPPED)
PROJECT(remote)

SET(remote_LIB_SRCS
        remote.cpp
        remotecfg.cpp
)

SET(remote_LIB_HDRS
        remote.h
        remotecfg.h
)

SET(remote_UI_FILES
        remotecfgbase.ui
)

# moc, if needed
QT4_WRAP_CPP(remote_mocs ${remote_LIB_HDRS})

# don't forget our ui's
QT4_WRAP_UI(remote_uis ${remote_UI_FILES})

ADD_LIBRARY(remote SHARED ${remote_LIB_SRCS} ${remote_mocs} ${remote_uis})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(remote simlib _core)

SET_TARGET_PROPERTIES(remote PROPERTIES PREFIX "")

# install target
INSTALL(TARGETS remote LIBRARY DESTINATION ${SIM_PLUGIN_DIR} RUNTIME  DESTINATION ${SIM_PLUGIN_DIR})

#########################
# next target - simctrl #
#########################

IF(0)
ADD_EXECUTABLE(simctrl simctrl.cpp)
IF(SIM_ENABLE_FPIE)
    SET_TARGET_PROPERTIES(simctrl PROPERTIES COMPILE_FLAGS "${SIM_CXX_FPIE_FLAGS}")
    SET_TARGET_PROPERTIES(simctrl PROPERTIES LINK_FLAGS "${SIM_PIE_LDFLAGS}")
ENDIF(SIM_ENABLE_FPIE)
TARGET_LINK_LIBRARIES(simctrl simlib)
INSTALL(TARGETS simctrl RUNTIME DESTINATION ${BINDIR})
IF(NOT WIN32)
    INSTALL(FILES simctrl.desktop DESTINATION ${SERVICES_INSTALL_DIR})
ENDIF(NOT WIN32)
    
#doesn't work yet
########################
# next target - simext #
########################

ADD_EXECUTABLE(simext simext.idl SIM_ext.cpp simext.cpp simext.rc)
INSTALL(TARGETS simext RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

########################
# next target - simipc #
########################

ADD_EXECUTABLE(simipc simipc.cpp)
INSTALL(TARGETS simipc RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

###########################
# next target - simremote #
###########################

ADD_EXECUTABLE(simremote simremote.idl sim_remote.cpp simControl.cpp simremote.rc)
INSTALL(TARGETS simremote RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

ENDIF(0)
ENDIF(BUILD_DROPPED)
