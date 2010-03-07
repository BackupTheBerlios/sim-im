#ifndef SIM_SSLCLIENT_H
#define SIM_SSLCLIENT_H

#include "buffer.h"
#include "socketnotify.h"
#include "socket.h"

namespace SIM
{
	class EXPORT SSLClient : public SocketNotify, public Socket
	{
	public:
		SSLClient(Socket*);
		~SSLClient();
		virtual int read(char *buf, unsigned int size);
		virtual void write(const char *buf, unsigned int size);
		virtual void connect(const QString &host, unsigned short port);
		virtual void close();
		virtual unsigned long localHost();
		virtual void pause(unsigned);
                virtual bool isEncrypted();
                virtual bool startEncryption();
                bool connected() const { return false; }
		Socket *socket() const { return sock; }
		void setSocket(Socket *s);
/*
		bool init();
		void accept();
		void shutdown();
		void process(bool bInRead=false, bool bWantRead=false);
		void write();
*/
	protected:
		virtual void connect_ready();
		virtual void read_ready();
		virtual void write_ready();
		virtual void error_state(const QString &err, unsigned code = 0);
		Socket *sock;
	};

}

#endif

// vim: set expandtab:

