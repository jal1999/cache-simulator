#ifndef CACHE_H
#define CACHE_H

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

/**
 * Frees all structs in cache
 *
 * @param cache the cache
 * @param input fundamental parameters of the cache
 */
 void free_cache(Cache *cache, Input input);

#endif