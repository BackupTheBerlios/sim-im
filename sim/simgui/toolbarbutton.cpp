#include <QMenu>
#include "toolbarbutton.h"

namespace SIM {

ToolbarButton::ToolbarButton(QWidget *parent) :
    QToolButton(parent)
{
}

void ToolbarButton::removeMenu()
{
    QMenu* m = menu();
    setMenu(m);
    delete m;
}

} // namespace SIM
