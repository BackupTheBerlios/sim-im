/***************************************************************************
                          login.cpp  -  description
                             -------------------
    begin                : Sun Mar 17 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan@shutoff.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>
#include <QPixmap>
#include <QApplication>
#include <QCryptographicHash>
#include <QByteArray>

#include "log.h"
#include "buffer.h"
#include "socket/socket.h"

#include "icqclient.h"
#include "verifydlg.h"

using namespace SIM;

const unsigned short ICQ_SNACxLOGIN_ERROR				= 0x0001;
const unsigned short ICQ_SNACxLOGIN_MD5xLOGIN			= 0x0002;
const unsigned short ICQ_SNACxLOGIN_LOGINxREPLY			= 0x0003;
const unsigned short ICQ_SNACxLOGIN_REGISTERxREQ		= 0x0004;
const unsigned short ICQ_SNACxLOGIN_REGISTER			= 0x0005;
const unsigned short ICQ_SNACxLOGIN_AUTHxREQUEST		= 0x0006;
const unsigned short ICQ_SNACxLOGIN_AUTHxKEYxRESPONSE	= 0x0007;
const unsigned short ICQ_SNACxLOGIN_REGISTERxREQ_IMG	= 0x000c;
const unsigned short ICQ_SNACxLOGIN_REGISTERxSEND_IMG	= 0x000d;

const unsigned ICQ_LOGIN_ERRxBAD_PASSWD1			= 0x0001;
const unsigned ICQ_LOGIN_ERRxBAD_PASSWD2			= 0x0004;
const unsigned ICQ_LOGIN_ERRxBAD_PASSWD3			= 0x0005;
const unsigned ICQ_LOGIN_ERRxBAD_LOGIN				= 0x0006;
const unsigned ICQ_LOGIN_ERRxNOT_EXISTS1			= 0x0007;
const unsigned ICQ_LOGIN_ERRxNOT_EXISTS2			= 0x0008;
const unsigned ICQ_LOGIN_ERRxUNAVAILABLE1			= 0x000c;
const unsigned ICQ_LOGIN_ERRxUNAVAILABLE2			= 0x000d;
const unsigned ICQ_LOGIN_ERRxSUSPENDED1				= 0x0011;
const unsigned ICQ_LOGIN_ERRxUNAVAILABLE3			= 0x0012;
const unsigned ICQ_LOGIN_ERRxUNAVAILABLE4			= 0x0013;
const unsigned ICQ_LOGIN_ERRxUNAVAILABLE5			= 0x0014;
const unsigned ICQ_LOGIN_ERRxUNAVAILABLE6			= 0x0015;
const unsigned ICQ_LOGIN_ERRxIP_RATE_LIMIT1			= 0x0016;
const unsigned ICQ_LOGIN_ERRxIP_RATE_LIMIT2			= 0x0017;
const unsigned ICQ_LOGIN_ERRxRATE_LIMIT1			= 0x0018;
const unsigned ICQ_LOGIN_ERRxUNAVAILABLE7			= 0x001a;
const unsigned ICQ_LOGIN_ERRxOLDCLIENT1				= 0x001b;
const unsigned ICQ_LOGIN_ERRxOLDCLIENT2				= 0x001c;
const unsigned ICQ_LOGIN_ERRxRATE_LIMIT2			= 0x001d;
const unsigned ICQ_LOGIN_ERRxCANT_REGISTER			= 0x001e;
const unsigned ICQ_LOGIN_ERRxUNAVAILABLE8			= 0x001f;
const unsigned ICQ_LOGIN_ERRxINVALID_ID 			= 0x0020;
const unsigned ICQ_LOGIN_ERRxTOO_YOUNG				= 0x0022;

void ICQClient::snac_login(unsigned short type, unsigned short)
{
    unsigned long newUin;
    switch (type){
    case ICQ_SNACxLOGIN_ERROR:
        if (data.owner.Uin.toULong()){
            m_reconnect = NO_RECONNECT;
            socket()->error_state(I18N_NOOP("Login error"), AuthError);
            break;
        }
        // in the process of registering;
        // it seems that we need to request bot protection picture;
        // reconnecting to send the request.
        log(L_DEBUG, "Verification required, reconnecting");
        m_bVerifying = true;
        socket()->close();
        socket()->connect(getServer(), getPort(), this);
        break;
    case ICQ_SNACxLOGIN_REGISTER:
        if (data.owner.Uin.toULong()){
            socket()->error_state(I18N_NOOP("Registered in no register state"));
            break;
        }
        socket()->readBuffer().incReadPos(0x2E);
        socket()->readBuffer().unpack(newUin);
        log(L_DEBUG, "Register %lu %08lX", newUin, newUin);
        setUin(newUin);
        setState(Connecting);
        socket()->connect(getServer(), getPort(), this);
        break;
    case ICQ_SNACxLOGIN_AUTHxKEYxRESPONSE:
        log(L_DEBUG, "Sending MD5 key");
        if (!data.owner.Screen.str().isEmpty() || data.owner.Uin.toULong()){
            QByteArray md5_key;
            socket()->readBuffer().unpackStr(md5_key);
            snac(ICQ_SNACxFOOD_LOGIN, ICQ_SNACxLOGIN_MD5xLOGIN, false, false);
            if (data.owner.Uin.toULong()){
                char uin[20];
                sprintf(uin, "%lu", data.owner.Uin.toULong());
                socket()->writeBuffer().tlv(0x0001, uin);
            }
			else
			{
                socket()->writeBuffer().tlv(0x0001, data.owner.Screen.str().toUtf8().data());
            }
            QByteArray md = md5_key;
            md += getContacts()->fromUnicode(NULL, getPassword());
            md += "AOL Instant Messenger (SM)";
            md = QCryptographicHash::hash(md, QCryptographicHash::Md5);
            socket()->writeBuffer().tlv(0x0025, md.data(), md.size());
	        if (data.owner.Uin.toULong()){
                socket()->writeBuffer().tlv(0x0003, "ICQBasic");  //ToDo: Should be updated anytime
                socket()->writeBuffer().tlv(0x0016, 0x010A); // ID Number
                socket()->writeBuffer().tlv(0x0017, 0x0014); // major
                socket()->writeBuffer().tlv(0x0018, 0x0034); // minor
                socket()->writeBuffer().tlv(0x0019, 0x0009);
                socket()->writeBuffer().tlv(0x001A, 0x0c18);
                socket()->writeBuffer().tlv(0x0014, 0x0000043dL);
                socket()->writeBuffer().tlv(0x000f, "en");
                socket()->writeBuffer().tlv(0x000e, "us");
	        }else{
                socket()->writeBuffer().tlv(0x0003, "AOL Instant Messenger, version 5.1.3036/WIN32"); //ToDo: Should be updated anytime
                socket()->writeBuffer().tlv(0x0016, (unsigned short)0x0109);
                socket()->writeBuffer().tlv(0x0017, (unsigned short)0x0005);
                socket()->writeBuffer().tlv(0x0018, (unsigned short)0x0001);
                socket()->writeBuffer().tlv(0x0019, (unsigned short)0x0000);
                socket()->writeBuffer().tlv(0x001A, (unsigned short)0x0BDC);
                socket()->writeBuffer().tlv(0x0014, 0x000000D2L);
                socket()->writeBuffer().tlv(0x000F, "en");		//Todo Send right language shortcut ;) same below
                socket()->writeBuffer().tlv(0x000E, "us");
                socket()->writeBuffer().tlv(0x004A, "\x01");
	        }
            sendPacket(true);
        }
        break;
    case ICQ_SNACxLOGIN_LOGINxREPLY:
        chn_close();
        break;
    case ICQ_SNACxLOGIN_REGISTERxSEND_IMG: {
        m_bVerifying = false;
        TlvList tlv(socket()->readBuffer());
        // currently there are 2 TLVs in SNAC(17,0D):
        // type = 1: the value contains the mime type of the image (image/jpeg); ignored
        // type = 2: the value contains the image itself in the binary form
        Tlv* tlvImage = tlv(2);
        if (!tlvImage)
            break;
        log(L_DEBUG, "Image length: %d bytes", tlvImage->Size());
        QByteArray buf = tlvImage->byteArray();
        QPixmap pict;
        if (!pict.loadFromData(buf))
            break;
        log(L_DEBUG, "Received verification image");
        VerifyDlg verdlg(qApp->activeWindow(), pict);
        if (verdlg.exec() == QDialog::Accepted) // what to do if the user has cancelled the dialog?
        {
            QString verifyStr = verdlg.getVerifyString();
            log(L_DEBUG, "User input: %s", qPrintable(verifyStr));
            snac(ICQ_SNACxFOOD_LOGIN, ICQ_SNACxLOGIN_REGISTERxREQ);
            ICQBuffer msg;
            msg
            << 0x00000000L << 0x28000300L << 0x00000000L
            << 0x00000000L << 0x94680000L << 0x94680000L
            << 0x00000000L << 0x00000000L << 0x00000000L
            << 0x00000000L;
            QByteArray pswd = getContacts()->fromUnicode(NULL, getPassword());
            unsigned short len = (unsigned short)(pswd.length() + 1);
            msg.pack(len);
            msg.pack(pswd.data(), len);
            msg << 0x94680000L << 0x00000602L;
            socket()->writeBuffer().tlv(0x0001, msg);
            socket()->writeBuffer().tlv(0x0009, verifyStr.toLatin1(), verifyStr.length());
            sendPacket(true);            
        }
        break;
        }
    default:
        log(L_WARN, "Unknown login foodgroup type %04X", type);
    }
}

void ICQClient::chn_login()
{
	m_bconnectionLost = false;
    if (m_cookie.size()){
        flap(ICQ_CHNxNEW);
        socket()->writeBuffer() << 0x00000001L;
        socket()->writeBuffer().tlv(6, m_cookie.data(), (unsigned short)(m_cookie.size()));
        m_cookie.resize(0);
        sendPacket(true);
        return;
    }
    if (data.owner.Uin.toULong() && ! getUseMD5()){
	QByteArray pswd = cryptPassword();
        log(L_DEBUG, "Login %lu [%s]", data.owner.Uin.toULong(), /*pswd.c_str()*/"");
        char uin[20];
        sprintf(uin, "%lu", data.owner.Uin.toULong());

        flap(ICQ_CHNxNEW);
        socket()->writeBuffer() << 0x00000001L;
        socket()->writeBuffer().tlv(0x0001, uin);
        socket()->writeBuffer().tlv(0x0002, pswd.data(), pswd.size());
		// Thanks to pidgin guys for those values
        socket()->writeBuffer().tlv(0x0003, "ICQBasic");  // ID String, currently ICQ 5.1 (21.08.2006)
        socket()->writeBuffer().tlv(0x0016, 0x010A); // ID Number
        socket()->writeBuffer().tlv(0x0017, 0x0014); // major
        socket()->writeBuffer().tlv(0x0018, 0x0034); // minor
        socket()->writeBuffer().tlv(0x0019, 0x0000);      // lesser
        socket()->writeBuffer().tlv(0x001A, 0x0c18);      // build number
        socket()->writeBuffer().tlv(0x0014, 0x0000043dL); // distribution number
        socket()->writeBuffer().tlv(0x000f, "en");		//Todo Send right language shortcut
        socket()->writeBuffer().tlv(0x000e, "us");
        sendPacket(true);
        return;
    }
    if (!data.owner.Screen.str().isEmpty() || getUseMD5()){
        log(L_DEBUG, "Requesting MD5 salt");
        flap(ICQ_CHNxNEW);
        socket()->writeBuffer() << 0x00000001L;
        sendPacket(true);
        snac(ICQ_SNACxFOOD_LOGIN, ICQ_SNACxLOGIN_AUTHxREQUEST, false, false);
        if (data.owner.Uin.toULong())
		{
            QString uin = QString::number(data.owner.Uin.toULong());
            socket()->writeBuffer().tlv(0x0001, uin.toUtf8().data());
        }else{
            socket()->writeBuffer().tlv(0x0001, data.owner.Screen.str().toUtf8().data());
        }
        socket()->writeBuffer().tlv(0x004B);
        socket()->writeBuffer().tlv(0x005A);
        sendPacket(true);
        return;
    }
    if (m_bVerifying){
        log(L_DEBUG, "Requesting verification picture");
        flap(ICQ_CHNxNEW);
        socket()->writeBuffer() << 0x00000001L;
        sendPacket(true);
        snac(ICQ_SNACxFOOD_LOGIN, ICQ_SNACxLOGIN_REGISTERxREQ_IMG);
        sendPacket(true);
        return;
    }
    flap(ICQ_CHNxNEW);
    socket()->writeBuffer() << 0x00000001L;
    sendPacket(true);
    // first try the old registration scheme
    snac(ICQ_SNACxFOOD_LOGIN, ICQ_SNACxLOGIN_REGISTERxREQ);
    ICQBuffer msg;
    msg
    << 0x00000000L << 0x28000300L << 0x00000000L
    << 0x00000000L << 0x94680000L << 0x94680000L
    << 0x00000000L << 0x00000000L << 0x00000000L
    << 0x00000000L;
    QByteArray pswd = getContacts()->fromUnicode(NULL, getPassword());
    unsigned short len = (unsigned short)(pswd.length() + 1);
    msg.pack(len);
    msg.pack(pswd.data(), len);
    msg << 0x94680000L << 0x00000602L;
    socket()->writeBuffer().tlv(0x0001, msg);
    sendPacket(true);
}

