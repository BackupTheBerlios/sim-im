/***************************************************************************
                          filetransfer.cpp  -  description
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

#include "simapi.h"

#include <time.h>

#include <QPixmap>
#include <QLineEdit>
#include <QSlider>
#include <QLabel>
#include <QTimer>
#include <QPushButton>
#include <QCheckBox>
#include <QFile>
#include <QDir>
#include <QRegExp>

#include "simgui/ballonmsg.h"
#include "icons.h"
#include "unquot.h"
#include "contacts/contact.h"

#include "filetransfer.h"
#include "core.h"

using namespace SIM;

const unsigned MAX_AVERAGE	= 40;
const unsigned SHOW_AVERAGE	= 5;

class FileTransferDlgNotify : public FileTransferNotify
{
public:
    FileTransferDlgNotify(FileTransferDlg *dlg);
    ~FileTransferDlgNotify();
    void skip();
    void replace();
    void resume();
protected:
    void process();
    void transfer(bool);
    void createFile(const QString &name, unsigned size, bool bCanResume);
    QString m_name;
	QString m_fn;
    unsigned m_size;
    FileTransferDlg *m_dlg;
};

FileTransferDlgNotify::FileTransferDlgNotify(FileTransferDlg *dlg)
    :   m_dlg (dlg)
{
}

FileTransferDlgNotify::~FileTransferDlgNotify()
{
    m_dlg->notifyDestroyed();
}

void FileTransferDlgNotify::process()
{
    m_dlg->process();
}

void FileTransferDlgNotify::transfer(bool bState)
{
    m_dlg->transfer(bState);
}

void FileTransferDlgNotify::createFile(const QString &name, unsigned size, bool bCanResume)
{
    m_name = name;
    m_size = size;
    m_name = m_name.replace('\\', '/');


    FileTransfer *ft = m_dlg->m_msg->m_transfer;
    int n = m_name.lastIndexOf('/');
    if (n >= 0)
    {
        QString path;
		QString p(m_name.left(n));
		m_fn = m_name.right(m_name.length()-n);
		m_fn = m_fn.replace(QRegExp("/"), "");

        while (!p.isEmpty())
        {
            QString pp = getToken(p, '/');
            if (!path.isEmpty())
                path += '/';
            if (pp == "..")
            {
                QString errMsg = i18n("Bad path: %1") .arg(m_name);
                m_dlg->m_msg->setError(errMsg);
                ft->setError();
                return;
            }
            path += pp;
            QDir dd(ft->dir() /* + '/' + path */);
            QDir d(ft->dir());
            if (dd.exists() || d.mkdir(path))
                continue;

            QString errMsg = i18n("Can't create: %1") .arg(path);
            m_dlg->m_msg->setError(errMsg);
            ft->setError();
            return;
        }
    }
    m_dlg->m_msg->addFile(m_name, size);
    if (m_name.isEmpty() || m_name[(int)(m_name.length() - 1)] == '/')
    {
        ft->startReceive(0);
        return;
    }

	QString shortName = m_name;
	//m_name = ft->dir() + m_name; Quickfix, noragen
	if (m_fn.isEmpty())
		m_fn=m_name;

	m_name = ft->dir() + m_fn; 

    if (ft->m_file)
        delete ft->m_file;
    m_dlg->process();
    ft->m_file = new QFile(m_name);
    if (!ft->m_file->exists())
    {
        if (ft->m_file->open(QIODevice::WriteOnly))
        {
            ft->startReceive(0);
            return;
        }
    }
    else switch (ft->overwrite())
    {
    case Skip:
        skip();
        return;
    case Replace:
        if (ft->m_file->open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            ft->startReceive(0);
            return;
        }
        break;
    case Resume:
        if (ft->m_file->open(QIODevice::WriteOnly | QIODevice::Append))
        {
            resume();
            return;
        }
        break;
    default:
        if (ft->m_file->open(QIODevice::WriteOnly | QIODevice::Append))
        {
            QStringList buttons;
            QString forAll;
            if (ft->files())
                forAll = i18n("For all files");
            buttons.append(i18n("&Replace"));
            buttons.append(i18n("&Skip"));
            if (bCanResume && (ft->m_file->size() < size))
                buttons.append(i18n("Resu&me"));
            m_dlg->m_ask = new BalloonMsg(NULL, quoteString(i18n("File %1 exists") .arg(shortName)),
                buttons, m_dlg->lblState, NULL, false, true, 150, forAll);
            QObject::connect(m_dlg->m_ask, SIGNAL(action(int, void*)), m_dlg, SLOT(action(int, void*)));
            raiseWindow(m_dlg);
            m_dlg->m_ask->show();
            return;
        }
    }
    QString errMsg = i18n("Can't create: %1") .arg(m_name);
    m_dlg->m_msg->setError(errMsg);
    ft->setError();
}

