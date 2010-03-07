##################
# filter library #
##################
IF(BUILD_DROPPED)
SET(filter_SRCS
        filter.cpp
        filtercfg.cpp
        ignorelist.cpp
)

SET(filter_HDRS
        filter.h
        filtercfg.h
        ignorelist.h
)

SET(filter_UICS
        filtercfgbase.ui
        ignorelistbase.ui
)

SIM_ADD_PLUGIN(filter)
ENDIF(BUILD_DROPPED)
