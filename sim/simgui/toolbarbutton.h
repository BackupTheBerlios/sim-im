#ifndef TOOLBARBUTTON_H
#define TOOLBARBUTTON_H

#include <QToolButton>

namespace SIM {

class ToolbarButton : public QToolButton
{
    Q_OBJECT
public:
    explicit ToolbarButton(QWidget *parent = 0);

signals:

public slots:
    void removeMenu();

};

} // namespace SIM

#endif // TOOLBARBUTTON_H
