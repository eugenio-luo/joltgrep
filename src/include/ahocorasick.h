#pragma once

#include <string>
#include <vector>

/*
 * Aho-Corasick Algorithm
 * https://en.wikipedia.org/wiki/Aho–Corasick_algorithm
*/

class AhoCorasick {
public:
    enum {
        SUCCESS = -100,
        FAIL = -1
    };

    AhoCorasick(std::string_view pattern);
    
    int go(int v, char c);

private:
    static constexpr std::size_t alphabetSize = 256;
    
    struct Node {
        bool output;
        int parent;
        char parCh;
        int link;
        std::array<int, alphabetSize> next;
        std::array<int, alphabetSize> go;

        Node(int p = -1, char ch = '$') : output(false), parent(p),
            parCh{ch}, link{-1}
        {
            std::fill(next.begin(), next.end(), FAIL);
            std::fill(go.begin(), go.end(), FAIL);
        }
    };

    int dotPattern(int size);
    int bracketPattern(std::string_view::iterator& it, int size);
    void addPattern(std::string_view pattern);
    int buildLink(int v);
    int buildGo(int v, char c);

    std::string_view m_pattern;
    std::vector<Node> m_nodes;
};
