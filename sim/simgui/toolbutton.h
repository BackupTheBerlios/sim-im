#ifndef TOOLBUTTON_H
#define TOOLBUTTON_H

#include <QToolButton>
#include "toolitem.h"

namespace SIM {

class ToolButton : public QToolButton, public ToolItem
{
    Q_OBJECT
public:
    ToolButton(const UiCommandPtr& cmd, QWidget* parent = 0);

public slots:
    void updateState();
};

} // namespace SIM

#endif // TOOLBUTTON_H
