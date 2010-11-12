#include "jabberstatuswidget.h"

#include <QPaintEvent>
#include <QResizeEvent>
#include <QPainter>

JabberStatusWidget::JabberStatusWidget(JabberClient* client, QWidget *parent) :
        SIM::StatusWidget(parent), m_client(client)
{
}

SIM::Client* JabberStatusWidget::client()
{
    return m_client;
}

QMenu* JabberStatusWidget::menu() const
{
    return 0;
}

void JabberStatusWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    if(isBlinking())
        drawBlink();
    else
        drawStatus();
}

void JabberStatusWidget::resizeEvent(QResizeEvent* event)
{
    if(event->size().width() != event->size().height())
        resize(QSize(event->size().height(), event->size().height()));
    SIM::StatusWidget::resizeEvent(event);
}


void JabberStatusWidget::drawBlink()
{

}

void JabberStatusWidget::drawStatus()
{
    SIM::IMStatusPtr status = m_client->currentStatus();
    if(!status)
        return;
    QPainter painter(this);
    painter.drawPixmap(rect(), status->icon());
}

QSize JabberStatusWidget::sizeHint() const
{
    SIM::IMStatusPtr status = m_client->currentStatus();
    if(!status)
        return QSize();
    return status->icon().size();
}
