<p align="center">
  <img src="data/icons/hicolor/scalable/apps/io.github.trendorin.TranslUnix.svg" width="144" alt="TranslUnix-Anwendungssymbol">
</p>

<h1 align="center">TranslUnix</h1>
<p align="center">Schnelle modellgestützte Übersetzung aus der Linux-Systemleiste.</p>

<p align="center">
  <a href="README.md"><img alt="English" src="https://img.shields.io/badge/EN-English-d7dade?style=flat-square&labelColor=15181b"></a>
  <a href="README.ru.md"><img alt="Русский" src="https://img.shields.io/badge/RU-Русский-d7dade?style=flat-square&labelColor=15181b"></a>
  <a href="README.uk.md"><img alt="Українська" src="https://img.shields.io/badge/UK-Українська-d7dade?style=flat-square&labelColor=15181b"></a>
  <a href="README.de.md"><img alt="Deutsch" src="https://img.shields.io/badge/DE-Deutsch-d7dade?style=flat-square&labelColor=15181b"></a>
</p>

<p align="center">
  <a href="https://github.com/Trendorin/TranslUnix/actions/workflows/ci.yml"><img alt="CI" src="https://img.shields.io/github/actions/workflow/status/Trendorin/TranslUnix/ci.yml?branch=main&style=flat-square&label=build&labelColor=15181b&color=5d666d"></a>
  <a href="https://github.com/Trendorin/TranslUnix/releases/latest"><img alt="Release" src="https://img.shields.io/github/v/release/Trendorin/TranslUnix?style=flat-square&label=release&labelColor=15181b&color=5d666d"></a>
  <img alt="C++20" src="https://img.shields.io/badge/C%2B%2B-20-5d666d?style=flat-square&labelColor=15181b">
  <img alt="Qt 6 Widgets" src="https://img.shields.io/badge/Qt-6_Widgets-5d666d?style=flat-square&labelColor=15181b">
  <a href="LICENSE"><img alt="GPL-3.0-or-later" src="https://img.shields.io/badge/license-GPL--3.0--or--later-5d666d?style=flat-square&labelColor=15181b"></a>
</p>

<p align="center">
  <a href="#installation">Installation</a> ·
  <a href="#konfiguration">Konfiguration</a> ·
  <a href="#aus-dem-quellcode-bauen">Quellcode</a> ·
  <a href="docs/SECURITY_MODEL.md">Sicherheit</a> ·
  <a href="https://github.com/Trendorin/TranslUnix/releases">Releases</a>
</p>

TranslUnix ist ein nativer Linux-Übersetzungsclient auf Basis von C++20 und Qt 6 Widgets. Die Systemleiste öffnet ein normales, vom Desktop verwaltetes Fenster, Antworten von OpenRouter oder einer anderen OpenAI-kompatiblen API werden gestreamt und der aktive Anbieter sowie das Modell bleiben sichtbar.

## Funktionen

| Bereich | Ergebnis |
|---|---|
| Fenster | Normales Qt-Fenster mit nativen Schaltflächen zum Minimieren, Maximieren und Schließen unter KDE, GNOME und weiteren Desktops. |
| Übersetzung | Rund 190 Sprachvarianten, automatische Erkennung, fünf Stile und Formaterhaltung. |
| Modelle | Beliebige Modell-ID, `openrouter/free` oder ein aktueller Katalog mit als kostenlos gemeldeten Modellen. |
| Anbieter | Standardmäßig OpenRouter; eigene OpenAI-kompatible Chat-Completions-Endpunkte. |
| Oberfläche | Sofortiger Wechsel zwischen Englisch, Russisch, Ukrainisch und Deutsch ohne Neustart. |
| Ausgabe | Inkrementelle SSE-Ausgabe, Abbruch und verständliche Anbieterfehler. |

## Datenschutzgrenzen

| TranslUnix leistet | TranslUnix kann nicht garantieren |
|---|---|
| Keine Telemetrie, Analyse, Absturz-Uploads oder Modellaufrufe im Hintergrund. | Dass ein externer Anbieter Texte niemals protokolliert, speichert oder zum Training nutzt. |
| Text wird erst nach einer ausdrücklichen Aktion an den konfigurierten Endpunkt gesendet. | Dass ein kostenloser Endpunkt dauerhaft verfügbar, schnell oder kostenlos bleibt. |
| Gespeicherte Schlüssel liegen via QtKeychain in KWallet / Secret Service, nicht in `QSettings`. | Anonymität gegenüber dem Anbieter, der normale Netzwerk- und Kontometadaten sieht. |
| OpenRouter `data_collection: deny` ist standardmäßig aktiv; striktes ZDR ist optional. | Modellverfügbarkeit nach dem Ausschluss nicht datenschutzkonformer Routen. |
| Der lokale Verlauf ist standardmäßig aus und kann vollständig gelöscht werden. | Schutz von Text nach dem Kopieren in andere Anwendungen oder die Zwischenablage. |

Vor sensiblen Übersetzungen sollten die aktuellen OpenRouter-Dokumente zu [Datenerfassung](https://openrouter.ai/docs/guides/privacy/data-collection), [Provider-Logging](https://openrouter.ai/docs/guides/privacy/provider-logging) und [ZDR](https://openrouter.ai/docs/guides/features/zdr) geprüft werden.

## Installation

Das passende Paket und `SHA256SUMS` aus dem [neuesten Release](https://github.com/Trendorin/TranslUnix/releases/latest) herunterladen.

| System | Datei | Befehl |
|---|---|---|
| Fedora 44 | `translunix-*.rpm` | `sudo dnf install ./translunix-*.rpm` |
| Ubuntu 24.04 | `translunix_*.deb` | `sudo apt install ./translunix_*.deb` |
| Arch Linux | `translunix-*-x86_64.pkg.tar.zst` | `sudo pacman -U ./translunix-*-x86_64.pkg.tar.zst` |

Jedes Binärpaket wird im jeweiligen Zielsystem gebaut und installiert getestet. Der Release enthält außerdem `PKGBUILD`, Quell- und Installationsarchive im Format `.tar.xz`, ein SPDX-Dokument und Prüfsummen. AppImage gehört bewusst nicht zum Release.

```bash
sha256sum --ignore-missing --check SHA256SUMS
```

## Konfiguration

1. Einen API-Schlüssel bei OpenRouter oder einem anderen Anbieter erstellen.
2. **Einstellungen → Anbieter** öffnen, den Schlüssel einfügen und die Speicherung im System-Schlüsselbund wählen.
3. `openrouter/free` verwenden, den aktuellen kostenlosen Katalog laden oder eine exakte Modell-ID eintragen.
4. Den Ausschluss datensammelnder Anbieter aktiviert lassen. ZDR nur aktivieren, wenn eine passende Route existiert.
5. Sprachen wählen und mit `Ctrl+Enter` übersetzen.
6. Die Sprache unter **Einstellungen → Allgemein** wählen; Fenster und Tray-Menü werden sofort aktualisiert.

Ein Sitzungsschlüssel wird nicht auf die Festplatte geschrieben. Für dauerhaftes Speichern wird KWallet, GNOME Keyring, KeePassXC oder ein anderer Secret Service benötigt. Der unsichere Klartext-Fallback von QtKeychain ist deaktiviert.

Für einen anderen Dienst werden die vollständige `/chat/completions`-URL und die Modell-ID eingetragen. Unverschlüsseltes HTTP ist nur für Loopback-Dienste erlaubt.

### Desktop-Verhalten

- KDE Plasma ist das Hauptziel mit nativer Tray-Unterstützung.
- GNOME benötigt möglicherweise AppIndicator; ohne Tray wird das Hauptfenster geöffnet.
- Andere Wayland-Compositoren verwenden Standard-Qt-Verhalten mit sicherem Fallback.
- Globale Taste: `translunix --toggle` in den KDE/GNOME-Tastatureinstellungen binden.

## Aus dem Quellcode bauen

Erforderlich sind CMake 3.25+, Ninja, ein C++20-Compiler, Qt 6.4+ und QtKeychain für Qt 6.

```bash
git clone https://github.com/Trendorin/TranslUnix.git
cd TranslUnix
cmake --preset release
cmake --build --preset release
ctest --preset release
cmake --install build/release --prefix "$HOME/.local"
```

Fedora-Abhängigkeiten: `gcc-c++ cmake ninja-build qt6-qtbase-devel qt6-qtsvg-devel qt6-qttools-devel qtkeychain-qt6-devel`.

Ubuntu-Abhängigkeiten: `build-essential cmake ninja-build qt6-base-dev qt6-svg-dev qt6-tools-dev qtkeychain-qt6-dev`.

## Deinstallation

| Installation | Befehl |
|---|---|
| Fedora 44 | `sudo dnf remove translunix` |
| Ubuntu 24.04 | `sudo apt remove translunix` |
| Arch Linux | `sudo pacman -Rns translunix` |
| Quellcode | `xargs -r -d '\n' rm -- < build/release/install_manifest.txt` |

## Sicherheitsgrenze

- HTTPS ist Pflicht; HTTP ist nur auf Loopback erlaubt, Zugangsdaten in URLs werden abgelehnt.
- Weiterleitungen werden verweigert, damit Autorisierungsheader nicht an andere Server gelangen.
- Antwortgrößen sind begrenzt, Fehlermeldungen werden bereinigt und schlüsselähnliche Werte redigiert.
- Übersetzungstext wird im Prompt als nicht vertrauenswürdiger Inhalt behandelt.
- Single-Instance-Befehle akzeptieren weder Schlüssel noch Übersetzungstext als Prozessargumente.

Weitere Details: [Architektur](docs/ARCHITECTURE.md), [Sicherheitsmodell](docs/SECURITY_MODEL.md), [Datenverarbeitung](docs/PRIVACY.md), [Schwachstellen](SECURITY.md), [Drittkomponenten](THIRD_PARTY_NOTICES.md).

## Projekt

[Änderungen](CHANGELOG.md) · [Mitwirken](CONTRIBUTING.md) · [Mitwirkende](CONTRIBUTORS.md) · [Support](SUPPORT.md)

Betreut von [Trendorin](https://github.com/Trendorin). Lizenziert unter [GPL-3.0-or-later](LICENSE).
