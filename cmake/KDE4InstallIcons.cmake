# from /trunk/KDE/kdelibs/cmake/modules/KDE4Macros.cmake

# Copyright (c) 2006, Alexander Neundorf, <neundorf@kde.org>
# Copyright (c) 2006, Laurent Montel, <montel@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# only used internally by KDE4_INSTALL_ICONS
MACRO (_KDE4_ADD_ICON_INSTALL_RULE _install_SCRIPT _install_PATH _group _orig_NAME _install_NAME)

   # if the string doesn't match the pattern, the result is the full string, so all three have the same content
   IF (NOT ${_group} STREQUAL ${_install_NAME} )
      SET(_icon_GROUP "actions")

      IF (${_group} STREQUAL "mime")
         SET(_icon_GROUP  "mimetypes")
      ENDIF (${_group} STREQUAL "mime")

      IF (${_group} STREQUAL "filesys")
         SET(_icon_GROUP  "filesystems")
      ENDIF (${_group} STREQUAL "filesys")

      IF (${_group} STREQUAL "device")
         SET(_icon_GROUP  "devices")
      ENDIF (${_group} STREQUAL "device")

      IF (${_group} STREQUAL "app")
         SET(_icon_GROUP  "apps")
      ENDIF (${_group} STREQUAL "app")

      IF (${_group} STREQUAL "action")
         SET(_icon_GROUP  "actions")
      ENDIF (${_group} STREQUAL "action")

#      message(STATUS "icon: ${_current_ICON} size: ${_size} group: ${_group} name: ${_name}" )
   INSTALL(FILES ${_orig_NAME} DESTINATION ${_install_PATH}/${_icon_GROUP}/ RENAME ${_install_NAME} )
   ENDIF (NOT ${_group} STREQUAL ${_install_NAME} )

ENDMACRO (_KDE4_ADD_ICON_INSTALL_RULE)


macro (KDE4_INSTALL_ICONS _defaultpath )

   # first the png icons
   file(GLOB _icons *.png)
   foreach (_current_ICON ${_icons} )
      string(REGEX REPLACE "^.*/([a-zA-Z]+)([0-9]+)\\-([a-z]+)\\-(.+\\.png)$" "\\1" _type  "${_current_ICON}")
      string(REGEX REPLACE "^.*/([a-zA-Z]+)([0-9]+)\\-([a-z]+)\\-(.+\\.png)$" "\\2" _size  "${_current_ICON}")
      string(REGEX REPLACE "^.*/([a-zA-Z]+)([0-9]+)\\-([a-z]+)\\-(.+\\.png)$" "\\3" _group "${_current_ICON}")
      string(REGEX REPLACE "^.*/([a-zA-Z]+)([0-9]+)\\-([a-z]+)\\-(.+\\.png)$" "\\4" _name  "${_current_ICON}")
      set(_theme_GROUP "nogroup")
      if( ${_type} STREQUAL "cr" )
	set(_theme_GROUP  "crystalsvg")	
      endif(${_type} STREQUAL "cr" )

      if( ${_type} STREQUAL "lo" )
      	set(_theme_GROUP  "locolor")
      endif(${_type} STREQUAL "lo" )

      if( ${_type} STREQUAL "hi" )
 	set(_theme_GROUP  "hicolor")
      endif(${_type} STREQUAL "hi" )
      
      if( NOT ${_theme_GROUP} STREQUAL "nogroup")
      		_KDE4_ADD_ICON_INSTALL_RULE(${CMAKE_CURRENT_BINARY_DIR}/install_icons.cmake
                    ${_defaultpath}/${_theme_GROUP}/${_size}x${_size}
                    ${_group} ${_current_ICON} ${_name})
      endif( NOT ${_theme_GROUP} STREQUAL "nogroup")

   endforeach (_current_ICON)

   # mng icons
   file(GLOB _icons *.mng)
   foreach (_current_ICON ${_icons} )
      STRING(REGEX REPLACE "^.*/([a-zA-Z]+)([0-9]+)\\-([a-z]+)\\-(.+\\.mng)$" "\\1" _type  "${_current_ICON}")
      STRING(REGEX REPLACE "^.*/([a-zA-Z]+)([0-9]+)\\-([a-z]+)\\-(.+\\.mng)$" "\\2" _size  "${_current_ICON}")
      STRING(REGEX REPLACE "^.*/([a-zA-Z]+)([0-9]+)\\-([a-z]+)\\-(.+\\.mng)$" "\\3" _group "${_current_ICON}")
      STRING(REGEX REPLACE "^.*/([a-zA-Z]+)([0-9]+)\\-([a-z]+)\\-(.+\\.mng)$" "\\4" _name  "${_current_ICON}")
      SET(_theme_GROUP "nogroup")
      if( ${_type} STREQUAL "cr" )
	SET(_theme_GROUP  "crystalsvg")
      endif(${_type} STREQUAL "cr" )
      
      if( ${_type} STREQUAL "lo" )
        set(_theme_GROUP  "locolor")
      endif(${_type} STREQUAL "lo" )

      if( ${_type} STREQUAL "hi" )
        set(_theme_GROUP  "hicolor")
      endif(${_type} STREQUAL "hi" )

      if( NOT ${_theme_GROUP} STREQUAL "nogroup")
        	_KDE4_ADD_ICON_INSTALL_RULE(${CMAKE_CURRENT_BINARY_DIR}/install_icons.cmake
                ${_defaultpath}/${_theme_GROUP}/${_size}x${_size}
                ${_group} ${_current_ICON} ${_name})
      endif( NOT ${_theme_GROUP} STREQUAL "nogroup")

   endforeach (_current_ICON)


   # and now the svg icons
   file(GLOB _icons *.svgz)
   foreach (_current_ICON ${_icons} )
	    STRING(REGEX REPLACE "^.*/([a-zA-Z]+)sc\\-([a-z]+)\\-(.+\\.svgz)$" "\\1" _type "${_current_ICON}")
            STRING(REGEX REPLACE "^.*/([a-zA-Z]+)sc\\-([a-z]+)\\-(.+\\.svgz)$" "\\2" _group "${_current_ICON}")
            STRING(REGEX REPLACE "^.*/([a-zA-Z]+)sc\\-([a-z]+)\\-(.+\\.svgz)$" "\\3" _name "${_current_ICON}")
	    SET(_theme_GROUP "nogroup")

	    if(${_type} STREQUAL "lo" )
		SET(_theme_GROUP  "locolor")
	    endif(${_type} STREQUAL "lo" )

            if(${_type} STREQUAL "cr" )
		SET(_theme_GROUP  "crystalsvg")
            endif(${_type} STREQUAL "cr" )
            
            if(${_type} STREQUAL "hi" )
                SET(_theme_GROUP  "hicolor")
            endif(${_type} STREQUAL "hi" )

            if( NOT ${_theme_GROUP} STREQUAL "nogroup")
                	_KDE4_ADD_ICON_INSTALL_RULE(${CMAKE_CURRENT_BINARY_DIR}/install_icons.cmake
                                 ${_defaultpath}/${_theme_GROUP}/scalable
                                 ${_group} ${_current_ICON} ${_name})
            endif( NOT ${_theme_GROUP} STREQUAL "nogroup")

   endforeach (_current_ICON)

endmacro (KDE4_INSTALL_ICONS)
