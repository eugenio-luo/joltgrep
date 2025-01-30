#include <iostream>

#include "tests.h"
#include "boyermoore.h"

void testBadCharTable1(void)
{
    static constexpr char testName[] = "testBadCharTable 1";

    static std::string pattern1 =  "bcabcabc";
    static std::vector<std::size_t> expected1(256, pattern1.size());
    expected1['b'] = 1, expected1['a'] = 2, expected1['c'] = 3;

    tests::start(testName);

    BoyerMoore boyerMoore(pattern1);
    auto& badCharTable = boyerMoore.getBadCharTable();
    tests::check(badCharTable == expected1, 
        "Bad Character Table was not build correctly");

    tests::end(testName);
}

void testBadCharTable2(void)
{
    static constexpr char testName[] = "testBadCharTable 2";

    static std::string pattern2 =  "adbda";
    static std::vector<std::size_t> expected2(256, pattern2.size());
    expected2['a'] = 4, expected2['d'] = 1, expected2['b'] = 2;

    tests::start(testName);

    BoyerMoore boyerMoore(pattern2);
    auto& badCharTable = boyerMoore.getBadCharTable();
    tests::check(badCharTable == expected2, 
        "Bad Character Table was not build correctly");

    tests::end(testName);
}

void testSuffixTable1(void)
{
    static constexpr char testName[] = "testSuffixTable 1";
    
    static constexpr char pattern1[] =  "bcabcabc";
    static std::vector<std::size_t> expected1{10, 9, 8, 10, 9, 8, 9, 1};
    
    tests::start(testName);

    BoyerMoore boyerMoore(pattern1);
    tests::check(boyerMoore.getSuffixTable() == expected1, 
        "Suffix table was not build correctly");

    tests::end(testName);
}

void testSuffixTable2(void)
{
    static constexpr char testName[] = "testSuffixTable 2";
    
    static constexpr char pattern2[] =  "adbda";
    static std::vector<std::size_t> expected2{8, 7, 6, 5, 1};

    tests::start(testName);

    BoyerMoore boyerMoore(pattern2);
    tests::check(boyerMoore.getSuffixTable() == expected2, 
        "Suffix table was not build correctly");

    tests::end(testName);
}

void testSearch(int i)
{
    static constexpr char testName[] = "testSearch";

    static std::vector<std::string> strings = {
        "hello world, my name is joltgrep",
        "nice to meet you, life is good",
        "do you prefer pancakes or chocolate"
    };

    static std::vector<std::string> patterns = { "joltgrep", "meet", "pancakes" };
    static std::vector<std::size_t> expected = { 31, 11, 21 };

    tests::start(testName, i);

    BoyerMoore boyerMoore(patterns[i]);
    
    std::vector<char> buffer(strings[i].begin(), strings[i].end());
    std::size_t pos = boyerMoore.start();
    std::size_t shift;
    while ((shift = boyerMoore.next(buffer, pos)) != 0) {
        pos += shift;
    }

    tests::check(pos == expected[i], "Boyer-Moore failed search"); 

    tests::end(testName, i);
}

void testFailedSearch(void)
{
    static constexpr char testName[] = "testFailedSearch";
    static constexpr char pattern[] =  "fail";
    static std::string string =  "hello world, my name is joltgrep";

    tests::start(testName);

    BoyerMoore boyerMoore(pattern);
    std::vector<char> buffer(string.begin(), string.end());
    std::size_t pos = boyerMoore.start();
    std::size_t shift;
    while ((shift = boyerMoore.next(buffer, pos)) != 0) {
        pos += shift;
    }

    tests::check(pos >= string.size(), "Boyer-Moore did not fail the search in the correct way"); 

    tests::end(testName);
}

void tests::boyerMoore(void)
{
    static constexpr char testName[] = "Tests Boyer-Moore";

    tests::sectionStart(testName);

    testBadCharTable1();
    testBadCharTable2();
    testSuffixTable1();
    testSuffixTable2();

    for (int i = 0; i < 3; ++i) {
        testSearch(i);
    }

    testFailedSearch();

    tests::sectionEnd(testName);
}