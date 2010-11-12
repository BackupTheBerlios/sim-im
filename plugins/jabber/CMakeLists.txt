##################
# jabber library #
##################
SET(jabber_SRCS
#        discoinfo.cpp
#        httppoll.cpp
#        infoproxy.cpp
        jabber.cpp
#        jabber_auth.cpp
#        jabber_rosters.cpp
#        jabber_ssl.cpp
#        jabberaboutinfo.cpp
#        jabberadd.cpp
#        jabberbrowser.cpp
#        jabberbuffer.cpp
        jabberclient.cpp
        jabbercontact.cpp
        jabbergroup.cpp
#        jabberconfig.cpp
#        jabberfiletransfer.cpp
#        jabberhomeinfo.cpp
#        jabberinfo.cpp
#        jabbermessage.cpp
#        jabberpicture.cpp
#        jabbersearch.cpp
        jabberstatus.cpp
        jabberstatuswidget.cpp
#        jabberworkinfo.cpp
#        jidadvsearch.cpp
#        jidsearch.cpp
)

SET(jabber_HDRS
#        discoinfo.h
#        infoproxy.h
        jabber.h
        jabber_api.h
#        jabber_events.h
#        jabber_ssl.h
#        jabberaboutinfo.h
#        jabberadd.h
#        jabberbrowser.h
#        jabberbuffer.h
        jabberclient.h
        jabbercontact.h
        jabbergroup.h
#        jabberconfig.h
#        jabberhomeinfo.h
#        jabberinfo.h
#        jabbermessage.h
#        jabberpicture.h
#        jabbersearch.h
        jabberstatus.h
        jabberstatuswidget.h
#        jabberworkinfo.h
#        jidadvsearch.h
#        jidsearch.h
)

SET(jabber_UICS
#        discoinfobase.ui
#        infoproxybase.ui
#        jabberaboutinfobase.ui
#        jabberaddbase.ui
#        jabberconfigbase.ui
#        jabberhomeinfobase.ui
#        jabberinfobase.ui
#        jabberpicturebase.ui
#        jabberworkinfobase.ui
#        jidadvsearchbase.ui
#        jidsearchbase.ui
)

SIM_ADD_PLUGIN(jabber)
SET_TARGET_PROPERTIES(jabber PROPERTIES DEFINE_SYMBOL JABBER_EXPORTS)

SET(jabber_test_sources
    tests/test.cpp
    tests/testjabbercontact.cpp
    tests/testjabberclient.cpp
    tests/testjabberstatus.cpp
)

ADD_EXECUTABLE(test_jabber ${jabber_test_sources})
TARGET_LINK_LIBRARIES(test_jabber simlib ${QT_LIBRARIES} gtest gmock jabber)
