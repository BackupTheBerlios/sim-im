%define subversion 1
%define date 20031105
%define is_release 1
%define desktop_file 1
%define gkrellm 0

Summary:   SIM - Simple Instant Messenger
Name:      sim
Version:   0.9.1
%if %{is_release}
Release:   1asp
%else
Release:   1.%{date}.1asp
%endif
License:   GPL
Url:       http://sim-im.berlios.de/
Group:     Applications/Communications
%if %{is_release}
Source:    sim-%{version}.tar.bz2
%else
Source:    sim-%{date}.tar.bz2
%endif
BuildRoot: %{_tmppath}/sim-buildroot
BuildRequires:  qt-devel kdelibs-devel arts-devel /usr/bin/dos2unix /usr/bin/perl
%if %{desktop_file}
BuildRequires: desktop-file-utils
%endif

%description
A simple ICQ client with v8 protocol support (2001) for X win system 
(requires QT, can be build for KDE). It also runs under MS Windows.

%if %{gkrellm}
%package gkrellm
Summary: sim GKrellM2 plugin
Group: Applications/Communications
Requires: gkrellm >= 2.1.0
Requires: %{name} = %{version}-%{release}

%description gkrellm
sim GKrellM2 plugin
%endif

%prep

%if %{is_release}
%setup
%else
%setup -q -n sim
%endif

%build

make -f admin/Makefile.common

%if %{gkrellm}
%configure
%else
%configure --without-gkrellm_plugin
%endif

make %{?_smp_mflags}

%install

rm -rf %{buildroot}
make install-strip DESTDIR=$RPM_BUILD_ROOT

# desktop file install

%if %{desktop_file}
mkdir -p $RPM_BUILD_ROOT/%{_datadir}/applications

dos2unix $RPM_BUILD_ROOT%{_datadir}/applnk-redhat/Internet/sim.desktop
# perl -pi -e "s|sim.png|licq|g" $RPM_BUILD_ROOT%{_datadir}/applnk-redhat/Internet/sim.desktop

desktop-file-install --delete-original --vendor net \
  --dir $RPM_BUILD_ROOT%{_datadir}/applications \
  --add-category X-Red-Hat-Base \
  --add-category Network \
  --add-category Application \
  $RPM_BUILD_ROOT%{_datadir}/applnk-redhat/Internet/sim.desktop
%endif

%find_lang %{name}

# clean unused files
rm -rf %{buildroot}%{_libdir}/menu

%clean
rm -rf %{buildroot}

%files -f %{name}.lang
%defattr(-, root, root, 755)
%doc AUTHORS COPYING README TODO INSTALL
%{_bindir}/sim
%{_libdir}/libsim*
%if %{desktop_file}
%{_datadir}/applications/net-sim.desktop
%else
%{_datadir}/applnk/Internet/sim.desktop
%endif
%dir %{_datadir}/apps/sim
%{_datadir}/apps/sim/COPYING
%dir %{_datadir}/apps/sim/plugins
%dir %{_datadir}/apps/sim/plugins/*
%dir %{_datadir}/apps/sim/pict
%{_datadir}/apps/sim/pict/*
%dir %{_datadir}/apps/sim/sounds
%{_datadir}/apps/sim/sounds/*
%{_datadir}/icons/*/*/*/*

%if %{gkrellm}
%files gkrellm
%{_libdir}/gkrellm2/plugins/*
%endif

%changelog

* Tue Nov 11 2005 Leonid Kanter <leon@asplinux.ru> 0.9.1-1asp
- 0.9.1

* Wed Nov 05 2005 Leonid Kanter <leon@asplinux.ru>
- Post-release 20031105 (should fix login problem after icq servers update)

* Wed Oct 21 2003 Leonid Kanter <leon@asplinux.ru>
- 20031021

* Wed Sep 24 2003 Leonid Kanter <leon@asplinux.ru>
- 20030924

* Wed Sep 17 2003 Leonid Kanter <leon@asplinux.ru>
- 20030917

* Tue Aug 26 2003 Leonid Kanter <leon@asplinux.ru>
- update to 20030826

* Mon Jul 28 2003 Leonid Kanter <leon@asplinux.ru> 0.9-cvs
- update to current 0.9 state

* Sat Apr 26 2003 Leonid Kanter <leon@asplinux.ru> 0.8.2-1asp
- 0.8.2 release

* Wed Apr 23 2003 Leonid Kanter <leon@asplinux.ru>
- build gkrellm2 plugin as separate package

* Mon Mar 03 2003 Leonid Kanter <leon@asplinux.ru>
- cvs snapshot 20030303, more improvements in spec

* Wed Feb 12 2003 Leonid Kanter <leon@asplinux.ru>
- new cvs snapshot, use desktop-file-utils

* Tue Dec 17 2002 Leon Kanter <leon@asplinux.ru>
- Universal spec, included missed simctrl

* Tue Dec 17 2002 Leon Kanter <leon@asplinux.ru>
- 0.8.1

* Wed Oct 23 2002 Leon Kanter <leon@asplinux.ru>
- Apply fontsize patch from cvs

* Tue Oct 22 2002 Leon Kanter <leon@asplinux.ru>
- Built official release for asp7.3

* Tue Sep 24 2002 Leon Kanter <leon@asplinux.ru>
- this cvs snapshot should fix font-related problems

* Wed Sep 11 2002 Leon Kanter <leon@asplinux.ru>
- Spec cleanup, built for asplinux%{_bindir}/sim
