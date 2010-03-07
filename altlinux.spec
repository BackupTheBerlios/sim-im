%define versuffix %nil

%def_enable simqt
%def_enable simkde

%if_enabled simqt
%define simqtinstalldir %_builddir/%name-%version/qtinstalldir/
%endif

%if_enabled simkde
%define siminstalldir %_builddir/%name-%version/installdir/
%endif

Name: sim
Version: 0.9.5
Release: alt0.2
Serial: 1

Group: Networking/Instant messaging
Summary: SIM - Simple Instant Messenger (with KDE support)
Summary(ru_RU.CP1251): SIM - клиент ICQ/AIM/MSN/Jabber (с поддержкой KDE)
License: GPL
Packager: SIM Development Team <sim@packages.altlinux.org>

Url: http://sim-im.org/

Source: %name-%version%versuffix.tar.bz2

Patch0: %name-alt-play_wrapper.patch

BuildPreReq: cmake >= 2.4.4

BuildPreReq: libXScrnSaver-devel

BuildPreReq: gcc-c++ flex libqt3-devel
BuildPreReq: libssl-devel libxslt-devel zip
BuildPreReq: libpng-devel 
BuildPreReq: libqt3-devel-cxx = %__gcc_version_base

%if_enabled simkde
BuildPreReq: kdelibs-devel
BuildPreReq: kdelibs-devel-cxx = %__gcc_version_base
%endif
%if_enabled simqt
BuildPreReq: libaspell-devel
%endif

%if_enabled simkde
Requires: %name-common >= %version-%release
Requires: sound_handler

Obsoletes: libsim sim-plugins
#Provides: libsim sim-plugins
Conflicts: libsim-qt
Conflicts: sim-qt < 0.9.3-alt0.2
%endif

%description
A simple ICQ client for X win system (requires QT,
can be build for KDE). It also runs under MS Windows.
Supported protocols: ICQ v8 (2001), Jabber, MSN, AIM, YIM.

This package contains version built with KDE support.

%description -l ru_RU.CP1251
Кроссплатформенный, многопротокольный клиент обмена мгновенными
сообщениями (требует Qt, может быть собран с поддержкой KDE).
Поддерживаются протоколы ICQ, Jabber, MSN, AIM, YIM, а также
LiveJournal. Кроме того, имеется множество плагинов, реализующих
дополнительные возможности.

Данный пакет содержит версию, собранную с поддержкой KDE.

%if_enabled simqt
%package qt
Group: Networking/Instant messaging
Summary: SIM - Simple Instant Messenger (without KDE support)
Summary(ru_RU.CP1251): SIM - клиент ICQ/AIM/MSN/Jabber (без поддержки KDE)

Requires: %name-common >= %version-%release
Requires: sound_handler

Obsoletes: libsim-qt sim-qt-plugins
#Provides: libsim-qt sim-qt-plugins
Conflicts: libsim
Conflicts: sim < 0.9.3-alt0.2

%description qt
A simple ICQ client for X win system (requires QT,
can be build for KDE). It also runs under MS Windows.
Supported protocols: ICQ v8 (2001), Jabber, MSN, AIM, YIM.

This package contains version built without KDE support.

%description qt -l ru_RU.CP1251
Кроссплатформенный, многопротокольный клиент обмена мгновенными
сообщениями (требует Qt, может быть собран с поддержкой KDE).
Поддерживаются протоколы ICQ, Jabber, MSN, AIM, YIM, а также
LiveJournal. Кроме того, имеется множество плагинов, реализующих
дополнительные возможности.

Данный пакет содержит версию, собранную без поддержки KDE.
%endif

%package common
Group: Networking/Instant messaging
Summary: SIM - Simple Instant Messenger (data files)
Summary(ru_RU.CP1251): SIM - клиент ICQ/AIM/MSN/Jabber (файлы данных)

Obsoletes: sim-data sim-qt-data
#Provides: sim-data sim-qt-data
Conflicts: sim < 0.9.0
Conflicts: sim-qt < 0.9.0

%description common
A simple ICQ client for X win system (requires QT,
can be build for KDE). It also runs under MS Windows.
Supported protocols: ICQ v8 (2001), Jabber, MSN, AIM, YIM.

This package contains common files for both sim and sim-qt.

%description common -l ru_RU.CP1251
Кроссплатформенный, многопротокольный клиент обмена мгновенными
сообщениями (требует Qt, может быть собран с поддержкой KDE).
Поддерживаются протоколы ICQ, Jabber, MSN, AIM, YIM, а также
LiveJournal. Кроме того, имеется множество плагинов, реализующих
дополнительные возможности.

