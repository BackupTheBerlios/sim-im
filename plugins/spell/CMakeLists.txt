IF(BUILD_DROPPED)
IF(ASPELL_FOUND AND NOT ENABLE_KDE3)

    #################
    # spell library #
    #################
    SET(spell_SRCS
            spell.cpp
            spellcfg.cpp
            speller.cpp
            spellhighlight.cpp
    )

    SET(spell_HDRS
            spell.h
            spellcfg.h
            speller.h
            spellhighlight.h
    )

    IF(WIN32)
      SET(spell_SRCS
              ${spell_SRCS}
              spellfind.cpp
      )
      SET(spell_HDRS
              ${spell_HDRS}
              spellfind.h
      )
    ENDIF(WIN32)

    SET(spell_UICS
            spellcfgbase.ui
            spellfindbase.ui
    )
    
    SET(spell_LIBS
            ${ASPELL_LIBRARIES}
    )

    # some needed include dirs
    INCLUDE_DIRECTORIES(${ASPELL_INCLUDE_DIR})

    SIM_ADD_PLUGIN(spell)

ELSE(ASPELL_FOUND AND NOT ENABLE_KDE3)
  IF(ENABLE_KDE3)
    MESSAGE(STATUS "Spell plugin is disabled when building with KDE")
  ELSE(ENABLE_KDE3)
    MESSAGE(STATUS "Cannot build spell plugin because aspell is missing on your system")
  ENDIF(ENABLE_KDE3)
ENDIF(ASPELL_FOUND AND NOT ENABLE_KDE3)
ENDIF(BUILD_DROPPED)
