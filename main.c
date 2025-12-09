#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include "./whiteList.h"
#include "./ipset.h"
#include "./logIp.h"
#include "./authInotifyIp.h"
#include "./eventScore.h"

void startAuthInotifyWatcher(void);
static const char *IP_LINES_FILE = "./logwatcher/ip-lines.log";

// 워처를 위한 쓰레드 함수
// 컴파일 시 -lpthread 꼭 붙이기 !!
static void* watcher_thread(void *arg) {
    (void)arg;
    startAuthInotifyWatcher();   // 여기서 무한 루프
    return NULL;
}

int main() {
	pthread_t tid;
	// inotify 워처 먼저 백그라운드 쓰레드로 실행
	if (pthread_create(&tid, NULL, watcher_thread, NULL) != 0) {
        perror("[main] pthread_create");
        return 1;
    }

	// whiteList.c 실행 -> 파일 생성-> ipset까지 넘어가 ip table 설정 완료
	checkFile();

	//eventScore에서 이벤트종류,ip파싱,점수매칭
    FILE *fp = NULL;

    while (!fp) {
        fp = fopen(IP_LINES_FILE, "r");
        if (!fp) {
            printf("[main] Waiting for %s ...\n", IP_LINES_FILE);
            usleep(500000); // 0.5초
        }
    }
	fseek(fp, 0, SEEK_END);	
	char line[2048];
	// 새 로그 확인
	while (1) {
        long pos = ftell(fp);
        if (!fgets(line, sizeof(line), fp)) {
            clearerr(fp);
            usleep(200000); // 재확인
            continue;
        }

        // 새 로그 읽음 - eventScore로 파싱
        EventScoreResult res;
        memset(&res, 0, sizeof(res));

        if (!eventscore_parse_line(line, &res)) {
            // skip
            continue;
        }

        // 파싱 성공 — 여기서 logIp → riskEngine으로 전달
        printf("[EVENT] ip=%s type=%s base_score=%d\n",
               res.ip,
               res.event_type_str,
               res.base_score
        );
        // 여기서 logIp 호출
	extractIp("/home/ubuntu/바탕화면/ptmt-main/logwatcher/ip-lines.log");
        // riskEngine 호출
    }

    fclose(fp);
	pthread_join(tid, NULL);
    return 0;

	//logIp.c로 화이트리스트,국가 비교 ->if문으로 1이면 riskEngine.c넘어감

}
