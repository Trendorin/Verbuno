# TranslUnix 0.1.1

This release makes TranslUnix behave like a conventional native Qt desktop application and adds complete runtime interface localization.

## Highlights

- standard system-decorated main window with native minimize, maximize, close and resize behavior;
- tray click and `translunix --toggle` operate on that normal window without a frameless popup;
- active provider and exact model ID remain visible in the toolbar and translation workspace;
- English, Russian, Ukrainian and German can be selected under **Settings → General**;
- the interface, tray menu, privacy guidance, validation messages and provider errors update without restarting;
- translator input is preserved while rebuilding the interface after a language change.

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
