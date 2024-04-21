set(TEST_SOURCES
        test_add.cpp
        test_assembler.cpp
        test_disassembler.cpp
        test_ldi.cpp
        test_ldm.cpp
        test_ldr.cpp
        test_stack.cpp
        test_store.cpp)

set(TEST_DEPENDENCIES
        ../src/assembler.cpp
        ../src/disassembler.cpp
)

add_executable(${PROJECT_NAME}_test ${TEST_SOURCES} ${TEST_DEPENDENCIES})
target_link_libraries(${PROJECT_NAME}_test fmt::fmt GTest::gtest_main)

enable_testing()
include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}_test)