void FileTransferDlgNotify::skip()
{
    FileTransfer *ft = m_dlg->m_msg->m_transfer;
    delete ft->m_file;
    ft->m_file = NULL;
    ft->startReceive(NO_FILE);
}

void FileTransferDlgNotify::replace()
{
    FileTransfer *ft = m_dlg->m_msg->m_transfer;
    ft->m_file->close();
    ft->m_file->open(QIODevice::WriteOnly | QIODevice::Truncate);
    ft->startReceive(0);
}

void FileTransferDlgNotify::resume()
{
    FileTransfer *ft = m_dlg->m_msg->m_transfer;
    if (ft->m_file->size() < m_size)
    {
        ft->m_file->seek(ft->m_file->size());
        ft->startReceive(ft->m_file->size());
        return;
    }
    delete ft->m_file;
    ft->m_file = NULL;
    ft->startReceive(NO_FILE);
    return;
}

FileTransferDlg::FileTransferDlg(FileMessage *msg) 
    : QDialog       (NULL)
    , m_msg         (msg)
    , m_bTransfer   (false)
    , m_file        (0)
    , m_transferTime(0)
    , m_displayTime (0)
    , m_speed       (0)
    , m_nAverage    (0)
    , m_files		(0)
    , m_bytes		(0)
    , m_fileSize	(0)
    , m_totalBytes  (0)
    , m_totalSize	(0)
    , m_time        (0)
    , m_timer       (new QTimer(this))
    , m_state       (FileTransfer::Unknown)
    , m_dir (msg->getFlags() & MESSAGE_RECEIVED ? m_msg->m_transfer->dir() : QString() )
{
    setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setWindowIcon(Icon("file"));
    setButtonsPict(this);
    QString name;
    Contact *contact = getContacts()->contact(m_msg->contact());
    if (contact)
    {
        name = contact->getName();
        name = getToken(name, '/');
    }
    setWindowTitle((msg->getFlags() & MESSAGE_RECEIVED) ?
                   i18n("Receive file from %1") .arg(name) :
                   i18n("Send file to %1") .arg(name));
    
    disableWidget(edtTime);
    disableWidget(edtEstimated);
    disableWidget(edtSpeed);
    btnGo->hide();
    btnGo->setIcon(Icon("file"));
    msg->m_transfer->setNotify(new FileTransferDlgNotify(this));
    sldSpeed->setValue(m_msg->m_transfer->speed());
    connect(sldSpeed, SIGNAL(valueChanged(int)), this, SLOT(speedChanged(int)));

    connect(m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
    m_timer->start(1000);
    printTime();

    connect(btnCancel, SIGNAL(clicked()), this, SLOT(close()));
    chkClose->setChecked(CorePlugin::instance()->value("CloseTransfer").toBool());
    connect(chkClose, SIGNAL(toggled(bool)), this, SLOT(closeToggled(bool)));
    connect(btnGo, SIGNAL(clicked()), this, SLOT(goDir()));
}

FileTransferDlg::~FileTransferDlg()
{
    if (m_msg == NULL)
        return;
    if (m_msg->m_transfer)
        m_msg->m_transfer->setNotify(NULL);
    EventMessageCancel(m_msg).process();
}

void FileTransferDlg::process()
{
    if (m_msg->m_transfer == NULL)
        return;
    if (m_msg->m_transfer->state() != m_state || m_msg->m_transfer->file() != m_file)
    {
        m_state = m_msg->m_transfer->state();
        m_file  = m_msg->m_transfer->file();
        QString status;
        switch (m_state)
        {
        case FileTransfer::Listen:
            status = i18n("Listen");
            break;
        case FileTransfer::Connect:
            status = i18n("Connect");
            break;
        case FileTransfer::Negotiation:
            status = i18n("Negotiation");
            break;
        case FileTransfer::Read:
            {
				m_fn = m_msg->getDescription();
                //const QString *n = it[m_file];
                //if (n)
                //    m_fn = *n;
				status = i18n(QString("Receiving file: %1").arg(m_fn));
                break;
            }
        case FileTransfer::Write:
            m_fn = m_msg->m_transfer->filename();
			status = i18n("Sending file: %1").arg(m_fn);
            break;
        case FileTransfer::Done:
            status = i18n(QString("Transfer done: %1").arg(m_fn));
            edtEstimated->setText("0:00:00");
            if (!m_dir.isEmpty())
                btnGo->show();
            break;
        case FileTransfer::Error:
            if (!m_msg->getError().isEmpty())
                status = i18n(m_msg->getError());
            break;
        default:
            break;
        }
        if (!m_fn.isEmpty())
        {
            status += ' ';
            status += QDir::convertSeparators(m_fn);
            if (m_files > 1)
                status += QString(" %1/%2")
                          .arg(m_file + 1)
                          .arg(m_msg->m_transfer->files());
        }
        lblState->setText(status);
        setBars();
    }
    calcSpeed(false);
    if ((int)m_msg->m_transfer->speed() != sldSpeed->value())
        sldSpeed->setValue(m_msg->m_transfer->speed());
    
    if (m_msg->m_transfer->files() == m_files)
        return;

    m_files = m_msg->m_transfer->files();
    if (m_files <= 1)
    {
        if (barFile->isVisible())
            barFile->hide();
    }
    else if (!barFile->isVisible())
        barFile->show();
}

void FileTransferDlg::transfer(bool bState)
{
    bool bTransfer = m_bTransfer;
    m_bTransfer = bState;
    if (bState && m_msg->m_transfer)
    {
        m_transferBytes = m_msg->m_transfer->transferBytes();
        m_transferTime  = time(NULL);
    }
    if (!m_bTransfer && bTransfer)
        calcSpeed(true);
}

void FileTransferDlg::notifyDestroyed()
{
    sldSpeed->hide();
    m_timer->stop();
    btnCancel->setText(i18n("&Close"));
    if (m_state == FileTransfer::Done)
    {
        // EventSent shouldn't be processed here
		// Protocol plugin should decide itself when to send it
        //EventSent(m_msg).process();
        if (chkClose->isChecked())
            close();
        return;
    }
    if (!m_msg->getError().isEmpty())
        lblState->setText(i18n(m_msg->getError()));
    else
        lblState->setText(i18n("Transfer failed"));
}

void FileTransferDlg::speedChanged(int value)
{
    if (m_msg->m_transfer)
        m_msg->m_transfer->setSpeed(value);
}

void FileTransferDlg::timeout()
{
    m_time++;
    printTime();
    calcSpeed(false);
    setBars();
}

void FileTransferDlg::setBars()
{
    if (m_msg->m_transfer == NULL)
        return;
    if (m_totalBytes != m_msg->m_transfer->totalBytes() 
     || m_totalSize != m_msg->m_transfer->totalSize())
    {
        m_totalBytes = m_msg->m_transfer->totalBytes();
        m_totalSize  = m_msg->m_transfer->totalSize();
        setProgress(barTotal, m_totalBytes, m_totalSize);
    }
    if (m_files <= 1 || 
         m_fileSize == m_msg->m_transfer->fileSize() && 
         m_bytes == m_msg->m_transfer->bytes() )
        return;

    m_fileSize = m_msg->m_transfer->fileSize();
    m_bytes = m_msg->m_transfer->bytes();
    setProgress(barFile, m_bytes, m_fileSize);
}

void FileTransferDlg::setProgress(QProgressBar *bar, unsigned bytes, unsigned size)
{
    while (size > 0x1000000)
    {
        size  = size  >> 1;
        bytes = bytes >> 1;
    }
    if (size == 0)
    {
        bar->setValue(0);
        return;
    }
    bar->setValue(bytes * 100 / size);
}

void FileTransferDlg::calcSpeed(bool bTransfer)
{
    if (!m_bTransfer && !bTransfer)
        return;
    time_t now = time(NULL);
    if ((unsigned)now == m_transferTime && !bTransfer)
        return;
    if (m_nAverage < MAX_AVERAGE)
        m_nAverage++;
    m_speed = (m_speed * (m_nAverage - 1) + m_msg->m_transfer->transferBytes() - m_transferBytes) / m_nAverage;
    if ((unsigned)now == m_displayTime)
        return;
    m_transferBytes = m_msg->m_transfer->transferBytes();
    m_transferTime  = now;
    m_displayTime   = now;
    unsigned n_speed = 0;
    double speed = m_speed;
    if (speed >= 1024)
    {
        speed = speed / 1024;
        n_speed++;
    }
    if (m_nAverage < SHOW_AVERAGE)
        return;
    if (speed == 0)
    {
        edtEstimated->setText("");
        edtSpeed->setText(i18n("Stalled"));
        return;
    }
    QString speedText;
    if (speed >= 100)
        speedText = QString::number((unsigned)speed);
    else
        speedText = QString::number(speed, 'f', 3);
    speedText += ' ';
    switch (n_speed)
    {
    case 1:
        speedText += i18n("Kb/s");
        break;
    case 2:
        speedText += i18n("Mb/s");
        break;
    default:
        speedText += i18n("B/s");
    }
    if (edtSpeed->text() != speedText)
        edtSpeed->setText(speedText);
    unsigned estimate = (m_msg->m_transfer->totalSize() - m_msg->m_transfer->totalBytes()) / m_speed;
    unsigned m = estimate / 60;
    unsigned h = m / 60;
    m = m % 60;
    char b[64];
    sprintf(b, "%u:%02u:%02u", h, m, estimate % 60);
    edtEstimated->setText(b);
}

void FileTransferDlg::printTime()
{
    unsigned m = m_time / 60;
    unsigned h = m / 60;
    m = m % 60;
    char b[64];
    sprintf(b, "%u:%02u:%02u", h, m, m_time % 60);
    edtTime->setText(b);
}

void FileTransferDlg::closeToggled(bool bState)
{
    CorePlugin::instance()->setValue("CloseTransfer", bState);
}

void FileTransferDlg::action(int nAct, void*)
{
	if(!m_msg->m_transfer)
		return;
    FileTransferDlgNotify *notify = static_cast<FileTransferDlgNotify*>(m_msg->m_transfer->notify());
    FileTransfer *ft = m_msg->m_transfer;
    switch (nAct)
    {
    case 1:
        notify->skip();
        if (m_ask->isChecked())
            ft->setOverwrite(Skip);
        break;
    case 2:
        notify->resume();
        if (m_ask->isChecked())
            ft->setOverwrite(Resume);
        break;
    default:
        notify->replace();
        if (m_ask->isChecked())
            ft->setOverwrite(Replace);
        break;
    }
}

void FileTransferDlg::goDir()
{
    if (m_dir.isEmpty())
        return;
    QString path = QString("file:") + m_dir;
    /* Now replace spaces with %20 so the path isn't truncated
       are there any other separators we need to care of ?*/
    //path.replace(' ',"%20");

    EventGoURL e(path);
    e.process();
}
