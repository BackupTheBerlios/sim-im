#include "containermanager.h"
#include "core.h"
#include "profilemanager.h"

ContainerManager::ContainerManager(CorePlugin* parent) :
    QObject(parent), m_core(parent)
{
}

ContainerPtr ContainerManager::makeContainer(int id)
{
    return ContainerPtr(new Container(id, 0));
}

bool ContainerManager::init()
{
    setContainerMode((ContainerMode)SIM::ProfileManager::instance()->getPropertyHub("_core")->value("ContainerMode").toUInt());
    return true;
}


void ContainerManager::addContainer(const ContainerPtr& cont)
{
    m_containers.append(cont);
}

int ContainerManager::containerCount()
{
    return m_containers.count();
}

ContainerPtr ContainerManager::container(int index)
{
    // Check boundaries ?
    return m_containers.at(index);
}

ContainerPtr ContainerManager::containerById(int id)
{
    foreach(const ContainerPtr& c, m_containers) {
        if(c->getId() == id)
            return c;
    }
    return ContainerPtr();
}

void ContainerManager::removeContainer(int index)
{
    if((index < 0) || (index >= containerCount()))
        return;
    m_containers.removeAt(index);
}

void ContainerManager::removeContainerById(int id)
{
    int index = 0;
    foreach(const ContainerPtr& c, m_containers) {
        if(c->getId() == id) {
            m_containers.removeAt(index);
            return;
        }
        index++;
    }
}

ContainerManager::ContainerMode ContainerManager::containerMode() const
{
    return m_containerMode;
}

void ContainerManager::setContainerMode(ContainerManager::ContainerMode mode)
{
    m_containerMode = mode;
}
