/*  This file is part of the KDE project
    Copyright (C) 2005 Michal Vaner <michal.vaner@kdemail.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef SKYPEPROTOCOL_H
#define SKYPEPROTOCOL_H

#include "SIMprotocol.h"
#include <qstring.h>

class SkypeAccount;
class SkypeProtocolPrivate;

namespace SIM {
	class OnlineStatus;
};

#define LAUNCH_ALLWAYS 0
#define LAUNCH_NEEDED 1
#define LAUNCH_NEVER 2

/**
 * @author Michal Vaner
 * @short Protocol to use external skype
 * This protocol is only binding for exteral skype program. The reason to write this was I did not like the skype as it was.
 */
class SkypeProtocol : public SIM::Protocol
{
	Q_OBJECT
	private:
		SkypeProtocolPrivate *d;
	public:
		const SIM::OnlineStatus Offline;
		const SIM::OnlineStatus Online;
		const SIM::OnlineStatus SkypeMe;
		const SIM::OnlineStatus Away;
		const SIM::OnlineStatus NotAvailable;
		const SIM::OnlineStatus DoNotDisturb;
		const SIM::OnlineStatus Invisible;
		const SIM::OnlineStatus Connecting;
		const SIM::OnlineStatus NotInList;
		const SIM::OnlineStatus NoAuth;
		const SIM::OnlineStatus Phone;
		// contact properties
/*		const SIM::ContactPropertyTmpl propAwayMessage;
		const SIM::ContactPropertyTmpl propFirstName;
		const SIM::ContactPropertyTmpl propLastName;*/
		const SIM::ContactPropertyTmpl propFullName;
// 		const SIM::ContactPropertyTmpl propEmailAddress;
		const SIM::ContactPropertyTmpl propPrivatePhone;
		const SIM::ContactPropertyTmpl propPrivateMobilePhone;
		const SIM::ContactPropertyTmpl propWorkPhone;
// 		const SIM::ContactPropertyTmpl propWorkMobilePhone;
		const SIM::ContactPropertyTmpl propLastSeen;
		/**
		 * Constructor. This is called automatically on library load.
		 * @param parent Parent of the object.
		 * @param name Name of the object.
		 * @param args Arguments to allow creation by KGenericFactory.
		 * @see KGenericFactory
		 */
		SkypeProtocol(QObject *parent, const char *name, const QStringList &args);
		/**
		 * Destructor.
		 */
		~SkypeProtocol();
		/**
		 * Reimplementation of the methot that creates a new skype account.
		 * @param accountID ID of the account.
		 * @return At the moment NULL, but it will change soon.
		 */
		virtual SIM::Account *createNewAccount(const QString &accountID);
		/**
		 * Reimplementation of the method that creates widget for adding contact to skype account.
		 * @param parent Parent widget. It will be showed inside.
		 * @param account Account to witch it aplies.
		 * @return At the moment NULL, but it will change soon.
		 */
		virtual AddContactPage *createAddContactWidget(QWidget *parent, SIM::Account *account);
		/**
		 * Reimplementation of the method that creates widget for editing/creation of the skype account.
		 * @param account Account to what it applies. (0 means we create a new one)
		 * @param parent Parent widget. It will be showed inside it.
		 * @return NULL at the moment, but it will change soon.
		 */
		virtual SIMEditAccountWidget* createEditAccountWidget(SIM::Account *account, QWidget *parent);
		/**
		 * Skype plugin allows only one skype account at once. This answers weather one exists or not.
		 * @return true if some account exists and false if not
		 */
		bool hasAccount() const;
		/**
		 * Tells skype to remember this account
		 * @param account Pointer to the instance of the account
		 */
		void registerAccount(SkypeAccount *account);
		/**
		 * Removes account is some exists
		 */
		void unregisterAccount();
		/**
		 * Creates a contact from provided data
		 * @param metaContact Metacontact to add the contact into
		 * @param serializedData Some data to store the contact
		 * @param addressBookData Data inside the address book
		 * @return Brand new loaded contact
		 */
		virtual SIM::Contact *deserializeContact(SIM::MetaContact *metaContact, const QMap<QString, QString> &serializedData, const QMap<QString, QString> &addressBokkData);
	public slots:
		/**
		 * This enables or disables the "Call by skype" action depending on weather a contact(s) are selected and have skype contacts
		 */
		void updateCallActionStatus();
		/**
		 * This calls all selected skype contacts
		 */
		void callContacts();
};

#endif
