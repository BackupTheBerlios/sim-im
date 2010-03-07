/***************************************************************************
                          textshow.cpp  -  description
                             -------------------
    begin                : Sat Mar 16 2002
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

#include "simgui/textshow.h"

#ifdef USE_KDE
# include <keditcl.h>
# include <kstdaccel.h>
# include <kglobal.h>
# include <kfiledialog.h>
# define QFileDialog	KFileDialog
#else
# include <QFileDialog>
# include <QContextMenuEvent>
# include <QKeyEvent>
# include <QLabel>
# include <QFrame>
# include <QFocusEvent>
# include <QMouseEvent>
#endif

#ifdef USE_KDE
# include <kcolordialog.h>
# include <kfontdialog.h>
#else
# include <QColorDialog>
# include <QFontDialog>
#endif

#include <QDateTime>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <QRegExp>
#include <QObject>
#include <QTimer>
#include <QStringList>
#include <QTextCodec>
#include <QToolBar>
#include <QLineEdit>
#include <QToolButton>
#include <QStatusBar>
#include <QToolTip>
#include <QLayout>

#include "simgui/toolbtn.h"
#include "html.h"
#include "misc.h"
#include "unquot.h"

#define MAX_HISTORY	100

using namespace SIM;
using namespace std;

TextEdit::TextEdit(QWidget *p, const char *name)
        : TextShow(p)
{
    setObjectName(name);
    m_param = NULL;
    m_bEmpty = true;
    m_bBold  = false;
    m_bItalic = false;
    m_bUnderline = false;
    m_bSelected  = false;
    m_bNoSelected = false;
    m_bInClick = false;
    m_bChanged = false;
    setReadOnly(false);
    curFG = palette().color(QPalette::Active,QPalette::Text);
    m_bCtrlMode = true;
    setWordWrapMode(QTextOption::WordWrap); //setWordWrap(WidgetWidth);
    setAutoFormatting(0);
    connect(this, SIGNAL(currentFontChanged(const QFont&)), this, SLOT(fontChanged(const QFont&)));
    connect(this, SIGNAL(currentColorChanged(const QColor&)), this, SLOT(slotColorChanged(const QColor&)));
    connect(this, SIGNAL(textChanged()), this, SLOT(slotTextChanged()));
    connect(this, SIGNAL(clicked(int,int)), this, SLOT(slotClicked(int,int)));
    viewport()->installEventFilter(this);
    fontChanged(font());
}

TextEdit::~TextEdit()
{
    emit finished(this);
}

void TextEdit::setFont(const QFont &f)
{
    TextShow::setFont(f);
    m_bNoSelected = true;
    fontChanged(f);
    m_bNoSelected = false;
    m_bSelected   = false;
}


void TextEdit::slotTextChanged()
{
    bool bEmpty = isEmpty();
    if (m_bEmpty == bEmpty)
        return;
    m_bEmpty = bEmpty;
   
	Command cmd;
    cmd->id    = CmdSend;
    cmd->flags = bEmpty ? COMMAND_DISABLED : 0;
    EventCommandDisabled(cmd).process();
}

void TextEdit::slotClicked(int,int)
{
    if (!QTextEdit::textCursor().hasSelection())
        return;

    //FIXME ContextMenu?

    /*int paraFrom, paraTo, indexFrom, indexTo;
    getSelection(&paraFrom, &indexFrom, &paraTo, &indexTo);

    if ((paraFrom != paraTo) || (indexFrom != indexTo))
        return;
    m_bInClick = true;
    QContextMenuEvent e(QContextMenuEvent::Other, QPoint(0, 0), QPoint(0, 0));
    contentsContextMenuEvent(&e);
    m_bInClick = false;*/
}

QMenu *TextEdit::createPopupMenu(const QPoint& pos)
{
    if (m_bInClick)
        return NULL;
    m_popupPos = pos;
    return createPopupMenu(pos);
}

bool TextEdit::isEmpty() //FIXME
{
    //return TextShow::toPlainText()->isEmpty();
  /*  if (paragraphs() < 2){
        QString t = text(0);
        if(textFormat() == Qt::RichText)
            t = unquoteText(t);
        return t.isEmpty() || (t == " ");
    }*/
    return false;
}

