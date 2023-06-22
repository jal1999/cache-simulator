#include "cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <ctype.h>

extern int g_time;

Input get_input() {
    Input input;
    scanf("%d", &input.S);
    scanf("%d", &input.E);
    scanf("%d", &input.B);
    scanf("%d", &input.m);
    scanf("%s", input.policy);
    scanf("%d", &input.hit_time);
    scanf("%d", &input.miss_penalty);
    input.s = (int) log2(input.S);
    input.b = (int) log2(input.B);
    input.M = (int) pow(2, input.m);
    input.t = input.m - (input.s + input.b);
    return input;
}

Line *create_line() {
    Line *line = malloc(sizeof(*line));
    line->tag = -1;
    line->valid = 0;
    return line;
}

Set *create_set(int E, int index) {
    Set *set = malloc(sizeof(*set));
    Line **lines = malloc(E * sizeof(Line *));

    for (int i = 0; i < E; ++i) {
        Line *line = create_line();
        lines[i] = line;
    }

    set->accesses = malloc(E * sizeof(int));
    set->times = malloc(E * sizeof(int));
    for (int i = 0; i < E; ++i) {
        set->accesses[i] = 0;
    }
    set->lines = lines;
    set->index = index;
    set->lines_used = 0;
    return set;
}

Cache *create_cache(int S, int E) {
    Cache *cache = malloc(sizeof(*cache));
    Set **sets = malloc(S * sizeof(Set *));

    for (int i = 0; i < S; ++i) {
        Set *set = create_set(E, i);
        sets[i] = set;
    }

    cache->sets = sets;
    cache->hits = 0;
    cache->misses = 0;
    return cache;
}

int get_set_index(int address, int s, int b) {
    int mask = (unsigned int) pow(2, s) - 1;
    address >>= b;
    return mask & address;
}

int get_tag(int address, int t, int s, int b) {
    address >>= s + b;
    int mask = (int) pow(2, t) - 1;
    return mask & address;
}

void find_lru(Set *set, int address, Input input) {
    int worst_idx = 0, worst_score = INT_MAX;
    /*
     * Will iterate entire times array, and each time we find
     * a line that has been used at a time stamp that is earlier
     * than the earliest we have found so far, we will update
     * our bookkeeping to reflect that, and will replace the line
     * with the earliest timestamp at the end of the function.
     */
    for (int i = 0; i < set->lines_used; ++i) {
        if (set->times[i] < worst_score) {
            worst_idx = i;
            worst_score = set->times[i];
        }
    }
    Line *curr_line = set->lines[worst_idx];
    replace(set, get_tag(address, input.t, input.s, input.b), worst_idx, input);
}

void find_lfu(Set *set, int address, Input input) {
    int worst_idx = -1, worst_score = INT_MAX;
    /*
     * Will iterate through entire accesses array, and keep track
     * of which line has been accessed the fewest number of times.
     * At the end of the function, we will replace the line with
     * the fewest number of accesses.
     */
    for (int i = 0; i < input.E; ++i) {
        if (set->accesses[i] < worst_score) {
            worst_idx = i;
            worst_score = set->accesses[i];
        }
    }
    replace(set, get_tag(address, input.t, input.s, input.b), worst_idx, input);
}

void replace(Set *set, int tag, int index, Input input) {
    Line *curr_line = set->lines[index];
    curr_line->tag = tag;
    curr_line->valid = 1;
    if (set->lines_used < input.E) ++set->lines_used;
    set->times[index] = ++g_time;
    ++set->accesses[index];
}

void access(Cache *cache, Input input, int address) {
    int set_idx = get_set_index(address, input.s, input.b);
    Set *curr_set = cache->sets[set_idx];
    /* Address' tag */
    int c_tag = get_tag(address, input.t, input.s, input.b);

    for (int i = 0; i < input.E; ++i) {
        Line *curr_line = curr_set->lines[i];
        /*
         * If the current line's valid bit is set & its tag
         * is equal to the addresses' tag, then this is a hit.
         */
        if (curr_line->valid == 1 && curr_line->tag == c_tag) {
                ++cache->hits;
                ++curr_set->accesses[i];
                printf("%#x H\n", address);
                return;
            }
    }
    /* Converting eviction policy to all lowercase to make the
     * conditionals less verbose */
    char *policy = lowercase(input.policy, 3);
    if (strcmp(policy, "lru") == 0) {
        find_lru(cache->sets[set_idx], address, input);
    } else {
        find_lfu(cache->sets[set_idx], address, input);
    }
    ++cache->misses;
    printf("%#x M\n", address);
}

void print_stats(Cache *cache, Input input) {
    float miss_rate = (float)cache->misses / (cache->misses + cache->hits);
    int total_cycles = ((cache->hits + cache->misses) * input.hit_time) + (cache->misses * input.miss_penalty);
    printf("%lf %d\n", miss_rate, total_cycles);
}

char* lowercase(char *str, int len) {
    char *lower = malloc(len * sizeof(*lower));

    for (int i = 0; i < len; ++i) {
        lower[i] = tolower(str[i]);
    }
    return lower;
}

void free_cache(Cache *cache, Input input) {
    for (int i = 0; i < input.S; ++i) {
        Set* curr_set = cache->sets[i];
        for (int j = 0; j < input.E; ++j) {
            Line *curr_line = curr_set->lines[j];
            free(curr_line);
        }
        free(curr_set->lines);
        free(curr_set);
    }
    free(cache->sets);
    free(cache);
}