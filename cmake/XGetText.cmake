SET(XGETTEXT_OPTIONS -j --foreign-user -C -ci18n -ki18n -ktr2i18n -kI18N_NOOP -kI18N_NOOP2 -kaliasLocale)

FIND_PROGRAM(XGETTEXT_EXECUTABLE kde-xgettext)
IF (XGETTEXT_EXECUTABLE)
    MESSAGE(STATUS "Found kde-xgettext: ${XGETTEXT_EXECUTABLE}")
ELSE()
    MESSAGE(SATUS "kde-xgettext not found")
ENDIF()

FIND_PROGRAM(MSGMERGE_EXECUTABLE msgmerge)
IF (MSGMERGE_EXECUTABLE)
    MESSAGE(STATUS "Found msgmerge: ${MSGMERGE_EXECUTABLE}")
ELSE()
    MESSAGE(STATUS "msgmerge not found")
ENDIF()

MACRO(EXTRACT_MESSAGES src_file po_file)
    SET(PROJECT_NAME Sim-IM)  # Change this if you move this file to another project

    SET(fake_ui_cpp_root "${CMAKE_CURRENT_BINARY_DIR}/fake_ui_cpp")

    IF(IS_ABSOLUTE ${src_file})
       FILE(RELATIVE_PATH relative_name ${${PROJECT_NAME}_SOURCE_DIR} ${src_file})
    ELSE(IS_ABSOLUTE ${src_file})
       SET(relative_name ${src_file})
    ENDIF(IS_ABSOLUTE ${src_file})

    GET_FILENAME_COMPONENT(ext ${relative_name} EXT)
    IF(ext STREQUAL .ui)
        # creatig fake .cpp file where messages are located in the same lines as in .ui file
        # and put it with the whole relative path to ${fake_ui_cpp_root}
        ADD_CUSTOM_COMMAND(TARGET update-messages
            COMMAND echo Extracting messages from  UI-file ${relative_name}
            COMMAND ${CMAKE_COMMAND}
            -D IN_FILE:STRING=${${PROJECT_NAME}_SOURCE_DIR}/${relative_name}
            -D OUT_FILE:STRING=${fake_ui_cpp_root}/${relative_name}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/UiToFakeCpp.cmake
        )
        # parsing fake .cpp file from the ${fake_ui_cpp_root} in order to get the same path as 
        # path to the real .ui file
        FILE(RELATIVE_PATH po_relative ${fake_ui_cpp_root} ${po_file}) # kde-xgettext do not accept full path to po-file
        ADD_CUSTOM_COMMAND(TARGET update-messages
            COMMAND ${XGETTEXT_EXECUTABLE} ${XGETTEXT_OPTIONS} -d${po_relative} ${relative_name}
            WORKING_DIRECTORY ${fake_ui_cpp_root}
        )
    ELSEIF(ext STREQUAL .cpp)
        # just parse .cpp file
        FILE(RELATIVE_PATH po_relative ${${PROJECT_NAME}_SOURCE_DIR} ${po_file}) # kde-xgettext do not accept full path to po-file
        ADD_CUSTOM_COMMAND(TARGET update-messages
            COMMAND echo  Extracting messages from CPP-file ${relative_name}
            COMMAND ${XGETTEXT_EXECUTABLE} ${XGETTEXT_OPTIONS} -d${po_relative} ${relative_name}
            WORKING_DIRECTORY ${${PROJECT_NAME}_SOURCE_DIR}
        )
    ENDIF(ext STREQUAL .ui)
ENDMACRO(EXTRACT_MESSAGES)

MACRO(EMPTY_PO_FILE po_file)
    # creating new po file with correct charset in it
    # if this file is created automaticly charset will not be set
    # and there would be a lot of warnings while adding new messages in it
    ADD_CUSTOM_COMMAND(TARGET update-messages
        COMMAND echo \"msgid \\\"\\\"\" > ${po_file}.po
        COMMAND echo \"msgstr \\\"Content-Type: text/plain\; charset=UTF-8\\\"\" >> ${po_file}.po
    )
ENDMACRO(EMPTY_PO_FILE)
