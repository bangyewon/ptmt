/* enum + 함수 선언 */

#ifndef EVENT_SCORE_H
#define EVENT_SCORE_H

typedef enum { /* 이벤트 타입 정리 */
    EVT_FAILED_PASSWORD = 0, /* ssh 비밀번호 실패*/
    EVT_INVALID_USER, /* 없는 사용자로 로그인 시도 */
    EVT_FAILED_PUBLICKEY, /* 공개키 인증 실패 */
    EVT_MAX_AUTH_EXCEEDED, /* 최대 인증 횟수 초과 */
    EVT_ACCEPTED_PASSWORD, /* 비밀번호 로그인 성공 */
    EVT_ACCEPTED_PUBLICKEY, /* 키 로그인 성공 */
    EVT_ROOT_LOGIN, /* root 로그인 성공 (경고성 이벤트) */
    EVT_OTHER, /* 기타 */
    EVT_TYPE_COUNT      // 마지막: 배열 크기 용
} EventType;

/* enum 기반 점수 계산 */
int get_base_score(EventType type);

/* 문자열(event_type) → enum 매핑 (문자열을 받았을 때 사용) */
EventType parse_event_type(const char *s);

#endif

