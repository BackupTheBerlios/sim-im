#include <QAction>
#include <QMenu>
#include "toolbarcombobox.h"
#include "commands/commandset.h"

namespace SIM {

ToolbarComboBox::ToolbarComboBox(const UiCommandPtr& cmd, QWidget *parent) :
    QPushButton(parent), m_cmd(cmd)
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
}

void ToolbarComboBox::actionSelected()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if(!action)
        return;
    setIcon(action->icon());
    setText(action->text());
}

void ToolbarComboBox::mouseReleaseEvent(QMouseEvent* e)
{
    if(m_cmd->subcommands())
    {
        QMenu* m = new QMenu(this);
        QList<QAction*> actions = m_cmd->subcommands()->createActionList(m);
        foreach(QAction* a, actions)
            connect(a, SIGNAL(triggered()), this, SLOT(actionSelected()));

        m->addActions(actions);
        m->exec(parentWidget()->mapToGlobal(pos() + QPoint(0, size().height() + 2)));
    }
    QPushButton::mouseReleaseEvent(e);
}

} // namespace SIM
