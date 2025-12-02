#ifndef EVENT_SCORE_H
#define EVENT_SCORE_H

// 이벤트 타입 정의
typedef enum {
    EVT_UNKNOWN = 0,
    EVT_FAILED_PASSWORD,
    EVT_INVALID_USER,
    EVT_ACCEPTED_PASSWORD,
    EVT_OTHER_AUTHFAIL,   // 기타
} EventType;

// 이벤트 타입 → 문자열 매핑
const char *event_type_to_string(EventType type);

// 문자열 → EventType enum
EventType parse_event_type(const char *event_type_str);

// 이벤트 타입 → 베이스 점수
int get_base_score(EventType type);

// 로그 파싱 결과 구조체
typedef struct {
    char ip[64];               // 파싱된 IP
    char event_type_str[64];   // 이벤트 타입
    EventType type;            // enum 값
    int base_score;            // 이벤트 타입에 매칭된 점수
} EventScoreResult;

int eventscore_parse_line(const char *line, EventScoreResult *out);

int eventscore_process_file(
    const char *filename,
    void (*callback)(const EventScoreResult *res, void *user_data),
    void *user_data
);

#endif

