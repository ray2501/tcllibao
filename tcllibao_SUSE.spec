%{!?directory:%define directory /usr}

%define buildroot %{_tmppath}/%{name}

Name:          tcllibao
Summary:       Tcl bindings for libao
Version:       0.3
Release:       0
License:       GPL 2.0
Group:         Development/Libraries/Tcl
Source:        %{name}-%{version}.tar.gz
URL:           https://github.com/ray2501/tcllibao
BuildRequires: autoconf
BuildRequires: make
BuildRequires: tcl-devel >= 8.4
BuildRequires: libao-devel
Requires:      tcl >= 8.4
Requires:      libao4
BuildRoot:     %{buildroot}

%description
Tcl bindings for libao.

%prep
%setup -q -n %{name}-%{version}

%build
./configure \
	--prefix=%{directory} \
	--exec-prefix=%{directory} \
	--libdir=%{directory}/%{_lib}
make 

%install
make DESTDIR=%{buildroot} pkglibdir=%{tcl_archdir}/%{name}%{version} install

%clean
rm -rf %buildroot

%files
%defattr(-,root,root)
%{tcl_archdir}
