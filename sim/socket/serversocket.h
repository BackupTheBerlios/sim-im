
#ifndef SIM_SERVERSOCKET_H
#define SIM_SERVERSOCKET_H

#include "simapi.h"

namespace SIM
{
	class ServerSocketNotify;
	class TCPClient;
	class EXPORT ServerSocket
	{
	public:
		ServerSocket();
		virtual ~ServerSocket() {}
		void setNotify(ServerSocketNotify *n) { notify = n; }
		virtual void bind(unsigned short mixPort, unsigned short maxPort, TCPClient *client) = 0;
		virtual void close() = 0;

	protected:
		ServerSocketNotify *notify;
	};
}

#endif

// vim: set expandtab:

