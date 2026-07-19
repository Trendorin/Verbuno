qt_add_executable(verbuno_tests
    CoreTests.cpp
)

target_link_libraries(verbuno_tests PRIVATE
    verbuno_core
    Qt6::Test
)
verbuno_set_project_warnings(verbuno_tests)
verbuno_enable_sanitizers(verbuno_tests)

add_test(NAME verbuno-core COMMAND verbuno_tests)
set_tests_properties(verbuno-core PROPERTIES TIMEOUT 30)

add_test(NAME verbuno-translations COMMAND verbuno --check-translations)
set_tests_properties(verbuno-translations PROPERTIES
    TIMEOUT 30
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)
