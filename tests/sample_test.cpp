#include <cmath>

#include "../src/tools.h"
#include "catch_amalgamated.hpp"

TEST_CASE("Vector operations", "[vector]") {
    std::vector<int> vec;
    
    SECTION("Empty vector") {
        REQUIRE(vec.empty());
        REQUIRE(vec.size() == 0);
    }
    
    SECTION("Vector with elements") {
        vec.push_back(1);
        vec.push_back(2);
        
        REQUIRE_FALSE(vec.empty());
        REQUIRE(vec.size() == 2);
        REQUIRE(vec[0] == 1);
        REQUIRE(vec[1] == 2);
    }
}

TEST_CASE("Math functions table-driven", "[math][table]") { // "[math][table]" groups tests by functionality (tags)
    struct TestCase {
        double input;
        double expected;
        std::string description;
    };

    const auto test_case = GENERATE(
        TestCase{0.0, 0.0, "zero"},
        TestCase{1.0, 1.0, "one"},
        TestCase{4.0, 2.0, "four"},
        TestCase{9.0, 3.0, "nine"}
    );

    CAPTURE(test_case.input, test_case.expected, test_case.description); // for debugging: printed additionally when the test fails

    REQUIRE(std::sqrt(test_case.input) == Catch::Approx(test_case.expected));
}

TEST_CASE("Filtered data provider", "[filter][generate]") {
    const auto number = GENERATE(filter(
        [](int n) { return n % 2 == 0; }, // Even numbers only
        range(1, 20) // From the range 1..19
    ));

    REQUIRE(number % 2 == 0);
    REQUIRE(number >= 2);
    REQUIRE(number <= 18);
}

std::vector<std::tuple<int, int, int, std::string>> test_data_provider() {
    return {
        {1, 1, 2, "111"},
        {2, 3, 5, "222"},
        {5, 5, 10, "333"},
        {-1, 1, 0, "444"},
    };
}

TEST_CASE("External data provider", "[external][data]") {
    auto [a, b, expected, message] = GENERATE(from_range(test_data_provider()));

    CAPTURE(a, b, expected, message);
    REQUIRE(a + b == expected);
}

TEMPLATE_TEST_CASE("Template test with different types", "[template]", int, float, double) {
    TestType a = 5;
    TestType b = 3;

    REQUIRE(a + b == TestType(8));
    REQUIRE(a - b == TestType(2));
}
