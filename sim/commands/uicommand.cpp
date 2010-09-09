#include "uicommand.h"

namespace SIM
{

UiCommandPtr UiCommand::create(const QString& text, const QString& iconId, const QString& id, const QStringList& tags, QObject* parent)
{
    return UiCommandPtr(new UiCommand(text, iconId, id, tags));
}

UiCommand::UiCommand(const QString& text, const QString& iconId, const QString& id, const QStringList& tags, QObject* parent) :
    QAction(text, parent), m_id(id), m_tags(tags)
{
    setIconId(iconId);
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
    m_iconId = id;
    // TODO update QAction::icon
}

QString UiCommand::iconId() const
{
    return m_iconId;
}

}