void ICQClient::chn_close()
{
    unsigned errorCode = 0;
    TlvList tlv(socket()->readBuffer());
    Tlv *tlv_error = tlv(8);
    if (tlv_error){
        unsigned short err = *tlv_error;
        QString errString;
        switch (err){
        case ICQ_LOGIN_ERRxOLDCLIENT1:
        case ICQ_LOGIN_ERRxOLDCLIENT2:
            errString = I18N_NOOP("This client is outdated");
            m_reconnect = NO_RECONNECT;
            break;
        case ICQ_LOGIN_ERRxIP_RATE_LIMIT1:
        case ICQ_LOGIN_ERRxIP_RATE_LIMIT2:
            errString = I18N_NOOP("Too many clients from same IP");
            m_reconnect = NO_RECONNECT;
            break;
        case ICQ_LOGIN_ERRxRATE_LIMIT1:
        case ICQ_LOGIN_ERRxRATE_LIMIT2:
            errString = I18N_NOOP("Rate limit");
            m_reconnect = NO_RECONNECT;
            break;
        case ICQ_LOGIN_ERRxBAD_PASSWD1:
        case ICQ_LOGIN_ERRxBAD_PASSWD2:
        case ICQ_LOGIN_ERRxBAD_PASSWD3:
            errString = I18N_NOOP("Invalid UIN and password combination");
            m_reconnect = NO_RECONNECT;
            errorCode = AuthError;
            break;
        case ICQ_LOGIN_ERRxNOT_EXISTS1:
        case ICQ_LOGIN_ERRxNOT_EXISTS2:
            errString = I18N_NOOP("Non-existant UIN");
            m_reconnect = NO_RECONNECT;
            errorCode = AuthError;
            break;
        case ICQ_LOGIN_ERRxBAD_LOGIN:
            errString = I18N_NOOP("Bad login procedure");
            m_reconnect = NO_RECONNECT;
            break;
        case ICQ_LOGIN_ERRxUNAVAILABLE1:
        case ICQ_LOGIN_ERRxUNAVAILABLE2:
        case ICQ_LOGIN_ERRxUNAVAILABLE3:
        case ICQ_LOGIN_ERRxUNAVAILABLE4:
        case ICQ_LOGIN_ERRxUNAVAILABLE5:
        case ICQ_LOGIN_ERRxUNAVAILABLE6:
        case ICQ_LOGIN_ERRxUNAVAILABLE7:
        case ICQ_LOGIN_ERRxUNAVAILABLE8:
            errString = I18N_NOOP("Service temporarly unavailable");
            m_reconnect = NO_RECONNECT;
            break;
        case ICQ_LOGIN_ERRxINVALID_ID:
            errString = I18N_NOOP("Invalid SecureID");
            m_reconnect = NO_RECONNECT;
            break;
        case ICQ_LOGIN_ERRxTOO_YOUNG:
            errString = I18N_NOOP("Too young!");
            m_reconnect = NO_RECONNECT;
            break;
        case ICQ_LOGIN_ERRxSUSPENDED1:
            errString = I18N_NOOP("UIN was suspended");
            m_reconnect = NO_RECONNECT;
            break;
        case ICQ_LOGIN_ERRxCANT_REGISTER:
            errString = I18N_NOOP("Can't login to ICQ network - Please try again later");
            m_reconnect = NO_RECONNECT;
            break;
        case 0:
            break;
        default:
            errString = "Unknown error ";
            errString += QString::number(err);
        }
        if (err){
            log(L_ERROR, "%s", qPrintable(errString));
            socket()->error_state(errString, errorCode);
            flap(ICQ_CHNxCLOSE);
            sendPacket(true);
            return;
        }
    }
    tlv_error = tlv(9);
    if (tlv_error){
        QString errString;
        unsigned short err = *tlv_error;
        switch (err){
        case 0x1:{
                errString = I18N_NOOP("Your UIN is being used from another location");
                m_reconnect = NO_RECONNECT;
                break;
            }
        case 0:
            break;
        default:
            errString = "Unknown run-time error ";
            errString += QString::number(err);
        }
        if (err){
            log(L_ERROR, "%s", qPrintable(errString));
            socket()->error_state(errString);
            return;
        }
    }
    flap(ICQ_CHNxCLOSE);
    sendPacket(true);

    Tlv *tlv_host = tlv(5);
    Tlv *tlv_cookie = tlv(6);
    if ((tlv_host == NULL) || (tlv_cookie == NULL)){
        socket()->error_state(I18N_NOOP("Close packet from server"));
        return;
    }
    QString host = tlv_host->byteArray().data();
    int idx = host.indexOf(':');
    if (idx == -1){
        log(L_ERROR, "Bad host address %s", qPrintable(host));
        socket()->error_state(I18N_NOOP("Bad host address"));
        return;
    }
    unsigned short port = host.mid(idx + 1).toUShort();
    host = host.left(idx);

    socket()->close();
    socket()->connect(host, port, this);
    m_cookie = tlv_cookie->byteArray();
    m_cookie.resize(m_cookie.size() - 1);   // tlv has \0 terminator... time for Qt4
}

