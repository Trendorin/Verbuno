# Verbuno 0.2.0

This release completes the product rename to Verbuno and makes routed OpenRouter requests transparent in the interface.

## Highlights

- the application, executable, package, desktop metadata and repository are now named Verbuno;
- the translation workspace shows the exact model reported by the API instead of presenting `openrouter/free` as if it were the selected model;
- OpenRouter responses also show the chosen upstream inference provider, for example `Chutes via OpenRouter`;
- the requested model or router remains visible in the provider-summary tooltip;
- the optional per-request context field has been removed;
- upgrades migrate ordinary settings, safe local history and remembered QtKeychain credentials from TranslUnix;
- the standard system-decorated window and runtime English, Russian, Ukrainian and German interface remain available.

## Upgrade note

The new binary and package name is `verbuno`. DEB, RPM and Arch package metadata replaces the old `translunix` package. On first launch, application data is copied or safely moved to the new Verbuno identity; the old history file is retained as a recovery copy.

## Release files

- Arch Linux `.pkg.tar.zst`
- Fedora 44 `.rpm`
- Ubuntu 24.04 `.deb`
- install tree `.tar.xz`
- source `.tar.xz`
- `PKGBUILD`
- SPDX 2.3 document
- `SHA256SUMS`

Every binary package is built and installed in its native target before publication. Verify downloaded files with:

```bash
sha256sum --ignore-missing --check SHA256SUMS
```

Translation content is sent to the configured external provider. The displayed upstream route is response metadata, not a privacy guarantee. Review OpenRouter and the selected inference provider's current retention and training policies before submitting sensitive text.
