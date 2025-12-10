#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

int addIp(const char *ip) {
	char cmd[256];
	snprintf(cmd,sizeof(cmd), "ipset add whitelist %s",ip);
	int success = system(cmd);
	return (success == 0);
}

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

int registerIp(const char *filename) {
	FILE *fp = fopen(filename,"r");

	char line[1024];
	int lineNu = 1;
	if(fp == NULL) {
		printf("파일을 열 수 없어요.\n");
		return 1;
	}

	while(fgets(line,sizeof(line),fp)) {
	// 두번째줄부터 읽기 시작해서 공백 다 제거 후 ip만 가져오기
		char *trimmed = trim(line);
		if(strlen(trimmed) > 0) {
			printf("%s\n",trimmed);
			}
		addIp(trimmed);
		}
	
	fclose(fp);
	return 0;
}
