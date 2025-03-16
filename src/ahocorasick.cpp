#include "ahocorasick.h"
#include "print.h"

AhoCorasick::AhoCorasick(std::string_view pattern)
    : m_pattern{pattern}, m_nodes(1)
{
    addPattern(pattern);

    for (int v = 0; v < m_nodes.size(); ++v) {
        buildLink(v);
        for (int i = 0; i < alphabetSize; ++i) {
            buildGo(v, i);
        }
    }
}

int AhoCorasick::go(int v, char c)
{
    if (m_nodes[v].output) {
        return SUCCESS;
    }

    return m_nodes[v].go[c];
}

int AhoCorasick::dotPattern(int size)
{
    int end = m_nodes.size();
    int start = end - size;
    for (int i = 0; i < size; ++i) {
        
        // visible ASCII characters
        for (int j = 0; j < 95; ++j) {
            
            m_nodes[start + i].next[j + 32] = end + i * 95 + j;
            m_nodes.emplace_back(start + i, j + 32);
        }
    }
    return size * 95;
}

int AhoCorasick::bracketPattern(std::string_view::iterator& it, int size)
{
    ++it;

    int end = m_nodes.size();
    int start = end - size;
    int count = 0;
    for (; it != m_pattern.end() && *it != ']'; ++it) {
        
        unsigned char c = static_cast<unsigned char>(*it);
        for (int i = 0; i < size; ++i) {
            m_nodes[start + i].next[c] = end + count * size + i;
            m_nodes.emplace_back(start + i, c);
        }

        ++count;
    }

    if (it == m_pattern.end()) {
        // TODO: handle error
        throw;
    }

    return size * count;
}

void AhoCorasick::addPattern(std::string_view pattern)
{
    // TODO: handle ^|

    int size = 1;
    int prevSize = 1;
    for (auto it = pattern.begin(); it != pattern.end(); ++it) {
        unsigned char c  = static_cast<unsigned char>(*it);
        switch (c) {
        case '.': {
            prevSize = size;
            size = dotPattern(size);
            break;
        }
            
        case '[': {
            prevSize = size;
            size = bracketPattern(it, size);
            break;
        }

        case '?': {
            int tmp = prevSize;
            prevSize = size;
            size += tmp;
            break;
        }

        default: {
            int end = m_nodes.size();
            int start = end - size; 
            for (int i = 0; i < size; ++i) {
                m_nodes[start + i].next[c] = end + i;
                m_nodes.emplace_back(start + i, c);
            }
            break;
        }
        }
    }
    int start = m_nodes.size() - size; 
    for (int i = 0; i < size; ++i) {
        m_nodes[start + i].output = true;
    }
}

int AhoCorasick::buildLink(int v)
{
    if (m_nodes[v].link == -1) {
        if (v == 0 || m_nodes[v].parent == 0) {
            m_nodes[v].link = 0;      
        } else {
            m_nodes[v].link = buildGo(buildLink(m_nodes[v].parent), m_nodes[v].parCh);
        }
    }

    return m_nodes[v].link;
}

int AhoCorasick::buildGo(int v, char ch)
{
    unsigned char c = static_cast<unsigned char>(ch);

    if (m_nodes[v].go[c] == -1) {
        if (m_nodes[v].next[c] != -1) {
            m_nodes[v].go[c] = m_nodes[v].next[c];
        } else {
            m_nodes[v].go[c] = (v == 0) ? 0 : buildGo(buildLink(v), c);
        }
    }

    return m_nodes[v].go[c];
}
