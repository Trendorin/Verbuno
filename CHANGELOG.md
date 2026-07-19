# Changelog

All notable changes follow [Keep a Changelog](https://keepachangelog.com/en/1.1.0/). Versions use semantic versioning.

## [0.1.1] - 2026-07-19

### Added

- Persistent provider and model summary in the main toolbar and translation workspace.
- Complete runtime interface switching between English, Russian, Ukrainian and German.
- Localized interface guidance, privacy notices, validation failures and provider errors.

### Changed

- Replaced the frameless tray popup with a standard system-decorated Qt window.
- Tray activation and `translunix --toggle` now operate on the normal main window.
- Window minimizing, maximizing, resizing and placement are handled natively by KDE, GNOME or the active compositor.

## [0.1.0] - 2026-07-19

### Added

- Native C++20 and Qt 6 Widgets application for Linux.
- Animated tray popup and full translation window.
- OpenRouter streaming plus custom OpenAI-compatible Chat Completions endpoints.
- Broad language catalog, automatic source detection and five translation styles.
- Live discovery of currently free OpenRouter models.
- Default `data_collection: deny` routing and optional Zero Data Retention routing.
- Session-only keys and opt-in KWallet / Secret Service persistence through QtKeychain.
- Opt-in bounded local history with atomic owner-only storage.
- DEB, RPM, Arch, TXZ, source, SPDX, PKGBUILD and SHA-256 release artifacts.
- Ubuntu, Fedora, Arch, sanitizer and CodeQL validation.

[0.1.1]: https://github.com/Trendorin/TranslUnix/releases/tag/v0.1.1
[0.1.0]: https://github.com/Trendorin/TranslUnix/releases/tag/v0.1.0
