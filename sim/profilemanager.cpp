
#include <QDir>
#include "profilemanager.h"
#include "log.h"
#include "event.h"

namespace SIM
{
#if defined(WIN32) || defined(QT_VISIBILITY_AVAILABLE)
    template <> EXPORT ProfileManager* Singleton<ProfileManager>::m_instance = 0;
#endif
    ProfileManager::ProfileManager(const QString& rootpath) : Singleton<ProfileManager>(),
    m_rootPath(rootpath)
    {
    }

    ProfileManager::~ProfileManager()
    {
    }

    QStringList ProfileManager::enumProfiles()
    {
        QStringList profiles;
        QDir dir(m_rootPath);
        dir.setFilter(QDir::Dirs|QDir::NoDotAndDotDot);
        QStringList list = dir.entryList();
        for(QStringList::iterator it = list.begin(); it != list.end(); ++it)
        {
            QString entry = *it;
            QString fname = QString(m_rootPath).append("/").append(entry).append("/").append("clients.conf");
            QString fname2 = QString(m_rootPath).append("/").append(entry).append("/").append("profile.conf");
            QFile f(fname);
            QFile f2(fname2);
            if(f.exists() || f2.exists())
            {
                profiles.append(entry);
            }
        }
        log(L_DEBUG, "Profiles are in: %s", qPrintable(m_rootPath));
        foreach(const QString& s, profiles)
        {
            log(L_DEBUG, "Profile: %s", qPrintable(s));
        }
        return profiles;
    }

    bool ProfileManager::selectProfile(const QString& name)
    {
        if(!enumProfiles().contains(name))
            return false;
        if(!m_currentProfile.isNull() && m_currentProfile->name() == name)
            return true;

        // TODO lock
        QString profile_conf = m_rootPath + QDir::separator() + name + QDir::separator() + "profile.conf";
        QString profile_xml = m_rootPath + QDir::separator() + name + QDir::separator() + "profile.xml";
        QString old_config = m_rootPath + QDir::separator() + name + QDir::separator() + "plugins.conf";
        log(L_DEBUG, "Selecting profile:  %s", profile_conf.toUtf8().data());
        ConfigPtr config = ConfigPtr(new Config(profile_conf));
        //config->load_old();
        QFile f(profile_xml);
        if(f.open(QIODevice::ReadOnly))
        {
            if(!config->deserialize(f.readAll()))
            {
                log(L_WARN, "Unable to deserialize: %s", qPrintable(profile_xml));
                return false;
            }
            f.close();
        }
        else {
            log(L_WARN, "Unable to open: %s", qPrintable(profile_xml));
            config->mergeOldConfig(old_config);
        }
        QDir::setCurrent(m_rootPath + QDir::separator() + name);
        m_currentProfile = ProfilePtr(new Profile(config, name));
        m_currentProfile->loadPlugins();

        return true;
    }

    ProfilePtr ProfileManager::currentProfile()
    {
        return m_currentProfile;
    }

    bool ProfileManager::profileExists(const QString& name) const
    {
        QDir d(rootPath());
        return d.exists(name);
    }

    QString ProfileManager::profilePath()
    {
        if(m_currentProfile.isNull())
            return QString::null;
        return m_rootPath + QDir::separator() + m_currentProfile->name();
    }

    bool ProfileManager::removeProfile(const QString& /*name*/)
    {
        // TODO
        return false;
    }

    bool ProfileManager::renameProfile(const QString& /*oldname*/, const QString& /*newname*/)
    {
        // TODO
        return false;
    }

    bool ProfileManager::newProfile(const QString& name)
    {
        QDir d(m_rootPath);
        if(!d.exists())
            d.mkdir(m_rootPath);
        if(!d.mkdir(name))
            return false;
        return true;
    }

    void ProfileManager::sync()
    {
        if(!m_currentProfile.isNull() && !m_currentProfile->config().isNull())
        {
            QFile f(m_rootPath + QDir::separator() + m_currentProfile->name() + QDir::separator() + "profile.xml");
            f.open(QIODevice::WriteOnly | QIODevice::Truncate);
            f.write(m_currentProfile->config()->serialize());
            f.close();
        }
    }

    QString ProfileManager::currentProfileName()
    {
        if(!m_currentProfile.isNull())
            return m_currentProfile->name();
        return QString::null;
    }

    PropertyHubPtr ProfileManager::getPropertyHub(const QString& name)
    {
        ProfilePtr curProfile = currentProfile();
        if(curProfile.isNull())
            return PropertyHubPtr();
        if(curProfile->config().isNull())
            return PropertyHubPtr();
        PropertyHubPtr hub = curProfile->config()->propertyHub(name);
        if(hub.isNull())
        {
            hub = PropertyHub::create(name);
            curProfile->config()->addPropertyHub(hub);
        }
        return hub;
    }
}

