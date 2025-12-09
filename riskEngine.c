#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "riskEngine.h"
#include "eventScore.h"

// 기본 설정
#define MAX_IP_COUNT   1024   // 동시에 관리할 최대 공격자 IP 수
#define MAX_IP_LEN     64

// 차단 점수 (100)
static const int BLOCK_THRESHOLD = 100;

// 차단에 사용할 ipset 이름
static const char *BLOCK_IPSET_NAME = "blocklist";

// 내부 구조체 & 전역 배열
typedef struct {
    char ip[MAX_IP_LEN];  // 공격 IP
    int  score;           // 누적 점수
    int  blocked;         // 차단 여부
} IpScoreEntry;

static IpScoreEntry g_ip_scores[MAX_IP_COUNT];
static int g_ip_count = 0;

// 문자열의 IPv4 형태 체크
static int is_ip_like(const char *str) {
    int dot = 0;
    int len = strlen(str);

    if (len < 7 || len > 15) return 0;

    for (int i = 0; i < len; i++) {
        if (str[i] == '.') {
            dot++;
        } else if (str[i] < '0' || str[i] > '9') {
            return 0;
        }
    }
    return dot == 3;
}

// IP 엔트리
static IpScoreEntry *find_or_create_ip_entry(const char *ip) {
    for (int i = 0; i < g_ip_count; i++) {
        if (strcmp(g_ip_scores[i].ip, ip) == 0) {
            return &g_ip_scores[i];
        }
    }

    if (g_ip_count >= MAX_IP_COUNT) {
        fprintf(stderr, "[riskEngine] IP 테이블이 꽉 찼습니다 (MAX_IP_COUNT=%d)\n", MAX_IP_COUNT);
        return NULL;
    }

    strncpy(g_ip_scores[g_ip_count].ip, ip, MAX_IP_LEN - 1);
    g_ip_scores[g_ip_count].ip[MAX_IP_LEN - 1] = '\0';
    g_ip_scores[g_ip_count].score   = 0;
    g_ip_scores[g_ip_count].blocked = 0;

    return &g_ip_scores[g_ip_count++];
}

// /24 대역으로 ipset 차단
static void block_ip_range(const char *ip) {
    unsigned int a, b, c, d;

    if (sscanf(ip, "%u.%u.%u.%u", &a, &b, &c, &d) != 4) {
        // 파싱 실패: 단일 IP 차단으로 fallback
        char cmd_single[256];
        snprintf(cmd_single, sizeof(cmd_single),
                 "ipset add %s %s -exist",
                 BLOCK_IPSET_NAME, ip);

        printf("[riskEngine] 단일 IP 차단 명령: %s\n", cmd_single);
        system(cmd_single);
        return;
    }

    char cmd[256];
    // 대역 차단
    snprintf(cmd, sizeof(cmd),
             "ipset add %s %u.%u.%u.0/24 -exist",
             BLOCK_IPSET_NAME, a, b, c);

    printf("[riskEngine] /24 대역 차단 명령: %s\n", cmd);
    system(cmd);
}

// 외부 호출 API 구현

void risk_engine_init(void) {
    g_ip_count = 0;
    for (int i = 0; i < MAX_IP_COUNT; i++) {
        g_ip_scores[i].ip[0]  = '\0';
        g_ip_scores[i].score  = 0;
        g_ip_scores[i].blocked = 0;
    }
}

// 이벤트 점수 누적 + 임계치 초과 시 차단
void risk_engine_process_event(const ParsedEvent *ev) {
    if (!ev || !ev->ip || !ev->event_type_str) {
        return;
    }

    IpScoreEntry *entry = find_or_create_ip_entry(ev->ip);
    if (!entry) {
        return;
    }

    // 이벤트 타입 문자열 : enum
    EventType type = parse_event_type(ev->event_type_str);
    int base = get_base_score(type);

    entry->score += base;

    printf("[riskEngine] IP=%s, EVENT=%s, BASE=%d, TOTAL=%d\n",
           ev->ip, ev->event_type_str, base, entry->score);

    // 이미 차단된 IP : X
    if (entry->blocked) {
        return;
    }

    // 임계치 초과: 대역 차단
    if (entry->score >= BLOCK_THRESHOLD) {
        printf("[riskEngine] 임계치 초과! IP=%s (score=%d) → 대역 차단 실행\n",
               ev->ip, entry->score);
        block_ip_range(ev->ip);
        entry->blocked = 1;
    }
}

// logIp 출력 : "EVENT IP" 형식일 때
void risk_engine_process_log_line(const char *line) {
    if (!line) return;

    // 공백 기준으로 두 토큰만 읽음
    static char event_buf[64];
    static char ip_buf[64];

    if (sscanf(line, "%63s %63s", event_buf, ip_buf) != 2) {
        if (strncmp(line, "추출된 ip :", strlen("추출된 ip :")) == 0) {
            if (sscanf(line, "추출된 ip : %63s", ip_buf) == 1) {
                // 이벤트 타입 X : 기본 공격 이벤트로 가정
                strcpy(event_buf, "FAILED_PASSWORD");
            } else {
                return;
            }
        } else {
            return;
        }
    }

    if (!is_ip_like(ip_buf)) {
        return;
    }

    ParsedEvent ev = {
        .event_type_str = event_buf,
        .ip             = ip_buf
    };

    risk_engine_process_event(&ev);
}


