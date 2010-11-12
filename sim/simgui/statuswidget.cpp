#include "statuswidget.h"

#include <QContextMenuEvent>

namespace SIM {

StatusWidget::StatusWidget(QWidget *parent) :
    QWidget(parent), m_blinking(false), m_blinkState(false)
{
    connect(&m_blinkTimer, SIGNAL(timeout()), this, SLOT(blink()));
    m_blinkTimer.setInterval(500); // FIXME hardcoded
}

void StatusWidget::setBlinking(bool v)
{
    m_blinking = v;
    if(v)
        m_blinkTimer.start();
    else
        m_blinkTimer.stop();
}

bool StatusWidget::isBlinking() const
{
    return m_blinking;
}

bool StatusWidget::blinkState() const
{
    return m_blinkState;
}

void StatusWidget::blink()
{
    m_blinkState = !m_blinkState;
    update();
}

void StatusWidget::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu* m = menu();
    if(!m)
        return;
    m->popup(event->globalPos());
    QWidget::contextMenuEvent(event);
}

} // namespace SIM
