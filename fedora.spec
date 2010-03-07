%define rh_release %(rh_release="`rpm -q --queryformat='%{VERSION}' redhat-release | grep -v install 2>/dev/null`" ; if test $? != 0 ; then rh_release="0" ; fi ; echo "$rh_release")
%define fdr_release %(fdr_release="`rpm -q --queryformat='%{VERSION}' fedora-release | grep -v install 2>/dev/null`" ; if test $? != 0 ; then fdr_release="0" ; fi ; echo "$fdr_release")
%if %{!?_without_KDE:0}%{?_without_KDE:1}
%define with_kde 0
%else
%define with_kde 1
%endif
%define release 2

Name: 		sim
Version: 	0.9.3
%if %{rh_release}
Release: 	%{release}.rh%(dist_release="`echo "%{rh_release} * 10" | bc 2>/dev/null`" ; echo "$dist_release")
Distribution:	Red Hat Linux %{rh_release}
%else
Release:	%{release}.fdr%(dist_release="`echo "%{fdr_release} * 10" | bc 2>/dev/null`" ; echo "$dist_release")
Distribution:	Fedora Core %{fdr_release}
%endif
Vendor: 	Vladimir Shutoff <vovan@shutoff.ru>
Packager:	Robert Scheck <sim@robert-scheck.de>
Summary:  	SIM - Multiprotocol Instant Messenger
Summary(de):	SIM - Multiprotokoll Instant Messenger
License: 	GPL
Group: 		Applications/Internet
URL: 		http://sim-im.berlios.de/
Source0: 	%{name}-%{version}.tar.gz
BuildRequires:	autoconf >= 2.52, automake >= 1.5
BuildRequires:  gcc, gcc-c++, XFree86-devel, zlib-devel, libjpeg-devel, expat-devel, flex, libart_lgpl-devel, libpng-devel, gettext
%if %{with_kde}
BuildRequires:  kdelibs-devel >= 3.0.0
Requires:       kdebase >= 3.0.0, kdelibs >= 3.0.0
%endif
BuildRequires:  qt-devel >= 3.0.0, openssl-devel, pcre-devel >= 3.9, arts-devel >= 1.0, libxml2-devel, libxslt-devel
Requires:       qt >= 3.0.0, openssl, arts >= 1.0, libxml2, libxslt
BuildRoot: 	%{_tmppath}/%{name}-%{version}-root

%description -l de
SIM - Multiprotokoll Instant Messenger

SIM (Simple Instant Messenger) ist ein Plugin-basierender
open-source Instant Messenger, der verschiedene Protokolle
(ICQ, Jabber, AIM, MSN, LiveJournal, Yahoo!) unterstützt. 
Dafür wird die QT-Bibliothek und X11 (mit optionaler KDE-
Unterstützung) verwendet.

SIM hat sehr unzählige Features, viele von diesen sind
aufgelistet unter: http://sim-im.berlios.de/

%description
SIM - Multiprotocol Instant Messenger

SIM (Simple Instant Messenger) is a plugins-based open-
source instant messenger that supports various protocols
(ICQ, Jabber, AIM, MSN, LiveJournal, Yahoo!). It uses the 
QT library and works on X11 (with optional KDE support).

SIM has countless features, many of them are listed at:
http://sim-im.berlios.de/

%prep
%setup -q
make -f admin/Makefile.common
CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS" 

%configure \
%if %{with_kde}
	--enable-kde \
%else
	--disable-kde \
%endif
	$LOCALFLAGS

%build
# Setup for parallel builds
numprocs=`egrep -c ^cpu[0-9]+ /proc/stat || :`
if [ "$numprocs" = "0" ]; then
  numprocs=1
fi

make -j $numprocs

%install
make install-strip DESTDIR=$RPM_BUILD_ROOT
%find_lang %{name}

%clean
rm -rf $RPM_BUILD_ROOT
rm -rf $RPM_BUILD_DIR/%{name}-%{version}

%files -f %{name}.lang
%defattr(-, root, root)
%doc AUTHORS COPYING ChangeLog README* TODO INSTALL
%{_bindir}/sim*
%{_libdir}/libsim*
%{_libdir}/menu/
%{_libdir}/sim/
%{_datadir}/applnk-redhat/Internet/sim.desktop
%{_datadir}/apps/
%{_datadir}/icons/*/*/*/*
%{_datadir}/mimelnk/
%{_datadir}/services/

%changelog
* Sat Apr 03 2004 - Robert Scheck <sim@robert-scheck.de> - 0.9.3-2
- Upgrade to 0.9.3-2 (second 0.9.3 release)

* Wed Mar 31 2004 - Robert Scheck <sim@robert-scheck.de> - 0.9.3-1
- Upgrade to 0.9.3
- Made the KDE support conditional
- Merged Red Hat Linux spec file into Fedora Core spec file

* Fri Dec 26 2003 - Robert Scheck <sim@robert-scheck.de> - 0.9.2-1
- Upgrade to 0.9.2
- Added sablotron to requirements

* Wed Nov 05 2003 - Robert Scheck <sim@robert-scheck.de> - 0.9.1-1
- Upgrade to 0.9.1

* Tue Oct 28 2003 - Robert Scheck <sim@robert-scheck.de> - 0.9.0-1
- Upgrade to 0.9.0
- Adapted spec file from Red Hat Linux

