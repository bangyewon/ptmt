#include <stdio.h>
#include <unistd.h>
#include "./ipset.h"
int strcom(const char *s1, const char *s2) {
	while(*s1 && *s2) {
		if(*s1 != *s2)
			return (*s1 - *s2);
		s1++;
		s2++;
	}
	return (*s1 - *s2);
}

int existFile(char *filePath) {
	const char *filename = "whiteList.conf";

	if(access(filename,F_OK) == 0) {
		return 1;
	}
	return 0;
}

int main() {
        /**
         * 사용자가 /etc경로에 있는지 확인 -(파일열기) ->  whitelist.conf가 있는
지 없는지 확인 
         * 있으면 파일 읽어서 ipset에 저장
         * 없으면 새로 생성
         */ 
        char *cwd; 
        char wd[BUFSIZ];

        cwd = getcwd(wd,sizeof(wd));
	// 현재 위치경로 어디인지 확인 -> 임의로 현재경로로 비교함
        if(strcom(cwd,"/home/byewon/Desktop/ptmt") == 0) {
                printf("해당 경로에 위치해 있습니다.: %s\n",cwd);
		// whiteList.conf가 해당 경로에 있는지 확인
		if(existFile(cwd) == 1) {
			printf("whiteList.conf파일이 있습니다.\n");
		}
		else {
			char answer;

			printf("whiteList.conf파일이 없습니다.\n");
			printf("whiteList.conf파일을 만들까요? (y / n): ");
			scanf("%c", &answer);
			if(answer == 'y'|| answer == 'Y') {
				printf("파일을 생성하는 중입니다.");
				FILE *newFile = fopen("whiteList.conf","w+");
				fprintf(newFile, "# 화이트리스트 ip를 적어주세요");
				fclose(newFile);
				printf("whiteList.conf파일 생성 완료: %s\n", cwd);
				printf("whiteList.conf에 화이트리스트 ip를 적어주세요\n");
			} else {
				printf("그럼 직접 파일을 생성하세요\n");	
			}
		}
		
        } else {
                printf("현재 다른 경로에 위치해 있습니다.: %s\n",cwd);

        }
	char ans;
	printf("다했으면 Y : \n");
	scanf("%c",&ans);
	if(ans == 'Y') {
		printf("\n");
		printf("ip를 넣는중 ~\n");
		registerIp("whiteList.conf");
	}
        return 0;
}

