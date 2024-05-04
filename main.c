#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

typedef struct
{
    int index;
    char* suffix;
} Suffix;

typedef struct
{
    const char* word;
    int count;
} WordCount;

typedef struct
{
    const char* text;
    const char* word;
    Suffix* suffixes;
    long text_length;
    WordCount* result;
    pthread_mutex_t mutex;
} ThreadData;

void*
count_occurrences_thread (void* arg);

WordCount*
get_occurances (const char* text, const char** words, int words_count);

long
binary_search (Suffix* suffixes, const char* name, long left, long right, long name_len);

Suffix*
create_suffixes (const char* text, long length);

int
compare_suffixes (const void* a, const void* b);

void
print_word_counts (WordCount* counts, int count)
{
    for (int i = 0; i < count; i++)
    {
        printf("%s: %d\n", counts[i].word, counts[i].count);
    }
}

int
main (void)
{

    const char* input = "abcdeabcdabcdeabcdeabcd";
    const char* names[] = {"abc", "bcd", "abcde"};
    int name_count = sizeof(names) / sizeof(names[0]);

    clock_t start = clock();
    WordCount* wc = get_occurances(input, names, name_count);
    clock_t end = clock();

    double time_taken = ((double) (end - start)) / CLOCKS_PER_SEC;

    printf("Time taken: %.6f seconds\n", time_taken);
    print_word_counts(wc, name_count);

    free(wc);

    return 0;
}

void*
count_occurrences_thread (void* arg)
{

    ThreadData* data = (ThreadData*) arg;
    Suffix* suffixes = data->suffixes;

    long name_len = (long) strlen(data->word);

    pthread_mutex_lock(&data->mutex);

    long first = binary_search(suffixes, data->word, 0, data->text_length - 1, name_len);
    int count = 0;

    if (first != -1)
    {
        for (long j = first; j < data->text_length; j++)
        {
            if (strncmp(suffixes[j].suffix, data->word, name_len) == 0)
            {
                count++;
            }
            else
            {
                break;
            }
        }
    }

    WordCount* result = (WordCount*) malloc(sizeof(WordCount));
    if (result)
    {
        result->word = data->word;
        result->count = count;
        data->result = result;
    }

    pthread_mutex_unlock(&data->mutex);

    pthread_exit(NULL);
}

WordCount*
get_occurances (const char* text, const char** words, int words_count)
{
    long text_len = (long) strlen(text);
    Suffix* suffixes = create_suffixes(text, text_len);

    if (!suffixes)
    {
        printf("Memory allocation error.\n");
        free(suffixes);
        return NULL;
    }

    qsort(suffixes, text_len, sizeof(Suffix), compare_suffixes);

    pthread_t threads[words_count];
    ThreadData thread_data[words_count];

    WordCount* wc = (WordCount*) malloc(sizeof(WordCount) * words_count);
    if (!wc)
    {
        printf("Memory allocation error.\n");
        free(suffixes);
        return NULL;
    }

    for (int i = 0; i < words_count; i++)
    {
        thread_data[i].text = text;
        thread_data[i].word = words[i];
        thread_data[i].suffixes = suffixes;
        thread_data[i].text_length = text_len;
        pthread_mutex_init(&thread_data[i].mutex, NULL);

        pthread_create(&threads[i], NULL, count_occurrences_thread, (void*) &thread_data[i]);
    }

    for (int i = 0; i < words_count; i++)
    {
        pthread_join(threads[i], NULL);
        wc[i] = *thread_data[i].result;
        pthread_mutex_destroy(&thread_data[i].mutex);
        free(thread_data[i].result);
    }

    for (int i = 0; i < text_len; i++)
    {
        free(suffixes[i].suffix);
    }
    free(suffixes);

    return wc;
}

Suffix*
create_suffixes (const char* text, long length)
{
    Suffix* suffixes = (Suffix*) malloc(length * sizeof(Suffix));
    if (!suffixes)
    {
        return NULL;
    }

    for (long i = 0; i < length; i++)
    {
        suffixes[i].index = i;
        suffixes[i].suffix = strdup(text + i);
    }

    return suffixes;
}

int
compare_suffixes (const void* a, const void* b)
{
    return strcmp(((Suffix*) a)->suffix, ((Suffix*) b)->suffix);
}

long
binary_search (Suffix* suffixes, const char* name, long left, long right, long name_len)
{
    while (left <= right)
    {
        long mid = left + (right - left) / 2;
        char* suffix = suffixes[mid].suffix;

        int cmp = strncmp(suffix, name, name_len);
        if (cmp == 0 && (mid == 0 || strncmp(suffixes[mid - 1].suffix, name, name_len) < 0))
        {
            return mid;
        }

        if (cmp < 0)
        {
            left = mid + 1;
        }
        else
        {
            right = mid - 1;
        }
    }

    return -1;
}
