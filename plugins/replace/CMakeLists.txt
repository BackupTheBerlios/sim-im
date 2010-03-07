###################
# replace library #
###################
IF(BUILD_DROPPED)
SET(replace_SRCS
        replace.cpp
        replacecfg.cpp
)

SET(replace_HDRS
        replace.h
        replacecfg.h
)

SET(replace_UICS
        replacecfgbase.ui
)

SIM_ADD_PLUGIN(replace)
ENDIF(BUILD_DROPPED)
