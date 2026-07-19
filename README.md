<p align="center">
  <img src="data/icons/hicolor/scalable/apps/io.github.trendorin.TranslUnix.svg" width="144" alt="TranslUnix application icon">
</p>

<h1 align="center">TranslUnix</h1>
<p align="center">Fast, model-powered translation from the Linux system tray.</p>

<p align="center">
  <a href="README.md"><img alt="English" src="https://img.shields.io/badge/EN-English-d7dade?style=flat-square&labelColor=15181b"></a>
  <a href="README.ru.md"><img alt="Русский" src="https://img.shields.io/badge/RU-Русский-d7dade?style=flat-square&labelColor=15181b"></a>
  <a href="README.uk.md"><img alt="Українська" src="https://img.shields.io/badge/UK-Українська-d7dade?style=flat-square&labelColor=15181b"></a>
  <a href="README.de.md"><img alt="Deutsch" src="https://img.shields.io/badge/DE-Deutsch-d7dade?style=flat-square&labelColor=15181b"></a>
</p>

<p align="center">
  <a href="https://github.com/Trendorin/TranslUnix/actions/workflows/ci.yml"><img alt="CI" src="https://img.shields.io/github/actions/workflow/status/Trendorin/TranslUnix/ci.yml?branch=main&style=flat-square&label=build&labelColor=15181b&color=5d666d"></a>
  <a href="https://github.com/Trendorin/TranslUnix/releases/latest"><img alt="Release" src="https://img.shields.io/github/v/release/Trendorin/TranslUnix?style=flat-square&label=release&labelColor=15181b&color=5d666d"></a>
  <img alt="C++20" src="https://img.shields.io/badge/C%2B%2B-20-5d666d?style=flat-square&labelColor=15181b">
  <img alt="Qt 6 Widgets" src="https://img.shields.io/badge/Qt-6_Widgets-5d666d?style=flat-square&labelColor=15181b">
  <a href="LICENSE"><img alt="GPL-3.0-or-later" src="https://img.shields.io/badge/license-GPL--3.0--or--later-5d666d?style=flat-square&labelColor=15181b"></a>
</p>

<p align="center">
  <a href="#install">Install</a> ·
  <a href="#configure">Configure</a> ·
  <a href="#build-from-source">Source</a> ·
  <a href="docs/SECURITY_MODEL.md">Security</a> ·
  <a href="https://github.com/Trendorin/TranslUnix/releases">Releases</a>
</p>

TranslUnix is a native Linux translation client built with C++20 and Qt 6 Widgets. The tray opens a standard system-decorated, resizable window, translations stream from OpenRouter or another OpenAI-compatible endpoint, and the active provider and model remain visible while you work.

## What it does

| Area | Result |
|---|---|
| Desktop window | A normal Qt window with native minimize, maximize and close controls on KDE, GNOME and other desktops. |
| Translation | Roughly 190 language variants, automatic source detection, five writing styles and formatting preservation. |
| Models | Any model ID, the `openrouter/free` router, or a refreshed catalog of models whose reported prices are zero. |
| Providers | OpenRouter by default; custom OpenAI-compatible Chat Completions endpoints are supported. |
| Interface | Runtime switching between English, Russian, Ukrainian and German without restarting the application. |
| Output | Server-sent events are decoded incrementally, with cancellation and clear provider errors. |

## Privacy boundary

| TranslUnix does | TranslUnix cannot guarantee |
|---|---|
| No telemetry, analytics, crash uploads or background model calls. | That an external provider never logs, retains or trains on submitted text. |
| Sends text only to the configured chat endpoint after an explicit translation action. | That a free endpoint remains available, fast or free in the future. |
| Stores remembered keys in KWallet / Secret Service through QtKeychain, never in `QSettings`. | Anonymity from the provider, which still sees normal network and account metadata. |
| Enables OpenRouter `data_collection: deny` by default and offers strict ZDR routing. | Availability of a model after privacy-incompatible endpoints have been excluded. |
| Keeps local history off by default; an enabled history is owner-only and can be erased immediately. | Protection for text copied to another application or retained by the desktop clipboard. |

