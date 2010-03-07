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

#include "skypechatsession.h"
#include "skypeaccount.h"
#include "skypeprotocol.h"
#include "skypecontact.h"

#include <kdebug.h>
#include <SIMchatsessionmanager.h>
#include <SIMmetacontact.h>
#include <qdict.h>
#include <qstring.h>
#include <kaction.h>
#include <klocale.h>
#include <kgenericfactory.h>

static SIM::MetaContact *dummyContacts = new SIM::MetaContact();

class ChatDummyContact : public SIM::Contact {
	public:
		ChatDummyContact(SkypeAccount *account, const QString &name) : SIM::Contact(account, name, dummyContacts) {};
		virtual SIM::ChatSession *manager (CanCreateFlags canCreate) {return 0L;};
};

class SkypeChatSessionPrivate {
	private:
		///Dummy contact representing this chat
		SIM::Contact *dummyContact;
	public:
		///Referenco to the protocol
		SkypeProtocol *protocol;
		///Reference to the account
		SkypeAccount *account;
		///Am I connected to the messageSent signal?
		bool connectedSent;
		///ID of this chat session
		QString chatId;
		/**
		 * Constructor
		 * @param _protocol Reference to the Skype protocol
		 * @param _account Reference to the account this chat belongs to
		 */
		SkypeChatSessionPrivate(SkypeProtocol *_protocol, SkypeAccount *_account) {
			kdDebug(14311) << k_funcinfo << endl;//some debug info
			//save given values
			account = _account;
			protocol = _protocol;

			connectedSent = false;
			chatId = "";
			dummyContact = 0L;
		};
		///Is it multi-user chat?
		bool isMulti;
		///Please give me a contact that stands for the whole chat so I can send it to it
		SIM::Contact *getDummyContact() {
			if (dummyContact)
				return dummyContact;
			else {
				return dummyContact = new ChatDummyContact(account, chatId);
			}
		};
		///The action to call the user(s)
		KAction *callAction;
		///The contact if any (and one)
		SkypeContact *contact;
};

static SIM::ContactPtrList constructList(SkypeContact *contact) {
	SIM::ContactPtrList list;//create the contact
	list.append(contact);//add there the contact

	return list;//and return the list
}

SkypeChatSession::SkypeChatSession(SkypeAccount *account, SkypeContact *contact) :
		SIM::ChatSession(account->myself(), constructList(contact), account->protocol(), (char *)0L) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	setInstance(KGenericFactory<SkypeProtocol>::instance());

	//create the D-pointer
	d = new SkypeChatSessionPrivate(account->protocol(), account);
	SIM::ChatSessionManager::self()->registerChatSession( this );
	connect(this, SIGNAL(messageSent(SIM::Message&, SIM::ChatSession*)), this, SLOT(message(SIM::Message& )));//this will send the messages from this user going out
	account->prepareChatSession(this);
	d->isMulti = false;

	d->callAction = new KAction(i18n("Call"), QString::fromLatin1("call"), 0, this, SLOT(callChatSession()), actionCollection(), "callSkypeContactFromChat");
	connect(contact, SIGNAL(setCallPossible(bool )), d->callAction, SLOT(setEnabled(bool )));
	connect(this, SIGNAL(becameMultiChat(const QString&, SkypeChatSession* )), this, SLOT(disallowCall()));

	d->contact = contact;

	setMayInvite(true);//It is possible to invite people to chat with Skype
	setXMLFile("skypechatui.rc");
}

SkypeChatSession::SkypeChatSession(SkypeAccount *account, const QString &session, const SIM::ContactPtrList &users) :
		SIM::ChatSession(account->myself(), users, account->protocol(), (char *) 0L) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	setInstance(KGenericFactory<SkypeProtocol>::instance());

	d = new SkypeChatSessionPrivate(account->protocol(), account);
	SIM::ChatSessionManager::self()->registerChatSession(this);
	connect(this, SIGNAL(messageSent(SIM::Message&, SIM::ChatSession*)), this, SLOT(message(SIM::Message& )));
	account->prepareChatSession(this);
	d->isMulti = true;
	d->chatId = session;
	emit updateChatId("", session, this);


	d->callAction = new KAction(i18n("Call"), QString::fromLatin1("call"), 0, this, SLOT(callChatSession()), actionCollection(), "callSkypeContactFromChat");
	disallowCall();//TODO I hope it will not be needed in future

	setMayInvite(true);//It is possible to invite people to chat with Skype
	setXMLFile("skypechatui.rc");
}

SkypeChatSession::~SkypeChatSession() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	if (d->account->leaveOnExit() && (d->isMulti))
		emit leaveChat(d->chatId);
	emit updateChatId(d->chatId, "", this);
	delete d;//remove the D pointer
}

void SkypeChatSession::message(SIM::Message &message) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	d->account->registerLastSession(this);
	d->account->sendMessage(message, (d->isMulti) ? (d->chatId) : "");//send it
	messageSucceeded();
}

void SkypeChatSession::setTopic(const QString &chat, const QString &topic) {
	///TODO This function
}

void SkypeChatSession::joinUser(const QString &chat, const QString &userId) {
	kdDebug(14311) << k_funcinfo << "Chat: " << chat << endl;//some debug info

	if (chat == d->chatId) {
		addContact(d->account->getContact(userId));
		d->isMulti = true;
		emit becameMultiChat(d->chatId, this);
	}
}

void SkypeChatSession::leftUser(const QString &chat, const QString &userId, const QString &reason) {
	kdDebug(14311) << "User: " << userId<< k_funcinfo << endl;//some debug info

	if (chat == d->chatId) {
		removeContact(d->account->getContact(userId), reason, SIM::Message::PlainText);
	}
}

void SkypeChatSession::setChatId(const QString &chatId) {
	kdDebug(14311) << k_funcinfo << "ID: " << chatId << endl;//some debug info

	if (d->chatId != chatId) {
		emit updateChatId(d->chatId, chatId, this);
		d->chatId = chatId;
		emit wantTopic(chatId);
	}
}

void SkypeChatSession::sentMessage(const QPtrList<SIM::Contact> *recv, const QString &body) {
	SIM::Message *mes;
	/*if (recv->count() == 1) {
		mes = new SIM::Message(d->account->myself(), *recv->begin(), body, SIM::Message::Outbound);
	} else {
		mes = new SIM::Message(d->account->myself(), d->account->myself(), body, SIM::Message::Outbound);
	}*/
	mes = new SIM::Message(d->account->myself(), *recv, body, SIM::Message::Outbound);
	appendMessage(*mes);
	delete mes;
}

void SkypeChatSession::disallowCall() {
	d->callAction->setEnabled(false);

	if (d->contact) {
		disconnect(d->contact, SIGNAL(setCallPossible(bool )), d->callAction, SLOT(setEnabled(bool )));
		d->contact = 0L;
	}
}

void SkypeChatSession::callChatSession() {
	if (d->contact)///@todo find a better way to do it later to allow multiple people to call
		d->contact->call();
}

void SkypeChatSession::inviteContact(const QString &contactId) {
	if (d->chatId.isEmpty()) {
		d->chatId = d->account->createChat(d->contact->contactId());
		emit updateChatId("", d->chatId, this);
	}

	emit inviteUserToChat(d->chatId, contactId);
}

#include "skypechatsession.moc"
