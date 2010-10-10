#include <QTextDocument>

#include "genericmessage.h"
#include "contacts/client.h"

namespace SIM {

GenericMessage::GenericMessage(const IMContactPtr& contact)
{
    m_contact = contact.toWeakRef();
    m_client = contact->client();
    ClientPtr cl = m_client.toStrongRef();
    if(cl)
    {
        m_originatingClientId = cl->name();
    }
}

GenericMessage::~GenericMessage()
{
}

QIcon GenericMessage::icon()
{
    return QIcon();
}

ClientWeakPtr GenericMessage::client()
{
    return m_client;
}

IMContactWeakPtr GenericMessage::contact() const
{
    return m_contact;
}

QDateTime GenericMessage::timestamp()
{
    return QDateTime();
}

QString GenericMessage::toHtml()
{
    return m_text;
}

QString GenericMessage::toPlainText()
{
    QTextDocument doc;
    doc.setHtml(m_text);
    return doc.toPlainText();
}

QString GenericMessage::originatingClientId() const
{
    return m_originatingClientId;
}

QStringList GenericMessage::choices()
{
    return QStringList();
}

void GenericMessage::setHtml(const QString& html)
{
    m_text = html;
}

} // namespace SIM
