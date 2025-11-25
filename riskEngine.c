#include <stdio.h>
#include "eventScore.h"

// 파서가 넘겨줄 이벤트 구조 정의 - 이벤트 타입, ip, 국가(삭제 고려)
typedef struct {
    const char *event_type_str;
    const char *ip;
    const char *country;
} ParsedEvent;

// 베이스 스코어 계산
int compute_base_score(const ParsedEvent *ev) {
    EventType type = parse_event_type(ev->event_type_str);
    return get_base_score(type);
}

int main(void) {
    // 테스트용 이벤트
    ParsedEvent ev1 = {
        .event_type_str = "FAILED_PASSWORD",
        .ip = "185.13.44.21",
        .country = "RU"
    };

    ParsedEvent ev2 = {
        .event_type_str = "INVALID_USER",
        .ip = "154.21.11.93",
        .country = "CN"
    };

    ParsedEvent ev3 = {
        .event_type_str = "ACCEPTED_PASSWORD",
        .ip = "192.168.0.5",
        .country = "KR"
    };

    printf("=== Risk Engine Base Score Test ===\n");

    int score1 = compute_base_score(&ev1);
    printf("Event: %s → Base Score = %d\n", ev1.event_type_str, score1);

    int score2 = compute_base_score(&ev2);
    printf("Event: %s → Base Score = %d\n", ev2.event_type_str, score2);

    int score3 = compute_base_score(&ev3);
    printf("Event: %s → Base Score = %d\n", ev3.event_type_str, score3);

    return 0;
}

