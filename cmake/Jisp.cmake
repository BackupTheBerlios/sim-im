# a small macro to create one or more jisp archives
# ADD_JISP_ARCHIVE(jisp-subdir)

# search zip
MACRO(FIND_ZIP)
    IF(NOT ZIP_EXECUTABLE)
        FIND_PROGRAM(ZIP_EXECUTABLE zip)
        IF (NOT ZIP_EXECUTABLE)
          MESSAGE(FATAL_ERROR "zip not found - aborting")
        ENDIF (NOT ZIP_EXECUTABLE)
    ENDIF(NOT ZIP_EXECUTABLE)
ENDMACRO(FIND_ZIP)


MACRO(ADD_JISP_ARCHIVE subdir jisp_name _sources)
    FIND_ZIP()

    GET_FILENAME_COMPONENT(_in_dir ${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/icondef.xml PATH)

    IF(EXISTS ${_in_dir}/icondef.xml)
        FILE(GLOB _in ${_in_dir}/*.png)
        SET(_in ${_in} ${_in_dir}/icondef.xml)

        IF(WIN32)
            GET_FILENAME_COMPONENT(_out ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/icons/${jisp_name} ABSOLUTE)
        ELSE(WIN32)
            IF(APPLE)
                GET_FILENAME_COMPONENT(_out ${SIM_ICONS_DIR}/${jisp_name} ABSOLUTE)
            ELSE(APPLE)
                GET_FILENAME_COMPONENT(_out ${CMAKE_CURRENT_BINARY_DIR}/${jisp_name} ABSOLUTE)
            ENDIF(APPLE)
        ENDIF(WIN32)
    
        FILE(WRITE ${_out}.files "") 
        FOREACH(_file ${_in})
            FILE(APPEND ${_out}.files "${_file}\n")
        ENDFOREACH(_file ${_in})

        ADD_CUSTOM_COMMAND(
           OUTPUT ${_out}
            COMMAND ${ZIP_EXECUTABLE}
            -j -q -9 ${_out} -@ < ${_out}.files
            DEPENDS ${_in}
        )
        SET(${_sources} ${${_sources}} ${_out})

        INSTALL(FILES ${_out} DESTINATION  ${SIM_ICONS_DIR})
    ENDIF(EXISTS ${_in_dir}/icondef.xml)
ENDMACRO(ADD_JISP_ARCHIVE)
