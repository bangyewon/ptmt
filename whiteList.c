#include <stdio.h>
#include <unistd.h>
#include "./ipset.h"

int strcom(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        if (*s1 != *s2)
            return (*s1 - *s2);
        s1++;
        s2++;
    }
    return (*s1 - *s2);
}

int existFile(char *filePath) {
    const char *filename = "whiteList.conf";

    if (access(filename, F_OK) == 0) {
        return 1;
    }
    return 0;
}

int checkFile() {
    /**
     * 사용자가 /etc경로에 있는지 확인 -(파일열기) ->  whitelist.conf가 있는
     * 지 없는지 확인 
     * 있으면 파일 읽어서 ipset에 저장
     * 없으면 새로 생성
     */ 

    char *cwd;
    char wd[BUFSIZ];

    cwd = getcwd(wd, sizeof(wd));

    // 현재 위치경로 어디인지 확인 -> 임의로 현재경로로 비교함
    printf("\n============================================================\n");
    printf(" [화이트리스트 환경 확인]\n");

    if (strcom(cwd, "/home/ubuntu/바탕화면/ptmt-main") == 0) {

        printf("  • 현재 작업 경로 : %s\n", cwd);

        // whiteList.conf 존재 여부 확인
        if (existFile(cwd) == 1) {
            printf("  • whiteList.conf 파일 확인 : 존재함\n");
            printf("  → 기존 화이트리스트 파일을 사용합니다.\n");
        } 
        else {
            char answer;

            printf("  • whiteList.conf 파일 확인 : 없음\n");
            printf("  → 새로 생성하시겠습니까? (y / n): ");
            scanf(" %c", &answer); 

            if (answer == 'y' || answer == 'Y') {
                printf("\n  → 파일 생성 중...\n");
                FILE *newFile = fopen("whiteList.conf", "w+");
                fclose(newFile);
                printf("  → whiteList.conf 파일 생성 완료!\n");
                printf("  → 파일에 허용할 IP 주소들을 입력하세요.\n");
            } 
            else {
                printf("  → 파일 생성이 취소되었습니다.\n");
            }
        }

        printf("============================================================\n");
    } 
    
    else {
        printf("  →  프로젝트 경로가 아닙니다. 실행을 중단합니다.\n");
        printf("============================================================\n\n");
        return 0;
    }

    // 파일 수정 후
    char ans;
    printf("\n화이트리스트 파일 수정이 끝났다면 Y를 입력하세요 : ");
    scanf(" %c", &ans);
    printf("\n");
        printf("============================================================\n\n");

    if (ans == 'Y' || ans == 'y') {
        printf("\n[ 진행 중 ] IP 목록을 ipset에 등록 중...\n");
        registerIp("whiteList.conf");
        printf("============================================================\n\n");
        printf("[ 진행 완료 ] 등록 작업 완료!\n");
    }

    return 1;
}
