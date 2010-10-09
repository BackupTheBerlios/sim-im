#include "toolitem.h"

namespace SIM {

ToolItem::ToolItem(const UiCommandPtr& cmd, QAction* action) : m_cmd(cmd),
    m_action(action)
{
}

UiCommandPtr ToolItem::cmd() const
{
    return m_cmd;
}

QAction* ToolItem::action() const
{
    return m_action;
}

} // namespace SIM
