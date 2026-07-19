qt_add_executable(translunix_tests
    CoreTests.cpp
)

target_link_libraries(translunix_tests PRIVATE
    translunix_core
    Qt6::Test
)
translunix_set_project_warnings(translunix_tests)
translunix_enable_sanitizers(translunix_tests)

add_test(NAME translunix-core COMMAND translunix_tests)
set_tests_properties(translunix-core PROPERTIES TIMEOUT 30)
