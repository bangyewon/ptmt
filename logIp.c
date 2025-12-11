#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "./ipset.h"
// 함수 선언
int checkCountry(const char *targetIp);
int isIp(const char *str);
int loadIpset(char ipsetIps[][50], int max);
int extractIp(const char *filename);
int in_range(int t[4], int s[4], int e[4]);

// 문자열이 IP인지 검사
int isIp(const char *str) {
	int a,b,c,d;
	if(sscanf(str, "%d.%d.%d.%d",&a,&b,&c,&d) != 4)
		return 0
	if (a < 0 || a > 255) return 0;
	if (b < 0 || b > 255) return 0;
	if (c < 0 || c > 255) return 0;
	if (d < 0 || d > 255) return 0;

	return 1;
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

            char *ip = strtok(trimmed, " \t\n");
            while (ip != NULL) {
                if (isIp(ip)) {
			 printf("============================================================\n");
                    printf("\n[IP_ 분석 완료]추출된 IP : %s\n", ip);
		    printf("\n");

                    int found = 0;

                    for (int j = 0; j < ipsetCount; j++) {
                        if (strcmp(ipsetIps[j], ip) == 0) {
                            found = 1;
                            break;
                        }
                    }

                    if (found) {
                       printf("============================================================\n");
		     	 printf("\n[ 결과 ] %s 는 ipset의 화이트리스트에 존재함.\n", ip);
			 printf("\n");
			 printf("============================================================\n");
                    }
                    else {
			 printf("============================================================\n");
                        printf("\n[ 결과 ] %s 는 ipset의 화이트리스트에 존재하지 않음. 위험!\n", ip);
                        printf("\n"); 
			printf("============================================================\n");
                        printf("\n[ 탐색 중 ] 어떤 국가의 ip인지 확인합니다.\n");
                        printf("\n");
			int countryFound = checkCountry(ip);
			if(countryFound == 1) {
				fclose(fp);
				return 1;
			}
                    }
                }
                ip = strtok(NULL, " \t\n");
            }
        }
    }

    fclose(fp);
    return 0;
}
// ip 비교 위한 long으로 ip변환하기 
unsigned long longIp(int ip[4]) {
	return ((unsigned long)ip[0] << 24) |
		((unsigned long)ip[1] << 16) |
		((unsigned long)ip[2] << 8) |
		(unsigned long)ip[3];
}
// ip 범위 체크
int in_range(int t[4], int s[4], int e[4]) {
	unsigned long T = longIp(t);
	unsigned long S = longIp(s);
	unsigned long E  = longIp(e);
	return (T >= S && T <= E);

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

        sscanf(line, "%99[^,],%49[^,],%49[^,\n]", country, start, end);

	trim(country);
	trim(start);
	trim(end);

        int s[4], e[4];
        sscanf(start, "%d.%d.%d.%d", &s[0], &s[1], &s[2], &s[3]);
        sscanf(end, "%d.%d.%d.%d", &e[0], &e[1], &e[2], &e[3]);

         if (in_range(t, s, e)) {
		 printf("============================================================\n"); 
            printf("\n[탐색 결과]\n");
            printf("  • 대상 IP        : %s\n", targetIp);
            printf("  • 국가 이름      : %s\n", country);
            printf("  • IP 대역        : %s ~ %s\n", start, end);
	    printf("\n");
            printf("============================================================\n");
            fclose(fp);
            return 1;
        }
    }
     printf("[탐색 완료] %s 에 대한 국가 정보를 찾지 못했습니다.\n", targetIp);
    fclose(fp);
    return 0;
}
