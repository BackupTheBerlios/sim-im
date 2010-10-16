#include <QMenu>
#include "toolbarbutton.h"
#include "commands/commandset.h"

namespace SIM {

ToolbarButton::ToolbarButton(const UiCommandPtr& cmd, QWidget *parent) :
    QToolButton(parent), m_cmd(cmd)
{
}

void ToolbarButton::mouseReleaseEvent(QMouseEvent* e)
{
    if(m_cmd->subcommands())
    {
        QMenu* m = new QMenu(this);
        m->addActions(m_cmd->subcommands()->createActionList(m));
        m->exec(parentWidget()->mapToGlobal(pos() + QPoint(0, size().height() + 2)));
    }
    QToolButton::mouseReleaseEvent(e);
}

} // namespace SIM
