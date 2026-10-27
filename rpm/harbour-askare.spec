Name:       harbour-askare

Summary:    Nextcloud tasks for Sailfish OS
Version:    0.1.0
Release:    1
License:    GPLv3
URL:        https://github.com/fravaccaro/harbour-askare
Source0:    %{name}-%{version}.tar.bz2
Requires:   sailfishsilica-qt5 >= 0.10.9
Requires:   sailfish-components-webview-qt5
Requires:   nemo-qml-plugin-notifications-qt5
Requires:   libkeepalive
BuildRequires:  pkgconfig(sailfishapp) >= 1.0.2
BuildRequires:  pkgconfig(sailfishsecrets)
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Network)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(Qt5Sql)
BuildRequires:  desktop-file-utils
BuildRequires:  qt5-qttools-linguist

%description
Askare lists and edits Nextcloud tasks over CalDAV. Sign in through the
server's own page, then keep the lists on the device and sync them by hand.

%if 0%{?_chum}
Title: Askare
Type: desktop-application
DeveloperName: fravaccaro
Categories:
 - Office
 - Network
Custom:
  Repo: https://github.com/fravaccaro/harbour-askare
PackageIcon: https://raw.githubusercontent.com/fravaccaro/harbour-askare/main/icons/172x172/harbour-askare.png
Links:
  Homepage: https://github.com/fravaccaro/harbour-askare
  Bugtracker: https://github.com/fravaccaro/harbour-askare/issues
%endif

%prep
%setup -q -n %{name}-%{version}

%build
%qmake5 "APP_VERSION=%{version}"
%make_build

%install
%qmake5_install

# Harbour refuses an unstripped binary; the SDK packaging path does not always
# strip by itself, so it is done here before the package is sealed.
strip --strip-unneeded %{buildroot}%{_bindir}/%{name}

desktop-file-install --delete-original \
    --dir %{buildroot}%{_datadir}/applications \
    %{buildroot}%{_datadir}/applications/*.desktop

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png
