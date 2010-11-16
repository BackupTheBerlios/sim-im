#include "oscarsocket.h"
#include "network/tcpasyncsocket.h"
#include "log.h"
#include "misc.h"


using namespace SIM;
OscarSocket::OscarSocket(QObject *parent) :
    QObject(parent)
{
}

OscarSocket::~OscarSocket()
{

}
