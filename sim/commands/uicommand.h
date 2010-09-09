#ifndef UICOMMAND_H
#define UICOMMAND_H

#include <QAction>
#include <QString>
#include <QStringList>
#include <QSharedPointer>

namespace SIM
{

class UiCommand;
typedef QSharedPointer<UiCommand> UiCommandPtr;

class UiCommand : public QAction
{
    Q_OBJECT
public:
    static UiCommandPtr create(const QString& text, const QString& iconId, const QString& id, const QStringList& tags = QStringList(), QObject* parent = NULL);

    void setIconId(const QString& id);
    QString iconId() const;

    QString id() const;

    void addTag(const QString& tag);
    void clearTags();
    QStringList tags() const;
signals:

public slots:

private:
    explicit UiCommand(const QString& text, const QString& iconId, const QString& id, const QStringList& tags = QStringList(), QObject* parent = NULL);

    QString m_id;
    QStringList m_tags;
    QString m_iconId;
};
}

#endif // UICOMMAND_H
