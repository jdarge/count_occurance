#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "thpool.h"

typedef struct {
    int index;
    char *suffix;
} Suffix;

typedef struct {
    const char *word;
    int count;
} WordCount;

typedef struct {
    const char *text;
    const char *word;
    Suffix *suffixes;
    long text_length;
    WordCount *result;
} OccurrenceData;

#ifdef __linux__
    #include <unistd.h>
    #define MAX_THREADS sysconf(_SC_NPROCESSORS_ONLN)
#else // TODO
    #define MAX_THREADS 4
#endif

void count_occurrences_thread(void *arg);//
WordCount *get_occurrences(const char *text, const char **words, int words_count);//
long binary_search(Suffix *suffixes, const char *name, long left, long right, long name_len);//
Suffix *create_suffixes(const char *text, long length);//
int compare_suffixes(const void *a, const void *b);//

void print_word_counts(WordCount *counts, int count) {
    for (int i = 0; i < count; i++) {
        printf("%s: %d\n", counts[i].word, counts[i].count);
    }
}

int main(void) {

    struct timespec start, end;
    double elapsed_time;
	
	const char *input = "abcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcde";
    const char *names[] = {"abc", "bcd", "abcde", "ab", "cde", "de"};
	
    int name_count = sizeof(names) / sizeof(names[0]);
	
	clock_gettime(CLOCK_MONOTONIC, &start);
    WordCount *wc = get_occurrences(input, names, name_count);
	clock_gettime(CLOCK_MONOTONIC, &end);
	duration = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

	printf("Time taken: %.6f seconds\n", time_taken);

    print_word_counts(wc, name_count);

    free(wc);

    return 0;
}

void count_occurrences_thread(void *arg) {

    OccurrenceData *data = (OccurrenceData *) arg;
    Suffix *suffixes = data->suffixes;

    long name_len = (long) strlen(data->word);

    long first = binary_search(suffixes, data->word, 0, data->text_length - 1, name_len);
    int count = 0;

    if (first != -1) {
        for (long j = first; j < data->text_length; j++) {
            if (strncmp(suffixes[j].suffix, data->word, name_len) == 0) {
                count++;
            } else {
                break;
            }
        }
    }

    WordCount *result = (WordCount *) malloc(sizeof(WordCount));
    if (result) {
        result->word = data->word;
        result->count = count;
        data->result = result;
    }
}

WordCount *get_occurrences(const char *text, const char **words, int words_count) {
    long text_len = (long) strlen(text);
    Suffix *suffixes = create_suffixes(text, text_len);

    if (!suffixes) {
        printf("Memory allocation error.\n");
        free(suffixes);
        return NULL;
    }

    WordCount *wc = (WordCount *) malloc(sizeof(WordCount) * words_count);
    if (!wc) {
        printf("Memory allocation error.\n");
        free(suffixes);
        free(wc);
        return NULL;
    }

    qsort(suffixes, text_len, sizeof(Suffix), compare_suffixes);

    OccurrenceData thread_data[words_count];
    threadpool thpool = thpool_init(MAX_THREADS);

    for (int i = 0; i < words_count; i++) {
        thread_data[i].text = text;
        thread_data[i].word = words[i];
        thread_data[i].suffixes = suffixes;
        thread_data[i].text_length = text_len;

        thpool_add_work(thpool, count_occurrences_thread, (void*) &thread_data[i]);
    }

    thpool_wait(thpool);

    for (int i = 0; i < words_count; i++) {
        wc[i] = *thread_data[i].result;
        free(thread_data[i].result);
    }

    thpool_destroy(thpool);

    for (int i = 0; i < text_len; i++) {
        free(suffixes[i].suffix);
    }
    free(suffixes);

    return wc;
}

Suffix *create_suffixes(const char *text, long length) {
    Suffix *suffixes = (Suffix *) malloc(length * sizeof(Suffix));
    if (!suffixes) {
        return NULL;
    }

    for (long i = 0; i < length; i++) {
        suffixes[i].index = i;
        suffixes[i].suffix = strdup(text + i);
    }

    return suffixes;
}

int compare_suffixes(const void *a, const void *b) {
    return strcmp(((Suffix *) a)->suffix, ((Suffix *) b)->suffix);
}

long binary_search(Suffix *suffixes, const char *name, long left, long right, long name_len) {
    while (left <= right) {
        long mid = left + (right - left) / 2;
        char *suffix = suffixes[mid].suffix;
        int cmp = strncmp(suffix, name, name_len);

        if (cmp == 0 && (mid == 0 || strncmp(suffixes[mid - 1].suffix, name, name_len) < 0)) {
            return mid;
        }

        if (cmp < 0) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    return -1;
}
