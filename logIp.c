#include <stdio.h>
#include <string.h>
#include <ctype.h>

// 함수 선언
int checkCountry(const char *targetIp);
char *trim(char *str);
int isIp(const char *str);
int loadIpset(char ipsetIps[][50], int max);
int extractIp(const char *filename);
int in_range(int t[4], int s[4], int e[4]);

// trim 함수
char *trim(char *str) {
    char *end;

    while (isspace((unsigned char)*str)) {
        str++;
    }

    if (*str) {
        end = str + strlen(str) - 1;
        while (end > str && isspace((unsigned char)*end)) {
            end--;
        }
        *(end + 1) = '\0';
    }
    return str;
}

// 문자열이 IP인지 검사
int isIp(const char *str) {
    int dot = 0;
    int len = strlen(str);

    for (int i = 0; i < len; i++) {
        if (str[i] == '.') dot++;
        else if (!isdigit((unsigned char)str[i])) return 0;
    }
    return dot == 3;
}

// ipset 화이트리스트 추출
int loadIpset(char ipsetIps[][50], int max) {
    FILE *pp = popen("ipset list whitelist", "r");
    if (!pp) {
        printf("ipset list를 열 수 없다.\n");
        return 0;
    }

    char line[1024];
    int count = 0;
    int readMembers = 0;

    while (fgets(line, sizeof(line), pp) && count < max) {
        char *t = trim(line);

        if (strlen(t) == 0) continue;

        if (strcmp(t, "Members:") == 0) {
            readMembers = 1;
            continue;
        }

        if (readMembers) {
            if (isIp(t)) {
                strcpy(ipsetIps[count], t);
                count++;
            }
        }
    }
    pclose(pp);
    return count;
}

// logfile에서 ip 추출
int extractIp(const char *filename) {
    FILE *fp = fopen(filename, "r");

    if (!fp) {
        printf("파일을 열 수 없어요.\n");
        return 1;
    }

    char line[1024];
    char ipsetIps[100][50];

    int ipsetCount = loadIpset(ipsetIps, 100);

    while (fgets(line, sizeof(line), fp)) {
        char *trimmed = trim(line);
        if (strlen(trimmed) > 0) {

            char *ip = strtok(trimmed, "\t\n");
            while (ip != NULL) {

                if (isIp(ip)) {
                    printf("추출된 ip : %s\n", ip);

                    int found = 0;

                    for (int j = 0; j < ipsetCount; j++) {
                        if (strcmp(ipsetIps[j], ip) == 0) {
                            found = 1;
                            break;
                        }
                    }

                    if (found) {
                        printf("%s 는 ipset의 화이트리스트에 존재함.\n", ip);
                    }
                    else {
                        printf("%s 는 ipset의 화이트리스트에 존재하지 않음. 위험!\n", ip);
                        printf("* * * * * \n");
                        printf("*** 어떤 국가의 ip인지 확인중 ***\n");
                        checkCountry(ip);
                    }
                }
                ip = strtok(NULL, "\t\n");
            }
        }
    }

    fclose(fp);
    return 0;
}

// 범위 체크 함수
int in_range(int t[4], int s[4], int e[4]) {
    for (int i = 0; i < 4; i++) {
        if (t[i] < s[i]) return 0;
        if (t[i] > e[i]) return 0;

        if (t[i] > s[i] && t[i] < e[i]) return 1;
    }
    return 1;
}

// 국가 IP 대역 판별
int checkCountry(const char *targetIp) {

    const char *filename = "country.csv";
    FILE *fp = fopen(filename, "r");

    if (!fp) {
        printf("파일을 열 수 없어요.\n");
        return -1;
    }

    int t[4];
    sscanf(targetIp, "%d.%d.%d.%d", &t[0], &t[1], &t[2], &t[3]);

    char line[500];
    char country[100], start[50], end[50];

    while (fgets(line, sizeof(line), fp)) {

        sscanf(line, "%[^,],%[^,],%s", country, start, end);

        int s[4], e[4];
        sscanf(start, "%d.%d.%d.%d", &s[0], &s[1], &s[2], &s[3]);
        sscanf(end, "%d.%d.%d.%d", &e[0], &e[1], &e[2], &e[3]);

        if (in_range(t, s, e)) {
            printf("%s 는 [%s] 대역에 포함되어 있음 (%s ~ %s)\n",
                   targetIp, country, start, end);
            fclose(fp);
            return 1;
        }
    }

    fclose(fp);
    return 0;
}
