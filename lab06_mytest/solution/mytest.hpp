#ifndef MYTEST_HPP_
#define MYTEST_HPP_

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <vector>
// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define CHECK(expr) \
    ([&]() { ::mytest::mytest_do_check((expr), #expr, __FILE__, __LINE__); }())
#define CHECK_MESSAGE(expr, msg)                                           \
    ([&]() {                                                               \
        ::mytest::mytest_do_check((expr), #expr, __FILE__, __LINE__, msg); \
    }())
#define MYTEST_INTERNAL_NEW_NAME2(s1, s2) s1##s2
#define MYTEST_INTERNAL_NEW_NAME1(s1, s2) MYTEST_INTERNAL_NEW_NAME2(s1, s2)
#define MYTEST_INTERNAL_NEW_NAME(x) MYTEST_INTERNAL_NEW_NAME1(x, __LINE__)
#define MYTEST_INTERNAL_RUN_BEFORE_MAIN(expr)          \
    namespace {                                        \
    int MYTEST_INTERNAL_NEW_NAME(NAME_) = ((expr), 0); \
    }
#define MYTEST_INTERNAL_REGISTER_NEW_FUNCTION(func, name) \
    MYTEST_INTERNAL_RUN_BEFORE_MAIN(::mytest::mytest_add_test(&(func), #name))
#define MYTEST_INTERNAL_VOID_REGISTER_NEW_FUNCTION(func, name) \
    static void func();                                        \
    MYTEST_INTERNAL_REGISTER_NEW_FUNCTION(func, name);         \
    static void func()

#define TEST_CASE_REGISTER(func, name) \
    MYTEST_INTERNAL_REGISTER_NEW_FUNCTION(func, name)
#define TEST_CASE(name)                                                     \
    MYTEST_INTERNAL_VOID_REGISTER_NEW_FUNCTION(                             \
        MYTEST_INTERNAL_NEW_NAME(MYTEST_INTERNAL_INTERNAL_ANON_FUNC_), name \
    )

// NOLINTEND(cppcoreguidelines-macro-usage)
namespace mytest {
// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
inline unsigned int tests_passed = 0;
inline bool fail = false;
inline int max_subcase_depth = 0;

// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)
struct TestCase {
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    std::function<void()> func;
    std::string name;

    // NOLINTEND(misc-non-private-member-variables-in-classes)
    bool operator<(const TestCase &other) const {
        return name < other.name;
    }
};

std::vector<TestCase> &all_tests();

void mytest_add_test(std::function<void()>, const std::string &);

void mytest_do_check(
    bool expr,
    const std::string &expression,
    const std::string &file,
    int line,
    const std::optional<std::string> &message = std::nullopt
);

}  // namespace mytest

#endif