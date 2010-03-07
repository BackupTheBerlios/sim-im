#################
# proxy library #
#################
IF(BUILD_DROPPED)
PROJECT(proxy)

SET(proxy_SRCS
        proxy.cpp
        proxycfg.cpp
        proxyerror.cpp
)

SET(proxy_HDRS
        proxy.h
        proxycfg.h
        proxyerror.h
)

SET(proxy_UICS
        proxycfgbase.ui
        proxyerrorbase.ui
)

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/plugins/_core)

SIM_ADD_PLUGIN(proxy)
ENDIF(BUILD_DROPPED)
