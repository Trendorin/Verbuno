Name:           translunix
Version:        0.1.0
Release:        1%{?dist}
Summary:        Native Linux AI translation client
License:        GPL-3.0-or-later
URL:            https://github.com/Trendorin/TranslUnix
Source0:        %{url}/archive/refs/tags/v%{version}/TranslUnix-%{version}.tar.gz

BuildRequires:  cmake >= 3.25
BuildRequires:  gcc-c++
BuildRequires:  ninja-build
BuildRequires:  qt6-qtbase-devel >= 6.4
BuildRequires:  qt6-qtsvg-devel >= 6.4
BuildRequires:  qt6-qttools-devel >= 6.4
BuildRequires:  qtkeychain-qt6-devel

%description
TranslUnix is a native C++20 and Qt 6 Widgets translation client. It opens
from the Linux system tray and streams translations from a user-configured
OpenRouter or OpenAI-compatible endpoint without application telemetry.

%prep
%autosetup -n TranslUnix-%{version}

%build
%cmake -G Ninja -DTRANSLUNIX_BUILD_TESTS=ON -DTRANSLUNIX_REQUIRE_KEYCHAIN=ON
%cmake_build

%check
%ctest

%install
%cmake_install

%files
%license %{_datadir}/doc/translunix/LICENSE
%doc %{_datadir}/doc/translunix/README.md
%doc %{_datadir}/doc/translunix/README.ru.md
%doc %{_datadir}/doc/translunix/README.uk.md
%doc %{_datadir}/doc/translunix/README.de.md
%doc %{_datadir}/doc/translunix/CHANGELOG.md
%doc %{_datadir}/doc/translunix/CONTRIBUTING.md
%doc %{_datadir}/doc/translunix/CONTRIBUTORS.md
%doc %{_datadir}/doc/translunix/RELEASE_NOTES.md
%doc %{_datadir}/doc/translunix/ARCHITECTURE.md
%doc %{_datadir}/doc/translunix/PRIVACY.md
%doc %{_datadir}/doc/translunix/PROJECT_STATE.md
%doc %{_datadir}/doc/translunix/SECURITY.md
%doc %{_datadir}/doc/translunix/SECURITY_MODEL.md
%doc %{_datadir}/doc/translunix/SUPPORT.md
%doc %{_datadir}/doc/translunix/THIRD_PARTY_NOTICES.md
%doc %{_datadir}/doc/translunix/copyright
%{_bindir}/translunix
%{_datadir}/applications/io.github.trendorin.TranslUnix.desktop
%{_datadir}/icons/hicolor/scalable/apps/io.github.trendorin.TranslUnix.svg
%{_datadir}/metainfo/io.github.trendorin.TranslUnix.metainfo.xml

%changelog
* Sun Jul 19 2026 Trendorin - 0.1.0-1
- Initial native Linux release
