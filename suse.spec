# spec created by Crissi
%define suse_release %(suse_release="`rpm -q --queryformat='%{VERSION}' suse_release | grep -v install 2>/dev/null`" ; if test $? != 0 ; then suse_release="0" ; fi ; echo "$suse_release")
%define release %(release="`echo "%{suse_release} * 10" | bc 2>/dev/null`" ; if test $? != 0 ; then release="" ; fi ; echo "$release")
%define build_release 2

%if %{!?_without_KDE:0}%{?_without_KDE:1}
%define with_kde 0
%else
%define with_kde 1
%endif
%define release 1

Name:           sim
Version:        0.9.3
Release:        %{build_release}.suse%{release}
Vendor:         Vladimir Shutoff <shutoff@mail.ru>
Packager:       Christian Ehrlicher <Ch.Ehrlicher@gmx.de>
Summary:        SIM - Multiprotocol Instant Messenger
Summary(de):    SIM - Multiprotokoll Instant Messenger
License:        GPL
Group:          X11/KDE/Network
URL:            http://sim-im.berlios.de/
Source0:        %{name}-%{version}-%{build_release}.tar.gz
BuildRoot:      %{_tmppath}/sim-buildroot
Distribution:   SuSE Linux %{suse_release}

# neededforbuild  kde3-devel-packages
# usedforbuild    aaa_base aaa_dir aaa_version arts arts-devel autoconf automake base bash bindutil binutils bison bzip compat cpio cpp cracklib cyrus-sasl db devs diffutils docbook-dsssl-stylesheets docbook_3 e2fsprogs fam file fileutils findutils flex freetype2 freetype2-devel gawk gcc gcc-c++ gdbm gdbm-devel gettext glibc glibc-devel glibc-locale gpm grep groff gzip iso_ent jade_dsl kbd kdelibs3 kdelibs3-devel less libgcc libjpeg liblcms libmng libmng-devel libpng libpng-devel libstdc++ libstdc++-devel libtiff libtool libxcrypt libxml2 libxml2-devel libxslt libxslt-devel libz m4 make man mesa mesa-devel mesaglu mesaglu-devel mesaglut mesaglut-devel mesasoft mktemp modutils ncurses ncurses-devel net-tools netcfg openssl openssl-devel pam pam-devel pam-modules patch perl ps qt3 qt3-devel rcs readline rpm sed sendmail sh-utils shadow sp sp-devel strace syslogd sysvinit tar texinfo textutils timezone unzip util-linux vim xdevel xf86 xshared

%description -l de
SIM - Multiprotokoll Instant Messenger

SIM (Simple Instant Messenger) ist ein Plugin-basierender
open-source Instant Messenger, der verschiedene Protokolle
(ICQ, Jabber, AIM, MSN, YIM) unterstuetzt. Dafuer wird die
QT-Bibliothek und X11 (mit optionaler KDE-Unterstuetzung)
verwendet.

SIM hat sehr viele Features, viele von diesen sind
aufgelistet unter: http://sim-im.berlios.de/

%description
SIM - Multiprotocol Instant Messenger

SIM (Simple Instant Messenger) is a plugins-based open-
source instant messenger that supports various protocols
(ICQ, Jabber, AIM, MSN, YIM). It uses the QT library and
works on X11 (with optional KDE-support).

SIM has a lot of features, many of them are listed
at: http://sim-im.berlios.de/

%prep
%setup -q
make -f admin/Makefile.common
CFLAGS="$RPM_OPT_FLAGS"
CXXFLAGS="$RPM_OPT_FLAGS"

%configure
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

cd $RPM_BUILD_ROOT
find . -type d | sed '1,2d;s,^\.,\%attr(-\,root\,root) \%dir ,' > $RPM_BUILD_DIR/file.list.sim
find . -type f | sed 's,^\.,\%attr(-\,root\,root) ,' >> $RPM_BUILD_DIR/file.list.sim
find . -type l | sed 's,^\.,\%attr(-\,root\,root) ,' >> $RPM_BUILD_DIR/file.list.sim

%clean
rm -rf $RPM_BUILD_ROOT
rm -rf $RPM_BUILD_DIR/sim*
rm -rf ../file.list.sim

%files -f ../file.list.sim

%changelog
* Thu Apr 01 2004 - Christian Ehrlicher <chehrlic@users.sf.net> - 0.9.3-2
- updated to 0.9.3-2
- took some code from fedeora-spec (suse-release, optional KDE-support)

