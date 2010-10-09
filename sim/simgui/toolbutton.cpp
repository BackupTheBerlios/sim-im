#include "toolbutton.h"

namespace SIM {

ToolButton::ToolButton(const UiCommandPtr& cmd, QWidget* parent) : QToolButton(parent), ToolItem(cmd)
{
}

void ToolButton::updateState()
{

}

} // namespace SIM
