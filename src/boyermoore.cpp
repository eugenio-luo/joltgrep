#include <iostream>

#include "boyermoore.h"

std::vector<std::size_t>& BoyerMoore::getBadCharTable(void)
{
    return m_badCharTable;
}

std::vector<std::size_t>& BoyerMoore::getSuffixTable(void)
{
    return m_suffixTable;
}

std::string& BoyerMoore::getPattern(void)
{
    return m_pattern;
}

void BoyerMoore::preprocessBadCharTable(std::string_view pattern)
{
    for (int i = 0; i < pattern.size() - 1; ++i) {
        m_badCharTable[pattern[i]] = pattern.size() - 1 - i;
    }
} 

bool isPrefix(std::string_view pattern, std::size_t pos)
{
    std::size_t suffixLen = pattern.size() - pos;

    for (int i = 0; i < suffixLen; ++i) {
        if (pattern[i] != pattern[pos + i]) {
            return false;
        }
    }

    return true;
}

std::size_t suffixLength(std::string_view pattern, std::size_t pos)
{
    std::size_t i = 0;
    for (; (pattern[pos - i] == pattern[pattern.size() - 1 - i]) && (i < pos); ++i);
    return i;
}

void BoyerMoore::preprocessSuffixTable(std::string_view pattern)
{
    std::size_t lastPrefix = 1;
    for (int i = pattern.size() - 1; i >= 0; --i) {
        if (isPrefix(pattern, i + 1)) {
            lastPrefix = i + 1;
        }
        m_suffixTable[i] = pattern.size() - 1 - i + lastPrefix;
    }

    for (int i = 0; i < pattern.size() - 1; ++i) {
        std::size_t len = suffixLength(pattern, i);
        if (pattern[i - len] != pattern[pattern.size() - 1 - len]) {
            m_suffixTable[pattern.size() - 1 - len] = pattern.size() - 1 - i + len;
        } 
    }
}

std::size_t BoyerMoore::start(void)
{
    return m_pattern.size() - 1;
}

std::size_t BoyerMoore::next(const std::vector<char>& buffer, std::size_t pos)
{
    if (pos >= buffer.size()) {
        return 0;
    }

    int j = m_pattern.size() - 1;
    while (j >= 0 && buffer[pos] == m_pattern[j]) {
        --pos;
        --j;
    }

    return (j >= 0) ? std::max(m_badCharTable[buffer[pos]], m_suffixTable[j]) : 0;
}

BoyerMoore::BoyerMoore(const std::string& pattern)
    : m_badCharTable(alphabetSize, pattern.size()), m_suffixTable(pattern.size()),
      m_pattern{pattern}
{
    preprocessBadCharTable(pattern);
    preprocessSuffixTable(pattern);
}