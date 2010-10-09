#include <QMenu>
#include "uicommand.h"
#include "imagestorage/imagestorage.h"
#include <cstdio>

namespace SIM
{

UiCommandPtr UiCommand::create(const QString& text, const QString& iconId, const QString& id, const QStringList& tags, QObject* parent)
{
    return UiCommandPtr(new UiCommand(text, iconId, id, tags));
}

UiCommand::UiCommand(const QString& text, const QString& iconId, const QString& id, const QStringList& tags, QObject* parent) : QObject(parent),
    m_id(id),
    m_tags(tags),
    m_text(text),
    m_widgetType(UiCommand::wtNone),
    m_checkable(false),
    m_checked(false)
{
    setIconId(iconId);
}

QString UiCommand::text() const
{
    return m_text;
}

QIcon UiCommand::icon() const
{
    return m_icon;
}

QString UiCommand::id() const
{
    return m_id;
}

void UiCommand::addTag(const QString& tag)
{
    m_tags.append(tag);
}

void UiCommand::clearTags()
{
    m_tags.clear();
}

QStringList UiCommand::tags() const
{
    return m_tags;
}

void UiCommand::setIconId(const QString& id)
{
    Q_ASSERT(getImageStorage());
    m_iconId = id;
    m_icon = getImageStorage()->icon(id);
}

QString UiCommand::iconId() const
{
    return m_iconId;
}

void UiCommand::addSubCommand(const UiCommandPtr& subcmd)
{
    m_subcmds.append(subcmd);
}

QList<UiCommandPtr> UiCommand::subCommands() const
{
    return m_subcmds;
}

void UiCommand::trigger()
{
    emit triggered();
}

bool UiCommand::subscribeTo(QObject* obj, const char* slot)
{
    return connect(this, SIGNAL(triggered()), obj, slot);
}

bool UiCommand::unsubscribe(QObject* obj, const char* slot)
{
    return disconnect(this, SIGNAL(triggered()), obj, slot);
}

UiCommand::WidgetType UiCommand::widgetType() const
{
    return m_widgetType;
}

void UiCommand::setWidgetType(UiCommand::WidgetType wt)
{
    m_widgetType = wt;
}

bool UiCommand::isCheckable() const
{
    return m_checkable;
}

void UiCommand::setCheckable(bool b)
{
    m_checkable = b;
}

bool UiCommand::isChecked() const
{
    return m_checked;
}

void UiCommand::setChecked(bool b)
{
    if(b != m_checked)
    {
        m_checked = b;
        emit checked(b);
    }
}
}
