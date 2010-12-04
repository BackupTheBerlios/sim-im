###############
# icq library #
###############
SET(icq_SRCS
#        aboutinfo.cpp
#        advsearch.cpp
#        aimconfig.cpp
#        aiminfo.cpp
#        aimsearch.cpp
#        aimservices.cpp
#        encodingdlg.cpp
#        homeinfo.cpp
        authorizationsnachandler.cpp
        bytearraybuilder.cpp
        bytearrayparser.cpp
        icq.cpp
#        icqbos.cpp
#        icqbuddy.cpp
#        icqbuffer.cpp
        icqclient.cpp
        icqcontact.cpp
        icqgroup.cpp
        icqstatuswidget.cpp
#        icqconfig.cpp
#        icqdirect.cpp
#        icqicmb.cpp
#        icqinfo.cpp
#        icqlists.cpp
#        icqlocation.cpp
#        icqmessage.cpp
#        icqpicture.cpp
#        icqping.cpp
#        icqsearch.cpp
#        icqsecure.cpp
#        icqservice.cpp
#        icqssbi.cpp
        icqstatus.cpp
        oscarsocket.cpp
        rateinfo.cpp
        servicesnachandler.cpp
        standardoscarsocket.cpp
#        icqvarious.cpp
#        interestsinfo.cpp
#        moreinfo.cpp
#        pastinfo.cpp
#        polling.cpp
#        rtfgen.cpp
#        securedlg.cpp
#        verifydlg.cpp
#        warndlg.cpp
#        workinfo.cpp
#        xml.cpp
        snac.cpp
        tlv.cpp
        tlvlist.cpp
)

SET(icq_HDRS
#        aboutinfo.h
#        advsearch.h
#        aimconfig.h
#        aiminfo.h
#        aimsearch.h
#        encodingdlg.h
#        homeinfo.h
        authorizationsnachandler.h
        bytearraybuilder.h
        bytearrayparser.h
        icq.h
#        icqbuffer.h
        icqclient.h
        icqcontact.h
        icqgroup.h
        icqstatuswidget.h
        icq_defines.h
#        icqconfig.h
#        icqinfo.h
#        icqlocation.h
#        icqmessage.h
#        icqdirect.h
#        icqicmb.h
#        icqpicture.h
#        icqsearch.h
#        icqsecure.h
#        icqssbi.h
        icqstatus.h
        oscarsocket.h
        rateinfo.h
        servicesnachandler.h
        standardoscarsocket.h
#        interestsinfo.h
#        moreinfo.h
#        pastinfo.h
#        polling.h
#        securedlg.h
#        verifydlg.h
#        warndlg.h
#        workinfo.h
#        xml.h
        snac.h
        tlv.h
        tlvlist.h
)

SET(icq_UICS
#        aboutinfobase.ui
#        advsearchbase.ui
#        aimconfigbase.ui
#        aiminfobase.ui
#        aimsearchbase.ui
#        encodingdlgbase.ui
#        homeinfobase.ui
#        icqconfigbase.ui
#        icqinfobase.ui
#        icqpicturebase.ui
#        icqsearchbase.ui
#        icqsecurebase.ui
#        interestsinfobase.ui
#        moreinfobase.ui
#        pastinfobase.ui
#        securedlgbase.ui
#        verifydlgbase.ui
#        warndlgbase.ui
#        workinfobase.ui
)

#ADD_FLEX_FILES(icq_SRCS rtf.ll)

SIM_ADD_PLUGIN(icq)
SET_TARGET_PROPERTIES(icq PROPERTIES DEFINE_SYMBOL ICQ_EXPORTS)

SET(icq_test_headers
    tests/qt-gtest.h
    tests/testicqclient.h
    tests/testoscarsocket.h
    tests/testauthorizationsnachandler.h
    tests/mocks/mockoscarsocket.h
)

QT4_WRAP_CPP(testmocs ${icq_test_headers})

SET(icq_test_sources
    tests/qt-gtest.cpp
    tests/test.cpp
    tests/testicqclient.cpp
    tests/testicqstatus.cpp
    tests/testicqcontact.cpp
    tests/testoscarsocket.cpp
    tests/testicqstatuswidget.cpp
    tests/testbytearraybuilder.cpp
    tests/testbytearrayparser.cpp
    tests/testauthorizationsnachandler.cpp
    tests/testservicesnachandler.cpp
    tests/testtlv.cpp
    tests/testtlvlist.cpp
)

ADD_EXECUTABLE(test_icq ${icq_test_sources} ${testmocs})
TARGET_LINK_LIBRARIES(test_icq simlib ${QT_LIBRARIES} gtest gmock icq)

