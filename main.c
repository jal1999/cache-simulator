#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <ctype.h>

/* Single line in set */
typedef struct Line {
    int valid, tag;
} Line;

/* Single set in cache */
typedef struct Set {
    Line **lines;
    int index, *times, *accesses, lines_used;
} Set;

/* Cache */
typedef struct Cache {
    Set **sets;
    int hits, misses;
} Cache;

/* Initial & computed fundamental parameters */
typedef struct Input {
    int S, E, B, M, m, t, b, s, hit_time, miss_penalty;
    char policy[4];
} Input;

/**
 * Retrieves and computes all fundamental paramters needed for the cache.
 * @return fundamental parameters of the cache.
 */
Input get_input();

/**
 * "Constructor" for the Line struct.
 * @return pointer to a Line struct.
 */
Line *create_line();

/**
 * "Constructor" Set struct.
 * @param E number of cache lines per set.
 * @param index index of Set being created.
 * @return pointer to a Set struct.
 */
Set *create_set(int E, int index);

/**
 * "Constructor" for cache.
 * @param S total number of sets in the cache.
 * @param E number of cache lines per set.
 * @return pointer to Cache struct.
 */
Cache *create_cache(int S, int E);

/**
 * Computes through bitwise operations the integer value of the
 * set index in the data item's address.
 *
 * @param address address retrieved from STDIN.
 * @param s number of bits in the set index.
 * @param b number of bits in the block index.
 * @return integer value of address's set index.
 */
int get_set_index(int address, int s, int b);

/**
 * Computes through bitwise operations the integer value of
 * the address's tag bits.
 *
 * @param address address retrieved from STDIN.
 * @param t number of bits in the tag.
 * @param s number of bits in the set index.
 * @param b number of bits in the block offset.
 * @return integer value of the tag index.
 */
int get_tag(int address, int t, int s, int b);

/**
 * Finds index of least recently used line in a given Set, and calls
 * replace() on the given line.
 *
 * @param set given Set.
 * @param address address retrieved from STDIN.
 * @param input fundamental parameters of the cache.
 */
void find_lru(Set *set, int address, Input input);

/**
 * Finds least frequently used line in a given Set.
 * @param set the given Set.
 * @param address address retrieved from STDIN.
 * @param input fundamental parameters of the cache.
 */
void find_lfu(Set *set, int address, Input input);

/**
 * Replaces a given line in a set by replacing its tag & possibly
 * its valid bit, with the tag of given address.
 *
 * @param set given set that contains the line to be replaced.
 * @param tag the integer value of the tag of a given address.
 * @param index index of the line in the Set's array to be replaced.
 * @param input fundamental parameters of the cache.
 */
void replace(Set *set, int tag, int index, Input input);

/**
 * Query function to be called with address from STDIN, when user wants to see
 * if a given data item is present in the cache.
 *
 * @param cache the cache.
 * @param input fundamental parameters of the cache.
 * @param address address retrieved from STDIN.
 */
void access(Cache *cache, Input input, int address);

/**
 * Prints miss rate & total cycles spent during simulation.
 *
 * @param cache the cache.
 */
void print_stats(Cache* cache, Input input);

/**
 * Converts string to all lowercase and returns a pointer
 * @param str the string to be put in lowercase
 * @return pointer to new string
 */
char* lowercase(char *str, int len);

/* Will be using this variable as a clock, as to keep track of how recently a
 * line has been accessed in a given set. Its value will be stored in the times[] in
 * each Set
 */
int g_time = 0;

int main() {
    Input input = get_input();
    Cache *c = create_cache(input.S, input.E);
    int add;
    scanf("%x", &add);
    while (add != -1) {
        access(c, input, add);
        scanf("%x", &add);
    }
    print_stats(c, input);
    return 0;
}

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