# Verbuno project state

This file is the durable source of truth for continuing the project without relying on chat history.

## Current release

- Product: **Verbuno**
- Repository: `Trendorin/Verbuno`
- Application ID: `io.github.trendorin.Verbuno`
- Binary/package: `verbuno`
- Version: `0.3.1`
- License: `GPL-3.0-or-later`
- Stack: `C++20`, `Qt 6 Widgets`, `CMake`, no Electron, WebView, QML or background service
- Primary desktop: KDE Plasma; supported fallback behavior for GNOME and other Qt-capable Linux desktops

## Product contract

Verbuno is a small tray-first client for high-quality model-assisted translation. OpenRouter is the default provider. A user can also supply a complete OpenAI-compatible Chat Completions endpoint, API key and model ID. The tray opens a standard system-decorated Qt window; the response-reported model and OpenRouter upstream provider are visible while the requested router remains available in a tooltip. The interface switches live between English, Russian, Ukrainian and German.

Photo input can be opened, pasted or dropped. Tesseract OCR is entirely local, asynchronous and limited by file, dimension and text bounds. Installed OCR languages are discovered at runtime; native packages include English, German, Russian and Ukrainian data. The user reviews editable extracted text before explicitly sending it through the normal translation flow. Image pixels and filenames never reach a provider or local history.

The application is not offline: translated text leaves the machine for the configured provider. The application itself has no telemetry, hidden network calls or hosted backend. This distinction must remain explicit in the UI, README and privacy documentation.

## Security defaults

- HTTPS is mandatory except for loopback endpoints.
- Redirects are refused to prevent credential forwarding.
- Newly entered API keys default to persistent QtKeychain storage; session-only mode remains available explicitly.
- QtKeychain insecure/plaintext fallback is always disabled.
- OpenRouter requests default to `provider.data_collection = "deny"`.
- `provider.zdr = true` is an optional stricter mode because it reduces route availability.
- Local history is disabled by default, owner-only when enabled, atomically written, bounded and user-clearable.
- API keys are excluded from normal settings, history, command-line arguments, logs and error messages.
- Non-secret settings are atomically synchronized to an owner-only file; write failures are visible in the General settings page.
- Photo pixels, paths, filenames, OCR language and confidence metadata are excluded from provider requests and history.

## Native Linux repository baseline

Use this baseline for future Trendorin native Linux desktop applications unless a later project explicitly changes it:

1. `C++20 + Qt 6 Widgets + CMake`; prefer system palette, standard controls and limited QSS.
2. KDE-first behavior with GNOME/Wayland fallbacks; no claim of identical tray behavior where the desktop does not provide a tray.
3. Centered transparent application mark, no wide README banner. The mark uses a neutral silver/graphite gradient, angular geometry, no background, no text and must remain readable at tray size.
4. README structure: centered icon and name, one-line factual description, language row, status row, compact navigation, capability table, install table, build, security boundary and project links.
5. Keep synchronized `README.md`, `README.ru.md`, `README.uk.md` and `README.de.md`; English is the default GitHub page.
6. Include `CHANGELOG`, `RELEASE_NOTES`, `CONTRIBUTING`, `CONTRIBUTORS`, `SECURITY`, `SUPPORT`, privacy/security docs, third-party notices, issue templates, CI and CodeQL.
7. Release assets: Ubuntu `.deb`, Fedora `.rpm`, real Arch `.pkg.tar.zst`, install-tree `.tar.xz`, source `.tar.xz`, `PKGBUILD`, SPDX 2.3 and `SHA256SUMS`.
8. Build each package in its native target and install-test it. Do not publish AppImage unless a future request explicitly restores it.
9. Use factual privacy language. Distinguish local application behavior from external service behavior.

## Release gate

A release is complete only when the following are green:

- C++ build and Qt tests on Ubuntu 24.04
- native Fedora 44 RPM build and installation
- clean Arch `makepkg` build as an unprivileged user and installation
- ASan/UBSan tests
- CodeQL C++ analysis
- desktop-file and AppStream validation
- local Tesseract OCR smoke coverage with installed language data
- source-archive content check, package file-set check, SPDX version check and SHA-256 verification
- tag version, CMake version, AppStream version and release title match
