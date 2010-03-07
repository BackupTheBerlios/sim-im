# Searching and enabling plugins
MACRO(SIM_ADD_PLUGIN _name)
    PROJECT(${_name})
    
    SET(_srcs ${${_name}_SRCS})
    SET(_hdrs ${${_name}_HDRS})
    SET(_uics ${${_name}_UICS})
    SET(_libs ${${_name}_LIBS})

    SET(${_name}_MESSAGE_SOURCES ${_srcs} ${_uics} PARENT_SCOPE)

    QT4_WRAP_CPP(_mocs ${_hdrs})
    QT4_WRAP_UI(_uis ${_uics})
    
    SOURCE_GROUP(UI-Files  FILES ${_uics})
    SOURCE_GROUP(MOC-Files FILES ${_mocs})

    ADD_LIBRARY(${_name} SHARED ${_srcs} ${_hdrs} ${_mocs} ${_uis})

    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

    TARGET_LINK_LIBRARIES(${_name} simlib ${_libs})

    SET_TARGET_PROPERTIES(${_name} PROPERTIES PREFIX "")

    STRING(TOUPPER ${_name} _NAME)
    SET_TARGET_PROPERTIES(${_name} PROPERTIES DEFINE_SYMBOL MAKE_${_NAME}_LIB)

    INSTALL(TARGETS ${_name} LIBRARY DESTINATION ${SIM_PLUGIN_DIR} RUNTIME  DESTINATION ${SIM_PLUGIN_DIR})

    IF(APPLE)                                                                                                                                                                                        
	SET(LIBRARY_OUTPUT_PATH ${SIM_PLUGIN_DIR})                                                                                                                                                           
    ENDIF(APPLE)                                                                                                                                                                                     
ENDMACRO(SIM_ADD_PLUGIN)

MACRO(SIM_FIND_PLUGINS sim_plugin_dir)
    FILE(GLOB plugins_dir ${sim_plugin_dir}/* )
    FOREACH(_cur_dir ${plugins_dir})
        FILE(GLOB cmakefile ${_cur_dir}/CMakeLists.txt)
        IF(cmakefile)
            GET_FILENAME_COMPONENT(plugin ${_cur_dir} NAME_WE)
            
            STRING(TOUPPER ${plugin} uc_plugin)
            OPTION(ENABLE_PLUGIN_${uc_plugin}  "Enable plugin ${plugin}" ON)

            # all plugins starting with _ are *not* optional!
            IF(${uc_plugin} MATCHES "^_.*$")
                ## prepend
                SET(SIM_PLUGINS ${plugin} ${SIM_PLUGINS})
                MARK_AS_ADVANCED(ENABLE_PLUGIN_${uc_plugin})
            ELSE(${uc_plugin} MATCHES "^_.*$")
                ## append
                SET(SIM_PLUGINS ${SIM_PLUGINS} ${plugin})
            ENDIF(${uc_plugin} MATCHES "^_.*$")
            
        LIST(SORT SIM_PLUGINS)
        ENDIF(cmakefile)
    ENDFOREACH(_cur_dir)
ENDMACRO(SIM_FIND_PLUGINS sim_plugin_dir)

MACRO(SIM_INCLUDE_PLUGINS)
    FOREACH(plugin ${SIM_PLUGINS})
        STRING(TOUPPER ${plugin} uc_plugin)
        IF(ENABLE_PLUGIN_${uc_plugin})
            MESSAGE(STATUS "Using plugin " ${plugin})
            ADD_SUBDIRECTORY(plugins/${plugin})
        ELSE(ENABLE_PLUGIN_${uc_plugin})
            IF ( ENABLE_TARGET_UPDATE_MESSAGES )
                MESSAGE(STATUS "Using plugin " ${plugin} " for message updating purposes only...")
                ADD_SUBDIRECTORY(plugins/${plugin} EXCLUDE_FROM_ALL)
            ELSE( ENABLE_TARGET_UPDATE_MESSAGES)
                MESSAGE(STATUS "Skipping plugin " ${plugin})
            ENDIF( ENABLE_TARGET_UPDATE_MESSAGES )
        ENDIF(ENABLE_PLUGIN_${uc_plugin})
    ENDFOREACH(plugin ${SIM_PLUGINS})
ENDMACRO(SIM_INCLUDE_PLUGINS)
