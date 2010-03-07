#################
# yahoo library #
#################
IF(BUILD_DROPPED)
if(OPENSSL_FOUND)
    SET(yahoo_SRCS
            crypt.cpp
            yahoo.cpp
            yahooauth.cpp
            yahoocfg.cpp
            yahooclient.cpp
            yahoofiletransfer.cpp
            yahoohttp.cpp
            yahooinfo.cpp
            yahoosearch.cpp
            yahootransformtables.c
    )
    
    SET(yahoo_HDRS
            yahoo.h
            yahoocfg.h
            yahooclient.h
            yahooinfo.h
            yahoosearch.h
    )
    
    SET(yahoo_UICS
            yahoocfgbase.ui
            yahooinfobase.ui
            yahoosearchbase.ui
    )
    
    INCLUDE_DIRECTORIES(${OPENSSL_INCLUDE_DIR})
    LINK_DIRECTORIES(${OPENSSL_LIBRARY_DIR})
    SIM_ADD_PLUGIN(yahoo)
    TARGET_LINK_LIBRARIES(yahoo ${OPENSSL_LIBRARIES} ${OPENSSL_EAY_LIBRARIES})
else(OPENSSL_FOUND)
    MESSAGE(STATUS "OpenSSL library not found, yahoo plugin disabled")
endif(OPENSSL_FOUND)
ENDIF(BUILD_DROPPED)
