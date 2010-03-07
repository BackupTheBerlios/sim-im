# install only

FILE(GLOB mng *.mng)
FILE(GLOB png *.png)

INSTALL(FILES ${mng} DESTINATION ${SIM_PICT_DIR})
INSTALL(FILES ${png} DESTINATION ${SIM_PICT_DIR})
