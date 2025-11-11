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

int main(int argc, char *argv[]) {
    const char *log_path = DEFAULT_LOG;
    int argi = 1;

    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    // 옵션 파싱 (-f logfile)
    if (argc >= 3 && strcmp(argv[argi], "-f") == 0) {
        log_path = argv[argi + 1];
        argi += 2;
    }

    if (argi >= argc) {
        // 키워드가 하나도 없음
        usage(argv[0]);
        return 1;
    }

    int kwc = argc - argi;
    char **kws = &argv[argi];

    FILE *fp = fopen(log_path, "r");
    if (!fp) {
        perror(log_path);
        return 1;
    }

    char *line = NULL;
    size_t len = 0;
    size_t n;

    while ((n = getline(&line, &len, fp)) != -1) {
        // 개행 제거
        if (n > 0 && (line[n-1] == '\n' || line[n-1] == '\r')) {
            line[n-1] = '\0';
        }
        if (match_all_keywords(line, kwc, kws)) {
            puts(line);
        }
    }

    free(line);
    fclose(fp);
    return 0;
}

