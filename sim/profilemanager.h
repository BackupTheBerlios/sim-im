
#ifndef PROFILEMANAGER_H
#define PROFILEMANAGER_H

#include <QString>

#include "cfg.h"
#include "singleton.h"
#include "profile.h"

namespace SIM
{
	class EXPORT ProfileManager : public Singleton<ProfileManager>
	{
	public:
		ProfileManager(const QString& rootpath);
		virtual ~ProfileManager();

		QStringList enumProfiles();

		bool selectProfile(const QString& name);

		ProfilePtr currentProfile();
		QString currentProfileName();

		QString profilePath();

		QString rootPath() { return m_rootPath; }

		void removeProfile(const QString& name);

		void renameProfile(const QString& oldname, const QString& newname);

		bool newProfile(const QString& name);

		void sync();

		PropertyHubPtr getPropertyHub(const QString& name);
	private:
		QString m_rootPath;
		ProfilePtr m_currentProfile;
	};
}

#endif

