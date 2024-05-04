#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    int index;
    char* suffix;
} Suffix;

void count_occurrences(const char* text, const char** words, int words_count);
int binary_search(Suffix* suffixes, const char* name, int left, int right, int name_len);
Suffix* create_suffixes(const char* text, int length);
int compare_suffixes(const void* a, const void* b);

void print_suffixes(Suffix* suffixes, int length) {
    for (int i = 0; i < length; i++) {
        printf("Suffix[%d]: %s\n", suffixes[i].index, suffixes[i].suffix);
    }
}

int main() {
    const char* input = "abcdeabcdabcdeabcdeabcd";
    const char* names[] = {"abc", "bcd", "abcde"};
    int name_count = sizeof(names) / sizeof(names[0]);

    clock_t start = clock();
    count_occurrences(input, names, name_count);
    clock_t end = clock();

    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Time taken: %.6f seconds\n", time_taken);

    return 0;
}

void count_occurrences(const char* text, const char** words, int words_count) {
    int text_len = strlen(text);
    Suffix* suffixes = create_suffixes(text, text_len);

    if (!suffixes) {
        printf("Memory allocation error.\n");
        return;
    }

    // print_suffixes(suffixes, text_len);

    qsort(suffixes, text_len, sizeof(Suffix), compare_suffixes);

    // print_suffixes(suffixes, text_len);

    for (int i = 0; i < words_count; i++) {
        const char* name = words[i];
        int name_len = strlen(name);

        int first = binary_search(suffixes, name, 0, text_len - 1, name_len);
        int count = 0;

        if (first != -1) {
            for (int j = first; j < text_len; j++) {
                if (strncmp(suffixes[j].suffix, name, name_len) == 0) {
                    count++;
                } else {
                    break;
                }
            }
        }

        printf("%s: %d\n", name, count);
    }

    free(suffixes);
}

Suffix* create_suffixes(const char* text, int length) {
    Suffix* suffixes = (Suffix*)malloc(length * sizeof(Suffix));
    if (!suffixes) {
        return NULL;
    }

    for (int i = 0; i < length; i++) {
        suffixes[i].index = i;
        suffixes[i].suffix = (char*)(text + i);
    }

    return suffixes;
}

int compare_suffixes(const void* a, const void* b) {
    return strcmp(((Suffix*)a)->suffix, ((Suffix*)b)->suffix);
}

int binary_search(Suffix* suffixes, const char* name, int left, int right, int name_len) {
    while (left <= right) {
        int mid = left + (right - left) / 2;
        char* suffix = suffixes[mid].suffix;

        int cmp = strncmp(suffix, name, name_len);
        if (cmp == 0) {
            if (mid == 0 || strncmp(suffixes[mid - 1].suffix, name, name_len) < 0) {
                return mid;
            } else {
                right = mid - 1;
            }
        } else if (cmp < 0) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    return -1;
}
