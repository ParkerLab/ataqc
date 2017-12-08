#include <chrono>
#include <thread>

#include "catch.hpp"

#include "Utils.hpp"


TEST_CASE("Test Utils::version_string", "[utils/version_string]" ) {
    REQUIRE(VERSION == version_string());
}

TEST_CASE("Test Utils::basename", "[utils/basename]" )
{
    SECTION("Test that basename strips directory") {
        REQUIRE("foo.bam" == basename("/path/to/foo.bam"));
    }


    SECTION("Test that basename strips directory and \".bam\" extension.") {
        REQUIRE("foo" == basename("/path/to/foo.bam", ".bam"));
    }
}


TEST_CASE("Test Utils::qq", "[utils/qq]" ) {
    SECTION("Test that qq quotes quotes") {
        REQUIRE("He said, \\\"This should work!\\\"" == qq("He said, \"This should work!\""));
    }
}


TEST_CASE("Test Utils::fraction", "[utils/fraction]" ) {
    SECTION("Test fraction(1, 2)") {
        REQUIRE(0.5 == fraction(1, 2));
    }

    SECTION("Test fraction(2, 1)") {
        REQUIRE(2.000 == fraction(2, 1));
    }

    SECTION("Test fraction(1, 0)") {
        REQUIRE(std::isnan(fraction(1, 0)));
    }
}


TEST_CASE("Test Utils::fraction_string", "[utils/fraction_string]" ) {
    SECTION("Test fraction_string(1, 2)") {
        REQUIRE("0.500" == fraction_string(1, 2));
    }

    SECTION("Test fraction_string(1, 3, 5)") {
        REQUIRE("0.33333" == fraction_string(1, 3, 5));
    }

    SECTION("Test fraction_string(2, 1)") {
        REQUIRE("2.000" == fraction_string(2, 1));
    }

    SECTION("Test fraction_string(1, 0)") {
        REQUIRE("nan" == fraction_string(1, 0));
    }
}


TEST_CASE("Test Utils::percentage", "[utils/percentage]" ) {
    SECTION("Test percentage(1, 2)") {
        REQUIRE(50 == percentage(1, 2));
    }

    SECTION("Test percentage(2, 1)") {
        REQUIRE(200 == percentage(2, 1));
    }

    SECTION("Test percentage(1, 0)") {
        REQUIRE(std::isnan(percentage(1, 0)));
    }
}


TEST_CASE("Test Utils::percentage_string", "[utils/percentage_string]" ) {
    SECTION("Test percentage_string(1, 2)") {
        REQUIRE(" (50.000%)" == percentage_string(1, 2));
    }

    SECTION("Test percentage_string(1, 3, 5)") {
        REQUIRE(" (33.33333%)" == percentage_string(1, 3, 5));
    }

    SECTION("Test percentage_string(2, 1)") {
        REQUIRE(" (200.000%)" == percentage_string(2, 1));
    }

    SECTION("Test percentage_string(1, 0)") {
        REQUIRE(" (nan%)" == percentage_string(1, 0));
    }
}

//
// The magic incantation for stringifying std::pair -- operator<< and
// Catch::toString did not work.
//
namespace Catch {
    template<> struct StringMaker<std::pair<std::string, std::string>> {
        static std::string convert(const std::pair<std::string, std::string>& pair) {
            std::stringstream ss;
            ss << pair.first << ", " << pair.second;
            return ss.str();
        }
    };
}


TEST_CASE("Test Utils::split", "[utils/split]" ) {
    SECTION("Test split with space") {
        std::vector<std::string> expected = {"just", "some", "space-separated", "words"};
        auto actual = split("just some space-separated words");
        REQUIRE(expected == actual);
    }

    SECTION("Test split with tab") {
        std::vector<std::string> expected = {"just", "some", "tab-separated", "words"};
        std::vector<std::string> actual = split("just\tsome\ttab-separated\twords", "\t");
        REQUIRE(expected == actual);
    }

    SECTION("Test split with no delimiters") {
        std::vector<std::string> expected = {"just some words"};
        std::vector<std::string> actual = split("just some words", "");
        REQUIRE(expected == actual);
    }

    SECTION("Test split with consecutive delimiters") {
        std::vector<std::string> expected = {"just", "some", "tab-separated", "words"};
        std::vector<std::string> actual = split("just\t\tsome\t\ttab-separated\t\t\t\twords", "\t");
        REQUIRE(expected == actual);
    }

    SECTION("Test split, keeping delimiters") {
        std::vector<std::string> expected = {"SRR", "891275", ".", "1234567890"};
        std::vector<std::string> actual = split("SRR891275.1234567890", "0123456789", true);
        REQUIRE(expected == actual);
    }
}

TEST_CASE("Test Utils::is_only_digits", "[utils/is_only_digits]" ) {
    SECTION("Test is_only_digits with mixed characters") {
        REQUIRE_FALSE(is_only_digits("one2three4"));
    }

    SECTION("Test is_only_digits with only digits") {
        REQUIRE(is_only_digits("1"));
    }

    SECTION("Test is_only_digits with only digits") {
        REQUIRE(is_only_digits("1234567890"));
    }
}


