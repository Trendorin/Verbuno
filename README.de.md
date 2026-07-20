<p align="center">
  <img src="data/icons/hicolor/scalable/apps/io.github.trendorin.Verbuno.svg" width="144" alt="Verbuno-Anwendungssymbol">
</p>

<h1 align="center">Verbuno</h1>
<p align="center">Schnelle modellgestützte Übersetzung aus der Linux-Systemleiste.</p>

<p align="center">
  <a href="README.md"><img alt="English" src="https://img.shields.io/badge/EN-English-d7dade?style=flat-square&labelColor=15181b"></a>
  <a href="README.ru.md"><img alt="Русский" src="https://img.shields.io/badge/RU-Русский-d7dade?style=flat-square&labelColor=15181b"></a>
  <a href="README.uk.md"><img alt="Українська" src="https://img.shields.io/badge/UK-Українська-d7dade?style=flat-square&labelColor=15181b"></a>
  <a href="README.de.md"><img alt="Deutsch" src="https://img.shields.io/badge/DE-Deutsch-d7dade?style=flat-square&labelColor=15181b"></a>
</p>

<p align="center">
  <a href="https://github.com/Trendorin/Verbuno/actions/workflows/ci.yml"><img alt="CI" src="https://img.shields.io/github/actions/workflow/status/Trendorin/Verbuno/ci.yml?branch=main&style=flat-square&label=build&labelColor=15181b&color=5d666d"></a>
  <a href="https://github.com/Trendorin/Verbuno/releases/latest"><img alt="Release" src="https://img.shields.io/github/v/release/Trendorin/Verbuno?style=flat-square&label=release&labelColor=15181b&color=5d666d"></a>
  <img alt="C++20" src="https://img.shields.io/badge/C%2B%2B-20-5d666d?style=flat-square&labelColor=15181b">
  <img alt="Qt 6 Widgets" src="https://img.shields.io/badge/Qt-6_Widgets-5d666d?style=flat-square&labelColor=15181b">
  <a href="LICENSE"><img alt="GPL-3.0-or-later" src="https://img.shields.io/badge/license-GPL--3.0--or--later-5d666d?style=flat-square&labelColor=15181b"></a>
</p>

<p align="center">
  <a href="#installation">Installation</a> ·
  <a href="#konfiguration">Konfiguration</a> ·
  <a href="#aus-dem-quellcode-bauen">Quellcode</a> ·
  <a href="docs/SECURITY_MODEL.md">Sicherheit</a> ·
  <a href="https://github.com/Trendorin/Verbuno/releases">Releases</a>
</p>

Verbuno ist ein nativer Linux-Übersetzungsclient auf Basis von C++20 und Qt 6 Widgets. Er übersetzt eingegebenen Text oder lokal aus Fotos erkannten Text, streamt Ergebnisse von OpenRouter oder einer anderen OpenAI-kompatiblen API und zeigt das tatsächlich verwendete Modell sowie den Zielanbieter an.

## Funktionen

| Bereich | Ergebnis |
|---|---|
| Fenster | Normales Qt-Fenster mit nativen Schaltflächen zum Minimieren, Maximieren und Schließen unter KDE, GNOME und weiteren Desktops. |
| Übersetzung | Rund 190 Sprachvarianten, automatische Erkennung, fünf Stile und Formaterhaltung. |
| Fotos | PNG-, JPEG-, WebP-, BMP- und TIFF-Bilder öffnen, einfügen oder ablegen; lokale Tesseract-OCR läuft außerhalb des UI-Threads und übergibt editierbaren Text an den normalen Übersetzungseditor. |
| Modelle | Beliebige Modell-ID, `openrouter/free` oder ein aktueller Katalog mit als kostenlos gemeldeten Modellen. |
| Anbieter | Standardmäßig OpenRouter; angezeigt werden Modell und Zielanbieter aus der Antwort statt nur der angeforderten Router-ID. Eigene OpenAI-kompatible Endpunkte werden unterstützt. |
| Oberfläche | Sofortiger Wechsel zwischen Englisch, Russisch, Ukrainisch und Deutsch ohne Neustart. |
| Lokaler Speicher | Anbieter, exaktes Modell, Oberflächen- und Übersetzungssprachen, Übersetzungs-/OCR-Optionen und Tray-Verhalten werden nach einem Neustart wiederhergestellt. |
| Ausgabe | Inkrementelle SSE-Ausgabe, Abbruch und verständliche Anbieterfehler. |

## Datenschutzgrenzen

