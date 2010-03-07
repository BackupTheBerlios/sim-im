#######################
# livejournal library #
#######################
IF(BUILD_DROPPED)
SET(livejournal_SRCS
        journalsearch.cpp
        livejournal.cpp
        livejournalcfg.cpp
        msgjournal.cpp
)

SET(livejournal_HDRS
        journalsearch.h
        livejournal.h
        livejournalcfg.h
        msgjournal.h
)

SET(livejournal_UICS
        journalsearchbase.ui
        livejournalcfgbase.ui
        msgjournalbase.ui
)

SIM_ADD_PLUGIN(livejournal)
ENDIF(BUILD_DROPPED)
