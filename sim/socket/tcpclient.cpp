
#include <QTimer>

#include "tcpclient.h"

#include "buffer.h"
#include "contacts.h"
#include "log.h"
#include "misc.h"

#include "clientsocket.h"
#include "socket.h"
#include "socketfactory.h"

namespace SIM
{
	const unsigned RECONNECT_TIME = 5;
	const unsigned RECONNECT_IFINACTIVE = 60;
	const unsigned LOGIN_TIMEOUT = 120;

	TCPClient::TCPClient(Protocol *protocol, Buffer *cfg, unsigned priority)
		: Client(protocol, cfg)
		  , EventReceiver(priority)
		  , m_reconnect(RECONNECT_TIME)
		  //, m_logonStatus(protocol->status("offline"))
		  , m_logonStatus(STATUS_OFFLINE)
		  , m_ip(0)
		  , m_timer(new QTimer(this))
		  , m_loginTimer(new QTimer(this))
		  , m_bWaitReconnect(false)
		  , m_clientSocket(NULL)
	{
		connect(m_timer, SIGNAL(timeout()), this, SLOT(reconnect()));
		connect(m_loginTimer, SIGNAL(timeout()), this, SLOT(loginTimeout()));
	}

	bool TCPClient::processEvent(Event *e)
	{
		if (e->type() == eEventSocketActive)
		{
			EventSocketActive *s = static_cast<EventSocketActive*>(e);
			if (m_bWaitReconnect && s->active())
				reconnect();
		}
		return false;
	}

        void TCPClient::resolve_ready(QHostAddress ip)
	{
                m_ip = ip.toIPv4Address();
	}

	bool TCPClient::error_state(const QString &err, unsigned code)
	{
		log(L_DEBUG, "Socket error %s (%u)", qPrintable(err), code);
		m_loginTimer->stop();
		if (m_reconnect == NO_RECONNECT){
			m_timer->stop();
			//setStatus(STATUS_OFFLINE, getCommonStatus());
			setState(Error, err, code);
			return false;
		}
		if (!m_timer->isActive()){
			unsigned reconnectTime = m_reconnect;
			if (!getSocketFactory()->isActive()){
				if (reconnectTime < RECONNECT_IFINACTIVE)
					reconnectTime = RECONNECT_IFINACTIVE;
			}
			//setClientStatus(protocol()->status("offline"));
			setClientStatus(STATUS_OFFLINE);
			setState((m_reconnect == NO_RECONNECT) ? Error : Connecting, err, code);
			m_bWaitReconnect = true;
			log(L_DEBUG, "Wait reconnect %u sec", reconnectTime);
			m_timer->start(reconnectTime * 1000);
		} else {
			/*
			   slot reconnect() neeeds this flag 
			   to be true to make actual reconnect,
			   but it was somehow false. serzh.
			   */
			m_bWaitReconnect = true;
		}
		return false;
	}

	void TCPClient::reconnect()
	{
		m_timer->stop();
		if (m_bWaitReconnect)
			setClientStatus(getManualStatus());
	}

    void TCPClient::changeStatus(const IMStatusPtr& status)
    {
        Client::changeStatus(status);
    }

	SIM_DEPRECATED void TCPClient::setStatus(unsigned status, bool bCommon)
	{
		setClientStatus(status);
		Client::setStatus(status, bCommon);
	}

	void TCPClient::connect_ready()
	{
		m_timer->stop();
		m_bWaitReconnect = false;
		m_loginTimer->stop();
		m_loginTimer->setSingleShot(true);
		m_loginTimer->start(LOGIN_TIMEOUT * 1000);
	}

	void TCPClient::loginTimeout()
	{
		m_loginTimer->stop();
		if ((getState() != Connected) && socket())
			socket()->error_state(I18N_NOOP("Login timeout"));
	}

	Socket *TCPClient::createSocket()
	{
		return NULL;
	}

	void TCPClient::socketConnect()
	{
		if (socket())
			socket()->close();
		if (socket() == NULL)
			m_clientSocket = createClientSocket();
		log(L_DEBUG, "Start connect %s:%u", qPrintable(getServer()), getPort());
		socket()->connect(getServer(), getPort(), this);
	}

	ClientSocket *TCPClient::createClientSocket()
	{
		return new ClientSocket(this, createSocket());
	}

    void TCPClient::setClientStatus(const IMStatusPtr& status)
    {
        if (status->id() != "offline") {
            if (getState() == Connected) {
                changeStatus(status);
                return;
            }
            //m_logonStatus = status;
            if ((getState() != Connecting) || m_bWaitReconnect){
                setState(Connecting, NULL);
                m_reconnect = RECONNECT_TIME;
                m_bWaitReconnect = false;
                setState(Connecting);
                socketConnect();
            }
            return;
        }
        m_bWaitReconnect = false;
        m_timer->stop();
        m_loginTimer->stop();
        changeStatus(status);
        setState(Offline);
        disconnected();
        if (socket()){
            socket()->close();
            delete socket();
            m_clientSocket = NULL;
        }
    }

	void TCPClient::setClientStatus(unsigned status)
	{
        if (status != STATUS_OFFLINE) {
            if (getState() == Connected) {
				setStatus(status);
				return;
			}
			m_logonStatus = status;
			if ((getState() != Connecting) || m_bWaitReconnect){
				setState(Connecting, NULL);
				m_reconnect = RECONNECT_TIME;
				m_bWaitReconnect = false;
				setState(Connecting);
				socketConnect();
			}
			return;
		}
		m_bWaitReconnect = false;
		m_timer->stop();
		m_loginTimer->stop();
		if (socket())
			setStatus(STATUS_OFFLINE);
		m_status = STATUS_OFFLINE;
		setState(Offline);
		disconnected();
		if (socket()){
			socket()->close();
			delete socket();
			m_clientSocket = NULL;
		}
	}

}

// vim: set expandtab:

