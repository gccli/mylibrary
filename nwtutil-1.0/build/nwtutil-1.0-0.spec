# Build a RPM

%define _prefix /usr/local

Summary: Network Testing Tool
Name: nwtutil
Version: 1.0
Release: 0
License: GPL
Group: Testing
URL: http://nwtutil.sourceforge.net
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root

%description
nwtutil ("Network Testing Tool") is a free and open source utility for
variable network testing task.

%prep
%setup -q

%build
%configure
make

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
%{_bindir}/nwtutil

%doc


%changelog
* Tue Oct 26 2010 root <root@vlin.li> - 1.0-0
- Initial build.

