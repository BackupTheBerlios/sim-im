
#ifndef CORE_USERVIEWDELEGATE_H
#define CORE_USERVIEWDELEGATE_H

#include <QItemDelegate>

class UserView;
class UserViewDelegate : public QItemDelegate
{
public:
    UserViewDelegate(UserView* uv);
    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

protected:
    int drawText(QPainter *p, int x, QSize size, const QString &text) const;
    void drawSeparator(QPainter *p, int x, QSize size, QStyle *style) const;

private:
    UserView* m_uv;
};

#endif

// vim: set expandtab:
