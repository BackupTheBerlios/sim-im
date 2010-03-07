#################
# sound library #
#################
SET(sound_SRCS
        sound.cpp
        soundconfig.cpp
        sounduser.cpp
)

SET(sound_HDRS
        sound.h
        soundconfig.h
        sounduser.h
)

SET(sound_UICS
        soundconfigbase.ui
        sounduserbase.ui
)

SET(sound_LIBS
        ${QT_PHONON_LIBRARY}
)

REMOVE_DEFINITIONS(-DQT3_SUPPORT)
REMOVE_DEFINITIONS(-DQT_3SUPPORT_LIB)
REMOVE_DEFINITIONS(-DQT3_SUPPORT_WARNINGS)
IF(QT_PHONON_FOUND)
  SIM_ADD_PLUGIN(sound)
ELSE(QT_PHONON_FOUND)
  MESSAGE(STATUS "Cannot build sound plugin because Phonon is missing on your system")
ENDIF(QT_PHONON_FOUND)

# install new sounds with known licence
INSTALL(FILES sounds/added.ogg		DESTINATION ${SIM_SOUND_DIR})
INSTALL(FILES sounds/alert.ogg		DESTINATION ${SIM_SOUND_DIR})
INSTALL(FILES sounds/auth.ogg		DESTINATION ${SIM_SOUND_DIR})
INSTALL(FILES sounds/authrequest.ogg	DESTINATION ${SIM_SOUND_DIR})
INSTALL(FILES sounds/contacts.ogg	DESTINATION ${SIM_SOUND_DIR})
INSTALL(FILES sounds/contactrequest.ogg	DESTINATION ${SIM_SOUND_DIR})
INSTALL(FILES sounds/deleted.ogg	DESTINATION ${SIM_SOUND_DIR})
INSTALL(FILES sounds/error.ogg		DESTINATION ${SIM_SOUND_DIR})
INSTALL(FILES sounds/file.ogg		DESTINATION ${SIM_SOUND_DIR})
INSTALL(FILES sounds/filedone.ogg	DESTINATION ${SIM_SOUND_DIR})
INSTALL(FILES sounds/mailpager.ogg	DESTINATION ${SIM_SOUND_DIR})
INSTALL(FILES sounds/message.ogg	DESTINATION ${SIM_SOUND_DIR})
INSTALL(FILES sounds/msgsent.ogg	DESTINATION ${SIM_SOUND_DIR})
INSTALL(FILES sounds/refused.ogg	DESTINATION ${SIM_SOUND_DIR})
INSTALL(FILES sounds/sms.ogg		DESTINATION ${SIM_SOUND_DIR})
INSTALL(FILES sounds/startup.ogg	DESTINATION ${SIM_SOUND_DIR})
INSTALL(FILES sounds/url.ogg		DESTINATION ${SIM_SOUND_DIR})
INSTALL(FILES sounds/vip-online.ogg	DESTINATION ${SIM_SOUND_DIR})
INSTALL(FILES sounds/web.ogg		DESTINATION ${SIM_SOUND_DIR})
