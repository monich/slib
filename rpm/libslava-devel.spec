Name: libslava-devel
Version: 1.82
Release: 0
Summary: General purpose library
Group: Development/Libraries
License: BSD
URL: https://github.com/monich/slib
Source: %{name}-%{version}.tar.bz2

BuildRequires: zlib-devel
BuildRequires: expat-devel
BuildRequires: libcurl-devel

%global debug_package %{nil}

%description
Provides slib headers and static library.

%prep
%setup -q

%build
make all LIBDIR=%{_libdir}

%check
make check

%install
rm -rf %{buildroot}
make install DESTDIR=%{buildroot} LIBDIR=%{_libdir}

%files
%defattr(-,root,root,-)
%{_libdir}/libslava*.a
%{_libdir}/pkgconfig/*.pc
%{_includedir}/slib