| Verbuno leistet | Verbuno kann nicht garantieren |
|---|---|
| Keine Telemetrie, Analyse, Absturz-Uploads oder Modellaufrufe im Hintergrund. | Dass ein externer Anbieter Texte niemals protokolliert, speichert oder zum Training nutzt. |
| Ausgewählte Fotos werden lokal dekodiert und erkannt; Pixel und Dateiname gehen nie an den Übersetzungsanbieter. | Perfekte OCR bei unscharfer, handschriftlicher, stark stilisierter oder kontrastarmer Schrift. Der erkannte Text sollte geprüft werden. |
| Text wird erst nach einer ausdrücklichen Aktion an den konfigurierten Endpunkt gesendet. | Dass ein kostenloser Endpunkt dauerhaft verfügbar, schnell oder kostenlos bleibt. |
| Neue Schlüssel werden standardmäßig via QtKeychain in KWallet / Secret Service gespeichert, nie in `QSettings`; der reine Sitzungsmodus bleibt verfügbar. | Anonymität gegenüber dem Anbieter, der normale Netzwerk- und Kontometadaten sieht. |
| OpenRouter `data_collection: deny` ist standardmäßig aktiv; striktes ZDR ist optional. | Modellverfügbarkeit nach dem Ausschluss nicht datenschutzkonformer Routen. |
| Der lokale Verlauf ist standardmäßig aus und kann vollständig gelöscht werden. | Schutz von Text nach dem Kopieren in andere Anwendungen oder die Zwischenablage. |

