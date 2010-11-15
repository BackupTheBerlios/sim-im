#include <QPainter>
#include <QResizeEvent>

#include "imagestorage/imagestorage.h"
#include "icqstatuswidget.h"
#include "icqclient.h"
#include "log.h"

using namespace SIM;

ICQStatusWidget::ICQStatusWidget(ICQClient* client, QWidget* parent) : SIM::StatusWidget(parent),
    m_client(client)
{
}

SIM::Client* ICQStatusWidget::client()
{
    return m_client;
}

QMenu* ICQStatusWidget::menu() const
{
    SIM::IMStatusPtr status = m_client->currentStatus();
    if(!status)
        return 0;
    QAction* a;
    QMenu* m = new QMenu();

    a = m->addAction(getImageStorage()->icon("ICQ_online"), I18N_NOOP("Online"), this, SLOT(online()));
    a->setCheckable(true);
    a->setChecked(status->id() == "online");

    a = m->addAction(getImageStorage()->icon("ICQ_offline"), I18N_NOOP("Offline"), this, SLOT(offline()));
    a->setCheckable(true);
    a->setChecked(status->id() == "offline");

    return m;
}

void ICQStatusWidget::resizeEvent(QResizeEvent* event)
{
    if(event->size().width() != event->size().height())
        resize(QSize(event->size().height(), event->size().height()));
    SIM::StatusWidget::resizeEvent(event);
}

void ICQStatusWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    if(isBlinking())
        drawBlink();
    else
        drawStatus();
}

void ICQStatusWidget::drawBlink()
{
    QPixmap p = getImageStorage()->pixmap(blinkState() ? "ICQ_online" : "ICQ_offline");
    QPainter painter(this);
    painter.drawPixmap(rect(), p);
}

void ICQStatusWidget::drawStatus()
{
    SIM::IMStatusPtr status = m_client->currentStatus();
    if(!status)
        return;
    QPainter painter(this);
    painter.drawPixmap(rect(), status->icon());
}

QSize ICQStatusWidget::sizeHint() const
{
    SIM::IMStatusPtr status = m_client->currentStatus();
    if(!status)
        return QSize();
    return status->icon().size();
}

void ICQStatusWidget::online()
{
    client()->changeStatus(m_client->getDefaultStatus("online"));
}

void ICQStatusWidget::offline()
{
    client()->changeStatus(m_client->getDefaultStatus("offline"));
}
