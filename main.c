#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <ctype.h>
#include "cache.h"

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
    free_cache(c, input);
    return 0;
}