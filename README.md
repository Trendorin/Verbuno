<p align="center">
  <img src="data/icons/hicolor/scalable/apps/io.github.trendorin.Verbuno.svg" width="144" alt="Verbuno application icon">
</p>

<h1 align="center">Verbuno</h1>
<p align="center">Fast, model-powered translation from the Linux system tray.</p>

<p align="center">
  <a href="README.md"><img alt="English" src="https://img.shields.io/badge/EN-English-d7dade?style=flat-square&labelColor=15181b"></a>
  <a href="README.ru.md"><img alt="Русский" src="https://img.shields.io/badge/RU-Русский-d7dade?style=flat-square&labelColor=15181b"></a>
  <a href="README.uk.md"><img alt="Українська" src="https://img.shields.io/badge/UK-Українська-d7dade?style=flat-square&labelColor=15181b"></a>
  <a href="README.de.md"><img alt="Deutsch" src="https://img.shields.io/badge/DE-Deutsch-d7dade?style=flat-square&labelColor=15181b"></a>
</p>

<p align="center">
  <a href="https://github.com/Trendorin/Verbuno/actions/workflows/ci.yml"><img alt="CI" src="https://img.shields.io/github/actions/workflow/status/Trendorin/Verbuno/ci.yml?branch=main&style=flat-square&label=build&labelColor=15181b&color=5d666d"></a>
  <a href="https://github.com/Trendorin/Verbuno/releases/latest"><img alt="Release" src="https://img.shields.io/github/v/release/Trendorin/Verbuno?style=flat-square&label=release&labelColor=15181b&color=5d666d"></a>
  <img alt="C++20" src="https://img.shields.io/badge/C%2B%2B-20-5d666d?style=flat-square&labelColor=15181b">
  <img alt="Qt 6 Widgets" src="https://img.shields.io/badge/Qt-6_Widgets-5d666d?style=flat-square&labelColor=15181b">
  <a href="LICENSE"><img alt="GPL-3.0-or-later" src="https://img.shields.io/badge/license-GPL--3.0--or--later-5d666d?style=flat-square&labelColor=15181b"></a>
</p>

<p align="center">
  <a href="#install">Install</a> ·
  <a href="#configure">Configure</a> ·
  <a href="#build-from-source">Source</a> ·
  <a href="docs/SECURITY_MODEL.md">Security</a> ·
  <a href="https://github.com/Trendorin/Verbuno/releases">Releases</a>
</p>

Verbuno is a native Linux translation client built with C++20 and Qt 6 Widgets. It translates typed text or text extracted locally from photos, streams results from OpenRouter or another OpenAI-compatible endpoint, and keeps the actual routed model and upstream provider visible while you work.

## What it does

| Area | Result |
|---|---|
| Desktop window | A normal Qt window with native minimize, maximize and close controls on KDE, GNOME and other desktops. |
| Translation | Roughly 190 language variants, automatic source detection, five writing styles and formatting preservation. |
| Photos | Open, paste or drop PNG, JPEG, WebP, BMP and TIFF images; local Tesseract OCR runs off the UI thread and places editable text in the normal translation editor. |
| Models | Any model ID, the `openrouter/free` router, or a refreshed catalog of models whose reported prices are zero. |
| Providers | OpenRouter by default; the response-reported model and upstream provider are shown instead of merely repeating the requested router ID. Custom OpenAI-compatible endpoints are supported. |
| Interface | Runtime switching between English, Russian, Ukrainian and German without restarting the application. |
| Local storage | Provider, exact model, interface and translation languages, translation/OCR preferences and tray behavior are restored after restart. |
| Output | Server-sent events are decoded incrementally, with cancellation and clear provider errors. |

## Privacy boundary

