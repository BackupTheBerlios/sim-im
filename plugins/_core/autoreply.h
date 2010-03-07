/***************************************************************************
                          autoreply.h  -  description
                             -------------------
    begin                : Sun Mar 17 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan@shutoff.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _AUTOREPLY_H
#define _AUTOREPLY_H

#include "contacts/imstatus.h"
#include "ui_autoreplybase.h"

class AutoReplyDialog : public QDialog, public Ui::AutoReplyBase
{
    Q_OBJECT
public:
    AutoReplyDialog(unsigned status);
    AutoReplyDialog(const SIM::IMStatusPtr& status);
    ~AutoReplyDialog();

protected slots:
    void timeout();
    void textChanged();
    void toggled(bool);
    void help();

protected:
    void accept();
    void stopTimer();
    bool loadIconAndIconText(unsigned status, QString &text, QString &icon);

private:
    void initIconMap();
    QString statusIcon(const SIM::IMStatusPtr& status);

    unsigned m_status;
    unsigned m_time;
    QTimer	*m_timer;
    SIM::IMStatusPtr m_imstatus;
    QMap<QString, QString> m_iconmap;
};

#endif