Данный пакет содержит файлы данных, необходимые для sim и sim-qt.

%prep
%if_disabled simqt
%if_disabled simkde
echo "Error: one of simkde and simqt must be enabled"
exit 1
%endif
%endif

%setup #-n %name

%patch0 -p1

%build
## Without KDE ##
%if_enabled simqt
mkdir simqt
pushd simqt
cmake %_builddir/%name-%version \
	-DCMAKE_C_FLAGS:STRING="%optflags" \
	-DCMAKE_CXX_FLAGS:STRING="%optflags" \
        -DCMAKE_INSTALL_PREFIX=%_prefix \
        -DCMAKE_SKIP_RPATH=YES \
        -DUSE_GCC_VISIBILITY=1 \
        -DENABLE_KDE3=0 \
        -DSIM_FLAVOUR="-qt"
%make_build
%makeinstall DESTDIR=%simqtinstalldir
popd
%endif

## With KDE ##
%if_enabled simkde
mkdir simkde
pushd simkde
cmake %_builddir/%name-%version \
	-DCMAKE_C_FLAGS:STRING="%optflags" \
	-DCMAKE_CXX_FLAGS:STRING="%optflags" \
        -DCMAKE_INSTALL_PREFIX=%_prefix \
        -DCMAKE_SKIP_RPATH=YES \
        -DUSE_GCC_VISIBILITY=1 \
        -DENABLE_KDE3=1
%make_build
%makeinstall DESTDIR=%siminstalldir
%endif

