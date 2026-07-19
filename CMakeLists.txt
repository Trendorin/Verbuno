cmake_minimum_required(VERSION 3.25)

project(
    TranslUnix
    VERSION 0.1.0
    DESCRIPTION "A native Linux AI translation client"
    HOMEPAGE_URL "https://github.com/Trendorin/TranslUnix"
    LANGUAGES CXX
)

include(GNUInstallDirs)
include(CTest)

option(TRANSLUNIX_BUILD_TESTS "Build the TranslUnix test suite" ${BUILD_TESTING})
option(TRANSLUNIX_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(TRANSLUNIX_REQUIRE_KEYCHAIN "Require a secure desktop keychain backend" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Core Gui Widgets Network Svg LinguistTools)
find_package(Qt6Keychain CONFIG QUIET)

set(TRANSLUNIX_KEYCHAIN_TARGET "")
if(TARGET Qt6Keychain::Qt6Keychain)
    set(TRANSLUNIX_KEYCHAIN_TARGET Qt6Keychain::Qt6Keychain)
elseif(TARGET qt6keychain)
    set(TRANSLUNIX_KEYCHAIN_TARGET qt6keychain)
endif()

if(TRANSLUNIX_REQUIRE_KEYCHAIN AND NOT TRANSLUNIX_KEYCHAIN_TARGET)
    message(FATAL_ERROR "QtKeychain for Qt 6 is required for secure API-key storage")
endif()

include(cmake/CompilerWarnings.cmake)
include(cmake/Sanitizers.cmake)

add_library(translunix_core STATIC
    src/core/AppSettings.cpp
    src/core/AppSettings.h
    src/core/EndpointValidator.cpp
    src/core/EndpointValidator.h
    src/core/HistoryStore.cpp
    src/core/HistoryStore.h
    src/core/LanguageCatalog.cpp
    src/core/LanguageCatalog.h
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

target_include_directories(translunix_core PUBLIC src)
target_compile_definitions(translunix_core PUBLIC
    TRANSLUNIX_VERSION="${PROJECT_VERSION}"
    TRANSLUNIX_APP_ID="io.github.trendorin.TranslUnix"
)
target_link_libraries(translunix_core
    PUBLIC
        Qt6::Core
        Qt6::Network
)

if(TRANSLUNIX_KEYCHAIN_TARGET)
    target_compile_definitions(translunix_core PRIVATE TRANSLUNIX_HAS_KEYCHAIN=1)
    target_link_libraries(translunix_core PRIVATE ${TRANSLUNIX_KEYCHAIN_TARGET})
endif()

translunix_set_project_warnings(translunix_core)
translunix_enable_sanitizers(translunix_core)

qt_add_executable(translunix
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
    src/ui/TranslatorPopup.cpp
    src/ui/TranslatorPopup.h
    resources/translunix.qrc
)

target_link_libraries(translunix PRIVATE
    translunix_core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Svg
)
translunix_set_project_warnings(translunix)
translunix_enable_sanitizers(translunix)

qt_add_translations(translunix
    TS_FILES translations/translunix_ru.ts
    RESOURCE_PREFIX "/i18n"
)

install(TARGETS translunix
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES data/io.github.trendorin.TranslUnix.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)
install(FILES data/io.github.trendorin.TranslUnix.metainfo.xml
    DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo
)
install(FILES data/icons/hicolor/scalable/apps/io.github.trendorin.TranslUnix.svg
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
    DESTINATION ${CMAKE_INSTALL_DATADIR}/doc/translunix
)
install(FILES packaging/debian/copyright
    DESTINATION ${CMAKE_INSTALL_DATADIR}/doc/translunix
)

if(TRANSLUNIX_BUILD_TESTS)
    find_package(Qt6 6.4 REQUIRED COMPONENTS Test)
    add_subdirectory(tests)
endif()

set(CPACK_PACKAGE_NAME "TranslUnix")
set(CPACK_PACKAGE_VENDOR "Trendorin")
set(CPACK_PACKAGE_CONTACT "Trendorin <193045225+Trendorin@users.noreply.github.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Native Linux AI translation client")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_FILE_NAME "TranslUnix-${PROJECT_VERSION}-${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_GENERATOR "TXZ;DEB;RPM")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_NAME "translunix")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_RPM_PACKAGE_NAME "translunix")
set(CPACK_RPM_FILE_NAME RPM-DEFAULT)
set(CPACK_RPM_PACKAGE_GROUP "Applications/Internet")
set(CPACK_RPM_PACKAGE_LICENSE "GPL-3.0-or-later")
set(CPACK_RPM_PACKAGE_AUTOREQ ON)
include(CPack)
