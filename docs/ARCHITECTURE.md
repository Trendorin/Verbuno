# Architecture

Verbuno is one native desktop process. It has no daemon, embedded browser, local HTTP server or project-operated cloud backend.

## Components

| Component | Responsibility |
|---|---|
| `TrayController` | Owns application lifetime, tray integration, the standard main window, settings and live UI-language rebuilds. |
| `TranslationPanel` | Translation interface with the actual provider/model summary, language pair and streamed output. |
| `PhotoOcrEngine` | Discovers installed Tesseract language data, safely decodes images and performs asynchronous, confidence-scored local OCR. |
| `TranslationController` | Validates user intent, obtains a credential, reports credential availability, starts one request and optionally records the result. |
| `ProviderClient` | Builds Chat Completions requests, enforces network policy, decodes SSE, reads response routing metadata and loads OpenRouter models. |
| `PromptBuilder` | Creates a provider-independent translation instruction with formatting and prompt-injection constraints. |
| `SecretStore` | Keeps a session credential in memory and optionally delegates persistence to QtKeychain. |
| `HistoryStore` | Implements opt-in, bounded, atomic local history with owner-only permissions. |
| `AppSettings` | Atomically persists non-secret preferences through `QSettings`, restricts the file to its owner and reports storage failures. |
| `InterfaceLanguageManager` | Loads the embedded Russian, Ukrainian or German Qt catalog and switches the complete UI at runtime. |
| `SingleInstance` | Sends only UI commands between local instances through `QLocalServer`. |

## Request flow

1. The user chooses a language pair and explicitly starts a translation.
2. `TranslationController` validates input length, language selection and endpoint policy.
3. `SecretStore` returns the session key or reads it from the desktop wallet when allowed.
4. `ProviderClient` sends a single HTTPS POST to the configured endpoint.
5. SSE events are parsed incrementally and displayed in the active main window. The API-reported model and, for OpenRouter, selected upstream provider update the visible route summary.
6. The final response is written to local history only when history was enabled before the request; the recorded model is the response-reported model when available.

OpenRouter-only routing fields and the metadata opt-in header are never sent to custom providers. Custom endpoints receive the common `model`, `messages`, `temperature` and `stream` Chat Completions fields.

## Photo flow

1. The user opens, pastes or drops one image and chooses an installed OCR language and page layout. No provider request is created.
2. `PhotoOcrEngine` rejects unreadable files, files over 32 MiB and unsafe dimensions before recognition. Metadata orientation is applied and large images are decoded at a bounded processing size.
3. A QtConcurrent worker initializes Tesseract with the selected system language data. Automatic, single-block and sparse-text segmentation modes are available.
4. The engine first recognizes a grayscale image. When confidence is low, it runs a locally contrast-normalized second pass and keeps the materially better result.
5. The UI shows a bounded preview, source dimensions, OCR language and mean confidence. Extracted text is copied into the editable translation input, truncated to the configured input limit when necessary.
6. Only an explicit **Translate** action starts the normal provider request. The image, filename, dimensions and OCR confidence are not included.

The worker receives value-owned image data and never updates widgets directly. Closing or rebuilding the window disconnects its result delivery safely; all Tesseract work remains outside the UI thread.

## Desktop behavior

`QSystemTrayIcon` is used where a desktop exposes a tray. KDE Plasma is the reference implementation. GNOME installations without an AppIndicator-compatible tray open the main window. The application uses a normal system-decorated `QMainWindow`; minimizing, maximizing, placement and resizing are delegated to the desktop compositor.

The command `verbuno --toggle` communicates with the primary instance. It exists so the desktop's own shortcut manager can provide a global shortcut without X11 grabs or compositor-specific private protocols.

## Non-goals for 0.3

- running a local language model
- handwriting-specific OCR, layout-preserving translated image overlays, PDF or speech translation
- sending images to multimodal cloud models
- automatic clipboard monitoring
- automatic screen capture
- project-operated accounts, sync or proxy service
- provider-specific SDKs beyond the common Chat Completions contract