OpenRouter states that prompt retention on its own platform is opt-in, while upstream providers have separate policies. Free routes can include endpoints with different data terms. Review the current [OpenRouter data collection](https://openrouter.ai/docs/guides/privacy/data-collection), [provider logging](https://openrouter.ai/docs/guides/privacy/provider-logging), and [ZDR](https://openrouter.ai/docs/guides/features/zdr) documentation before sending sensitive text.

<a id="install"></a>
## Install

Download the matching asset and `SHA256SUMS` from the [latest release](https://github.com/Trendorin/TranslUnix/releases/latest).

| System | Asset | Command |
|---|---|---|
| Fedora 44 | `translunix-*.rpm` | `sudo dnf install ./translunix-*.rpm` |
| Ubuntu 24.04 | `translunix_*.deb` | `sudo apt install ./translunix_*.deb` |
| Arch Linux | `translunix-*-x86_64.pkg.tar.zst` | `sudo pacman -U ./translunix-*-x86_64.pkg.tar.zst` |

Each binary package is built and install-tested in its native target. `PKGBUILD`, source and install-tree `.tar.xz` archives, an SPDX document and checksums are included in the same release. AppImage is intentionally not part of the release set.

Verify downloads before installation:

```bash
sha256sum --ignore-missing --check SHA256SUMS
```

<a id="configure"></a>
## Configure

1. Create an API key in OpenRouter or another provider.
2. Open **Settings → Provider**, paste the key and choose whether it may be remembered in the system keychain.
3. Keep `openrouter/free`, refresh the current free-model list, or enter an exact model ID.
4. Leave **Exclude providers that collect prompt data** enabled. Enable **Zero Data Retention** only when the selected model has a compatible route.
5. Choose source and target languages, then press `Ctrl+Enter` to translate.
6. Select the interface language under **Settings → General**; the visible UI and tray menu update immediately.

Keys kept only for the session never reach disk. A remembered key requires an available Secret Service implementation such as KWallet, GNOME Keyring or KeePassXC. TranslUnix never enables QtKeychain's plaintext fallback.

For another provider, select **OpenAI-compatible endpoint** and enter the complete `/chat/completions` URL and model ID. Plain HTTP is refused except for loopback services such as a local model server.

### Desktop behavior

- KDE Plasma: native tray integration is the primary target.
- GNOME: a tray requires AppIndicator support; without it, the main window opens normally.
- Other desktops and Wayland compositors: the application uses a normal Qt top-level window managed by the system compositor.
- Global shortcut: bind `translunix --toggle` in the desktop's keyboard settings.

<a id="build-from-source"></a>
## Build from source

Requirements: CMake 3.25+, Ninja, a C++20 compiler, Qt 6.4+ (Core, Widgets, Network, SVG, Tools) and QtKeychain for Qt 6.

<details>
<summary>Fedora build dependencies</summary>

```bash
sudo dnf install gcc-c++ cmake ninja-build \
  qt6-qtbase-devel qt6-qtsvg-devel qt6-qttools-devel \
  qtkeychain-qt6-devel
```
</details>

<details>
<summary>Ubuntu 24.04+ build dependencies</summary>

```bash
sudo apt install build-essential cmake ninja-build \
  qt6-base-dev qt6-svg-dev qt6-tools-dev qtkeychain-qt6-dev
```
</details>

```bash
git clone https://github.com/Trendorin/TranslUnix.git
cd TranslUnix
cmake --preset release
cmake --build --preset release
ctest --preset release
cmake --install build/release --prefix "$HOME/.local"
```

## Uninstall

| Installation | Command |
|---|---|
| Fedora 44 | `sudo dnf remove translunix` |
| Ubuntu 24.04 | `sudo apt remove translunix` |
| Arch Linux | `sudo pacman -Rns translunix` |
| Source build | `xargs -r -d '\n' rm -- < build/release/install_manifest.txt` |

Uninstalling does not remove the optional local history or a key stored in the desktop wallet. Remove the key inside TranslUnix first, clear history, then optionally delete `$XDG_CONFIG_HOME/Trendorin/TranslUnix.conf` and `$XDG_DATA_HOME/Trendorin/TranslUnix/`.

## Security boundary

- Provider URLs must use HTTPS, except for loopback hosts; credentials embedded in URLs are rejected.
- HTTP redirects are refused so an authorization header cannot be forwarded to a different endpoint.
- Model responses are size-bounded, API errors are sanitized, and likely credential strings are redacted.
- Translation text embedded in the request is explicitly treated as untrusted data, not as instructions.
- Single-instance commands do not accept translation text or API keys on the command line.

Read the [architecture](docs/ARCHITECTURE.md), [security model](docs/SECURITY_MODEL.md), [data-handling policy](docs/PRIVACY.md), [vulnerability policy](SECURITY.md), and [third-party notices](THIRD_PARTY_NOTICES.md).

## Project

[Changelog](CHANGELOG.md) · [Contributing](CONTRIBUTING.md) · [Contributors](CONTRIBUTORS.md) · [Support](SUPPORT.md)

Maintained by [Trendorin](https://github.com/Trendorin). Licensed under [GPL-3.0-or-later](LICENSE).
