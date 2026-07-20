cmake_minimum_required(VERSION 3.25)

project(
    Verbuno
    VERSION 0.3.1
    DESCRIPTION "A native Linux AI translation client"
    HOMEPAGE_URL "https://github.com/Trendorin/Verbuno"
    LANGUAGES CXX
)

include(GNUInstallDirs)
include(CTest)

option(VERBUNO_BUILD_TESTS "Build the Verbuno test suite" ${BUILD_TESTING})
option(VERBUNO_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(VERBUNO_REQUIRE_KEYCHAIN "Require a secure desktop keychain backend" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Core Concurrent Gui Widgets Network Svg LinguistTools)
find_package(Qt6Keychain CONFIG QUIET)
find_package(PkgConfig REQUIRED)
pkg_check_modules(Tesseract REQUIRED IMPORTED_TARGET tesseract>=5.0)

set(VERBUNO_KEYCHAIN_TARGET "")
if(TARGET Qt6Keychain::Qt6Keychain)
    set(VERBUNO_KEYCHAIN_TARGET Qt6Keychain::Qt6Keychain)
elseif(TARGET qt6keychain)
    set(VERBUNO_KEYCHAIN_TARGET qt6keychain)
endif()

if(VERBUNO_REQUIRE_KEYCHAIN AND NOT VERBUNO_KEYCHAIN_TARGET)
    message(FATAL_ERROR "QtKeychain for Qt 6 is required for secure API-key storage")
endif()

include(cmake/CompilerWarnings.cmake)
include(cmake/Sanitizers.cmake)

add_library(verbuno_core STATIC
    src/core/AppSettings.cpp
    src/core/AppSettings.h
    src/core/EndpointValidator.cpp
    src/core/EndpointValidator.h
    src/core/HistoryStore.cpp
    src/core/HistoryStore.h
    src/core/InterfaceLanguageManager.cpp
    src/core/InterfaceLanguageManager.h
    src/core/LanguageCatalog.cpp
    src/core/LanguageCatalog.h
    src/core/PhotoOcrEngine.cpp
    src/core/PhotoOcrEngine.h
    src/core/PromptBuilder.cpp
    src/core/PromptBuilder.h
    src/core/ProviderClient.cpp
    src/core/ProviderClient.h
    src/core/SecretStore.cpp
    src/core/SecretStore.h
    src/core/SseDecoder.cpp
    src/core/SseDecoder.h
    src/core/TranslationController.cpp
    src/core/TranslationController.h
    src/core/TranslationTypes.h
)

target_include_directories(verbuno_core PUBLIC src)
target_compile_definitions(verbuno_core PUBLIC
    VERBUNO_VERSION="${PROJECT_VERSION}"
    VERBUNO_APP_ID="io.github.trendorin.Verbuno"
)
target_link_libraries(verbuno_core
    PUBLIC
        Qt6::Core
        Qt6::Concurrent
        Qt6::Gui
        Qt6::Network
        PkgConfig::Tesseract
)

if(VERBUNO_KEYCHAIN_TARGET)
    target_compile_definitions(verbuno_core PRIVATE VERBUNO_HAS_KEYCHAIN=1)
    target_link_libraries(verbuno_core PRIVATE ${VERBUNO_KEYCHAIN_TARGET})
endif()

verbuno_set_project_warnings(verbuno_core)
verbuno_enable_sanitizers(verbuno_core)

qt_add_executable(verbuno
    src/app/main.cpp
    src/app/SingleInstance.cpp
    src/app/SingleInstance.h
    src/app/TrayController.cpp
    src/app/TrayController.h
    src/ui/AboutDialog.cpp
    src/ui/AboutDialog.h
    src/ui/HistoryPage.cpp
    src/ui/HistoryPage.h
    src/ui/MainWindow.cpp
    src/ui/MainWindow.h
    src/ui/SettingsDialog.cpp
    src/ui/SettingsDialog.h
    src/ui/TranslationPanel.cpp
    src/ui/TranslationPanel.h
    resources/verbuno.qrc
)

target_link_libraries(verbuno PRIVATE
    verbuno_core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Svg
)
verbuno_set_project_warnings(verbuno)
verbuno_enable_sanitizers(verbuno)

qt_add_translations(verbuno
    TS_FILES
        translations/verbuno_ru.ts
        translations/verbuno_uk.ts
        translations/verbuno_de.ts
    RESOURCE_PREFIX "/i18n"
)

install(TARGETS verbuno
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES data/io.github.trendorin.Verbuno.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)
install(FILES data/io.github.trendorin.Verbuno.metainfo.xml
    DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo
)
install(FILES data/icons/hicolor/scalable/apps/io.github.trendorin.Verbuno.svg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
)
install(FILES
    README.md
    README.ru.md
    README.uk.md
    README.de.md
    CHANGELOG.md
    CONTRIBUTING.md
    CONTRIBUTORS.md
    RELEASE_NOTES.md
    LICENSE
    SECURITY.md
    SUPPORT.md
    THIRD_PARTY_NOTICES.md
    docs/ARCHITECTURE.md
    docs/PRIVACY.md
    docs/PROJECT_STATE.md
    docs/SECURITY_MODEL.md
    DESTINATION ${CMAKE_INSTALL_DATADIR}/doc/verbuno
)
install(FILES packaging/debian/copyright
    DESTINATION ${CMAKE_INSTALL_DATADIR}/doc/verbuno
)

if(VERBUNO_BUILD_TESTS)
    find_package(Qt6 6.4 REQUIRED COMPONENTS Test)
    add_subdirectory(tests)
endif()

set(CPACK_PACKAGE_NAME "Verbuno")
set(CPACK_PACKAGE_VENDOR "Trendorin")
set(CPACK_PACKAGE_CONTACT "Trendorin <193045225+Trendorin@users.noreply.github.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Native Linux AI translation client")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_FILE_NAME "Verbuno-${PROJECT_VERSION}-${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_GENERATOR "TXZ;DEB;RPM")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_NAME "verbuno")
set(CPACK_DEBIAN_PACKAGE_PROVIDES "translunix")
set(CPACK_DEBIAN_PACKAGE_CONFLICTS "translunix (<< 0.2.0)")
set(CPACK_DEBIAN_PACKAGE_REPLACES "translunix (<< 0.2.0)")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_DEPENDS
    "tesseract-ocr-eng, tesseract-ocr-deu, tesseract-ocr-rus, tesseract-ocr-ukr"
)
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_RPM_PACKAGE_NAME "verbuno")
set(CPACK_RPM_PACKAGE_PROVIDES "translunix = ${PROJECT_VERSION}")
set(CPACK_RPM_PACKAGE_OBSOLETES "translunix < 0.2.0")
set(CPACK_RPM_FILE_NAME RPM-DEFAULT)
set(CPACK_RPM_PACKAGE_GROUP "Applications/Internet")
set(CPACK_RPM_PACKAGE_LICENSE "GPL-3.0-or-later")
set(CPACK_RPM_PACKAGE_AUTOREQ ON)
set(CPACK_RPM_PACKAGE_REQUIRES
    "tesseract-langpack-eng, tesseract-langpack-deu, tesseract-langpack-rus, tesseract-langpack-ukr"
)
include(CPack)
