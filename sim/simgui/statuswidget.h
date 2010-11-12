#ifndef STATUSWIDGET_H
#define STATUSWIDGET_H

#include <QWidget>
#include <QMenu>
#include <QTimer>
#include "simapi.h"

namespace SIM {

class Client;
class EXPORT StatusWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StatusWidget(QWidget *parent = 0);

    virtual Client* client() = 0;

    virtual QMenu* menu() const = 0;

    bool isBlinking() const;

public slots:
    void setBlinking(bool v);

protected:
    bool blinkState() const;
    virtual void contextMenuEvent(QContextMenuEvent* event);

signals:

protected slots:
    void blink();

private:
    bool m_blinking;
    bool m_blinkState;
    QTimer m_blinkTimer;

};

} // namespace SIM

#endif // STATUSWIDGET_H
