#include "eventScore.h"
#include <string.h>

/* Event Type에 기반한 점수 부여 - 배열 기반 */
static const int BASE_SCORE_TABLE[EVT_TYPE_COUNT] = {
    [EVT_FAILED_PASSWORD]    = 5,
    [EVT_INVALID_USER]       = 8,
    [EVT_FAILED_PUBLICKEY]   = 4,
    [EVT_MAX_AUTH_EXCEEDED]  = 15,
    [EVT_ACCEPTED_PASSWORD]  = -10,
    [EVT_ACCEPTED_PUBLICKEY] = -5,
    [EVT_ROOT_LOGIN]         = 10,
    [EVT_OTHER]              = 0
};

/* 배열 기반 점수 계산 */
int get_base_score(EventType type) {
    if (type < 0 || type >= EVT_TYPE_COUNT) {
        return 0;   // 방어 코드
    }
    return BASE_SCORE_TABLE[type];
} /* 이벤트 하나 당 베이스 점수 1개 (1:1) */

/* 문자열로 넘어온 event_type을 enum으로 매핑 */
EventType parse_event_type(const char *s) {
    if (!s) return EVT_OTHER;

    if (strcmp(s, "FAILED_PASSWORD") == 0)      return EVT_FAILED_PASSWORD;
    if (strcmp(s, "INVALID_USER") == 0)         return EVT_INVALID_USER;
    if (strcmp(s, "FAILED_PUBLICKEY") == 0)     return EVT_FAILED_PUBLICKEY;
    if (strcmp(s, "MAX_AUTH_EXCEEDED") == 0)    return EVT_MAX_AUTH_EXCEEDED;
    if (strcmp(s, "ACCEPTED_PASSWORD") == 0)    return EVT_ACCEPTED_PASSWORD;
    if (strcmp(s, "ACCEPTED_PUBLICKEY") == 0)   return EVT_ACCEPTED_PUBLICKEY;
    if (strcmp(s, "ROOT_LOGIN") == 0)           return EVT_ROOT_LOGIN;

    return EVT_OTHER;
}

