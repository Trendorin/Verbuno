Name:           verbuno
Version:        0.3.1
Release:        1%{?dist}
Summary:        Native Linux AI translation client
License:        GPL-3.0-or-later
URL:            https://github.com/Trendorin/Verbuno
Source0:        %{url}/archive/refs/tags/v%{version}/Verbuno-%{version}.tar.gz
Provides:       translunix = %{version}-%{release}
Obsoletes:      translunix < 0.2.0

BuildRequires:  cmake >= 3.25
BuildRequires:  gcc-c++
BuildRequires:  ninja-build
BuildRequires:  qt6-qtbase-devel >= 6.4
BuildRequires:  qt6-qtsvg-devel >= 6.4
BuildRequires:  qt6-qttools-devel >= 6.4
BuildRequires:  qtkeychain-qt6-devel
BuildRequires:  tesseract-devel >= 5.0
Requires:       tesseract-langpack-eng
Requires:       tesseract-langpack-deu
Requires:       tesseract-langpack-rus
Requires:       tesseract-langpack-ukr

%description
Verbuno is a native C++20 and Qt 6 Widgets translation client. It opens
from the Linux system tray and streams translations from a user-configured
OpenRouter or OpenAI-compatible endpoint without application telemetry.

%prep
%autosetup -n Verbuno-%{version}

%build
%cmake -G Ninja -DVERBUNO_BUILD_TESTS=ON -DVERBUNO_REQUIRE_KEYCHAIN=ON
%cmake_build

%check
%ctest

%install
%cmake_install

%files
%license %{_datadir}/doc/verbuno/LICENSE
%doc %{_datadir}/doc/verbuno/README.md
%doc %{_datadir}/doc/verbuno/README.ru.md
%doc %{_datadir}/doc/verbuno/README.uk.md
%doc %{_datadir}/doc/verbuno/README.de.md
%doc %{_datadir}/doc/verbuno/CHANGELOG.md
%doc %{_datadir}/doc/verbuno/CONTRIBUTING.md
%doc %{_datadir}/doc/verbuno/CONTRIBUTORS.md
%doc %{_datadir}/doc/verbuno/RELEASE_NOTES.md
%doc %{_datadir}/doc/verbuno/ARCHITECTURE.md
%doc %{_datadir}/doc/verbuno/PRIVACY.md
%doc %{_datadir}/doc/verbuno/PROJECT_STATE.md
%doc %{_datadir}/doc/verbuno/SECURITY.md
%doc %{_datadir}/doc/verbuno/SECURITY_MODEL.md
%doc %{_datadir}/doc/verbuno/SUPPORT.md
%doc %{_datadir}/doc/verbuno/THIRD_PARTY_NOTICES.md
%doc %{_datadir}/doc/verbuno/copyright
%{_bindir}/verbuno
%{_datadir}/applications/io.github.trendorin.Verbuno.desktop
%{_datadir}/icons/hicolor/scalable/apps/io.github.trendorin.Verbuno.svg
%{_datadir}/metainfo/io.github.trendorin.Verbuno.metainfo.xml

%changelog
* Mon Jul 20 2026 Trendorin - 0.3.1-1
- Persist settings reliably and remember API keys by default

* Mon Jul 20 2026 Trendorin - 0.3.0-1
- Add local photo OCR with editable text extraction and language selection

* Sun Jul 19 2026 Trendorin - 0.2.0-1
- Rename the application to Verbuno and report the actual routed provider and model

* Sun Jul 19 2026 Trendorin - 0.1.1-1
- Use a standard native window and add runtime interface languages

* Sun Jul 19 2026 Trendorin - 0.1.0-1
- Initial native Linux release
