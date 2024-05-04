#include <iostream>
#include <utility>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>
#include <string_view>

struct Suffix {
    std::string_view suffix;
};

std::vector<Suffix> createSuffixes(const std::string_view &text) {
    std::vector<Suffix> suffixes;
    suffixes.reserve(text.size());

    for (unsigned long i = 0; i < text.size(); ++i) {
        suffixes.push_back({text.substr(i)});
    }

    return suffixes;
}

void countOccurrences1(const std::string_view &text, const std::vector<std::string> &words) {
    auto suffixes = createSuffixes(text);

    std::sort(suffixes.begin(), suffixes.end(), [](const Suffix &a, const Suffix &b) {
        return a.suffix < b.suffix;
    });

    for (const auto &word: words) {
        auto first = std::lower_bound(suffixes.begin(), suffixes.end(), word,
                                      [](const Suffix &a, const std::string &b) {
                                          return a.suffix < b;
                                      });

        if (first != suffixes.end() && first->suffix.substr(0, word.size()) == word) {
            std::ptrdiff_t count = std::count_if(first, suffixes.end(), [&](const Suffix &s) {
                return s.suffix.substr(0, word.size()) == word;
            });

            std::cout << word << ": " << static_cast<int>(count) << "\n";  // Explicit cast to int
        } else {
            std::cout << word << ": 0\n";
        }
    }
}

void countOccurrences2(const std::string_view &text, const std::vector<std::string> &words) {
    for (const auto &word: words) {
        size_t count = 0;
        size_t pos = 0;
        while ((pos = text.find(word, pos)) != std::string::npos) {
            pos += word.length();
            ++count;
        }
        std::cout << word << ": " << count << "\n";
    }
}

class TrieNode {
public:
    std::unordered_map<char, TrieNode *> children;
    bool isEndOfWord;

    TrieNode() : isEndOfWord(false) {}
};

class Trie {

public:
    Trie() : root(new TrieNode()) {}

    void insert(const std::string &word) const {
        TrieNode *current = root;
        for (char c: word) {
            if (!current->children[c]) {
                current->children[c] = new TrieNode();
            }
            current = current->children[c];
        }
        current->isEndOfWord = true;
    }

    [[nodiscard]] bool search(const std::string &word) const {
        TrieNode *current = root;
        for (char c: word) {
            if (!current->children[c]) {
                return false;
            }
            current = current->children[c];
        }
        return current != nullptr && current->isEndOfWord;
    }

    TrieNode *root;
};

void countOccurrences3(const std::string_view &text, const Trie &trie) {
    size_t textLength = text.length();
    size_t wordsCount = 0;

    for (size_t i = 0; i < textLength; ++i) {
        TrieNode *current = trie.root;
        size_t j = i;

        while (j < textLength && current->children[text[j]]) {
            current = current->children[text[j]];
            if (current->isEndOfWord) {
                ++wordsCount;
            }
            ++j;
        }
    }

    std::cout << "Total words found: " << wordsCount << "\n";
}

namespace local_chrono {
    using namespace std::chrono;

    time_point<steady_clock> getTime() {
        return steady_clock::now();
    }

    microseconds getTimeDiff(time_point<steady_clock> &start,
                             time_point<steady_clock> &end) {
        return duration_cast<microseconds>(end - start);
    }
}

int main() {

    const std::string input = "abcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcde";
    std::vector<std::string> names = {"abc", "bcd", "abcde", "ab", "cde", "de"};

    std::cout << "countOccurrences1()\n";
    auto start = local_chrono::getTime();
    countOccurrences1(input, names);
    auto end = local_chrono::getTime();
    auto duration = local_chrono::getTimeDiff(start, end);
    std::cout << "Time taken: " << duration.count() << " microseconds\n";

    std::cout << "\ncountOccurrences2()\n";
    start = local_chrono::getTime();
    countOccurrences2(input, names);
    end = local_chrono::getTime();
    duration = local_chrono::getTimeDiff(start, end);
    std::cout << "Time taken: " << duration.count() << " microseconds\n";

    std::cout << "\ncountOccurrences3()\n";
    start = local_chrono::getTime();
    Trie trie;
    for (const std::string &name: names) {
        trie.insert(name);
    }
    countOccurrences3(input, trie);
    end = local_chrono::getTime();
    duration = local_chrono::getTimeDiff(start, end);
    std::cout << "Time taken: " << duration.count() << " microseconds\n";

    return 0;
}
