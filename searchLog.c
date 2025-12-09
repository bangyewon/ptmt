#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_LOG "./logwatcher/ip-lines.log"

static void usage(const char *prog) {
    fprintf(stderr,
        "Usage: %s [-f logfile] keyword1 [keyword2 ...]\n"
        "  -f logfile : search target (default: %s)\n"
        "  keywords   : all must appear in a line to match (AND search)\n",
        prog, DEFAULT_LOG);
}

/* line에 모든 keyword가 포함되어 있으면 1, 아니면 0 */
static int match_all_keywords(const char *line, int kwc, char **kws) {
    for (int i = 0; i < kwc; i++) {
        if (strstr(line, kws[i]) == NULL) {
            return 0; // 하나라도 없으면 실패
        }
    }
    return 1;
}



