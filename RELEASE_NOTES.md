# TranslUnix 0.1.0

The first native Linux release provides a fast tray-first translation workflow without an Electron runtime or project-operated backend.

## Highlights

- compact animated tray popup and complete desktop window;
- OpenRouter streaming and custom OpenAI-compatible endpoints;
- roughly 190 language variants, automatic detection, context and translation styles;
- `openrouter/free` plus live filtering of the current zero-price model catalog;
- secure session or QtKeychain credential storage with no plaintext fallback;
- data-collection-deny routing by default and optional strict ZDR;
- local history off by default and immediately clearable when enabled.

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

Translation content is sent to the configured external provider. Review that provider's current retention and training policy before submitting sensitive text.
