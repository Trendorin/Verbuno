<p align="center">
  <img src="data/icons/hicolor/scalable/apps/io.github.trendorin.Verbuno.svg" width="144" alt="Піктограма Verbuno">
</p>

<h1 align="center">Verbuno</h1>
<p align="center">Швидкий переклад обраною моделлю із системного трея Linux.</p>

<p align="center">
  <a href="README.md"><img alt="English" src="https://img.shields.io/badge/EN-English-d7dade?style=flat-square&labelColor=15181b"></a>
  <a href="README.ru.md"><img alt="Русский" src="https://img.shields.io/badge/RU-Русский-d7dade?style=flat-square&labelColor=15181b"></a>
  <a href="README.uk.md"><img alt="Українська" src="https://img.shields.io/badge/UK-Українська-d7dade?style=flat-square&labelColor=15181b"></a>
  <a href="README.de.md"><img alt="Deutsch" src="https://img.shields.io/badge/DE-Deutsch-d7dade?style=flat-square&labelColor=15181b"></a>
</p>

<p align="center">
  <a href="https://github.com/Trendorin/Verbuno/actions/workflows/ci.yml"><img alt="CI" src="https://img.shields.io/github/actions/workflow/status/Trendorin/Verbuno/ci.yml?branch=main&style=flat-square&label=build&labelColor=15181b&color=5d666d"></a>
  <a href="https://github.com/Trendorin/Verbuno/releases/latest"><img alt="Реліз" src="https://img.shields.io/github/v/release/Trendorin/Verbuno?style=flat-square&label=release&labelColor=15181b&color=5d666d"></a>
  <img alt="C++20" src="https://img.shields.io/badge/C%2B%2B-20-5d666d?style=flat-square&labelColor=15181b">
  <img alt="Qt 6 Widgets" src="https://img.shields.io/badge/Qt-6_Widgets-5d666d?style=flat-square&labelColor=15181b">
  <a href="LICENSE"><img alt="GPL-3.0-or-later" src="https://img.shields.io/badge/license-GPL--3.0--or--later-5d666d?style=flat-square&labelColor=15181b"></a>
</p>

<p align="center">
  <a href="#встановлення">Встановлення</a> ·
  <a href="#налаштування">Налаштування</a> ·
  <a href="#збирання-з-коду">Код</a> ·
  <a href="docs/SECURITY_MODEL.md">Безпека</a> ·
  <a href="https://github.com/Trendorin/Verbuno/releases">Релізи</a>
</p>

Verbuno — нативний Linux-клієнт перекладу на C++20 і Qt 6 Widgets. Трей відкриває звичайне системне вікно з рамкою та зміною розміру, відповідь OpenRouter або іншого OpenAI-сумісного API надходить потоком, а фактично вибрані модель і кінцевий провайдер завжди видимі.

## Можливості

| Розділ | Результат |
|---|---|
| Вікно | Звичайне Qt-вікно з нативними кнопками згортання, розгортання та закриття в KDE, GNOME й інших середовищах. |
| Переклад | Близько 190 мовних варіантів, автовизначення, п’ять стилів і збереження форматування. |
| Моделі | Будь-який ID, маршрутизатор `openrouter/free` або актуальний перелік моделей із нульовою заявленою ціною. |
| Провайдери | OpenRouter типово; інтерфейс показує модель і кінцевого провайдера з відповіді, а не лише ID запитаного маршрутизатора. Підтримуються власні OpenAI-сумісні endpoint. |
| Інтерфейс | Миттєве перемикання між англійською, російською, українською та німецькою без перезапуску. |
| Відповідь | Потоковий SSE-вивід, скасування запиту та зрозумілі помилки. |

## Межі приватності

| Verbuno робить | Verbuno не може гарантувати |
|---|---|
| Не містить телеметрії, аналітики чи фонових звернень до моделей. | Що зовнішній провайдер ніколи не зберігає запити й не навчається на них. |
| Надсилає текст лише налаштованому endpoint після явної дії користувача. | Що безкоштовний endpoint завжди буде доступним, швидким і безкоштовним. |
| Зберігає ключ через QtKeychain у KWallet / Secret Service, а не в `QSettings`. | Анонімність від провайдера, який бачить звичайні мережеві та облікові метадані. |
| Типово застосовує OpenRouter `data_collection: deny`; строгий ZDR вмикається окремо. | Доступність моделі після виключення несумісних із приватністю маршрутів. |
| Типово не зберігає історію; увімкнену локальну історію можна негайно стерти. | Захист тексту після копіювання до іншої програми або буфера обміну. |

