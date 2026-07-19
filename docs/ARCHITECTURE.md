# Architecture

TranslUnix is one native desktop process. It has no daemon, embedded browser, local HTTP server or project-operated cloud backend.

## Components

| Component | Responsibility |
|---|---|
| `TrayController` | Owns application lifetime, tray integration, the standard main window, settings and live UI-language rebuilds. |
| `TranslationPanel` | Translation interface with the active provider/model summary, language pair, context and streamed output. |
| `TranslationController` | Validates user intent, obtains a credential, starts one request and optionally records the result. |
| `ProviderClient` | Builds Chat Completions requests, enforces network policy, decodes SSE and loads OpenRouter models. |
| `PromptBuilder` | Creates a provider-independent translation instruction with formatting and prompt-injection constraints. |
| `SecretStore` | Keeps a session credential in memory and optionally delegates persistence to QtKeychain. |
| `HistoryStore` | Implements opt-in, bounded, atomic local history with owner-only permissions. |
| `AppSettings` | Persists non-secret preferences through `QSettings`. |
| `InterfaceLanguageManager` | Loads the embedded Russian, Ukrainian or German Qt catalog and switches the complete UI at runtime. |
| `SingleInstance` | Sends only UI commands between local instances through `QLocalServer`. |

## Request flow

1. The user chooses a language pair and explicitly starts a translation.
2. `TranslationController` validates input length, language selection and endpoint policy.
3. `SecretStore` returns the session key or reads it from the desktop wallet when allowed.
4. `ProviderClient` sends a single HTTPS POST to the configured endpoint.
5. SSE events are parsed incrementally and displayed in the active main window.
6. The final response is written to local history only when history was enabled before the request.

OpenRouter-only routing fields are never sent to custom providers. Custom endpoints receive the common `model`, `messages`, `temperature` and `stream` Chat Completions fields.

## Desktop behavior

`QSystemTrayIcon` is used where a desktop exposes a tray. KDE Plasma is the reference implementation. GNOME installations without an AppIndicator-compatible tray open the main window. The application uses a normal system-decorated `QMainWindow`; minimizing, maximizing, placement and resizing are delegated to the desktop compositor.

The command `translunix --toggle` communicates with the primary instance. It exists so the desktop's own shortcut manager can provide a global shortcut without X11 grabs or compositor-specific private protocols.

## Non-goals for 0.1

- running a local language model
- OCR, image, document or speech translation
- automatic clipboard monitoring
- automatic screen capture
- project-operated accounts, sync or proxy service
- provider-specific SDKs beyond the common Chat Completions contract
