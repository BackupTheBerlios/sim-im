#include <QTextDocument>

#include "genericmessage.h"
#include "contacts/client.h"

namespace SIM {

GenericMessage::GenericMessage(const IMContactPtr& contact)
{
    m_contact = contact.toWeakRef();
    m_client = contact->client();
    if(m_client)
    {
        m_originatingClientId = m_client->name();
    }
}

GenericMessage::~GenericMessage()
{
}

QIcon GenericMessage::icon()
{
    return QIcon();
}

Client* GenericMessage::client()
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