Перед надсиланням чутливого тексту перевірте актуальні правила OpenRouter щодо [збирання даних](https://openrouter.ai/docs/guides/privacy/data-collection), [логування провайдерів](https://openrouter.ai/docs/guides/privacy/provider-logging) і [ZDR](https://openrouter.ai/docs/guides/features/zdr).

## Встановлення

Завантажте відповідний пакет і `SHA256SUMS` з [останнього релізу](https://github.com/Trendorin/Verbuno/releases/latest).

| Система | Файл | Команда |
|---|---|---|
| Fedora 44 | `verbuno-*.rpm` | `sudo dnf install ./verbuno-*.rpm` |
| Ubuntu 24.04 | `verbuno_*.deb` | `sudo apt install ./verbuno_*.deb` |
| Arch Linux | `verbuno-*-x86_64.pkg.tar.zst` | `sudo pacman -U ./verbuno-*-x86_64.pkg.tar.zst` |

Кожен пакет збирається і перевіряється встановленням у цільовій системі. Реліз також містить `PKGBUILD`, вихідний код і переносне дерево `.tar.xz`, SPDX та контрольні суми. AppImage навмисно не додається.

```bash
sha256sum --ignore-missing --check SHA256SUMS
```

## Налаштування

1. Створіть API-ключ OpenRouter або іншого провайдера.
2. Відкрийте **Налаштування → Провайдер**, вставте ключ і виберіть, чи зберігати його в системному сховищі секретів.
3. Залиште `openrouter/free`, оновіть список безкоштовних моделей або введіть точний ID.
4. Не вимикайте виключення провайдерів, що збирають запити. ZDR вмикайте лише за наявності сумісного маршруту.
5. Виберіть мови й натисніть `Ctrl+Enter`.
6. Мова застосунку змінюється в **Налаштування → Загальні** й одразу застосовується до вікна та меню трея.

Після початку відповіді робоча область показує точну модель з API. Для OpenRouter також видно вибраного кінцевого провайдера, наприклад `Chutes через OpenRouter · qwen/qwen3-…`. У підказці лишається запитана модель або маршрут на кшталт `openrouter/free`.

Сеансовий ключ не записується на диск. Для запам’ятовування потрібен KWallet, GNOME Keyring, KeePassXC або інший Secret Service. Небезпечний plaintext fallback QtKeychain не використовується.

Для іншого сервісу введіть повний URL `/chat/completions` та ID моделі. Незашифрований HTTP дозволений лише для loopback-сервісів із локальною моделлю.

### Робочі середовища

- KDE Plasma — основна ціль із нативним треєм.
- GNOME — для трея може знадобитися AppIndicator; без нього відкриється звичайне вікно.
- Інші Wayland-композитори — стандартна поведінка Qt із безпечним fallback.
- Глобальна клавіша — прив’яжіть `verbuno --toggle` у налаштуваннях KDE/GNOME.

## Збирання з коду

Потрібні CMake 3.25+, Ninja, компілятор C++20, Qt 6.4+ і QtKeychain для Qt 6.

```bash
git clone https://github.com/Trendorin/Verbuno.git
cd Verbuno
cmake --preset release
cmake --build --preset release
ctest --preset release
cmake --install build/release --prefix "$HOME/.local"
```

Залежності Fedora: `gcc-c++ cmake ninja-build qt6-qtbase-devel qt6-qtsvg-devel qt6-qttools-devel qtkeychain-qt6-devel`.

Залежності Ubuntu: `build-essential cmake ninja-build qt6-base-dev qt6-svg-dev qt6-tools-dev qtkeychain-qt6-dev`.

## Видалення

| Встановлення | Команда |
|---|---|
| Fedora 44 | `sudo dnf remove verbuno` |
| Ubuntu 24.04 | `sudo apt remove verbuno` |
| Arch Linux | `sudo pacman -Rns verbuno` |
| З коду | `xargs -r -d '\n' rm -- < build/release/install_manifest.txt` |

## Безпека

- Обов’язковий HTTPS; HTTP дозволений лише для loopback, а облікові дані в URL відхиляються.
- Перенаправлення заборонені, щоб заголовок авторизації не потрапив на інший сервер.
- Розмір відповіді обмежено, повідомлення про помилки очищуються, схожі на ключі рядки приховуються.
- Текст позначається в prompt як недовірені дані, а не інструкції.
- Single-instance команди не приймають ключі або текст через аргументи процесу.

Докладніше: [архітектура](docs/ARCHITECTURE.md), [модель безпеки](docs/SECURITY_MODEL.md), [обробка даних](docs/PRIVACY.md), [уразливості](SECURITY.md), [сторонні компоненти](THIRD_PARTY_NOTICES.md).

## Проєкт

[Зміни](CHANGELOG.md) · [Участь](CONTRIBUTING.md) · [Автори](CONTRIBUTORS.md) · [Підтримка](SUPPORT.md)

Проєкт підтримує [Trendorin](https://github.com/Trendorin). Ліцензія: [GPL-3.0-or-later](LICENSE).
