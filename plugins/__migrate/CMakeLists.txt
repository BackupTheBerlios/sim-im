#####################
# __migrate library #
#####################
IF(BUILD_DROPPED)
SET(__migrate_SRCS
        migrate.cpp
        migratedlg.cpp
)

SET(__migrate_HDRS
        migrate.h
        migratedlg.h
)

SET(__migrate_UICS
        migratedlgbase.ui
)

REMOVE_DEFINITIONS(-DQT3_SUPPORT)
REMOVE_DEFINITIONS(-DQT_3SUPPORT_LIB)
REMOVE_DEFINITIONS(-DQT3_SUPPORT_WARNINGS)
SIM_ADD_PLUGIN(__migrate)
ENDIF(BUILD_DROPPED)
