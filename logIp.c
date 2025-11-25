#include <stdio.h>
#include <unistd.h>

#define MAX_LINE
// 국가별ip와 ip-lines.log비교후 차단
char *trim(char *str) {
        char *end;

        while(isspace(*str)) {
                str++;
        }
        if (*str) {
                end = str + strlen(str) - 1;
                while(end > str && isspace(*end)) {

                        end--;
                }
                *(end + 1) = '\0';
        }
        return str;
}
// ip가 맞는지 확인
int isIp(char *str) {
	int dot = 0;
	int len = strlen(str);

	for(int i = 0; i < len; i++) {
		if(str[i] == '.') dot++;
		else if(!isdigit(str[i])) return 0;
	}
	return dot == 3;

}
// ipset의 화이트리스트ip추출
int loadIpset(char ipsetIps[][50], int max) {
	FILE *pp = popen("ipset list whitelist", "r");
	if(pp == NULL) {
		printf("ipset list를 열 수 없다.\n");
		return 0;
	}
	char line[1024];
	int count = 0;
	int readMembers = 0;
	while(fgets(line,sizeof(line),pp) && count < max) {
		char *t = trim(line);

		if(strlen(t) == 0) continue;

		if(strcmp(t,"Members:") == 0) {
			readMembers = 1;
			continue;
		}
		if(readMembers) {
			if(isIp(t)) {
				strcpy(ipsetIps[count],t);
				count++;
			}
		}
	}
	pclose(pp);
	return count;

}
// ip-lines.log에서 ip추출 후 whiteList와 비교
// 이거 어디서 연결할꺼임 ?.. 
int extractIp(const char *filename) {
	FILE *fp = fopen(filename,"r");

	char line[1024];
	if(fp == NULL) {
		printf("파일을 열 수 없어요.\n");
		return 1;
	}

	char ipsetIps[100][100];
	int ipsetCount = loadIpset(ipsetIps,100);
	
	while(fgets(line,sizeof(line),fp)) {
		char *trimmed = trim(line);
		if(strlen(trimmed) > 0) {
			// strtok으로 단어 단위로 쪼개기
			char *ip = strtok(trimmed,"\t\n");
			while(ip != NULL) {
				if(isIp(ip)) {
					printf("추출된 ip : %s\n", ip);
					
					int found = 0;
					for(int j = 0; j < ipsetCount; j++) {
						if(strcmp(ipsetIps[j], ip) == 0) {
							found = 1;
							break;
						}
					}
					if(found) {
						printf("%s 는 ipset의 화이트리스트에 존재함.\n",ip);
					}
					else {
						 printf("%s 는 ipset의 화이트리스트에 존재하지 않음. 차단대상!\n",ip);
					}
				}
				ip = strtok(NULL,"\t\n");
			}
		}
	}

	fclose(fp);
	return 0;
}
