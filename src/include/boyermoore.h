#pragma once

#include <array>
#include <vector>
#include <string>

/*
 / Boyer-Moore String-Search Algorithm
 / https://en.wikipedia.org/wiki/Boyer–Moore_string-search_algorithm
*/

class BoyerMoore {
public:
    BoyerMoore(const std::string& pattern);

    std::vector<std::size_t>& getBadCharTable(void);
    std::vector<std::size_t>& getSuffixTable(void);
    std::string& getPattern(void);

    std::size_t start(void);
    std::size_t next(const std::vector<char>& buffer, std::size_t pos);

private:
    void preprocessBadCharTable(std::string_view pattern); 
    void preprocessSuffixTable(std::string_view pattern); 

    static constexpr std::size_t alphabetSize = 256;

    std::vector<std::size_t> m_badCharTable;
    std::vector<std::size_t> m_suffixTable;

    std::string              m_pattern;
};