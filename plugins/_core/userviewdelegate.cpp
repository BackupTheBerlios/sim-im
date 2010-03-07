#include "userviewdelegate.h"

#include <QPainter>

#include "userview.h"
#include "log.h"
#include "core.h"
#include "icons.h"
#include "contacts/group.h"

using namespace SIM;

UserViewDelegate::UserViewDelegate(UserView* uv) : m_uv(uv)
{
}

void UserViewDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    UserViewItemBase *base = dynamic_cast<UserViewItemBase*>(m_uv->itemFromIndex(index));
    if( NULL == base )
        return;

    painter->save();
    painter->translate(option.rect.x(), option.rect.y());

    QPainter *p = painter;
    QPalette cg = option.palette;
    int width = option.rect.width();
    int height = option.rect.height();
    QSize itemsize = option.rect.size();
    int margin = 1;

    switch( base->type() )
    {
        case GRP_ITEM:
        {
            GroupItem *item = static_cast<GroupItem*>(base);
            QString text = index.data( Qt::DisplayRole ).toString();
            QImage img = Image( ( option.state & QStyle::State_Open ) ? "expanded" : "collapsed");
            if (!img.isNull())
                p->drawImage(2 + margin, (height - img.height()) / 2, img);
            int x = 24 + margin;
            if (!( option.state & QStyle::State_Open ) && item->m_unread){
                CommandDef *def = CorePlugin::instance()->messageTypes.find(item->m_unread);
                if (def){
                    img = Image(def->icon);
                    if (!img.isNull()){
                        if (m_uv->m_bUnreadBlink)
                            p->drawImage(x, (height - img.height()) / 2, img);
                        x += img.width() + 2;
                    }
                }
            }
            if (!CorePlugin::instance()->value("UseSysColors").toBool())
                p->setPen(CorePlugin::instance()->value("ColorGroup").toUInt());
            QFont f(option.font);
            if (CorePlugin::instance()->value("SmallGroupFont").toBool()){
                int size = f.pixelSize();
                if (size <= 0){
                    size = f.pointSize();
                    f.setPointSize(size * 3 / 4);
                }else{
                    f.setPixelSize(size * 3 / 4);
                }
            }
            f.setBold(true);
            p->setFont(f);
            x = drawText(p, x, itemsize, text);
            if (CorePlugin::instance()->value("GroupSeparator").toBool())
                drawSeparator(p, x, itemsize, m_uv->style());
            break;
        }
        case USR_ITEM:
        {
            ContactItem *item = static_cast<ContactItem*>(base);
            QFont f(option.font);
            if (item->style() & CONTACT_ITALIC){
                if (CorePlugin::instance()->value("VisibleStyle").toUInt()  & STYLE_ITALIC)
                    f.setItalic(true);
                if (CorePlugin::instance()->value("VisibleStyle").toUInt()  & STYLE_UNDER)
                    f.setUnderline(true);
                if (CorePlugin::instance()->value("VisibleStyle").toUInt()  & STYLE_STRIKE)
                    f.setStrikeOut(true);
            }
            if (item->style() & CONTACT_UNDERLINE){
                if (CorePlugin::instance()->value("AuthStyle").toUInt()  & STYLE_ITALIC)
                    f.setItalic(true);
                if (CorePlugin::instance()->value("AuthStyle").toUInt()  & STYLE_UNDER)
                    f.setUnderline(true);
                if (CorePlugin::instance()->value("AuthStyle").toUInt()  & STYLE_STRIKE)
                    f.setStrikeOut(true);
            }
            if (item->style() & CONTACT_STRIKEOUT){
                if (CorePlugin::instance()->value("InvisibleStyle").toUInt()  & STYLE_ITALIC)
                    f.setItalic(true);
                if (CorePlugin::instance()->value("InvisibleStyle").toUInt()  & STYLE_UNDER)
                    f.setUnderline(true);
                if (CorePlugin::instance()->value("InvisibleStyle").toUInt()  & STYLE_STRIKE)
                    f.setStrikeOut(true);
            }
            int x = margin;
            QIcon mainIcon = index.data( Qt::DecorationRole ).value<QIcon>();
            if (!mainIcon.isNull()){
                QPixmap img = mainIcon.pixmap( 16 );
                x += 2;
                p->drawPixmap(x, ( height - img.height() ) / 2, img);
                x += img.width() + 2;
            }
            if (x < 24)
                x = 24;
            if (!item->isSelected() || !m_uv->hasFocus() || !CorePlugin::instance()->value("UseDblClick").toBool()){
                if (CorePlugin::instance()->value("UseSysColors").toBool()){
                    if (item->status() != STATUS_ONLINE && item->status() != STATUS_FFC)
                        p->setPen(m_uv->palette().color(QPalette::Disabled,QPalette::Text));
                }else{
                    switch (item->status()){
                    case STATUS_ONLINE:
                        p->setPen(CorePlugin::instance()->value("ColorOnline").toUInt());
                        break;
                    case STATUS_FFC:
                        p->setPen(CorePlugin::instance()->value("ColorOnline").toUInt());
                        break;
                    case STATUS_AWAY:
                        p->setPen(CorePlugin::instance()->value("ColorAway").toUInt());
                        break;
                    case STATUS_NA:
                        p->setPen(CorePlugin::instance()->value("ColorNA").toUInt());
                        break;
                    case STATUS_DND:
                        p->setPen(CorePlugin::instance()->value("ColorDND").toUInt());
                        break;
                    default:
                        p->setPen(CorePlugin::instance()->value("ColorOffline").toUInt());
                        break;
                    }
                }
            }
            if (item->m_bBlink)
                f.setBold(true);
            else
                f.setBold(false);

            p->setFont(f);
            QString highlight;
            QString text = index.data( Qt::DisplayRole ).toString();
            int pos=0;
            if(!m_uv->m_search.isEmpty())
            {
                pos=text.toUpper().indexOf(m_uv->m_search.toUpper());
                //Search for substring in contact name
                if (pos > -1)
                    highlight=text.mid(pos,m_uv->m_search.length());
            }
            int save_x = x;
            //p->setPen(QColor(0, 0, 0));
            x = drawText(p, x, itemsize, text);
            if (pos > 0)
                save_x = drawText(p, save_x, itemsize, text.left(pos)) - 4;
            x += 2;
            if (!highlight.isEmpty())
            {
                QPen oldPen = p->pen();
                QColor oldBg = p->background().color();
                p->setBackgroundMode(Qt::OpaqueMode);
                if (item == m_uv->m_searchItem){
                    if ((item == m_uv->currentItem()) && CorePlugin::instance()->value("UseDblClick").toBool()){
                        p->setBackground(cg.color(QPalette::HighlightedText));
                        p->setPen(cg.color(QPalette::Highlight));
                    }else{
                        p->setBackground(cg.color(QPalette::Highlight));
                        p->setPen(cg.color(QPalette::HighlightedText));
                    }
                }else{
                    p->setBackground(oldPen.color());
                    p->setPen(oldBg);
                }
                drawText(p, save_x, itemsize, highlight);
                p->setPen(oldPen);
                p->setBackground(oldBg);
                p->setBackgroundMode(Qt::TransparentMode);
            }
            unsigned xIcon = width;
            QString icons = index.data( SIM::ExtraIconsRole ).toString();
            while( !icons.isEmpty() ) {
                QString icon = getToken(icons, ',');
                QImage img = Image(icon);
                if (!img.isNull()){
                    xIcon -= img.width() + 2;
                    if (xIcon < (unsigned)x)
                        break;
                    p->drawImage(xIcon, (height - img.height()) / 2, img);
                }
            }
            break;
        }
        case DIV_ITEM:
        {
            QString text = index.data( Qt::DisplayRole ).toString();
            QFont f(option.font);
            int size = f.pixelSize();
            if (size <= 0)
            {
                size = f.pointSize();
                f.setPointSize(size * 3 / 4);
            }
            else
            {
                f.setPixelSize(size * 3 / 4);
            }
            p->setFont(f);
            int x = drawText(p, 24 + margin, itemsize, text);
            drawSeparator(p, x, itemsize, m_uv->style());
            break;
        }
    }

    painter->restore();
}

