# Data-handling policy

## Summary

Verbuno has no telemetry and no project-operated server. It is a client for a provider selected by the user. A translation cannot be performed without sending the submitted or locally extracted text to that provider. Photo recognition itself is local and does not require a provider.

## Network requests

Verbuno makes only these application-level requests:

1. an explicit translation request to the configured Chat Completions endpoint;
2. an explicit refresh of the public OpenRouter model catalog.

There is no automatic model refresh, update checker, analytics request, crash upload or background synchronization. Normal DNS, TLS and operating-system networking behavior still applies.

The translation request contains:

- submitted text;
- source and target language instructions;
- custom translation preference;
- selected model ID and generation settings;
- OpenRouter routing privacy fields when OpenRouter is selected.

It does not contain local history, unrelated clipboard content, filenames, device identifiers or normal application settings.

## Photos and OCR

- An image is read only after the user opens, pastes or drops it.
- Decoding, orientation handling, scaling, contrast normalization and Tesseract OCR run locally in the Verbuno process.
- Verbuno does not upload image pixels, the local path, filename, dimensions, selected OCR language or confidence score.
- Extracted text is editable and is not sent anywhere until the user explicitly starts a translation.
- Images are not written into local history or copied to a Verbuno cache. A selected image remains in process memory only while the workspace uses it.
- File size, source dimensions, processing dimensions and OCR output are bounded. Unsupported or oversized input is rejected.

Pasting a photo reads the current image offered by the desktop clipboard. Verbuno does not monitor the clipboard. The clipboard manager and the application that originally placed the image there remain outside Verbuno's control.

## Provider responsibility

OpenRouter and each upstream inference provider are separate services with their own terms. `data_collection: deny` asks OpenRouter to exclude endpoints that collect user data. ZDR asks it to use only endpoints marked Zero Data Retention. These controls can make a model unavailable and do not hide ordinary account, billing, IP-address or request-metadata information.

For a custom endpoint, Verbuno cannot infer or enforce provider retention policy. OpenRouter-specific privacy fields are not sent to that endpoint.

## API keys

- A session key is held in process memory only.
- Persistent storage is enabled by default for newly entered keys and uses QtKeychain with KWallet or Secret Service. It can be explicitly disabled for session-only use.
- QtKeychain's insecure fallback is explicitly disabled.
- A stored key is never displayed back in the settings window.
- Keys are never written to `QSettings`, history, logs, release diagnostics or command-line arguments.
- Removing a key requests deletion from both process memory and the system keychain.

As with any desktop application, secrets exist in process memory while they are in use. Verbuno reduces unnecessary copies but does not claim protection from a compromised user session, debugger or kernel.

## Local settings and history

Non-secret preferences are stored through Qt `QSettings` under the stable `Trendorin/Verbuno` identity. The file is synchronized after every change, written atomically where the platform supports it, restricted to owner read/write permissions and refused when its path is a symbolic link. The General page shows the exact path and any detected write or format error.

On the first launch after the product rename, Verbuno copies non-secret preferences and safe local history from the former TranslUnix identity. A remembered key is moved between the corresponding QtKeychain service names only after the new wallet entry is written successfully.

History is disabled by default. When enabled, each record contains time, language codes, text input, text output and model ID. It never contains the source image or its filename. The file is size-bounded, record-bounded, age-bounded, written through `QSaveFile`, set to owner read/write permissions and never written through a symbolic link. It can be deleted from Settings or the History page.

The desktop clipboard is controlled by the desktop session. After a translation is copied, its lifetime is outside Verbuno's control.

## Deletion

1. Remove the API key in **Settings → Provider**.
2. Delete local history in **Settings → Privacy**.
3. Quit Verbuno to clear session memory.
4. Optionally remove the configuration and application-data directories described in the README.

Deletion from an external provider must be handled through that provider's controls and policy.
