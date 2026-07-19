# Data-handling policy

## Summary

TranslUnix has no telemetry and no project-operated server. It is a client for a provider selected by the user. A translation cannot be performed without sending the submitted text to that provider.

## Network requests

TranslUnix makes only these application-level requests:

1. an explicit translation request to the configured Chat Completions endpoint;
2. an explicit refresh of the public OpenRouter model catalog.

There is no automatic model refresh, update checker, analytics request, crash upload or background synchronization. Normal DNS, TLS and operating-system networking behavior still applies.

The translation request contains:

- submitted text;
- source and target language instructions;
- optional context and custom translation preference;
- selected model ID and generation settings;
- OpenRouter routing privacy fields when OpenRouter is selected.

It does not contain local history, unrelated clipboard content, filenames, device identifiers or normal application settings.

## Provider responsibility

OpenRouter and each upstream inference provider are separate services with their own terms. `data_collection: deny` asks OpenRouter to exclude endpoints that collect user data. ZDR asks it to use only endpoints marked Zero Data Retention. These controls can make a model unavailable and do not hide ordinary account, billing, IP-address or request-metadata information.

For a custom endpoint, TranslUnix cannot infer or enforce provider retention policy. OpenRouter-specific privacy fields are not sent to that endpoint.

## API keys

- A session key is held in process memory only.
- Persistent storage is opt-in and uses QtKeychain with KWallet or Secret Service.
- QtKeychain's insecure fallback is explicitly disabled.
- A stored key is never displayed back in the settings window.
- Keys are never written to `QSettings`, history, logs, release diagnostics or command-line arguments.
- Removing a key requests deletion from both process memory and the system keychain.

As with any desktop application, secrets exist in process memory while they are in use. TranslUnix reduces unnecessary copies but does not claim protection from a compromised user session, debugger or kernel.

## Local settings and history

Non-secret preferences are stored through Qt `QSettings` under the `Trendorin/TranslUnix` identity.

History is disabled by default. When enabled, each record contains time, language codes, input, output and model ID. The file is size-bounded, record-bounded, age-bounded, written through `QSaveFile`, set to owner read/write permissions and never written through a symbolic link. It can be deleted from Settings or the History page.

The desktop clipboard is controlled by the desktop session. After a translation is copied, its lifetime is outside TranslUnix's control.

## Deletion

1. Remove the API key in **Settings → Provider**.
2. Delete local history in **Settings → Privacy**.
3. Quit TranslUnix to clear session memory.
4. Optionally remove the configuration and application-data directories described in the README.

Deletion from an external provider must be handled through that provider's controls and policy.
