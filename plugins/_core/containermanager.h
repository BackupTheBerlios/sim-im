#ifndef CONTAINERMANAGER_H
#define CONTAINERMANAGER_H

#include <QObject>
#include <QList>
#include "container.h"

class CorePlugin;
class ContainerManager : public QObject
{
    Q_OBJECT
public:
    explicit ContainerManager(CorePlugin* parent);

    ContainerPtr makeContainer(int id);

    bool init();

    void addContainer(const ContainerPtr& cont);
    int containerCount();
    ContainerPtr container(int index);
    ContainerPtr containerById(int id);
    void removeContainer(int index);
    void removeContainerById(int id);

    enum ContainerMode
    {
        cmSimpleMode = 0,
        cmContactContainers = 1,
        cmGroupContainers = 2,
        cmOneContainer = 3
    };

    ContainerMode containerMode() const;
    void setContainerMode(ContainerMode mode);

signals:

public slots:

private:
    QList<ContainerPtr> m_containers;
    ContainerMode m_containerMode;
    CorePlugin* m_core;
};

#endif // CONTAINERMANAGER_H
