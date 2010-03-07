#include "mockclient.h"

namespace test
{
    MockClient::MockClient(const QString& n, Protocol* protocol) : Client(protocol, 0),
    m_name(n)
    {
    }

    QString MockClient::name()
    {
        return m_name;
    }

    QString MockClient::dataName(void*)
    {
        return QString();
    }

    QWidget* MockClient::setupWnd()
    {
        return NULL;
    }

    bool MockClient::isMyData(IMContact*& /*data*/, Contact*& /*contact*/)
    {
        return false;
    }

    bool MockClient::createData(IMContact*& /*data*/, Contact* /*contact*/)
    {
        return false;
    }

    void MockClient::contactInfo(void* /*clientData*/, unsigned long& /*status*/, unsigned& /*style*/, QString& /*statusIcon*/, QSet<QString>* /*icons*/)
    {
    }

    void MockClient::setupContact(Contact*, void* /*data*/)
    {
    }

    bool MockClient::send(Message*, void* /*data*/)
    {
        return false;
    }

    bool MockClient::canSend(unsigned /*type*/, void* /*data*/)
    {
        return false;
    }

    QWidget* MockClient::searchWindow(QWidget* /*parent*/)
    {
        return NULL;
    }


}
