<p align="center">
  <img src="data/icons/hicolor/scalable/apps/io.github.trendorin.Verbuno.svg" width="144" alt="Иконка Verbuno">
</p>

<h1 align="center">Verbuno</h1>
<p align="center">Быстрый перевод через выбранную модель прямо из системного трея Linux.</p>

<p align="center">
  <a href="README.md"><img alt="English" src="https://img.shields.io/badge/EN-English-d7dade?style=flat-square&labelColor=15181b"></a>
  <a href="README.ru.md"><img alt="Русский" src="https://img.shields.io/badge/RU-Русский-d7dade?style=flat-square&labelColor=15181b"></a>
  <a href="README.uk.md"><img alt="Українська" src="https://img.shields.io/badge/UK-Українська-d7dade?style=flat-square&labelColor=15181b"></a>
  <a href="README.de.md"><img alt="Deutsch" src="https://img.shields.io/badge/DE-Deutsch-d7dade?style=flat-square&labelColor=15181b"></a>
</p>

<p align="center">
  <a href="https://github.com/Trendorin/Verbuno/actions/workflows/ci.yml"><img alt="CI" src="https://img.shields.io/github/actions/workflow/status/Trendorin/Verbuno/ci.yml?branch=main&style=flat-square&label=build&labelColor=15181b&color=5d666d"></a>
  <a href="https://github.com/Trendorin/Verbuno/releases/latest"><img alt="Релиз" src="https://img.shields.io/github/v/release/Trendorin/Verbuno?style=flat-square&label=release&labelColor=15181b&color=5d666d"></a>
  <img alt="C++20" src="https://img.shields.io/badge/C%2B%2B-20-5d666d?style=flat-square&labelColor=15181b">
  <img alt="Qt 6 Widgets" src="https://img.shields.io/badge/Qt-6_Widgets-5d666d?style=flat-square&labelColor=15181b">
  <a href="LICENSE"><img alt="GPL-3.0-or-later" src="https://img.shields.io/badge/license-GPL--3.0--or--later-5d666d?style=flat-square&labelColor=15181b"></a>
</p>

<p align="center">
  <a href="#установка">Установка</a> ·
  <a href="#настройка">Настройка</a> ·
  <a href="#сборка-из-исходников">Исходники</a> ·
  <a href="docs/SECURITY_MODEL.md">Безопасность</a> ·
  <a href="https://github.com/Trendorin/Verbuno/releases">Релизы</a>
</p>

Verbuno — нативный Linux-клиент для перевода на C++20 и Qt 6 Widgets. Он переводит введённый текст и текст, локально распознанный на фото, потоково получает результат от OpenRouter или другого OpenAI-совместимого API и всегда показывает фактические модель и конечного провайдера.

## Возможности

| Раздел | Результат |
|---|---|
| Окно | Обычное Qt-окно с нативными кнопками свернуть, развернуть и закрыть в KDE, GNOME и других средах. |
| Перевод | Около 190 языковых вариантов, автоопределение, пять стилей и сохранение форматирования. |
| Фото | Открытие, вставка и перетаскивание PNG, JPEG, WebP, BMP и TIFF; локальный Tesseract OCR работает вне UI-потока и помещает редактируемый текст в обычное поле перевода. |
| Модели | Любой ID модели, роутер `openrouter/free` или актуальный список моделей с нулевой заявленной ценой. |
| Провайдеры | OpenRouter по умолчанию; интерфейс показывает модель и конечного провайдера из ответа, а не просто повторяет ID запрошенного роутера. Поддерживаются свои OpenAI-совместимые endpoint. |
| Интерфейс | Мгновенное переключение между английским, русским, украинским и немецким без перезапуска. |
| Локальное хранение | Провайдер, точная модель, языки интерфейса и перевода, параметры перевода/OCR и поведение трея восстанавливаются после перезапуска. |
| Ответ | Потоковый SSE-вывод, отмена запроса и понятные ошибки провайдера. |

## Границы приватности

