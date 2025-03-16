#include "ahocorasick.h"
#include "tests.h"

#include <iostream>

void testDefault(void)
{
    static constexpr char testName[] = "testDefault";

    static std::string string = "hello world, my name is joltgrep";
    static std::string pattern = "name";

    tests::start(testName);

    AhoCorasick ahoCorasick(pattern);
    
    int v = 0, j = 0;
    for (; j < string.size(); ++j) {
        v = ahoCorasick.go(v, string[j]);
        if (v == -1) {
            break;
        }
    }

    tests::check(j == 20, "Failed default search"); 

    tests::end(testName);
}

void testDot(int i)
{
    static constexpr char testName[] = "testDot";

    static std::vector<std::string> strings = {
        "hello world, my name is joltgrep",
        "nice to meet you, life is the same",
        "do you prefer fame or power"
    };

    static std::string pattern = ".ame";
    static std::vector<std::size_t> expected = { 20, 34, 18 };

    tests::start(testName, i);

    AhoCorasick ahoCorasick(pattern);
    
    int v = 0, j = 0;
    for (; j < strings[i].size(); ++j) {
        v = ahoCorasick.go(v, strings[i][j]);
        if (v == -1) {
            break;
        }
    }

    tests::check(j == expected[i], "Failed dot search"); 

    tests::end(testName, i);
}

void testDoubleDot(int i)
{
    static constexpr char testName[] = "testDoubleDot";

    static std::vector<std::string> strings = {
        "hello world, my name is joltgrep",
        "nice to meet you, life is the same",
        "do you prefer fame or power"
    };

    static std::string pattern = "..me";
    static std::vector<std::size_t> expected = { 20, 10, 18 };

    tests::start(testName, i);

    AhoCorasick ahoCorasick(pattern);
    
    int v = 0, j = 0;
    for (; j < strings[i].size(); ++j) {
        v = ahoCorasick.go(v, strings[i][j]);
        if (v == -1) {
            break;
        }
    }

    tests::check(j == expected[i], "Failed double dot search"); 

    tests::end(testName, i);
}

void testBracket(int i)
{
    static constexpr char testName[] = "testBracket";

    static std::vector<std::string> strings = {
        "hello world, my name is joltgrep",
        "nice to meet you, life is the same",
        "do you prefer fame or power"
    };

    static std::string pattern = "[nsf]ame";
    static std::vector<std::size_t> expected = { 20, 34, 18 };

    tests::start(testName, i);

    AhoCorasick ahoCorasick(pattern);
    
    int v = 0, j = 0;
    for (; j < strings[i].size(); ++j) {
        v = ahoCorasick.go(v, strings[i][j]);
        if (v == -1) {
            break;
        }
    }

    tests::check(j == expected[i], "Failed bracket search"); 

    tests::end(testName, i);
}

void testDoubleBracket(int i)
{
    static constexpr char testName[] = "testDoubleBracket";

    static std::vector<std::string> strings = {
        "hello world, my name is joltgrep",
        "nice to meet you, life is the sane",
        "do you prefer fade or power"
    };

    static std::string pattern = "[nsf]a[mnd]e";
    static std::vector<std::size_t> expected = { 20, 34, 18 };

    tests::start(testName, i);

    AhoCorasick ahoCorasick(pattern);
    
    int v = 0, j = 0;
    for (; j < strings[i].size(); ++j) {
        v = ahoCorasick.go(v, strings[i][j]);
        if (v == -1) {
            break;
        }
    }

    tests::check(j == expected[i], "Failed double bracket search"); 

    tests::end(testName, i);
}

void testOptional(int i)
{
    static constexpr char testName[] = "testOptional";

    static std::vector<std::string> strings = {
        "hello world, my name is joltgrep",
        "nice to meet you, life is the nme",
        "do you prefer nmes or power"
    };

    static std::string pattern = "na?mes?";
    static std::vector<std::size_t> expected = { 20, 33, 18 };

    tests::start(testName, i);

    AhoCorasick ahoCorasick(pattern);
    
    int v = 0, j = 0;
    for (; j < strings[i].size(); ++j) {
        v = ahoCorasick.go(v, strings[i][j]);
        if (v == -1) {
            break;
        }
    }

    tests::check(j == expected[i], "Failed double bracket search"); 

    tests::end(testName, i);
}

void tests::ahoCorasick(void)
{
    static constexpr char testName[] = "Tests Aho-Corasick";

    tests::sectionStart(testName);
    
    testDefault();
    for (int i = 0; i < 3; ++i) {
        testDot(i);
    }
    for (int i = 0; i < 3; ++i) {
        testDoubleDot(i);
    }
    for (int i = 0; i < 3; ++i) {
        testBracket(i);
    }
    for (int i = 0; i < 3; ++i) {
        testDoubleBracket(i);
    }
    for (int i = 0; i < 3; ++i) {
        testOptional(i);
    }

    tests::sectionEnd(testName);
}