Vor sensiblen Übersetzungen sollten die aktuellen OpenRouter-Dokumente zu [Datenerfassung](https://openrouter.ai/docs/guides/privacy/data-collection), [Provider-Logging](https://openrouter.ai/docs/guides/privacy/provider-logging) und [ZDR](https://openrouter.ai/docs/guides/features/zdr) geprüft werden.

## Installation

Das passende Paket und `SHA256SUMS` aus dem [neuesten Release](https://github.com/Trendorin/Verbuno/releases/latest) herunterladen.

| System | Datei | Befehl |
|---|---|---|
| Fedora 44 | `verbuno-*.rpm` | `sudo dnf install ./verbuno-*.rpm` |
| Ubuntu 24.04 | `verbuno_*.deb` | `sudo apt install ./verbuno_*.deb` |
| Arch Linux | `verbuno-*-x86_64.pkg.tar.zst` | `sudo pacman -U ./verbuno-*-x86_64.pkg.tar.zst` |

Jedes Binärpaket wird im jeweiligen Zielsystem gebaut und installiert getestet. Der Release enthält außerdem `PKGBUILD`, Quell- und Installationsarchive im Format `.tar.xz`, ein SPDX-Dokument und Prüfsummen. AppImage gehört bewusst nicht zum Release.

Die nativen Pakete installieren OCR-Daten für Englisch, Deutsch, Russisch und Ukrainisch. Verbuno erkennt außerdem weitere Tesseract-Sprachpakete des Systems. Japanisch lässt sich beispielsweise mit `tesseract-ocr-jpn` unter Ubuntu, `tesseract-langpack-jpn` unter Fedora oder `tesseract-data-jpn` unter Arch ergänzen; danach Verbuno neu starten.

```bash
sha256sum --ignore-missing --check SHA256SUMS
```

## Konfiguration

1. Einen API-Schlüssel bei OpenRouter oder einem anderen Anbieter erstellen.
2. **Einstellungen → Anbieter** öffnen und den Schlüssel einfügen. Sichere Wiederverwendung nach einem Neustart ist standardmäßig aktiv und kann für einen reinen Sitzungsschlüssel deaktiviert werden.
3. `openrouter/free` verwenden, den aktuellen kostenlosen Katalog laden oder eine exakte Modell-ID eintragen.
4. Den Ausschluss datensammelnder Anbieter aktiviert lassen. ZDR nur aktivieren, wenn eine passende Route existiert.
5. Für ein Foto **Foto öffnen** wählen, ein Bild einfügen oder in den Arbeitsbereich ziehen. Bei Bedarf OCR-Sprache und Layout wählen, den lokal erkannten Text prüfen und **Übersetzen** drücken.
6. Für normalen Text die Sprachen wählen und mit `Ctrl+Enter` übersetzen.
7. Die Sprache unter **Einstellungen → Allgemein** wählen; Fenster und Tray-Menü werden sofort aktualisiert.

Die Fotoerkennung allein erzeugt keine Anbieteranfrage. Das Bild wird mit strikten Größenlimits dekodiert, anhand seiner Metadaten ausgerichtet, sicher skaliert und im Hintergrund erkannt. Bei geringer Genauigkeit läuft ein zweiter lokaler Kontrastdurchgang. Nur der editierbare erkannte Text gelangt nach einem ausdrücklichen Klick in den bestehenden Übersetzungsablauf.

Sobald die Antwort beginnt, zeigt der Arbeitsbereich das von der API gemeldete Modell. Bei OpenRouter wird auch der gewählte Zielanbieter angezeigt, etwa `Chutes über OpenRouter · qwen/qwen3-…`. Der Tooltip enthält weiterhin das angeforderte Modell oder den Router wie `openrouter/free`.

Ein Sitzungsschlüssel wird nicht auf die Festplatte geschrieben. Für dauerhaftes Speichern wird KWallet, GNOME Keyring, KeePassXC oder ein anderer Secret Service benötigt. Verbuno wartet auf das Ergebnis des Schlüsselbunds; der unsichere Klartext-Fallback von QtKeychain bleibt deaktiviert.

Alle nicht geheimen Optionen werden automatisch in der stabilen lokalen Einstellungsdatei gespeichert, deren Pfad unter **Einstellungen → Allgemein** angezeigt wird. Die Datei ist nur für den Eigentümer zugänglich; Schreib- oder Formatfehler werden gemeldet, statt Oberflächensprache, Anbieter, Modell oder Sprachpaar still zu verlieren.

Für einen anderen Dienst werden die vollständige `/chat/completions`-URL und die Modell-ID eingetragen. Unverschlüsseltes HTTP ist nur für Loopback-Dienste erlaubt.

### Desktop-Verhalten

- KDE Plasma ist das Hauptziel mit nativer Tray-Unterstützung.
- GNOME benötigt möglicherweise AppIndicator; ohne Tray wird das Hauptfenster geöffnet.
- Andere Wayland-Compositoren verwenden Standard-Qt-Verhalten mit sicherem Fallback.
- Globale Taste: `verbuno --toggle` in den KDE/GNOME-Tastatureinstellungen binden.

## Aus dem Quellcode bauen

Erforderlich sind CMake 3.25+, Ninja, ein C++20-Compiler, Qt 6.4+ mit Concurrent, QtKeychain für Qt 6 und Tesseract OCR 5+.

```bash
git clone https://github.com/Trendorin/Verbuno.git
cd Verbuno
cmake --preset release
cmake --build --preset release
ctest --preset release
cmake --install build/release --prefix "$HOME/.local"
```

Fedora-Abhängigkeiten: `gcc-c++ cmake ninja-build qt6-qtbase-devel qt6-qtsvg-devel qt6-qttools-devel qtkeychain-qt6-devel tesseract-devel tesseract-langpack-eng tesseract-langpack-deu tesseract-langpack-rus tesseract-langpack-ukr`.

Ubuntu-Abhängigkeiten: `build-essential cmake ninja-build qt6-base-dev qt6-svg-dev qt6-tools-dev qtkeychain-qt6-dev libtesseract-dev tesseract-ocr-eng tesseract-ocr-deu tesseract-ocr-rus tesseract-ocr-ukr`.

## Deinstallation

| Installation | Befehl |
|---|---|
| Fedora 44 | `sudo dnf remove verbuno` |
| Ubuntu 24.04 | `sudo apt remove verbuno` |
| Arch Linux | `sudo pacman -Rns verbuno` |
| Quellcode | `xargs -r -d '\n' rm -- < build/release/install_manifest.txt` |

## Sicherheitsgrenze

- HTTPS ist Pflicht; HTTP ist nur auf Loopback erlaubt, Zugangsdaten in URLs werden abgelehnt.
- Weiterleitungen werden verweigert, damit Autorisierungsheader nicht an andere Server gelangen.
- Antwortgrößen sind begrenzt, Fehlermeldungen werden bereinigt und schlüsselähnliche Werte redigiert.
- Dateigröße, dekodierte Bildabmessungen und OCR-Ausgabe sind begrenzt; nicht unterstützte oder übergroße Eingaben werden vor der Erkennung abgelehnt.
- Übersetzungstext wird im Prompt als nicht vertrauenswürdiger Inhalt behandelt.
- Single-Instance-Befehle akzeptieren weder Schlüssel noch Übersetzungstext als Prozessargumente.

Weitere Details: [Architektur](docs/ARCHITECTURE.md), [Sicherheitsmodell](docs/SECURITY_MODEL.md), [Datenverarbeitung](docs/PRIVACY.md), [Schwachstellen](SECURITY.md), [Drittkomponenten](THIRD_PARTY_NOTICES.md).

## Projekt

[Änderungen](CHANGELOG.md) · [Mitwirken](CONTRIBUTING.md) · [Mitwirkende](CONTRIBUTORS.md) · [Support](SUPPORT.md)

Betreut von [Trendorin](https://github.com/Trendorin). Lizenziert unter [GPL-3.0-or-later](LICENSE).
