%define name	sim
%define version	0.9.3
%define release	1mdk

%define major 0
%define libname %mklibname %name %major
%define libnamedev %mklibname %name %major -d

Name:		%name
Summary:        SIM - Multiprotocol Instant Messenger
Version:	%version
Release:	%release
Source:		%{name}-%{version}.tar.bz2
License:	GPL
Group:		Networking/Instant messaging
Url:		http://sim-im.berlios.de/
BuildRoot:	%{_tmppath}/%{name}-buildroot
Requires:	libqt3 > 3.0.4, kdelibs > 3.0, sablotron
BuildRequires:	kdelibs-devel > 3.0
BuildRequires:  autoconf2.5, automake1.7
BuildRequires:	libfam-devel
BuildRequires:  flex, sablotron-devel

%description
SIM - Multiprotocol Instant Messenger

SIM (Simple Instant Messenger) is a plugins-based open-
source instant messenger that supports various protocols
(ICQ, Jabber, AIM, MSN). It uses the QT library and works
on X11 (with optional KDE-support).

SIM has a lot of features, many of them are listed
at: http://sim-im.berlios.de/

%package -n %libname
Summary:	SIM library
Group:		System/Libraries

%description -n %libname
SIM (Simple Instant Messenger) is a plugins-based open-
source instant messenger that supports various protocols
(ICQ, Jabber, AIM, MSN). It uses the QT library and works
on X11 (with optional KDE-support).

Libraries

%package -n %libnamedev
Summary:        SIM library
Group:          System/Libraries
Requires:	%libname = %version
Provides:	libsim-devel

%description -n %libnamedev
SIM (Simple Instant Messenger) is a plugins-based open-
source instant messenger that supports various protocols
(ICQ, Jabber, AIM, MSN). It uses the QT library and works
on X11 (with optional KDE-support).

Devel files

%prep
rm -rf $RPM_BUILD_ROOT

%setup -q

%build
WANT_AUTOCONF_2_5=1 gmake -f admin/Makefile.common

# mdk libtool doesn't know the --tag option :-(
for i in `find sim/ -name Makefile.in`; do perl -pi -e 's/^(LTCXXCOMPILE.*?)\s*--tag=CXX/$1/' "$i"; done
for i in `find sim/ -name Makefile.in`; do perl -pi -e 's/^(CXXLINK.*?)\s*--tag=CXX/$1/' "$i"; done
for i in `find plugins/ -name Makefile.in`; do perl -pi -e 's/^(LTCXXCOMPILE.*?)\s*--tag=CXX/$1/' "$i"; done
for i in `find plugins/ -name Makefile.in`; do perl -pi -e 's/^(CXXLINK.*?)\s*--tag=CXX/$1/' "$i"; done

%configure --disable-rpath
%make

%install
%makeinstall

# Menu
mkdir -p %buildroot/%_menudir
cat > %buildroot/%_menudir/%name  <<EOF
?package(%name): command="%_bindir/%name" needs="X11" \
icon="%name.png" section="Networking/Instant messaging" \
title="SIM" longtitle="Simple Instant Messenger."
EOF

%{find_lang} %{name}

%post
%{update_menus}

%post -n %libname -p /sbin/ldconfig

%postun
%{clean_menus}

%postun -n %libname -p /sbin/ldconfig

%files -f %name.lang
%defattr(-,root,root,0755)
%{_bindir}/sim
%{_datadir}/apps/%name
%{_datadir}/applnk-mdk/Internet/sim.desktop
%{_iconsdir}/hicolor/*/*/*
%{_iconsdir}/locolor/*/*/*
%doc ChangeLog README
%{_menudir}/*

%files -n %libname
%defattr(-,root,root,0755)
%{_libdir}/*.so.*

%files -n %libnamedev
%defattr(-,root,root,0755)
%{_libdir}/*.so
%{_libdir}/*.la

%clean
rm -r $RPM_BUILD_ROOT

%changelog
* Fri Jan 02 2004 Robert Scheck <sim@robert-scheck.de> 0.9.3-1mdk
- Upgrade to 0.9.3
- Added sablotron and flex to requirements

* Sun Nov 23 2003 Robert Scheck <sim@robert-scheck.de> 0.9.2-1mdk
- Upgrade to 0.9.2

* Wed Nov 05 2003 Robert Scheck <sim@robert-scheck.de> 0.9.1-1mdk
- Upgrade to 0.9.1
- Initial spec file based on the official one from Mandrake
