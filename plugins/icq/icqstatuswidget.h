#ifndef ICQSTATUSWIDGET_H
#define ICQSTATUSWIDGET_H

#include "simgui/statuswidget.h"
#include <QButtonGroup>

class ICQClient;
class ICQStatusWidget : public SIM::StatusWidget
{
    Q_OBJECT
public:
    explicit ICQStatusWidget(ICQClient* client, QWidget* parent = 0);

    virtual SIM::Client* client();

    virtual QMenu* menu() const;

    virtual QSize sizeHint() const;

protected slots:
    void online();
    void offline();

protected:
    virtual void paintEvent(QPaintEvent* event);
    virtual void resizeEvent(QResizeEvent* event);

    void drawBlink();
    void drawStatus();

private:
    ICQClient* m_client;
};

#endif // ICQSTATUSWIDGET_H