QSize UserViewDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    UserViewItemBase *base = dynamic_cast<UserViewItemBase*>(m_uv->itemFromIndex(index));

    QSize size;
    if(!base)
        return size;

    size.setWidth( m_uv->width() );

    QFont f(option.font);
    int h = 0;
    if (base->type() == GRP_ITEM){
        if (CorePlugin::instance()->value("SmallGroupFont").toBool()){
            int size = f.pixelSize();
            if (size <= 0){
                size = f.pointSize();
                f.setPointSize(size * 3 / 4);
            }else{
                f.setPixelSize(size * 3 / 4);
            }
        }
        h = 14;
    }
    if (base->type() == USR_ITEM){
        ContactItem *item = static_cast<ContactItem*>(base);
        QString icons = item->text(CONTACT_ICONS);
        while (!icons.isEmpty()){
            QString icon = getToken(icons, ',');
            QImage img = Image(icon);
            if (img.height() > h)
                h = img.height();
        }
        if (item->m_unread){
            CommandDef *def = CorePlugin::instance()->messageTypes.find(item->m_unread);
            if (def){
                QImage img = Image(def->icon);
                if (img.height() > h)
                    h = img.height();
            }
        }
    }
    QFontMetrics fm(f);
    int fh = fm.height();
    if (fh > h)
        h = fh;

    size.setHeight( h + 2 );

    return size;
}

int UserViewDelegate::drawText(QPainter *p, int x, QSize size, const QString &text) const
{
    QRect br;
    p->drawText(x, 0, size.width(), size.height(), Qt::AlignLeft | Qt::AlignVCenter, text, &br);
    return br.right() + 5;
}

void UserViewDelegate::drawSeparator(QPainter *p, int x, QSize size, QStyle *style) const
{
    if (x < size.width() - 6)
    {
        QStyleOption option;
        option.rect = QRect(x, size.height()/2, size.width() - 6 - x, 1);
        style->drawPrimitive(QStyle::PE_Q3Separator, &option, p);
    }
}

// vim: set expandtab:
