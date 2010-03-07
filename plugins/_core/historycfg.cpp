/***************************************************************************
historycfg.cpp  -  description
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

#include <algorithm>

#include <QCheckBox>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QValidator>
#include <QLabel>
#include <QFile>
#include <QDir>
#include <QRegExp>
#include <QTimer>
#include <QTabWidget>
#include <QSpinBox>
#include <QSyntaxHighlighter>
#include <QTextStream>
#include <QKeyEvent>
#include <QEvent>

#ifdef USE_KDE
# include <QApplication>
# include <kglobal.h>
# include <kstddirs.h>
#endif


#include "log.h"
#include "unquot.h"
#include "xsl.h"

#include "historycfg.h"
#include "core.h"
#include "msgview.h"

#include "simgui/ballonmsg.h"
#include "simgui/textshow.h"
#include "contacts/contact.h"
#include "contacts/client.h"

#ifdef __OS2__
#undef COMMENT
#endif

static char STYLES[] = "styles/";
static char EXT[]    = ".xsl";

//#undef QTextEdit

using namespace std;
using namespace SIM;

class XmlHighlighter : public QSyntaxHighlighter
{
public:
    XmlHighlighter(QTextEdit *textEdit) : QSyntaxHighlighter(textEdit) {}
    virtual int highlightParagraph( const QString &text, int endStateOfLastPara ) ;
    void highlightBlock( const QString &text ) //FIXME whole Method!!!
    { 
        QTextCharFormat myClassFormat;
        myClassFormat.setFontWeight(QFont::Bold);
        myClassFormat.setForeground(Qt::darkMagenta);
        QString pattern = "\\bMy[A-Za-z]+\\b";

        QRegExp expression(pattern);
        int index = text.indexOf(expression);
        while (index >= 0) 
        {
            int length = expression.matchedLength();
            setFormat(index, length, myClassFormat);
            index = text.indexOf(expression, index + length);
        }
    }
};

const int TEXT			= -2;
const int COMMENT		= 1;
const int TAG			= 2;
const int XML_TAG		= 3;
const int XSL_TAG		= 4;
const int STRING		= 5;
const int XML_STRING	= 6;
const int XSL_STRING	= 7;

const unsigned COLOR_COMMENT	= 0x808080;
const unsigned COLOR_TAG		= 0x008000;
const unsigned COLOR_STRING		= 0x000080;
const unsigned COLOR_XSL_TAG	= 0x800000;
const unsigned COLOR_XML_TAG	= 0x808080;

int XmlHighlighter::highlightParagraph(const QString &s, int state)
{
    int pos = 0;
    for (; pos < (int)(s.length());)
    {
        int n = pos;
        int n1;
        QColor c;
        switch (state)
        {
        case TEXT:
            n = s.indexOf('<', pos);
            if (n == -1)
                n = s.length();
            else
            {
                state = TAG;
                if (s.mid(n + 1, 2) == "--")
                    state = COMMENT;
                else if (s.mid(n + 1, 4) == "?xml")
                    state = XML_TAG;
                else if (s.mid(n + 1, 4) == "xsl:")
                    state = XSL_TAG;
                else if (s.mid(n + 1, 5) == "/xsl:")
                    state = XSL_TAG;
            }
            break;
        case COMMENT:
            n = s.indexOf("-->", pos);
            if (n == -1)
                n = s.length();
            else
                state = TEXT;
            c = QColor(COLOR_COMMENT);
            break;
        case TAG:
        case XSL_TAG:
        case XML_TAG:
            switch (state)
            {
            case XSL_TAG:
                c = QColor(COLOR_XSL_TAG);
                break;
            case XML_TAG:
                c = QColor(COLOR_XML_TAG);
                break;
            default:
                c = QColor(COLOR_TAG);
            }
            n = s.indexOf('>', pos);
            n1 = s.indexOf('\"', pos);
            if (n >= 0 && (n < n1 || n1 == -1))
            {
                state = TEXT;
                n++;
            }
            else if (n1 >= 0 && (n1 < n || n == -1))
            {
                switch (state)
                {
                case XSL_TAG:
                    state = XSL_STRING;
                    break;
                case XML_TAG:
                    state = XML_STRING;
                    break;
                default:
                    state = STRING;
                }
                n = n1;
            }
            else n = s.length();
            break;
        case STRING:
        case XML_STRING:
        case XSL_STRING:
            n = s.indexOf('\"', pos + 1);
            if (n >= 0)
            {
                switch (state)
                {
                case XML_STRING:
                    state = XML_TAG;
                    break;
                case XSL_STRING:
                    state = XSL_TAG;
                    break;
                default:
                    state = TAG;
                }
                n++;
            }
            else
                n = s.length();
            c = QColor(COLOR_STRING);
            break;
        }
        if (n - pos > 0)
            setFormat(pos, n - pos, c);
        pos = n;
    }
    return state;
}


HistoryConfig::HistoryConfig(QWidget *parent)
: QWidget   (parent)
, m_cur     (-1)
, m_bDirty  (false)
{
    setupUi(this);
    chkOwn->setChecked(CorePlugin::instance()->value("OwnColors").toBool());
    chkSmile->setChecked(CorePlugin::instance()->value("UseSmiles").toBool());
    chkExtViewer->setChecked(CorePlugin::instance()->value("UseExtViewer").toBool());
    edtExtViewer->setText(CorePlugin::instance()->value("ExtViewer").toString());
    chkAvatar->setChecked(CorePlugin::instance()->value("ShowAvatarInHistory").toBool());
    cmbPage->setEditable(true);
    QLineEdit *edit = cmbPage->lineEdit();
    edit->setValidator(new QIntValidator(1, 10000, edit));
    edit->setText(QString::number(CorePlugin::instance()->value("HistoryPage").toUInt()));
    QString str1 = i18n("Show %1 messages per page");
    QString str2;
    int n = str1.indexOf("%1");
    if (n >= 0)
    {
        str2 = str1.mid(n + 2);
        str1 = str1.left(n);
    }
    lblPage1->setText(str1);
    lblPage2->setText(str2);
    //edtStyle->setWordWrap(QTextEdit::NoWrap);
    edtStyle->setWordWrapMode(QTextOption::NoWrap);
    highlighter = new XmlHighlighter(edtStyle);
    addStyles(user_file(STYLES), true);
    str1 = i18n("Use external viewer");
    chkExtViewer->setText(str1);
    str1 = i18n("Show user avatar");
    chkAvatar->setText(str1);
#ifdef USE_KDE
    QStringList lst = KGlobal::dirs()->findDirs("data", "sim");
    for (QStringList::Iterator it = lst.begin(); it != lst.end(); ++it){
        QFile fi(*it + STYLES);
        if (!fi.exists())
            continue;
        addStyles(fi.name(), false);
    }
#else
    addStyles(app_file(STYLES), false);
#endif
    fillCombo(CorePlugin::instance()->value("HistoryStyle").toString());
    connect(cmbStyle, SIGNAL(activated(int)), this, SLOT(styleSelected(int)));
    connect(btnCopy, SIGNAL(clicked()), this, SLOT(copy()));
    connect(btnRename, SIGNAL(clicked()), this, SLOT(rename()));
    connect(btnDelete, SIGNAL(clicked()), this, SLOT(del()));
    connect(tabStyle, SIGNAL(currentChanged(QWidget*)), this, SLOT(viewChanged(QWidget*)));
    connect(edtStyle, SIGNAL(textChanged()), this, SLOT(textChanged()));
    connect(chkOwn, SIGNAL(toggled(bool)), this, SLOT(toggled(bool)));
    connect(chkSmile, SIGNAL(toggled(bool)), this, SLOT(toggled(bool)));
    connect(chkDays, SIGNAL(toggled(bool)), this, SLOT(toggledDays(bool)));
    connect(chkSize, SIGNAL(toggled(bool)), this, SLOT(toggledSize(bool)));
    connect(chkExtViewer, SIGNAL(toggled(bool)), this, SLOT(toggledExtViewer(bool)));
    SIM::PropertyHubPtr data = getContacts()->getUserData("history");
    chkDays->setChecked(data->value("CutDays").toBool());
    chkSize->setChecked(data->value("CutSize").toBool());
    edtDays->setValue(data->value("Days").toUInt());
    edtSize->setValue(data->value("MaxSize").toUInt());
    toggledDays(chkDays->isChecked());
    toggledSize(chkSize->isChecked());
    toggledExtViewer(chkExtViewer->isChecked());
}

HistoryConfig::~HistoryConfig()
{
    delete highlighter;
}

static char BACKUP_SUFFIX[] = "~";

void HistoryConfig::apply()
{
    bool bChanged = false;
    int idxStyle = cmbStyle->currentIndex();
    if (tabStyle->currentWidget() == source && m_bDirty && idxStyle >= 0)
        m_styles[idxStyle].text = unquoteText(edtStyle->toHtml());
    for (unsigned i = 0; i < m_styles.size(); i++)
    {
        if (m_styles[i].text.isEmpty() || !m_styles[i].bCustom)
            continue;
        if ((int)i == cmbStyle->currentIndex())
            bChanged = true;
        QString name = STYLES;
        name += m_styles[i].name;
        name += EXT;
        name = user_file(name);
        QFile f(QString(name).append(BACKUP_SUFFIX)); // use backup file for this ...
        if (f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            QString s;
            s = m_styles[i].text;
            f.write(s.toUtf8());

            const QFile::FileError status = f.error();
            const QString errorMessage = f.errorString();
            f.close();
            if (status != QFile::NoError)
            {
                log(L_ERROR, "IO error writing to file %s : %s", qPrintable(f.fileName()), qPrintable(errorMessage));
                continue;
            }
            // rename to normal file
            QFileInfo fileInfo(f.fileName());
            QString desiredFileName = fileInfo.fileName();
            desiredFileName = desiredFileName.left(desiredFileName.length() - strlen(BACKUP_SUFFIX));
            fileInfo.dir().remove(desiredFileName);
            if (fileInfo.dir().rename(fileInfo.fileName(), desiredFileName)) 
                continue;

            log(L_ERROR, "Can't rename file %s to %s", qPrintable(fileInfo.fileName()), qPrintable(desiredFileName));
            continue;
        }
        log(L_WARN, "[1]Can't create %s", qPrintable(name));
    }
    int cur = cmbStyle->currentIndex();
    if ((cur >= 0) && m_styles.size() &&
        (m_styles[cur].bChanged ||
        (m_styles[cur].name != CorePlugin::instance()->value("HistoryStyle").toString()))){
            CorePlugin::instance()->setValue("HistoryStyle", m_styles[cur].name);
            bChanged = true;
            delete CorePlugin::instance()->historyXSL;
            CorePlugin::instance()->historyXSL = new XSL(m_styles[cur].name);
    }

    if (chkOwn->isChecked() != CorePlugin::instance()->value("OwnColors").toBool()){
        bChanged = true;
        CorePlugin::instance()->setValue("OwnColors", chkOwn->isChecked());
    }
    if (chkSmile->isChecked() != CorePlugin::instance()->value("UseSmiles").toBool()){
        bChanged = true;
        CorePlugin::instance()->setValue("UseSmiles", chkSmile->isChecked());
    }
    if (chkExtViewer->isChecked() != CorePlugin::instance()->value("UseExtViewer").toBool()){
        bChanged = true;
        CorePlugin::instance()->setValue("UseExtViewer", chkExtViewer->isChecked());
    }
    if (chkAvatar->isChecked() != CorePlugin::instance()->value("ShowAvatarInHistory").toBool()){
        bChanged = true;
        CorePlugin::instance()->setValue("ShowAvatarInHistory", chkAvatar->isChecked());
    }
    CorePlugin::instance()->setValue("ExtViewer", edtExtViewer->text().toLocal8Bit());
    CorePlugin::instance()->setValue("HistoryPage", (uint)cmbPage->lineEdit()->text().toULong());
    if (bChanged){
        EventHistoryConfig(0).process();
    }
    fillPreview();
    SIM::PropertyHubPtr data = getContacts()->getUserData("history");
    data->setValue("CutDays", chkDays->isChecked());
    data->setValue("CutSize", chkSize->isChecked());
    data->setValue("Days", edtDays->text().toUInt());
    data->setValue("MaxSize", edtSize->text().toUInt());
}

void HistoryConfig::addStyles(const QString &dir, bool bCustom)
{
    QDir d(dir);
    QStringList files = d.entryList(QStringList("*.xsl"), QDir::Files, QDir::Name);
    for (QStringList::Iterator it = files.begin(); it != files.end(); ++it){
        QString name = *it;
        int n = name.lastIndexOf('.');
        name = name.left(n);
        vector<StyleDef>::iterator its;
        for (its = m_styles.begin(); its != m_styles.end(); ++its){
            if (name == (*its).name)
                break;
        }
        if (its == m_styles.end()){
            StyleDef s;
            s.name     = name;
            s.bCustom  = bCustom;
            s.bChanged = false;
            m_styles.push_back(s);
        }
    }
}

void HistoryConfig::toggled(bool)
{
    if (tabStyle->currentWidget() == preview)
        fillPreview();
}

void HistoryConfig::styleSelected(int n)
{
    if (n == m_cur)
        return;
    if (m_styles.size() == 0) return;
    if (m_bDirty && (m_cur >= 0))
        m_styles[m_cur].text = unquoteText(edtStyle->toHtml());
    m_cur = n;
    bool bCustom = m_styles[n].bCustom;
    btnRename->setEnabled(bCustom);
    btnDelete->setEnabled(bCustom);
    edtStyle->setReadOnly(!bCustom);
    fillPreview();
    if (tabStyle->currentWidget() == source)
        viewChanged(source);
}

void HistoryConfig::copy()
{
    int cur = cmbStyle->currentIndex();
    if ((cur < 0) || (!m_styles.size()))
        return;
    QString name    = m_styles[cur].name;
    QString newName;
    QRegExp re("\\.[0-9]+$");
    unsigned next = 0;
    for (vector<StyleDef>::iterator it = m_styles.begin(); it != m_styles.end(); ++it){
        QString nn = it->name;
        int n = nn.indexOf(re);
        if (n < 0)
            continue;
        nn = nn.mid(n + 1);
        next = qMax(next, nn.toUInt());
    }
    int nn = name.indexOf(re);
    if (nn >= 0){
        newName = name.left(nn);
    }else{
        newName = name;
    }
    newName += '.';
    newName += QString::number(next + 1);
    QString n;
    n = STYLES;
    n += name;
    n += EXT;
    if (m_styles[cur].bCustom){
        n = user_file(n);
    }else{
        n = app_file(n);
    }
    QFile from(n);
    if (!from.open(QIODevice::ReadOnly)){
        log(L_WARN, "Can't open %s", qPrintable(n));
        return;
    }
    n = STYLES;
    n += newName;
    n += EXT;
    n = user_file(n);
    QFile to(QString(n).append(BACKUP_SUFFIX));
    if (!to.open(QIODevice::WriteOnly | QIODevice::Truncate)){
        log(L_WARN, "Cam't create %s", qPrintable(n));
        return;
    }
    QDataStream ds1(&from);
    QDataStream ds2(&to);
    ds2 << ds1;
    from.close();

    const QFile::FileError status = to.error();
    const QString errorMessage = to.errorString();
    to.close();
    if (status != QFile::NoError) {
        log(L_ERROR, "IO error writing to file %s : %s", qPrintable(to.fileName()), qPrintable(errorMessage));
        return;
    }

    // rename to normal file
    QFileInfo fileInfo(to.fileName());
    QString desiredFileName = fileInfo.fileName();
    desiredFileName = desiredFileName.left(desiredFileName.length() - strlen(BACKUP_SUFFIX));
    fileInfo.dir().remove(desiredFileName);
    if (!fileInfo.dir().rename(fileInfo.fileName(), desiredFileName)) {
        log(L_ERROR, "Can't rename file %s to %s", qPrintable(fileInfo.fileName()), qPrintable(desiredFileName));
        return;
    }

    StyleDef d;
    d.name    = newName;
    d.bCustom = true;
    m_styles.push_back(d);
    fillCombo(newName);
}

void HistoryConfig::fillCombo(const QString &current)
{
    sort(m_styles.begin(), m_styles.end());
    unsigned cur = 0;
    cmbStyle->clear();
    for (unsigned i = 0; i < m_styles.size(); i++){
        QString name = m_styles[i].name;
        cmbStyle->insertItem(INT_MAX,name);
        if (name == current)
            cur = i;
    }
    cmbStyle->setCurrentIndex(cur);
    styleSelected(cur);
}

void HistoryConfig::del()
{
    int cur = cmbStyle->currentIndex();
    if ((cur < 0) || (!m_styles.size()))
        return;
    if (!m_styles[cur].bCustom)
        return;
    BalloonMsg::ask(NULL, i18n("Remove style '%1'?") .arg(m_styles[cur].name),
                    btnDelete, SLOT(realDelete()), NULL, NULL, this);
}

void HistoryConfig::realDelete()
{
    int cur = cmbStyle->currentIndex();
    if ((cur < 0) || (!m_styles.size()))
        return;
    if (!m_styles[cur].bCustom)
        return;
    QString name = m_styles[cur].name;
    vector<StyleDef>::iterator it;
    for (it = m_styles.begin(); it != m_styles.end(); ++it)
        if (cur-- == 0)
            break;
    m_styles.erase(it);
    QString n;
    n = STYLES;
    n += name;
    n += EXT;
    n = user_file(n);
    QFile::remove(n);
    fillCombo(CorePlugin::instance()->value("HistoryStyle").toString());
}

void HistoryConfig::rename()
{
    int cur = cmbStyle->currentIndex();
    if ((cur < 0) || (!m_styles.size()))
        return;
    if (!m_styles[cur].bCustom)
        return;
    m_edit = cur;
    cmbStyle->setEditable(true);
    cmbStyle->lineEdit()->setText(m_styles[cur].name);
    cmbStyle->lineEdit()->setFocus();
    cmbStyle->lineEdit()->installEventFilter(this);
}

void HistoryConfig::cancelRename()
{
    cmbStyle->lineEdit()->removeEventFilter(this);
    cmbStyle->setEditable(false);
}

void HistoryConfig::realRename()
{
    QString newName = cmbStyle->lineEdit()->text();
    cmbStyle->lineEdit()->removeEventFilter(this);
    cmbStyle->setEditable(false);
    if (newName != m_styles[m_edit].name){
        int n = 0;
        vector<StyleDef>::iterator it;
        for (it = m_styles.begin(); it != m_styles.end(); ++it, n++){
            if (it->name == newName){
                if (n < m_edit)
                    m_edit--;
                m_styles.erase(it);
                break;
            }
        }
        QString nn;
        nn = STYLES;
        nn += m_styles[m_edit].name;
        nn += EXT;
        nn = user_file(nn);
        if (m_styles[m_edit].text.isEmpty()){
            QFile f(nn);
            if (f.open(QIODevice::ReadOnly)){
                QTextStream ts(&f);
                m_styles[m_edit].text = ts.readAll();
            }
        }
        QFile::remove(nn);
        m_styles[m_edit].name = newName;
    }
    fillCombo(newName);
}

bool HistoryConfig::eventFilter(QObject *o, QEvent *e)
{
	if(e->type() == QEvent::FocusOut)
	{
		QTimer::singleShot(0, this, SLOT(realRename()));
	}
	if (e->type() == QEvent::KeyPress)
	{
		QKeyEvent *ke = static_cast<QKeyEvent*>(e);
		switch (ke->key()){
			case Qt::Key_Enter:
			case Qt::Key_Return:
				QTimer::singleShot(0, this, SLOT(realRename()));
				return true;
			case Qt::Key_Escape:
				QTimer::singleShot(0, this, SLOT(cancelRename()));
				return true;
		}
	}
	return QWidget::eventFilter(o, e);
}

void HistoryConfig::viewChanged(QWidget *w)
{
    int cur = cmbStyle->currentIndex();
    if ((cur < 0) || (!m_styles.size()))
        return;
    if (w == preview){
        if (!m_styles[cur].bCustom)
            return;
        if (m_bDirty){
            m_styles[cur].text = unquoteText(edtStyle->toHtml());
            fillPreview();
        }
    }else{
        QString xsl;
        if (m_styles[cur].text.isEmpty()){
            QString name = STYLES;
            name += m_styles[cur].name;
            name += EXT;
            name = m_styles[cur].bCustom ? user_file(name) : app_file(name);
            QFile f(name);
            if (f.open(QIODevice::ReadOnly)){
                QTextStream ts(&f);
                xsl = ts.readAll();
            }else{
                log(L_WARN, "Can't open %s", qPrintable(name));
            }
        }else{
            xsl = m_styles[cur].text;
        }
        edtStyle->setText(quoteString(xsl));
        QTimer::singleShot(0, this, SLOT(sync()));
    }
}

void HistoryConfig::textChanged()
{
    m_bDirty = true;
    int cur = cmbStyle->currentIndex();
    if ((cur < 0) || (!m_styles.size()))
        return;
    m_styles[cur].bChanged = true;
}

void HistoryConfig::fillPreview()
{
    m_bDirty = false;
    int cur = cmbStyle->currentIndex();
    if ((cur < 0) || (cur >= (int)m_styles.size()))
        return;
    XSL *xsl = new XSL(m_styles[cur].name);
    if (!m_styles[cur].text.isEmpty())
        xsl->setXSL(m_styles[cur].text);
    Contact *contact = getContacts()->contact(0, true);
    contact->setName("Buddy");
    contact->setFlags(CONTACT_TEMP);
    edtPreview->clear();
    edtPreview->setXSL(xsl);
    time_t now = time(NULL);
    bool saveSmiles = CorePlugin::instance()->value("UseSmiles").toBool();
    bool saveOwn    = CorePlugin::instance()->value("OwnColors").toBool();
    CorePlugin::instance()->setValue("UseSmiles", chkSmile->isChecked());
    CorePlugin::instance()->setValue("OwnColors", chkOwn->isChecked());
    Message m1;
    m1.setId((unsigned)(-1));
    m1.setFlags(MESSAGE_RECEIVED | MESSAGE_LIST);
    m1.setText(i18n("Hi! ;)"));
    m1.setTime(now - 360);
    m1.setContact(contact->id());
    edtPreview->addMessage(&m1);
    Message m2;
    m2.setId((unsigned)(-2));
    m2.setText(i18n("Hi!"));
    m2.setTime(now - 300);
    m2.setContact(contact->id());
    edtPreview->addMessage(&m2);
    Message m3;
    m3.setId((unsigned)(-3));
    m3.setText(i18n("<b><font color=\"#FF0000\">C</font><font color=\"#FFFF00\">olored</font></b> message"));
    m3.setTime(now - 120);
    m3.setFlags(MESSAGE_SECURE | MESSAGE_URGENT | MESSAGE_RICHTEXT);
    m3.setBackground(0xC0C0C0);
    m3.setForeground(0x008000);
    m3.setContact(contact->id());
    edtPreview->addMessage(&m3);
    Message m4;
    m4.setId((unsigned)(-4));
    m4.setText(i18n("New message"));
    m4.setFlags(MESSAGE_RECEIVED);
    m4.setTime(now - 60);
    m4.setContact(contact->id());
    edtPreview->addMessage(&m4, true);
    StatusMessage m5;
    m5.setId((unsigned)(-5));
    m5.setStatus(STATUS_OFFLINE);
    m5.setTime(now);
    m5.setContact(contact->id());
    if (getContacts()->nClients())
        m5.setClient((getContacts()->getClient(0)->name() + '.'));
    edtPreview->addMessage(&m5);
    delete contact;
    CorePlugin::instance()->setValue("UseSmiles", saveSmiles);
    CorePlugin::instance()->setValue("OwnColors", saveOwn);
}

void HistoryConfig::toggledDays(bool bState)
{
    lblDays->setEnabled(bState);
    lblDays1->setEnabled(bState);
    edtDays->setEnabled(bState);
}

void HistoryConfig::toggledSize(bool bState)
{
    lblSize->setEnabled(bState);
    lblSize1->setEnabled(bState);
    edtSize->setEnabled(bState);
}

void HistoryConfig::toggledExtViewer(bool bState)
{
    edtExtViewer->setEnabled(bState);
}

/*
#ifndef NO_MOC_INCLUDES
#include "historycfg.moc"
#endif
*/

