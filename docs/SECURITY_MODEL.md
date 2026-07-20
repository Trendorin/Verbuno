# Security model

## Protected assets

- provider API keys;
- translation text and translation preferences;
- selected image content and locally extracted text;
- integrity of the configured provider destination;
- local translation history;
- predictable application lifetime and local instance commands.

## Trust boundaries

The local user session, Qt libraries and desktop wallet are inside the operational boundary. The configured API endpoint, OpenRouter, upstream model providers, DNS, certificate authorities and the network are external dependencies.

Verbuno assumes the operating system and current desktop session are not already compromised. It does not defend against root, same-user debuggers, malicious input methods, screen capture, hostile clipboard managers or a provider that violates its stated policy.

## Controls

### Destination and transport

- HTTPS is required for remote endpoints.
- Plain HTTP is accepted only for literal loopback hosts.
- URL user information, query strings and fragments are rejected.
- Automatic HTTP redirects are disabled to avoid forwarding bearer credentials.
- TLS certificate errors use Qt's default fail-closed behavior and are never ignored.

### Credentials

- Credentials are carried only in the authorization header of the selected request.
- Persistent credentials use QtKeychain without plaintext fallback.
- Persistent key storage is the default for a newly entered key; the user can explicitly choose session-only memory instead.
- Provider identity is mapped to a SHA-256-derived wallet account name.
- Likely token values in surfaced provider errors are redacted.
- No command-line option accepts a key.

### Content and responses

- Input length is bounded before a request is created.
- The system instruction treats all submitted text as untrusted translation content.
- Buffered provider responses are capped.
- SSE and non-streaming JSON are parsed structurally; HTML error pages are not rendered.
- Partial requests are cancellable and only completed results enter history.

### Local image processing

- Image selection is explicit; there is no clipboard monitoring or automatic screen capture.
- Files are decoded by Qt and recognized by the system Tesseract library without a network request.
- Input files are limited to 32 MiB, reported dimensions and pixel counts are validated, and processing resolution is bounded before OCR.
- OCR runs outside the UI thread and returns value-owned data through Qt's event system.
- OCR output is bounded again by the configured translation input limit before it can be sent.
- Images and filenames never enter translation history or provider request JSON.

### Local storage and IPC

- History is opt-in, bounded and atomically saved with owner-only permissions.
- Existing history symlinks are rejected.
- Non-secret settings are synchronously written with owner-only permissions; settings-file symlinks are refused and write failures are shown in the UI.
- The single-instance channel accepts a small JSON array of known UI arguments; it does not accept content to translate.
- A stale local socket is removed only after a connection attempt fails.

## Known limitations

- A provider can observe requests sent to it and can return malicious or incorrect text.
- Privacy-routing metadata is only meaningful when the provider implements it; Verbuno currently enforces it only for OpenRouter.
- Wayland window placement is compositor-controlled.
- QtKeychain can prompt or block while a locked wallet is being unlocked by the desktop.
- Free models have external quotas and can disappear without a Verbuno release.
- OCR accuracy depends on image quality, typography and installed Tesseract language data; extracted text must be reviewed before translating sensitive or safety-critical material.

## Reporting

Report security issues using the private process in the repository `SECURITY.md`. Do not include real API keys, private translated text or provider response dumps in a public issue.
