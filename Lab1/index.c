#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define INITIAL_CAPACITY 1024
#define WORD_MAX 128
#define LINE_LIST_INITIAL 16

typedef struct {
    char *word;            // stored in lowercase
    int *lines;            // dynamic array of line numbers (unique)
    int lineCount;
    int lineCapacity;
    int count;             // total occurrences
} IndexEntry;

typedef struct {
    IndexEntry *entries;
    int count;
    int capacity;
} Index;

static void *xmalloc(size_t sz) {
    void *p = malloc(sz);
    if (!p) { fprintf(stderr, "Out of memory\n"); exit(1); }
    return p;
}

static void *xrealloc(void *ptr, size_t sz) {
    void *p = realloc(ptr, sz);
    if (!p) { fprintf(stderr, "Out of memory (realloc)\n"); exit(1); }
    return p;
}

static void initIndex(Index *idx) {
    idx->capacity = 256;
    idx->count = 0;
    idx->entries = (IndexEntry *) xmalloc(sizeof(IndexEntry)*idx->capacity);
}

static int compareEntries(const void *a, const void *b) {
    const IndexEntry *ea = (const IndexEntry *)a;
    const IndexEntry *eb = (const IndexEntry *)b;
    return strcmp(ea->word, eb->word);
}

static IndexEntry *findEntry(Index *idx, const char *word) {
    for (int i = 0; i < idx->count; ++i) {
        if (strcmp(idx->entries[i].word, word) == 0) return &idx->entries[i];
    }
    return NULL;
}

static void addLine(IndexEntry *entry, int lineNumber) {
    // avoid duplicates
    for (int i = 0; i < entry->lineCount; ++i) {
        if (entry->lines[i] == lineNumber) return;
    }
    if (entry->lineCount == entry->lineCapacity) {
        entry->lineCapacity *= 2;
        entry->lines = (int *) xrealloc(entry->lines, entry->lineCapacity * sizeof(int));
    }
    entry->lines[entry->lineCount++] = lineNumber;
}

static void addWord(Index *idx, const char *word, int lineNumber) {
    IndexEntry *entry = findEntry(idx, word);
    if (!entry) {
        if (idx->count == idx->capacity) {
            idx->capacity *= 2;
            idx->entries = (IndexEntry *) xrealloc(idx->entries, idx->capacity * sizeof(IndexEntry));
        }
        entry = &idx->entries[idx->count++];
        entry->word = (char *) xmalloc(strlen(word)+1);
        strcpy(entry->word, word);
        entry->lineCapacity = LINE_LIST_INITIAL;
        entry->lines = (int *) xmalloc(entry->lineCapacity * sizeof(int));
        entry->lineCount = 0;
        entry->count = 0;
    }
    entry->count++;
    addLine(entry, lineNumber);
}

static int isStopWord(char **stopWords, int stopCount, const char *word) {
    for (int i = 0; i < stopCount; ++i) {
        if (strcmp(stopWords[i], word) == 0) return 1;
    }
    return 0;
}

static char **readStopWords(const char *filename, int *outCount) {
    FILE *f = fopen(filename, "r");
    if (!f) { fprintf(stderr, "Cannot open stop words file: %s\n", filename); exit(1); }
    int capacity = 128;
    int count = 0;
    char **list = (char **) xmalloc(capacity * sizeof(char*));
    char buf[WORD_MAX];
    while (fgets(buf, sizeof(buf), f)) {
        // trim newline
        size_t len = strlen(buf);
        while (len && (buf[len-1] == '\n' || buf[len-1] == '\r')) buf[--len] = '\0';
        if (!len) continue;
        // to lower
        for (size_t i=0;i<len;++i) buf[i] = (char)tolower((unsigned char)buf[i]);
        if (count == capacity) {
            capacity *= 2;
            list = (char **) xrealloc(list, capacity * sizeof(char*));
        }
        list[count] = (char *) xmalloc(len+1);
        strcpy(list[count], buf);
        count++;
    }
    fclose(f);
    *outCount = count;
    return list;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <stopFile> <textFile>\n", argv[0]);
        return 1;
    }
    const char *stopFile = argv[1];
    const char *textFile = argv[2];

    int stopCount = 0;
    char **stopWords = readStopWords(stopFile, &stopCount);

    FILE *f = fopen(textFile, "r");
    if (!f) { fprintf(stderr, "Cannot open text file: %s\n", textFile); return 1; }

    FILE *outf = fopen("result.txt", "w");
    if (!outf) { fprintf(stderr, "Cannot open result.txt for writing\n"); fclose(f); return 1; }

    Index idx;
    initIndex(&idx);

    char lineBuf[4096];
    int lineNumber = 0;
    int atSentenceStart = 1; // beginning of file counts as sentence start

    while (fgets(lineBuf, sizeof(lineBuf), f)) {
        lineNumber++;
        size_t len = strlen(lineBuf);
        int i = 0;
        while (i < (int)len) {
            if (isalpha((unsigned char)lineBuf[i])) {
                char wordBuf[WORD_MAX];
                int wlen = 0;
                int start = i;
                while (i < (int)len && isalpha((unsigned char)lineBuf[i])) {
                    if (wlen < WORD_MAX-1) wordBuf[wlen++] = lineBuf[i];
                    i++;
                }
                wordBuf[wlen] = '\0';

                // Determine if proper noun: starts uppercase AND not at sentence start.
                int isProperNoun = 0;
                if (isupper((unsigned char)wordBuf[0]) && !atSentenceStart) {
                    isProperNoun = 1;
                }
                // After processing a word, we are no longer at sentence start.
                atSentenceStart = 0;

                if (isProperNoun) continue; // skip proper nouns

                // lowercase
                for (int k=0;k<wlen;++k) wordBuf[k] = (char)tolower((unsigned char)wordBuf[k]);

                if (isStopWord(stopWords, stopCount, wordBuf)) continue;

                addWord(&idx, wordBuf, lineNumber);
            } else {
                // update sentence start if encountering terminal punctuation
                if (lineBuf[i] == '.' || lineBuf[i] == '!' || lineBuf[i] == '?') {
                    atSentenceStart = 1;
                }
                i++;
            }
        }
    }
    fclose(f);

    // Sort entries lexicographically
    qsort(idx.entries, idx.count, sizeof(IndexEntry), compareEntries);

    // Output to file
    for (int i = 0; i < idx.count; ++i) {
        fprintf(outf, "%s %d", idx.entries[i].word, idx.entries[i].count);
        for (int j = 0; j < idx.entries[i].lineCount; ++j) {
            fprintf(outf, ", %d", idx.entries[i].lines[j]);
        }
        fprintf(outf, "\n");
    }
    fclose(outf);

    // Cleanup
    for (int i = 0; i < idx.count; ++i) {
        free(idx.entries[i].word);
        free(idx.entries[i].lines);
    }
    free(idx.entries);
    for (int i=0;i<stopCount;++i) free(stopWords[i]);
    free(stopWords);

    return 0;
}
