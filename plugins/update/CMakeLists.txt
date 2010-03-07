##################
# update library #
##################
IF(BUILD_DROPPED)
IF(FALSE)

SET(update_SRCS
        update.cpp
)

SET(update_HDRS
        update.h
)

SIM_ADD_PLUGIN(update)

ENDIF(FALSE)
ENDIF(BUILD_DROPPED)