void TextEdit::setParam(void *param)
{
    m_param = param;
}

void TextEdit::slotColorChanged(const QColor &c) //FIXME
{
    if (c == curFG)
        return;
    //int parag;
    //int index;
    //getCursorPosition(&parag, &index);
    if (QTextEdit::textCursor().selectedText().isEmpty()){
        setTextColor(curFG);
        return;
    }
    if (c != curFG)
        setForeground(c, false);
}

void TextEdit::bgColorChanged(QColor c)
{
    setBackground(c);
    emit colorsChanged();
}

void TextEdit::fgColorChanged(QColor c)
{
    setForeground(c, true);
    curFG = c;
    emit colorsChanged();
}

void TextEdit::focusOutEvent(QFocusEvent *e)
{
    if (e->lostFocus())
        emit lostFocus();
}

void TextEdit::fontChanged(const QFont &f)
{
    if (m_bSelected){
        if (!m_bNoSelected)
            emit fontSelected(f);
        m_bSelected = false;
    }
    if (m_param == NULL)
        return;
    m_bChanged = true;
    if (f.bold() != m_bBold){
        m_bBold = f.bold();
        Command cmd;
        cmd->id    = CmdBold;
        cmd->flags = m_bBold ? COMMAND_CHECKED : 0;
        cmd->param = m_param;
        EventCommandChecked(cmd).process();
    }
    if (f.italic() != m_bItalic){
        m_bItalic = f.italic();
        Command cmd;
        cmd->id    = CmdItalic;
        cmd->flags = m_bItalic ? COMMAND_CHECKED : 0;
        cmd->param = m_param;
         EventCommandChecked(cmd).process();
    }
    if (f.underline() != m_bUnderline){
        m_bUnderline = f.underline();
        Command cmd;
        cmd->id    = CmdUnderline;
        cmd->flags = m_bUnderline ? COMMAND_CHECKED : 0;
        cmd->param = m_param;
         EventCommandChecked(cmd).process();
    }
    m_bChanged = false;
}

void TextEdit::setCtrlMode(bool mode)
{
    m_bCtrlMode = mode;
}

void TextEdit::keyPressEvent(QKeyEvent *e)
{
    if (((e->key() == Qt::Key_Enter) || (e->key() == Qt::Key_Return)))
    {
        //   in m_bCtrlMode:    enter      --> newLine
        //                      ctrl+enter --> sendMsg
        //   in !m_bCtrlMode:   enter      --> sendMsg
        //                      ctrl+enter --> newLine
        // the (bool) is required due to the bitmap
        if (m_bCtrlMode == (bool)(e->modifiers() & Qt::ControlModifier)){
            emit ctrlEnterPressed();
            return;
        }
    }
    // Note: We no longer translate Enter to Ctrl-Enter since we need
    // to know about paragraph breaks now.
    TextShow::keyPressEvent(e);
}