| Verbuno делает | Verbuno не может гарантировать |
|---|---|
| Не содержит телеметрии, аналитики и фоновых запросов к моделям. | Что внешний провайдер никогда не хранит запросы и не обучается на них. |
| Декодирует и распознаёт выбранные фото локально; пиксели и имя файла не отправляются провайдеру перевода. | Идеальное OCR размытого, рукописного, стилизованного или малоконтрастного текста. Результат нужно проверить. |
| Отправляет текст только выбранному endpoint после явного запуска перевода. | Что бесплатный endpoint навсегда останется быстрым, доступным и бесплатным. |
| По умолчанию запоминает новый ключ через QtKeychain в KWallet / Secret Service, а не в `QSettings`; сессионный режим остаётся доступен. | Анонимность от провайдера, который видит обычные сетевые и аккаунтные метаданные. |
| По умолчанию включает OpenRouter `data_collection: deny`; строгий ZDR включается отдельно. | Доступность модели после исключения несовместимых с приватностью маршрутов. |
| По умолчанию не ведёт историю; включённую историю можно сразу полностью удалить. | Защиту текста после его копирования в другое приложение или буфер обмена. |

По данным OpenRouter, хранение содержимого запросов на самой платформе включается добровольно, но у конечных провайдеров действуют отдельные правила. Перед передачей чувствительного текста проверьте актуальные документы OpenRouter о [сборе данных](https://openrouter.ai/docs/guides/privacy/data-collection), [логировании провайдеров](https://openrouter.ai/docs/guides/privacy/provider-logging) и [ZDR](https://openrouter.ai/docs/guides/features/zdr).

## Установка

Скачайте подходящий пакет и `SHA256SUMS` из [последнего релиза](https://github.com/Trendorin/Verbuno/releases/latest).

| Система | Файл | Команда |
|---|---|---|
| Fedora 44 | `verbuno-*.rpm` | `sudo dnf install ./verbuno-*.rpm` |
| Ubuntu 24.04 | `verbuno_*.deb` | `sudo apt install ./verbuno_*.deb` |
| Arch Linux | `verbuno-*-x86_64.pkg.tar.zst` | `sudo pacman -U ./verbuno-*-x86_64.pkg.tar.zst` |

Каждый бинарный пакет собирается и проверяется установкой в родной системе. В релиз также входят `PKGBUILD`, исходники и переносимое дерево в `.tar.xz`, SPDX и контрольные суммы. AppImage намеренно исключён.

Нативные пакеты устанавливают данные OCR для английского, немецкого, русского и украинского. Verbuno также обнаруживает другие системные языковые пакеты Tesseract. Например, для японского установите `tesseract-ocr-jpn` в Ubuntu, `tesseract-langpack-jpn` в Fedora или `tesseract-data-jpn` в Arch, затем перезапустите Verbuno.

```bash
sha256sum --ignore-missing --check SHA256SUMS
```

## Настройка

1. Создайте API-ключ OpenRouter или другого провайдера.
2. Откройте **Настройки → Провайдер** и вставьте ключ. Безопасное повторное использование после перезапуска включено по умолчанию; отключайте его только для сессионного ключа.
3. Оставьте `openrouter/free`, обновите список бесплатных моделей или укажите точный ID модели.
4. Не отключайте **Исключать провайдеров, собирающих запросы**. ZDR включайте, только если у модели есть совместимый маршрут.
5. Для фото нажмите **Открыть фото**, вставьте изображение или перетащите его в рабочую область. При необходимости выберите язык и разметку OCR, проверьте локально извлечённый текст и нажмите **Перевести**.
6. Для обычного текста выберите языки и нажмите `Ctrl+Enter`.
7. Язык приложения меняется в **Настройки → Общие** и применяется сразу ко всему окну и меню трея.

Распознавание фото само по себе не создаёт запрос к провайдеру. Изображение декодируется с жёсткими ограничениями размера, автоматически поворачивается по метаданным, безопасно масштабируется и обрабатывается в фоне. При низкой точности выполняется второй локальный проход с усилением контраста. Во внешний API попадает только проверяемый текст и только после явного нажатия кнопки перевода.

После начала ответа рабочая область показывает точную модель из API. Для OpenRouter также виден выбранный конечный провайдер, например `Chutes через OpenRouter · qwen/qwen3-…`. В подсказке остаётся запрошенная модель или роутер, например `openrouter/free`.

Сессионный ключ не записывается на диск. Для сохранения нужен KWallet, GNOME Keyring, KeePassXC или другой Secret Service. Verbuno дожидается результата системного хранилища и никогда не включает небезопасный plaintext fallback QtKeychain.

Все несекретные параметры автоматически сохраняются в стабильный локальный файл, путь к которому показан в **Настройки → Общие**. Доступ к файлу имеет только владелец; при ошибке записи или формата Verbuno сообщает об этом, а не молча теряет язык интерфейса, провайдера, модель или языковую пару.

Для другого сервиса укажите полный URL `/chat/completions` и ID модели. Обычный HTTP запрещён, кроме loopback-сервисов с локальной моделью.

### Рабочие столы

- KDE Plasma — основной вариант с нативным треем.
- GNOME — для трея может понадобиться AppIndicator; без него откроется обычное окно.
- Другие Wayland-композиторы — используется стандартное поведение Qt с безопасным fallback.
- Глобальная клавиша — назначьте команду `verbuno --toggle` в настройках KDE/GNOME.

## Сборка из исходников

Нужны CMake 3.25+, Ninja, компилятор C++20, Qt 6.4+ с Concurrent, QtKeychain для Qt 6 и Tesseract OCR 5+.

<details>
<summary>Fedora</summary>

```bash
sudo dnf install gcc-c++ cmake ninja-build \
  qt6-qtbase-devel qt6-qtsvg-devel qt6-qttools-devel \
  qtkeychain-qt6-devel tesseract-devel \
  tesseract-langpack-eng tesseract-langpack-deu \
  tesseract-langpack-rus tesseract-langpack-ukr
```
</details>

<details>
<summary>Ubuntu 24.04+</summary>

```bash
sudo apt install build-essential cmake ninja-build \
  qt6-base-dev qt6-svg-dev qt6-tools-dev qtkeychain-qt6-dev \
  libtesseract-dev tesseract-ocr-eng tesseract-ocr-deu \
  tesseract-ocr-rus tesseract-ocr-ukr
```
</details>

```bash
git clone https://github.com/Trendorin/Verbuno.git
cd Verbuno
cmake --preset release
cmake --build --preset release
ctest --preset release
cmake --install build/release --prefix "$HOME/.local"
```

## Удаление

| Установка | Команда |
|---|---|
| Fedora 44 | `sudo dnf remove verbuno` |
| Ubuntu 24.04 | `sudo apt remove verbuno` |
| Arch Linux | `sudo pacman -Rns verbuno` |
| Из исходников | `xargs -r -d '\n' rm -- < build/release/install_manifest.txt` |

Перед удалением приложения удалите сохранённый ключ и очистите историю из настроек. При необходимости затем удалите `$XDG_CONFIG_HOME/Trendorin/Verbuno.conf` и `$XDG_DATA_HOME/Trendorin/Verbuno/`.

При обновлении с прежнего TranslUnix первый запуск переносит обычные настройки, локальную историю и сохранённый ключ кошелька. Старый пакет заменяется пакетом `verbuno`.

## Безопасность

- Разрешён HTTPS и HTTP только для loopback; учётные данные внутри URL отклоняются.
- Редиректы запрещены, чтобы заголовок авторизации не ушёл на другой сервер.
- Ответ ограничен по размеру, ошибки очищаются, похожие на ключи строки скрываются.
- Размер файла, декодированного изображения и текста OCR ограничен; неподдерживаемые и чрезмерно большие данные отклоняются до распознавания.
- Переводимый текст отмечается в prompt как недоверенные данные, а не инструкции.
- Single-instance команды не принимают API-ключи или переводимый текст через аргументы процесса.

Подробнее: [архитектура](docs/ARCHITECTURE.md), [модель безопасности](docs/SECURITY_MODEL.md), [обработка данных](docs/PRIVACY.md), [уязвимости](SECURITY.md), [сторонние компоненты](THIRD_PARTY_NOTICES.md).

## Проект

[Изменения](CHANGELOG.md) · [Участие](CONTRIBUTING.md) · [Авторы](CONTRIBUTORS.md) · [Поддержка](SUPPORT.md)

Проект поддерживает [Trendorin](https://github.com/Trendorin). Лицензия: [GPL-3.0-or-later](LICENSE).
