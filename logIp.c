#include <stdio.h>
#include <unistd.h>

#define MAX_LINE
// 국가별ip와 ip-lines.log비교 -> 어떤 국가인지 판단 
char *trim(const char *str) {
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
int isIp(const char *str) {
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
						// checkCountry(ip); 호출하기
						 printf("%s 는 ipset의 화이트리스트에 존재하지 않음. 위험!\n",ip);
						 printf("* * * * * \n");
						 printf("*** 어떤 국가의 ip인지 확인중 ***\n");
						 checkCountry(ip);

					}
				}
				ip = strtok(NULL,"\t\n");
			}
		}
	}

	fclose(fp);
	return 0;
}

// 국가 ip에 해당하는지 판단
// log에서 추출한 ip로 extractIp에서 호출
int checkCountry(const char *targetIp) {
	 FILE *fp = fopen(filename,"r"); 
         
         if(fp == NULL) {
                 printf("파일을 열 수 없어요.\n");
                 return -1; 
 	 }

	 char line[500];
	 char country[100], start[50], end[50];
// 문자열 비교만으로는 안되려나 
	 unsigned int a,b,c,d;
	 sscanf(targetIp, "%u.%u.%u.%u", &a,&b,&c,&d);
	 unsigned long target = ((unsigned long)a << 24) |((unsigned long)b << 16) | ((unsigned long)c << 8) | ((unsigned long)d;

	 fgets(line,sizeof(line),fp);
	 // 국가ip파일읽기-> 변환
	 while(fgets(line,sizeof(line),fp)) {
	 	sscanf(line,"%[^,],%[^,],%s", country,start,end);

		unsigned int sa,sb,sc,sd;
          	sscanf(start, "%u.%u.%u.%u", &sa,&sb,&sc,&d);
          	unsigned long targetStart = ((unsigned long)sa << 24) |((unsigned long)sb << 16) | ((unsigned long)sc << 8) | ((unsigned long)sd;
	 
		unsigned int ba,bb,bc,bd;
  	        sscanf(end, "%u.%u.%u.%u", &ba,&bb,&bc,&bd);
        	unsigned long targetEnd = ((unsigned long)ba << 24) |((unsigned long)bb << 16) | ((unsigned long)bc << 8) | ((unsigned long)bd;

		if(targetIp >= targetStart && targetIp <= targetEnd) {
			printf("%s는 [%s] 대역에 포함됨 (%s ~ %s) \n", targetIp,country,start,end);
			fclose(fp);
			return q;
		}
	 }
	 fclose(fp);
	 return 0;

	
}