bool TextEdit::processEvent(Event *e)
{
    if (m_param == NULL)
        return false;
    if (e->type() == eEventCheckCommandState){
        EventCheckCommandState *ecs = static_cast<EventCheckCommandState*>(e);
        CommandDef *cmd = ecs->cmd();
        if (cmd->param != m_param)
            return false;
        switch (cmd->id){
        case CmdBgColor:
        case CmdFgColor:
        case CmdBold:
        case CmdItalic:
        case CmdUnderline:
        case CmdFont:
            if (!isReadOnly()){
                cmd->flags &= ~BTN_HIDE;
            }else{
                cmd->flags |= BTN_HIDE;
            }
            return true;
        default:
            break;
        }
    } else
    if (e->type() == eEventCommandExec){
        EventCommandExec *ece = static_cast<EventCommandExec*>(e);
        CommandDef *cmd = ece->cmd();
        if (cmd->param != m_param)
            return false;
        switch (cmd->id){
        case CmdBgColor:{
                EventCommandWidget eWidget(cmd);
                eWidget.process();
                CToolButton *btnBg = dynamic_cast<CToolButton*>(eWidget.widget());
                if (btnBg){
                    ColorPopup *popup = new ColorPopup(this, background());
                    popup->move(CToolButton::popupPos(btnBg, popup));
                    connect(popup, SIGNAL(colorChanged(QColor)), this, SLOT(bgColorChanged(QColor)));
                    popup->show();
                }
                return true;
            }
        case CmdFgColor:{
                EventCommandWidget eWidget(cmd);
                eWidget.process();
                CToolButton *btnFg = dynamic_cast<CToolButton*>(eWidget.widget());
                if (btnFg){
                    ColorPopup *popup = new ColorPopup(this, foreground());
                    popup->move(CToolButton::popupPos(btnFg, popup));
                    connect(popup, SIGNAL(colorChanged(QColor)), this, SLOT(fgColorChanged(QColor)));
                    popup->show();
                }
                return true;
            }
        case CmdBold:
            if (!m_bChanged){
                m_bSelected = true;
                setFontWeight((cmd->flags & COMMAND_CHECKED) != 0 ? QFont::Bold : QFont::Normal);
            }
            return true;
        case CmdItalic:
            if (!m_bChanged){
                m_bSelected = true;
                setFontItalic((cmd->flags & COMMAND_CHECKED) != 0);
            }
            return true;
        case CmdUnderline:
            if (!m_bChanged){
                m_bSelected = true;
                setFontUnderline((cmd->flags & COMMAND_CHECKED) != 0);
            }
            return true;
        case CmdFont:{
#ifdef USE_KDE
                QFont f = font();
                if (KFontDialog::getFont(f, false, topLevelWidget()) != KFontDialog::Accepted)
                    break;
#else
                bool ok = false;
                QFont f = QFontDialog::getFont(&ok, font(), topLevelWidget());
                if (!ok)
                    break;
#endif
                m_bSelected = true;
                setCurrentFont(f);
                break;
            }
        default:
            return false;
        }
    }
    return false;
}

void TextEdit::setForeground(const QColor& c, bool bDef)
{
    curFG = c;
    if (bDef)
        defFG = c;
    if (textCursor().selectedText().isEmpty())
        setTextColor(c);
    int r = c.red();
    if (r){
        r--;
    }else{
        r++;
    }
    QPalette pal = palette();
    pal.setColor(QPalette::Active, QPalette::Text, QColor(r, c.green(), c.blue()));
    setPalette(pal);
}

const QColor &TextEdit::foreground() const
{
    return curFG;
}

const QColor &TextEdit::defForeground() const
{
    return defFG;
}

TextShow::TextShow(QWidget *p, const char *name)
    : QTextBrowser(p)
{
    setObjectName(name);
    setReadOnly(true);
    connect(this, SIGNAL(anchorClicked ( const QUrl &)  ),this,SLOT(setURL(const QUrl &)));
    setOpenExternalLinks(true);
    setOpenLinks(false);
}

TextShow::~TextShow()
{
    emit finished();
}

void TextShow::setBackground(const QColor& c)
{
    QPalette pal = palette();
    pal.setColor(QPalette::Active  , QPalette::Base, c);
    pal.setColor(QPalette::Inactive, QPalette::Base, c);
    pal.setColor(QPalette::Disabled, QPalette::Base, c);
    setPalette(pal);
}

void TextShow::setForeground(const QColor& c)
{
    QPalette pal = palette();
    pal.setColor(QPalette::Active, QPalette::Text, c);
    setPalette(pal);
}

const QColor &TextShow::background() const
{
    return palette().color(QPalette::Active, QPalette::Base);
}

const QColor &TextShow::foreground() const
{
    return palette().color(QPalette::Active, QPalette::Text);
}

void TextShow::emitLinkClicked(const QString &name)
{
    setSource(name);
}

void TextShow::setURL(const QUrl &url)
{
    setSource(url.toString());
}

void TextShow::setSource(const QUrl &url)
{
    setSource(url.toString());
}

void TextShow::setSource(const QString &name)
{
#ifndef QT_NO_CURSOR
    if ( isVisible() )
        qApp->setOverrideCursor( Qt::WaitCursor );
#endif
    QString source = name;
    QString mark;
    int hash = name.indexOf('#');
    if ( hash != -1) {
        source = name.left( hash );
        mark = name.mid( hash+1 );
    }

    QString url(source); //mimeSourceFactory()->makeAbsolute( source, context() );  //FIXME
    QString txt;

    if (!mark.isEmpty()) {
        url += '#';
        url += mark;
    }

    EventGoURL e(url);
    e.process();
#ifndef QT_NO_CURSOR
    if ( isVisible() )
        qApp->restoreOverrideCursor();
#endif
}

