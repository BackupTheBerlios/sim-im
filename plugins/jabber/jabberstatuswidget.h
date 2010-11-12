#ifndef JABBERSTATUSWIDGET_H
#define JABBERSTATUSWIDGET_H

#include "jabberclient.h"
#include "simgui/statuswidget.h"

class JabberStatusWidget : public SIM::StatusWidget
{
    Q_OBJECT
public:
    explicit JabberStatusWidget(JabberClient* client, QWidget *parent = 0);

    virtual SIM::Client* client();

    virtual QMenu* menu() const;

    virtual QSize sizeHint() const;

signals:

public slots:

protected:
    virtual void paintEvent(QPaintEvent* event);
    virtual void resizeEvent(QResizeEvent* event);

    void drawBlink();
    void drawStatus();

private:
    JabberClient* m_client;

};

#endif // JABBERSTATUSWIDGET_H