TEST_CASE("Test Utils::is_only_whitespace", "[utils/is_only_whitespace]" ) {
    SECTION("Test is_only_whitespace with mixed characters") {
        REQUIRE_FALSE(is_only_whitespace("one two"));
    }

    SECTION("Test is_only_whitespace with only whitespace") {
        REQUIRE(is_only_whitespace(" \t\r\n"));
    }
}


TEST_CASE("Test Utils::sort_strings_numerically", "[utils/sort_strings_numerically]" ) {
    std::vector<std::string> subject = {"1", "10", "2", "20", "chr30", "chr10", "chr20", "chr1", "chr2", "chr1:10-100", "chr1:2-1000", "SRR891275.1234567890", "SRR891275.1", ""};
    std::vector<std::string> expected = {"", "1", "2", "10", "20", "SRR891275.1", "SRR891275.1234567890", "chr1", "chr1:2-1000", "chr1:10-100", "chr2", "chr10", "chr20", "chr30"};
    std::sort(subject.begin(), subject.end(), sort_strings_numerically);
    REQUIRE(expected == subject);
}


TEST_CASE("Test Utils::iso8601_timestamp", "[utils/iso8601_timestamp]" ) {
    SECTION("Test with current time") {
        std::string expected = iso8601_timestamp();
        std::string actual = iso8601_timestamp();
        REQUIRE(expected == actual);
    }

    SECTION("Test with current time, one second apart") {
        std::string expected = iso8601_timestamp();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::string actual = iso8601_timestamp();
        REQUIRE_FALSE(expected == actual);
    }

    SECTION("Test with time given") {
        time_t subject = 1475769894;
        std::string expected = "2016-10-06T16:04:54Z";
        std::string actual = iso8601_timestamp(&subject);
        REQUIRE(expected == actual);
    }
}


TEST_CASE("Test Utils::slice", "[utils/slice]" ) {
    REQUIRE("foo" == slice("foobar", 0, 3));
    REQUIRE("bar" == slice("foobar", 3));
    REQUIRE("ooba" == slice("foobar", 1, 5));
    REQUIRE("oobar" == slice("foobar", 1, 100));
    REQUIRE("" == slice("foobar", 100, 5));
}


TEST_CASE("Test Utils::wrap", "[utils/wrap]" ) {
    SECTION("Test without indent") {
        std::string subject = (
            "This cosmos, which is the same for all, no one of gods or men has made. "
            "But it always was and will be: an ever-living fire, with measures of it "
            "kindling, and measures going out."
        );

        std::stringstream expected;
        std::vector<std::string> lines = {
            "This cosmos, which",
            "is the same for all,",
            "no one of gods or",
            "men has made. But it",
            "always was and will",
            "be: an ever-living",
            "fire, with measures",
            "of it kindling, and",
            "measures going out.",
        };

        for (auto line : lines) {
            expected << line << std::endl;
        }

        REQUIRE(expected.str() == wrap(subject, 20));
    }

    SECTION("Test with indent") {
        std::string subject = (
            "This cosmos, which is the same for all, no one of gods or men has made. "
            "But it always was and will be: an ever-living fire, with measures of it "
            "kindling, and measures going out."
        );

        std::stringstream expected;
        std::vector<std::string> lines = {
            "  This cosmos, which",
            "  is the same for all,",
            "  no one of gods or",
            "  men has made. But it",
            "  always was and will",
            "  be: an ever-living",
            "  fire, with measures",
            "  of it kindling, and",
            "  measures going out.",
        };

        for (auto line : lines) {
            expected << line << std::endl;
        }

        REQUIRE(expected.str() == wrap(subject, 24, 2));
    }

}

TEST_CASE("Test Utils::integer_to_roman", "[utils/integer_to_roman]" ) {
    REQUIRE("MMXVII" == integer_to_roman(2017));
    REQUIRE("MMMMCMXCIX" == integer_to_roman(4999));
    REQUIRE("MMMMMMMMMMMMCCCXLV" == integer_to_roman(12345));
}

TEST_CASE("Test Utils::sort_strings_of_roman_numerals", "[utils/sort_strings_of_roman_numerals]" ) {
    std::vector<std::string> subject = {"IV", "III", "XI", "IX", "II", "I", "C"};
    std::vector<std::string> expected = {"I", "II", "III", "IV", "IX", "XI", "C"};
    std::sort(subject.begin(), subject.end(), sort_strings_with_roman_numerals);
    REQUIRE(expected == subject);
}

TEST_CASE("Test Utils::sort_strings_with_roman_numerals", "[utils/sort_strings_with_roman_numerals]" ) {
    std::vector<std::string> subject = {"010", "", "10", "01", "1", "chrIV", "chrIII", "chrXI", "chrIX", "chrII", "chrI", "chrC", "chr1", "chrY"};
    std::vector<std::string> expected = {"", "01", "1", "010", "10", "chr1", "chrI", "chrII", "chrIII", "chrIV", "chrIX", "chrXI", "chrC", "chrY"};
    std::sort(subject.begin(), subject.end(), sort_strings_with_roman_numerals);
    REQUIRE(expected == subject);
}
