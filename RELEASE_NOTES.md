# Verbuno 0.3.1

This maintenance release makes the selected interface language, provider, exact model and other non-secret preferences survive every application restart, and makes secure API-key reuse the default.

## Highlights

- newly entered API keys are remembered through KWallet or Secret Service by default and are automatically reused after restart;
- session-only key handling remains available by explicitly disabling persistent storage;
- the settings window confirms whether a reusable key is available without displaying the secret;
- secure key writes finish before the settings window closes, so wallet failures remain visible and recoverable;
- non-secret settings are synchronized after every change to the stable `Trendorin/Verbuno` settings file;
- the settings file is restricted to owner read/write permissions and symbolic-link targets are refused;
- the General page displays the exact local settings path and any access or format error;
- restart-level tests cover interface language, provider model, source and target languages, translation style, API-key storage preference and OCR options.

API keys are still never written to `QSettings`, history or logs. Persistent keys remain in the encrypted desktop credential service through QtKeychain, whose plaintext fallback is disabled. Photo OCR remains entirely local; only explicitly translated text reaches the selected provider.

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

Translation text is sent to the configured external provider. The displayed upstream route is response metadata, not a privacy guarantee. Review OpenRouter and the selected inference provider's current retention and training policies before submitting sensitive text.
