#pragma once

#include <string>

namespace tests {

void sectionStart(const std::string_view output);
void sectionEnd(const std::string_view output);

void start(const std::string_view output);
void start(const std::string_view output, int n);
void end(const std::string_view output); 
void end(const std::string_view output, int n); 

void check(bool condition, const std::string_view output);

void main(void);
void queue(void);
void memoryCharacter(void);
void boyerMoore(void);
void ahoCorasick(void);

} // namespace tests
