#include "mytest.hpp"
#include <iostream>
#include <optional>
#include <string>
#include "mytest_internal.hpp"

namespace mytest {
std::vector<TestCase> &all_tests() {
    static std::vector<TestCase> test_cases;
    return test_cases;
}

void mytest_do_check(  // cppcheck-suppress [unusedFunction]
    const bool expr,
    const std::string &expression,
    const std::string &file,
    const int line,
    const std::optional<std::string> &message
) {
    if (expr) {
        return;
    }

    ::mytest::fail = true;
    ::std::cerr << "CHECK(" << expression << ") at " << file << ':' << line
                << " failed!\n";
    if (message.has_value()) {
        ::std::cerr << "    message: " << message.value() << "\n";
    }
}

void mytest_add_test(  // cppcheck-suppress [unusedFunction]
    std::function<void()> func,
    const std::string &name
) {
    all_tests().emplace_back(TestCase{std::move(func), name});
}

bool mytest_run_tests() {
    std::sort(all_tests().begin(), all_tests().end());
    for (const auto &test_case : all_tests()) {
        std::cerr << "Running " << test_case.name << "...\n";
        fail = false;

        test_case.func();

        if (!fail) {
            tests_passed++;
        }
    }
    std::cerr << "===== Tests passed: " << tests_passed << "/"
              << all_tests().size() << " =====\n";
    return !(tests_passed == all_tests().size());
}

}  // namespace mytest