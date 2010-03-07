# a small macro to create mo files out of po's

MACRO(FIND_MSGFMT)
    IF(NOT MSGFMT_EXECUTABLE)
        IF(NOT MSGFMT_NOT_FOUND)
            IF(WIN32)
              SET(MSGFMT_NAME "msg2qm")
            ELSE(WIN32)
              SET(MSGFMT_NAME "msgfmt")
            ENDIF(WIN32)
            FIND_PROGRAM(MSGFMT_EXECUTABLE ${MSGFMT_NAME})

            IF (NOT MSGFMT_EXECUTABLE)
                SET(MSGFMT_NAME "msgfmt")
                FIND_PROGRAM(MSGFMT_EXECUTABLE ${MSGFMT_NAME})
            ENDIF(NOT MSGFMT_EXECUTABLE)

            IF (NOT MSGFMT_EXECUTABLE)
              MESSAGE(FATAL_ERROR "${MSGFMT_NAME} not found - po files can't be processed")
              SET(MSGFMT_NOT_FOUND "1")     # to avoid double checking in one cmake run
            ENDIF (NOT MSGFMT_EXECUTABLE)

            MARK_AS_ADVANCED(MSGFMT_EXECUTABLE)
        ENDIF(NOT MSGFMT_NOT_FOUND)
    ENDIF(NOT MSGFMT_EXECUTABLE)
ENDMACRO(FIND_MSGFMT)

MACRO(COMPILE_PO_FILES po_subdir _sources)
    FIND_MSGFMT()

    IF(MSGFMT_EXECUTABLE)
        FILE(GLOB po_files ${po_subdir}/*.po)

        FOREACH(po_input ${po_files})

            GET_FILENAME_COMPONENT(_in       ${po_input} ABSOLUTE)
            GET_FILENAME_COMPONENT(_basename ${po_input} NAME_WE)

            IF(WIN32)
                FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/po)
                GET_FILENAME_COMPONENT(_out ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/po/${_basename}.qm ABSOLUTE)
                FILE(TO_NATIVE_PATH ${_in}  _in_native)
                FILE(TO_NATIVE_PATH ${_out} _out_native)
                GET_FILENAME_COMPONENT(_tmp ${MSGFMT_EXECUTABLE} NAME_WE)

                IF("${_tmp}" STREQUAL "msg2qm")
                    ADD_CUSTOM_COMMAND(
                        OUTPUT ${_out}
                        COMMAND ${CMAKE_COMMAND}
                            -E echo
                            "Generating" ${_out_native} "from" ${_in_native}
                        COMMAND ${MSGFMT_EXECUTABLE}
                            ${_in_native}
                            ${_out_native}
                        DEPENDS ${_in}
                    )
                ELSE("${_tmp}" STREQUAL "msg2qm")
                    ADD_CUSTOM_COMMAND(
                        OUTPUT ${_out}
                        COMMAND ${CMAKE_COMMAND}
                            -E echo
                            "Generating" ${_out_native} "from" ${_in_native}
                        COMMAND ${MSGFMT_EXECUTABLE}
                            --qt
                            ${_in_native}
                            -o
                            ${_out_native}
                        DEPENDS ${_in}
                    )
                ENDIF("${_tmp}" STREQUAL "msg2qm")
            ELSE(WIN32)
                FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/po)
                GET_FILENAME_COMPONENT(_out ${CMAKE_CURRENT_BINARY_DIR}/po/${_basename}.mo ABSOLUTE)

                ADD_CUSTOM_COMMAND(
                    OUTPUT ${_out}
                    COMMAND ${CMAKE_COMMAND}
                        -E echo
                        "Generating" ${_out} "from" ${_in}
                    COMMAND ${MSGFMT_EXECUTABLE}
                        ${_in}
                        -o ${_out}
                    DEPENDS ${_in}
                )
            ENDIF(WIN32)
            SET(mo_files ${mo_files} ${_out})

            IF(NOT WIN32)
               INSTALL(FILES ${_out} DESTINATION ${SIM_I18N_DIR}/${_basename}/LC_MESSAGES RENAME sim.mo)
            ENDIF(NOT WIN32)
        ENDFOREACH(po_input ${po_files})

        IF(WIN32)
           INSTALL(FILES ${mo_files} DESTINATION ${SIM_I18N_DIR})
        ENDIF(WIN32)
        SET(${_sources} ${${_sources}} ${mo_files})
    ENDIF(MSGFMT_EXECUTABLE)
ENDMACRO(COMPILE_PO_FILES po_subdir)
