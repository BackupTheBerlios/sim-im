###################
# weather library #
###################
IF(BUILD_DROPPED)
SET(weather_SRCS
        weather.cpp
        weathercfg.cpp
        wifacecfg.cpp
)

SET(weather_HDRS
        weather.h
        weathercfg.h
        wifacecfg.h
)

SET(weather_UICS
        weathercfgbase.ui
        wifacecfgbase.ui
)

ADD_JISP_ARCHIVE(jisp weather.jisp weather_SRCS)

REMOVE_DEFINITIONS(-DQT3_SUPPORT)
REMOVE_DEFINITIONS(-DQT_3SUPPORT_LIB)
REMOVE_DEFINITIONS(-DQT3_SUPPORT_WARNINGS)
SIM_ADD_PLUGIN(weather)
ENDIF(BUILD_DROPPED)
