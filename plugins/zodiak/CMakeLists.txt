##################
# zodiak library #
##################
IF(BUILD_DROPPED)
SET(zodiak_SRCS
        zodiak.cpp
)

SET(zodiak_HDRS
        zodiak.h
)

SIM_ADD_PLUGIN(zodiak)
ENDIF(BUILD_DROPPED)