%install
mkdir -p %buildroot/
cp -a %siminstalldir/* %buildroot/
cp -a %simqtinstalldir/* %buildroot/

%if_enabled simqt
cp %buildroot%_desktopdir/kde/%name.desktop %buildroot%_desktopdir/%name-qt.desktop
%__subst 's,^Exec=sim$,\0-qt,' %buildroot%_desktopdir/%name-qt.desktop
%__subst 's,^Name.*=Sim.*,\0 (without KDE),g' %buildroot%_desktopdir/%name-qt.desktop
%__subst '\,Categ,s,KDE;,,' %buildroot%_desktopdir/%name-qt.desktop
%endif

rm -rf %buildroot%_libdir/libsim.so
rm -rf %buildroot%_libdir/libsim-qt.so

%find_lang %name

%if_enabled simkde
%post
%post_ldconfig
%update_menus
%postun
%postun_ldconfig
%clean_menus
%endif

%if_enabled simqt
%post qt
%post_ldconfig
%update_menus
%postun qt
%postun_ldconfig
%clean_menus
%endif

%if_enabled simkde
%files
%_bindir/%name
%_desktopdir/kde/%name.desktop
%_libdir/libsim.so.*
%dir %_libdir/%name
%_libdir/%name/*.so*
%_libdir/%name/styles/
%endif

%if_enabled simqt
%files qt
%_bindir/%name-qt
%_desktopdir/%name-qt.desktop
%_libdir/libsim-qt.so.*
%dir %_libdir/%name-qt
%_libdir/%name-qt/*.so*
%_libdir/%name-qt/styles/
%endif

%files common -f %name.lang
%_bindir/simctrl
%_datadir/apps/%name
%_datadir/services/simctrl.desktop
%_iconsdir/*/*/*/*.png

%changelog
* Tue Feb 20 2007 Andrey Rahmatullin <wrar@altlinux.ru> 1:0.9.5-alt0.2
- 0.9.5 r1860

* Thu Jan 04 2007 Andrey Rahmatullin <wrar@altlinux.ru> 1:0.9.5-alt0.1
- 0.9.5 r1738
- Daedalus build
- use CMake
- spec cleanup

* Sat Oct 21 2006 Andrey Rahmatullin <wrar@altlinux.ru> 1:0.9.4.1-alt1
- 0.9.4.1
- spec cleanup
- update BuildRequires

* Thu Jul 06 2006 Andrey Rahmatullin <wrar@altlinux.ru> 1:0.9.4-alt14
- bump release

* Thu Jun 29 2006 Andrey Rahmatullin <wrar@altlinux.ru> 1:0.9.4-alt13
- 0.9.4 release, finally :)

* Wed May 03 2006 Andrey Rahmatullin <wrar@altlinux.ru> 1:0.9.4-alt12
- 0.9.4 RC2

* Sat Feb 25 2006 Andrey Rahmatullin <wrar@altlinux.ru> 1:0.9.4-alt11
- 0.9.4 RC1
- update URL
- remove menu file

* Mon Apr 11 2005 Andrey Rahmatullin <wrar@altlinux.ru> 1:0.9.4-alt10
- update from CVS 20050411

* Sat Dec 11 2004 Andrey Rahmatullin <wrar@altlinux.ru> 1:0.9.4-alt9
- update from CVS 20041211

* Thu Oct 07 2004 Andrey Rahmatullin <wrar@altlinux.ru> 1:0.9.4-alt8.1
- remove %_datadir/mimelnk/application/x-icq.desktop from sim-common
  (#5278)

* Tue Sep 21 2004 Andrey Rahmatullin <wrar@altlinux.ru> 1:0.9.4-alt8
- CVS 20040921
- build autoaway plugin since it now doesn't crash
- add ability to build packages for Master 2.2 (use --enable M22)
- all fixes gone upstream

* Sat Aug 14 2004 Andrey Rahmatullin <wrar@altlinux.ru> 1:0.9.4-alt7
- add Packager: sim@packages.a.o
- do `make clean' between two builds (fixes some segfaults in sim-kde)
- add %%{post,postun}_ldconfig
- update Russian translation for sim-qt
- some fixes

* Fri Aug 06 2004 Andy Gorev <horror@altlinux.ru> 1:0.9.4-alt6
- %%packager removed from spec, use ~/.rpmmacros please
- BuildPreReq and BuildReq fixed for build in hasher

* Mon Aug 02 2004 Andrey Rahmatullin <wrar@altlinux.ru> 1:0.9.4-alt5
- fix buildreqs
- fix Patch4
- fix Russian translation

* Mon Jul 26 2004 Andrey Rahmatullin <wrar@altlinux.ru> 1:0.9.4-alt4
- update translations from CVS 20040726
- add requires: sound_handler (#4314)

* Thu Jul 22 2004 Andrey Rahmatullin <wrar@altlinux.ru> 1:0.9.4-alt3.1
- update buildreqs
- update obsoletes/conflicts

* Fri Jul 16 2004 Andrey Rahmatullin <wrar@altlinux.ru> 1:0.9.4-alt3
- CVS 20040716
- rebuilt without kdelibs-gcc_compiled (#4436)
- menu files fixed (#4312)
- default sound player changed from play to play_wrapper.sh (#4314)
- .so files removed (#4672)
- include all locales (see `fortune ALT -m '19 .*\?'` :))
- Russian summaries and descriptions added
- add ability to build only sim or sim-qt
  (use --disable simqt or --disable simkde)
- do not build autoaway plugin, since it segfaults on exit
- a lot of new features and bugs :(

* Tue May 11 2004 ALT QA Team Robot <qa-robot@altlinux.org> 1:0.9.4-alt2.1
- Rebuilt with openssl-0.9.7d.

* Tue Apr 27 2004 Albert R. Valiev <darkstar@altlinux.ru> 1:0.9.4-alt2
- Fixed contact list information retrieve problem

* Fri Apr 23 2004 Albert R. Valiev <darkstar@altlinux.ru> 1:0.9.4-alt1
- Change Jabber browser
- MSN: Add hotmail notification
- GPG: Add support passphrase
- ICQ: Add packets flow control
- ICQ: Fix send URL and contacts
- ICQ: Fix send large messages
- Redesign search window

* Mon Mar 29 2004 Albert R. Valiev <darkstar@altlinux.ru> 1:0.9.3-alt2
- Fixed Jabber plugin

* Thu Mar 25 2004 Albert R. Valiev <darkstar@altlinux.ru> 1:0.9.3-alt1
- Fixed build scheme
- Release build
- Add LiveJournal plugin
- Jabber: fix send rich-text messages
- ICQ: Fix set birthday flag
- Fix apply custom fonts
- Fix communication-problem with AIM-Screennames > 13 chars
- Fix not receive AuthGrant from some ICQ-users
- Proxy: add support listen sockets for SOCKS4 and SOCKS5
- Add Yahoo! plugin
- MSN: Fix remove contact
- Add replace text plugin
- Add option "Show/hide empty groups"
- Add weather plugin
- Move plugins from prefix/share into prefix/lib
- Fix acinclude for automake-1.8
- MSN: add HTTP polling
- Jabber: add HTTP polling
- OSD: add show message content
- Jabber: add select resource for send

* Thu Mar 18 2004 Albert R. Valiev <darkstar@altlinux.ru> 1:0.9.3-alt0.2
- Build from CVS 20040318
- build scheme change (3 packages from 1 source RPM)
- removed kdebase from requires (#3651)
- removed /usr/share/locale/ru etc. from package (#3502)
- rename libs for kde-disabled build (to make possible installing both builds
  simultaneously)
- Very big thanks to Andrey Rahmatullin <wrar@altlinux.ru> for new scheme patches

* Wed Mar 10 2004 Albert R. Valiev <darkstar@altlinux.ru> 1:0.9.3-alt0.1
- Build from CVS 20040310
- Fixed buildrequres

* Tue Nov 04 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.9.0-alt2
- Fixed login rate bug in icq plugin

* Mon Nov 03 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.9.0-alt1
- sim-0.9.0 release

* Mon Oct 27 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.9.0-alt0.11
- Rebuild with 20031027 cvs snapshot

* Tue Sep 23 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.9.0-alt0.10
- Fixed conflicts (thanks to Sergey V. Turchin <zerg@altlinux.org> for help

* Mon Sep 22 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.9.0-alt0.9
- Now data files are in sim-common package
- Now plugins are in sim-plugins package
- Fixed conflicts and deps

* Sun Sep 21 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.9.0-alt0.8
- Added conflicts with sim-qt, libsim-qt, sim-qt-data
- Build from cvs 20030921

* Fri Sep 19 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.9.0-alt0.7
- Fixed depends

* Thu Sep 18 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.9.0-alt0.6
- Fixed buildrequires
- Enabled Jabber protocol build

* Thu Sep 18 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.9.0-alt0.5
- Build from CVS 20030918
- translation fixes
- Makefile fixes

* Sun Sep 14 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.9.0-alt0.4
- Build from CVS 20030914

* Tue Sep 09 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.9.0-alt0.3
- Build from CVS 20030909

* Wed Sep 03 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.9.0-alt0.2
- Build from CVS 20030903

* Fri Aug 29 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.9.0-alt0.1
- Build from CVS 20030828

* Tue Aug 12 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.8.3-alt1
- 0.8.3 Release build

* Sat Jun 14 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.8.2-alt1
- Release build

* Mon Apr 21 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.8.2-alt0.6
- BuildRequires fix

* Tue Apr 15 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.8.2-alt0.5
- Fixed crash at startup

* Sun Apr 13 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.8.2-alt0.4
- New cvs snapshot build
- Fixed bug #0002387

* Fri Apr 04 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.8.2-alt0.3
- Test for bug with libqt3-3.1.2

* Mon Mar 17 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.8.2-alt0.2
- Build cvs snapshot for Sisyphus
- Add Bulgarian translation
- Add Hebrew translation
- Add random chat
- Add settings for groups
- Add GKrellM2 plugin
- Add French translation

* Mon Feb 24 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.8.2-alt0.1
- Build cvs snapshot for Daedalus

* Sun Feb 02 2003 Albert R. Valiev <darkstar@altlinux.ru> 1:0.8.1-alt1
- downgrade to sim-0.8.1

* Mon Jan 20 2003 Albert R. Valiev <darkstar@altlinux.ru> 0.9-alt0.5
- Build cvs snapshot (2003.01.20)

* Sat Jan 04 2003 Albert R. Valiev <darkstar@altlinux.ru> 0.9-alt0.4
- Build cvs snapshot (2003.01.05)

* Thu Nov 28 2002 Albert R. Valiev <darkstar@altlinux.ru> 0.9-alt0.3
- Build CVS Snaphot (fixed some bugs)

* Sat Nov 16 2002 Albert R. Valiev <darkstar@altlinux.ru> 0.9-alt0.2
- menu files attr corrections

* Tue Nov 12 2002 Albert R. Valiev <darkstar@altlinux.ru> 0.9-alt0.1
- Build CVS snapshot (because current version incompatible with glibc)

* Sat Oct 26 2002 Albert R. Valiev <darkstar@altlinux.ru> 0.8-alt1
- New version

* Wed Oct 9 2002 Albert R. Valiev <darkstar@altlinux.ru> 0.7-alt3
- Spec corrections

* Wed Oct 9 2002 Albert R. Valiev <darkstar@altlinux.ru> 0.7-alt2
- Rebuild with new KDE requirements

* Thu Sep 12 2002 Sergey V Turchin <zerg@altlinux.ru> 0.7-alt1
- initial spec