| Verbuno does | Verbuno cannot guarantee |
|---|---|
| No telemetry, analytics, crash uploads or background model calls. | That an external provider never logs, retains or trains on submitted text. |
| Decodes and recognizes selected photos locally; image pixels and filenames are never sent to the translation provider. | Perfect OCR for blurred, handwritten, heavily stylized or low-contrast text. Review extracted text before sending it. |
| Sends text only to the configured chat endpoint after an explicit translation action. | That a free endpoint remains available, fast or free in the future. |
| Remembers newly entered keys in KWallet / Secret Service by default, never in `QSettings`; session-only mode remains available. | Anonymity from the provider, which still sees normal network and account metadata. |
| Enables OpenRouter `data_collection: deny` by default and offers strict ZDR routing. | Availability of a model after privacy-incompatible endpoints have been excluded. |
| Keeps local history off by default; an enabled history is owner-only and can be erased immediately. | Protection for text copied to another application or retained by the desktop clipboard. |

OpenRouter states that prompt retention on its own platform is opt-in, while upstream providers have separate policies. Free routes can include endpoints with different data terms. Review the current [OpenRouter data collection](https://openrouter.ai/docs/guides/privacy/data-collection), [provider logging](https://openrouter.ai/docs/guides/privacy/provider-logging), and [ZDR](https://openrouter.ai/docs/guides/features/zdr) documentation before sending sensitive text.

<a id="install"></a>
## Install

Download the matching asset and `SHA256SUMS` from the [latest release](https://github.com/Trendorin/Verbuno/releases/latest).

| System | Asset | Command |
|---|---|---|
| Fedora 44 | `verbuno-*.rpm` | `sudo dnf install ./verbuno-*.rpm` |
| Ubuntu 24.04 | `verbuno_*.deb` | `sudo apt install ./verbuno_*.deb` |
| Arch Linux | `verbuno-*-x86_64.pkg.tar.zst` | `sudo pacman -U ./verbuno-*-x86_64.pkg.tar.zst` |

Each binary package is built and install-tested in its native target. `PKGBUILD`, source and install-tree `.tar.xz` archives, an SPDX document and checksums are included in the same release. AppImage is intentionally not part of the release set.

English, German, Russian and Ukrainian OCR data are installed with the native packages. Verbuno also discovers other system Tesseract language packs. For example, add Japanese with `tesseract-ocr-jpn` on Ubuntu, `tesseract-langpack-jpn` on Fedora or `tesseract-data-jpn` on Arch, then restart Verbuno.

Verify downloads before installation:

```bash
sha256sum --ignore-missing --check SHA256SUMS
```

<a id="configure"></a>
## Configure

1. Create an API key in OpenRouter or another provider.
2. Open **Settings → Provider** and paste the key. Secure reuse after restart is enabled by default; disable it only for a session-only key.
3. Keep `openrouter/free`, refresh the current free-model list, or enter an exact model ID.
4. Leave **Exclude providers that collect prompt data** enabled. Enable **Zero Data Retention** only when the selected model has a compatible route.
5. For a photo, press **Open photo**, paste an image, or drop it onto the workspace. Choose the OCR language and layout when needed, check the locally extracted text, then press **Translate**.
6. For typed text, choose source and target languages, then press `Ctrl+Enter` to translate.
7. Select the interface language under **Settings → General**; the visible UI and tray menu update immediately.

Photo recognition never starts a provider request. The image is decoded with strict size limits, auto-rotated from its metadata, resized safely, and recognized in a background task. A second local contrast pass is used when the first result has low confidence. Only the editable extracted text enters the existing translation flow after an explicit click.

After a response begins, the workspace shows the exact model returned by the API. With OpenRouter it also shows the selected upstream provider, for example `Chutes via OpenRouter · qwen/qwen3-…`. Hover the summary to see the requested model or router such as `openrouter/free`.

Keys kept only for the session never reach disk. A remembered key requires an available Secret Service implementation such as KWallet, GNOME Keyring or KeePassXC. Verbuno waits for the wallet result and never enables QtKeychain's plaintext fallback.

All non-secret choices are saved automatically to the stable local settings file shown under **Settings → General**. The file is owner-only; Verbuno reports write or format errors instead of silently losing the selected interface language, provider, model or language pair.

For another provider, select **OpenAI-compatible endpoint** and enter the complete `/chat/completions` URL and model ID. Plain HTTP is refused except for loopback services such as a local model server.

### Desktop behavior

- KDE Plasma: native tray integration is the primary target.
- GNOME: a tray requires AppIndicator support; without it, the main window opens normally.
- Other desktops and Wayland compositors: the application uses a normal Qt top-level window managed by the system compositor.
- Global shortcut: bind `verbuno --toggle` in the desktop's keyboard settings.

<a id="build-from-source"></a>
## Build from source

Requirements: CMake 3.25+, Ninja, a C++20 compiler, Qt 6.4+ (Core, Concurrent, Widgets, Network, SVG, Tools), QtKeychain for Qt 6 and Tesseract OCR 5+.

<details>
<summary>Fedora build dependencies</summary>

```bash
sudo dnf install gcc-c++ cmake ninja-build \
  qt6-qtbase-devel qt6-qtsvg-devel qt6-qttools-devel \
  qtkeychain-qt6-devel tesseract-devel \
  tesseract-langpack-eng tesseract-langpack-deu \
  tesseract-langpack-rus tesseract-langpack-ukr
```
</details>

<details>
<summary>Ubuntu 24.04+ build dependencies</summary>

```bash
sudo apt install build-essential cmake ninja-build \
  qt6-base-dev qt6-svg-dev qt6-tools-dev qtkeychain-qt6-dev \
  libtesseract-dev tesseract-ocr-eng tesseract-ocr-deu \
  tesseract-ocr-rus tesseract-ocr-ukr
```
</details>

```bash
git clone https://github.com/Trendorin/Verbuno.git
cd Verbuno
cmake --preset release
cmake --build --preset release
ctest --preset release
cmake --install build/release --prefix "$HOME/.local"
```

## Uninstall

| Installation | Command |
|---|---|
| Fedora 44 | `sudo dnf remove verbuno` |
| Ubuntu 24.04 | `sudo apt remove verbuno` |
| Arch Linux | `sudo pacman -Rns verbuno` |
| Source build | `xargs -r -d '\n' rm -- < build/release/install_manifest.txt` |

Uninstalling does not remove the optional local history or a key stored in the desktop wallet. Remove the key inside Verbuno first, clear history, then optionally delete `$XDG_CONFIG_HOME/Trendorin/Verbuno.conf` and `$XDG_DATA_HOME/Trendorin/Verbuno/`.

Upgrading from the former TranslUnix package preserves non-secret settings, local history and a remembered wallet key on first launch. The old package name is replaced by `verbuno`.

## Security boundary

- Provider URLs must use HTTPS, except for loopback hosts; credentials embedded in URLs are rejected.
- HTTP redirects are refused so an authorization header cannot be forwarded to a different endpoint.
- Model responses are size-bounded, API errors are sanitized, and likely credential strings are redacted.
- Image files, decoded dimensions and OCR output are bounded; unsupported or oversized input is rejected before recognition.
- Translation text embedded in the request is explicitly treated as untrusted data, not as instructions.
- Single-instance commands do not accept translation text or API keys on the command line.

Read the [architecture](docs/ARCHITECTURE.md), [security model](docs/SECURITY_MODEL.md), [data-handling policy](docs/PRIVACY.md), [vulnerability policy](SECURITY.md), and [third-party notices](THIRD_PARTY_NOTICES.md).

## Project

[Changelog](CHANGELOG.md) · [Contributing](CONTRIBUTING.md) · [Contributors](CONTRIBUTORS.md) · [Support](SUPPORT.md)

Maintained by [Trendorin](https://github.com/Trendorin). Licensed under [GPL-3.0-or-later](LICENSE).
