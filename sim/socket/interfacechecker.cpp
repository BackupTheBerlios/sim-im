
#include <QObject>
#include <QTimerEvent>

#ifdef WIN32
	#include <winsock.h>
#else
#ifndef Q_OS_MAC
	#include <net/if.h>
#endif
	#include <sys/ioctl.h>
	#include <sys/socket.h>
	#include <sys/time.h>
	#include <netinet/in.h>
	#include <netdb.h>
	#include <arpa/inet.h>
#endif

#include "event.h"
#include "interfacechecker.h"
#include "log.h"

namespace SIM
{
    InterfaceChecker::InterfaceChecker(int polltime, bool raiseevents) : QObject(), m_pollTime(polltime), m_raiseEvents(raiseevents)
    {
        m_timerID = startTimer(polltime);
        m_testSocket = socket(PF_INET, SOCK_STREAM, 0);
    }

    InterfaceChecker::~InterfaceChecker()
    {
        killTimer(m_timerID);
#ifdef WIN32
        closesocket(m_testSocket);
#else
        close(m_testSocket);
#endif
    }

    void InterfaceChecker::setPollTime(int polltime)
    {
        killTimer(m_timerID);
        m_timerID = startTimer(polltime);
        m_pollTime = polltime;
    }

    void InterfaceChecker::timerEvent(QTimerEvent* ev)
    {
        if(ev->timerId() != m_timerID)
            return;
#if !defined(WIN32) && !defined(Q_OS_MAC)
        if(m_testSocket == -1)
        {
            log(L_DEBUG, "testsocket == -1");
            // TODO try to reinitialize test socket
            return;
        }
        struct ifreq ifr;
        struct ifreq* ifrp;
        struct ifreq ibuf[16];
        struct ifconf ifc;

        ifc.ifc_len = sizeof(ifr)*16;
        ifc.ifc_buf = (caddr_t)&ibuf;
        memset(ibuf, 0, sizeof(struct ifreq)*16);

        int hret = ioctl(m_testSocket, SIOCGIFCONF, &ifc);
        if(hret == -1)
        {
            log(L_DEBUG, "hret == -1");
            return;
        }

        for(std::map<std::string, tIFState>::iterator it = m_states.begin(); it != m_states.end(); ++it)
        {
            it->second.present = false;
        }

        for(unsigned int i = 0; i < ifc.ifc_len/sizeof(struct ifreq); i++)
        {
            ifrp = ibuf + i; 
            strncpy(ifr.ifr_name, ifrp->ifr_name, sizeof(ifr.ifr_name));

            if(strcmp(ifr.ifr_name, "lo") == 0 )
                continue;

            std::map<std::string, tIFState>::iterator it = m_states.find(ifr.ifr_name);
            if(it == m_states.end())
            {
                // New interface
                tIFState s = {true, true};
                m_states[ifr.ifr_name] = s;
                emit interfaceUp(QString(ifr.ifr_name));
                log(L_DEBUG, "%s: appeared", ifr.ifr_name);
            }

            m_states[ifr.ifr_name].present = true;
            hret = ioctl(m_testSocket, SIOCGIFFLAGS, &ifr);
            if(hret != -1)
            {
                int state = ifr.ifr_flags & IFF_RUNNING;
                if(state < 0)
                {
                    log(L_DEBUG, "Incorrect state: %d (%s)", state, ifr.ifr_name);
                    return;
                }
                if((state == 0) && (m_states[ifr.ifr_name].state))
                {
                    m_states[ifr.ifr_name].state = false;
                    emit interfaceDown(QString(ifr.ifr_name));
                    if(m_raiseEvents)
                    {
                        EventInterfaceDown e(-1);
                        e.process();
                    }
                    return;
                }
                if((state != 0) && (!m_states[ifr.ifr_name].state))
                {
                    m_states[ifr.ifr_name].state = true;
                    emit interfaceUp(QString(ifr.ifr_name));
                    if(m_raiseEvents)
                    {
                        EventInterfaceUp e(-1);
                        e.process();
                    }
                    return;
                }
                return;
            }
        }

        for(std::map<std::string, tIFState>::iterator it = m_states.begin(); it != m_states.end(); ++it)
        {
            if(it->second.present == false)
            {
                log(L_DEBUG, "%s: disappeared", it->first.c_str());
                it->second.state = false;
                emit interfaceDown(QString(it->first.c_str()));
                if(m_raiseEvents)
                {
                    EventInterfaceDown e(-1);
                    e.process();
                }
                // TODO make it work with more than one disappeared interface
                m_states.erase(it);
                return;
            }
        }
#else
        return;
#endif
    }
}


// vim: set expandtab:

