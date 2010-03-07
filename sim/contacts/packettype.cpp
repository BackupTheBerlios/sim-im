
#include "packettype.h"

namespace SIM
{
	PacketType::PacketType(unsigned id, const QString &name, bool bText)
	{
		m_id    = id;
		m_name  = name;
		m_bText = bText;
	}

	PacketType::~PacketType()
	{
	}
}

// vim: set expandtab:

