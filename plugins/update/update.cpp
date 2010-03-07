/***************************************************************************
                          update.cpp  -  description
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

#include "update.h"
#include "socket/socketfactory.h"
#include "core.h"

#include "simgui/ballonmsg.h"
#include "aboutdata.h"
#include "log.h"
#include "mainwin.h"

#include <time.h>
#include <stdio.h>
#ifdef WIN32
#include <windows.h>
#endif
#include <QTimer>
#include <QApplication>
#include <QWidgetlist>
#include <QRegExp>
#include <QUrl>
#include <QMessageBox>
#include <QFile>
#include <QProcess>
#include <QtEvents>
#include <QFileInfo>

using namespace SIM;



QWidget *UpdatePlugin::getMainWindow() //obsolete
{
	CorePlugin *core = GET_CorePlugin();
    return core->getMainWindow();
}


Plugin *createUpdatePlugin(unsigned base, bool, Buffer *config)
{
    Plugin *plugin = new UpdatePlugin(base, config);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("Update"),
        I18N_NOOP("Plugin provides notifications about update SIM software"),
        VERSION,
        createUpdatePlugin,
        PLUGIN_NOLOAD_DEFAULT
    };

static QWidget *pMain   = NULL;

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

static DataDef updateData[] =
    {
        { "Time", DATA_ULONG, 1, 0 },
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

UpdatePlugin::UpdatePlugin(unsigned base, Buffer *config)
        : Plugin(base)
{
    load_data(updateData, &data, config);
    CmdGo = registerType();
	this->timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
	this->show=false;
	this->upToDate=false;
	this->ignore=false;
	this->isInstalling=false;
	this->CHECK_INTERVAL = 60; //seconds for the first time wait
	setTime(time(NULL)); //this was missing ;)
	this->msgret=-1;
	this->bupdateMsgMissing=true;
    this->timer->start(15000);

}

UpdatePlugin::~UpdatePlugin()
{
    free_data(updateData, &data);
}

QByteArray UpdatePlugin::getConfig()
{
    return save_data(updateData, &data);
}

void UpdatePlugin::timeout()
{
    testForUpdate();
}

void UpdatePlugin::testForUpdate(){
	if (ignore) return;
	if (!getSocketFactory()->isActive() || !isDone())
        return;
    if (((unsigned)time(NULL)) >= getTime() + CHECK_INTERVAL){
		versionurl=QString("");
		this->CHECK_INTERVAL=60*60*12; //checking every half day for an update, after first time
        //url = "http://sim-im.org/index.php?v=" + VERSION;
#ifdef WIN32
		versionurl = "http://www.sim-icq.de/update.php?";
        versionurl += "os=1";
#else
#ifdef QT_MACOSX_VERSION
        versionurl += "&os=2";
		location="??";
#endif
#endif
#ifdef CVS_BUILD
        //url += "&svn=";
		QString date(__DATE__);
		versionurl += date;
		versionurl.replace(' ',"%20");
		#ifdef WIN32
			location="http://www.sim-icq.de";
		#endif 
#else
        versionurl += "&release";
#endif
        /*versionurl += "&l=";
        QString s = i18n("Message", "%n messages", 1);
        s = s.remove("1 ");
        for (int i = 0; i < (int)(s.length()); i++){
            unsigned short c = s[i].unicode();
            if ((c == ' ') || (c == '%') || (c == '=') || (c == '&')){
                char b[5];
                sprintf(b, "%02X", c);
                versionurl += b;
            }else if (c > 0x77){
                char b[10];
                sprintf(b, "#%04X", c);
                versionurl += b;
            }else{
                versionurl += (char)c;
            }
        }*/
        QUrl um( "http://www.sim-icq.de/updatemsg.php" );
		httpmsg = new QHttp(this);
		connect(httpmsg, SIGNAL(requestFinished(int, bool)),this, SLOT(UpdateMsgDownloadFinished(int, bool)));
		QBuffer *buffer_um = new QBuffer(&bytes_um);
        buffer_um->open(QIODevice::ReadWrite);
		httpmsg->setHost(um.host());
		Request_um=httpmsg->get(um.path(),buffer_um);
    }
}

void UpdatePlugin::UpdateMsgDownloadFinished(int requestId, bool error){
	if (error || msgret==QMessageBox::Yes
			  || msgret==QMessageBox::No 
			  || msgret==QMessageBox::Ok
			  || upToDate) return; //Don't show the dialog more than once SIM starts.
	if (Request_um==requestId) {
		QString updateMsg(bytes_um);
		this->m_updateMsg=updateMsg;
		this->bupdateMsgMissing=false;
		disconnect(httpmsg, SIGNAL(requestFinished(int, bool)),this, SLOT(UpdateMsgDownloadFinished(int, bool)));
		QUrl u=QUrl(versionurl);
		http = new QHttp(this);
		connect(http, SIGNAL(requestFinished(int, bool)),this, SLOT(Finished(int, bool)));
		QBuffer *buffer = new QBuffer(&bytes);
		buffer->open(QIODevice::ReadWrite);
		http->setHost(u.host());
		Request=http->get(u.path(),buffer);
	}
}

void UpdatePlugin::Finished(int requestId, bool error){
	if (error || msgret==QMessageBox::Yes
			  || msgret==QMessageBox::No 
			  || msgret==QMessageBox::Ok
			  || upToDate
			  || bupdateMsgMissing) return; //Don't show the dialog more than once SIM starts.

	
    if (Request==requestId) {
		QString remoteVersion(bytes);
		QDate date=QDate::fromString(remoteVersion,Qt::LocalDate);
		QString currentVersion = SIM::getAboutData()->version();
		QString majorVersion = currentVersion.section(' ',0,2,QString::SectionDefault);
		if (!isUpdateNeeded(currentVersion, remoteVersion))
        { //If no Update is needed don't go further.
			if(remoteVersion.right(4).compare("HTML")==0) 
            {
				upToDate=false;
				return;
			}
			upToDate=true;
			log(L_DEBUG, "Update::You have the latest online Version.");		
			return;
		}
		
		if (!show) {
			show=!show;
			disconnect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
			msgret = QMessageBox::question( 0, i18n("Sim-IM Update"),
				i18n("A new update ist available.\n\nYou have Version %1:\n%2\n\n").arg(majorVersion).arg(dlocal.toString()) +
				i18n("New Version is:\n%1\n\n").arg(dremote.toString()) + 
				i18n("Changes are:\n%1\n\n").arg(this->m_updateMsg) + 
#ifdef WIN32
				i18n("I can now DOWNLOAD the Update\navailable at: %1\nIN BACKROUND and install the update\nfor Sim-IM, automatically after finishing.\n\nWould like you to ALLOW to carry out THE UPDATE?").arg(location), 
				QMessageBox::Yes,QMessageBox::No);
			
			address=QString("http://www.sim-icq.de/setup.exe");
				
			if (msgret == QMessageBox::Yes) 
				download_and_install();
			else {
				msgret = QMessageBox::question( 0, i18n("Sim-IM Update Remember?"),
				i18n("Should I remember you to update in some minutes again?"),
				QMessageBox::Yes,QMessageBox::No);

				if (msgret == QMessageBox::No)
					this->timer->stop();
				else {
					connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
					disconnect(http, SIGNAL(requestFinished(int, bool)),this, SLOT(fileRequestFinished(int, bool)));
					connect(http, SIGNAL(requestFinished(int, bool)),this, SLOT(Finished(int, bool)));
					show=!show;
					msgret=0;
					setTime(time(NULL));
					this->CHECK_INTERVAL=60*20;
				}
				return;
			}
#else
			i18n("Please go to %1\nand download the new version from:\n\n%2").arg(location).arg(datestr), 
			QMessageBox::Ok);
			address=QString::null;
#endif
		}
	}
}

bool UpdatePlugin::isUpdateNeeded(QString& local, QString& remote){

	/*
		The remote-String in win32 is generated in following format:
		echo %SIMVERSION% %SIMTAG% %SVNTAG% %DATE% %TIME% >update.php
	*/
	//Cut the Time away
	remote = remote.trimmed();
	remote = remote.left(remote.length()-11);
	remote = remote.trimmed();
	
	remote	= remote.replace("  "," "); //No double whitespaces, because scanning is wrong then
	local	= local.replace("  "," ");
	
	local   = local.section (' ',2,5,QString::SectionDefault);
	remote  = remote.section(' ',4,4,QString::SectionDefault);
	
	QString month("Jan,Feb,Mar,Apr,May,Jun,Jul,Aug,Sep,Oct,Nov,Dec");
	QStringList ml = month.split(',', QString::SkipEmptyParts);
	int i=0;
	for ( QStringList::Iterator it = ml.begin() ; it != ml.end(); ++it, ++i ) 
    {
		QString search(*it);
		if (search.compare(local.section(' ',0,0, QString::SectionDefault))==0)
			break;
	}
	
	this->dlocal  = QDate(local.right(4).toInt(), i+1 , local.section(' ',1,1, QString::SectionDefault).toInt());
	this->dremote = QDate(remote.right(4).toInt(), remote.mid(3,2).toInt(), remote.left(2).toInt());
    
	if (dremote.isNull())
	{	
		QMessageBox::critical( 0, i18n("Update Plugin: Error fetching the date of the current Update Setup."),
							i18n("Please contact and notify me via noragen@gmx.net about this issue. \n//\\//oRaGen."));
		log(L_DEBUG, "Update::Error in parsing Version-String. Perhaps you forgot to set SVNTAG and SIMTAG - Environment-Vars");		
	}

	//local=dlocal->toString();
	//remote=dremote->toString();
	if (dlocal.daysTo(dremote)>0)
	{
		log(L_DEBUG, "Update::There is a new Version on www.sim-icq.de");		
		return true;
	}
	else
	{
		log(L_DEBUG, "Update::Your SIM-Version is up to date.");		
		return false;
	}
}

void UpdatePlugin::download_and_install(){
	disconnect(http, SIGNAL(requestFinished(int, bool)),this, SLOT(Finished(int, bool)));
	connect(http, SIGNAL(requestFinished(int, bool)),this, SLOT(fileRequestFinished(int, bool)));
	ignore=true;
	downloadFile();
}

void UpdatePlugin::installFile(){
#ifdef WIN32
	if (isInstalling) return;

    qint64 pid;
	if ( !QProcess::startDetached("setup.exe", QStringList(), ".", &pid) ) {
		 QMessageBox::critical( 0, i18n("Error launching the Update-Setup"),
				i18n("Make sure the Sim-IM Dirctory\n") +
				i18n("is writable and you have rights to install.\n"));
		 ignore=false;
		 disconnect(http, SIGNAL(requestFinished(int, bool)),this, SLOT(fileRequestFinished(int, bool)));
		 return;
	}

	//Shutdown SIM here, because we are now ready to install:
	isInstalling=true;
	QCloseEvent *e = new QCloseEvent();
	//pMain=getMainWindow(); //obsolete

    pMain=GET_CorePlugin()->getMainWindow();
    
	(static_cast<MainWindow*>(pMain))->closeEvent(e); 

#endif
}

void UpdatePlugin::downloadFile()
 {
     QUrl url(address);
     QFileInfo fileInfo(url.path());
     QString fileName = fileInfo.fileName();

	 if (QFile::exists(fileName)){
		 QFile::remove(fileName);
		 download_and_install();
         return;
	 }

     file = new QFile(fileName);
	 if (!file->open(QIODevice::WriteOnly)) {
		 QMessageBox::critical( 0, i18n("HTTP"),
				i18n("Unable to save the file %1: %2.")
                .arg(fileName).arg(file->errorString()));
         delete file;
         file = 0;
         return;
     }
     httpRequestAborted = false;
     Request = http->get(url.path(), file);
 }

void UpdatePlugin::fileRequestFinished(int requestId, bool error)
 {
     if (httpRequestAborted) {
         if (file) {
             file->close();
             file->remove();
             delete file;
             file = 0;
         }

         //progressDialog->hide();
         return;
     }

     if (requestId != Request)
         return;

     //progressDialog->hide();
     file->close();

     if (error) {
         file->remove();
         download_and_install();
		 return;
     }
	 installFile();
     //downloadButton->setEnabled(true);
 }



#if 0
I18N_NOOP("Show details")
I18N_NOOP("Remind later")
#endif

bool UpdatePlugin::done(unsigned, Buffer&, const QString &headers)
{
    QString h = getHeader("Location", headers);	
    if (!h.isEmpty()){
        Command cmd;
        cmd->id		= CmdStatusBar;
        EventCommandWidget eWidget(cmd);
        eWidget.process();
        QWidget *statusWidget = eWidget.widget();
        if (statusWidget == NULL)
            return false;
        m_url = h;
        EventNotification::ClientNotificationData d;
        d.client  = NULL;
        d.text	  = I18N_NOOP("New version SIM is released");
        d.code	  = 0;
        d.args    = QString::null;
        d.flags   = EventNotification::ClientNotificationData::E_INFO;
        d.options = "Show details\x00Remind later\x00\x00";
        d.id	  = CmdGo;
        EventShowNotification e(d);
        e.process();
    }
    setTime(time(NULL));
    EventSaveState e;
    e.process();
    return false;
}

bool UpdatePlugin::processEvent(Event *e)
{
    if (e->type() == eEventCommandExec){
        EventCommandExec *ece = static_cast<EventCommandExec*>(e);
        CommandDef *cmd = ece->cmd();
        if (cmd->id == CmdGo){
            EventGoURL(m_url).process();
            setTime(time(NULL));
            m_url = QString::null;
            EventSaveState().process();
            return true;
        }
    }
    return false;
}

QString UpdatePlugin::getHeader(const QString &name, const QString &headers)
{
    int idx = headers.indexOf(name + ':');
    if(idx != -1) {
        int end = headers.indexOf('\n', idx);
        QString res;
        if(end == -1)
            res = headers.mid(idx);
        else
            res = headers.mid(idx, end - idx + 1);
        return res.trimmed();
    }
    return QString::null;
}
/*
#ifndef NO_MOC_INCLUDES
#include "update.moc"
#endif
*/