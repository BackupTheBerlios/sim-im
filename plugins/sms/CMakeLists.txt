###############
# sms library #
###############
IF(BUILD_DROPPED)
SET(sms_SRCS
        gsm_ta.cpp
        serial.cpp
        sms.cpp
        smssetup.cpp
)

SET(sms_HDRS
        gsm_ta.h
        serial.h
        sms.h
        smssetup.h
)

SET(sms_UICS
        smssetupbase.ui
)

SET(sms_LIBS
        _core
)

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/plugins/_core)

SIM_ADD_PLUGIN(sms)
ENDIF(BUILD_DROPPED)