class BgColorParser : public HTMLParser
{
public:
    BgColorParser(TextEdit *edit);
protected:
    virtual void text(const QString &text);
    virtual void tag_start(const QString &tag, const list<QString> &options);
    virtual void tag_end(const QString &tag);
    TextEdit *m_edit;
};

BgColorParser::BgColorParser(TextEdit *edit)
{
    m_edit = edit;
}

void BgColorParser::text(const QString&)
{
}

void BgColorParser::tag_start(const QString &tag, const list<QString> &options)
{
    if (tag != "body")
        return;
    for (list<QString>::const_iterator it = options.begin(); it != options.end(); ++it){
        QString key = *it;
        ++it;
        QString val = *it;
        if (key == "bgcolor"){
            if (val[0] == '#'){
                bool bOK;
                unsigned rgb = val.mid(1).toUInt(&bOK, 16);
                if (bOK)
                    m_edit->setBackground(QColor(rgb));
            }
        }
    }
}

void BgColorParser::tag_end(const QString&)
{
}

static const unsigned colors[16] =
    {
        0x000000,
        0xFF0000,
        0x00FF00,
        0x0000FF,
        0xFFFF00,
        0xFF00FF,
        0x00FFFF,
        0xFFFFFF,
        0x404040,
        0x800000,
        0x008000,
        0x000080,
        0x808000,
        0x800080,
        0x008080,
        0x808080
    };

static const int CUSTOM_COLOR	= 100;

ColorPopup::ColorPopup(QWidget *popup, const QColor &color)
        : QFrame(popup, Qt::Popup)
        , m_color(color)
		, lay(new QGridLayout(this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(Sunken);

    //QGridLayout *lay = new QGridLayout(this);
    lay->setMargin(4);
    lay->setSpacing(2);
    for (unsigned i = 0; i < 4; i++){
        for (unsigned j = 0; j < 4; j++){
            unsigned n = i*4+j;
            QWidget *w = new ColorLabel(this, QColor(colors[n]), n, "");
            connect(w, SIGNAL(selected(int)), this, SLOT(colorSelected(int)));
            lay->addWidget(w, i, j);
        }
    }
    QWidget *w = new ColorLabel(this, color, CUSTOM_COLOR, i18n("Custom"));
    lay->addWidget(w, 5, 0, 1, 4);
    connect(w, SIGNAL(selected(int)), this, SLOT(colorSelected(int)));
    resize(minimumSizeHint());
}

void ColorPopup::colorSelected(int id)
{
    if (id == CUSTOM_COLOR)
	{
        hide();
        QWidget *top = NULL;
        if (parent())
            top = static_cast<QWidget*>(parent())->topLevelWidget();
#ifdef USE_KDE
        QColor c = m_color;
        if (KColorDialog::getColor(c, top) != KColorDialog::Accepted)
		{
            close();
            return;
        }
#else
        QColor c = QColorDialog::getColor(m_color, top);
        if (!c.isValid())
		{
            close();
            return;
        }
#endif
        Q_EMIT colorChanged(c);
    }
	else
	{
		QColor c;
		c.setRgb(colors[id]&0xFF0000, colors[id]&0x00FF00,colors[id]&0x0000FF);
		Q_EMIT colorChanged(c);
    }
    close();
}

ColorLabel::ColorLabel(QWidget *parent, const QColor &c, int id, const QString &text)
        : QLabel(parent)
        , m_id(id)
{
    setText(text);

    QPalette palette;
    palette.setColor(backgroundRole(), c);
    palette.setColor(QPalette::WindowText, !c.value());
    setPalette(palette);

    setAutoFillBackground(true);
    setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    setFrameShape(StyledPanel);
    setFrameShadow(Sunken);
    setLineWidth(2);
}

void ColorLabel::mouseReleaseEvent(QMouseEvent*)
{
    Q_EMIT selected(m_id);
}

QSize ColorLabel::minimumSizeHint() const
{
    QSize s = QLabel::minimumSizeHint();
    if (s.width() < s.height())
        s.setWidth(s.height());
    return s;
}

