#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "eventScore.h"

// 문자열 앞/뒤 공백 제거
static char *trim(char *str) {
    char *end;

    while (*str && isspace((unsigned char)*str)) str++;

    if (*str == '\0') return str;

    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';

    return str;
}

// 기본 함수들

const char *event_type_to_string(EventType type) {
    switch (type) {
    case EVT_FAILED_PASSWORD:   return "FAILED_PASSWORD";
    case EVT_INVALID_USER:      return "INVALID_USER";
    case EVT_ACCEPTED_PASSWORD: return "ACCEPTED_PASSWORD";
    case EVT_OTHER_AUTHFAIL:    return "OTHER_AUTHFAIL";
    default:                    return "UNKNOWN";
    }
}

EventType parse_event_type(const char *event_type_str) {
    if (!event_type_str) return EVT_UNKNOWN;

    if (strcmp(event_type_str, "FAILED_PASSWORD") == 0) {
        return EVT_FAILED_PASSWORD;
    } else if (strcmp(event_type_str, "INVALID_USER") == 0) {
        return EVT_INVALID_USER;
    } else if (strcmp(event_type_str, "ACCEPTED_PASSWORD") == 0) {
        return EVT_ACCEPTED_PASSWORD;
    } else if (strcmp(event_type_str, "OTHER_AUTHFAIL") == 0) {
        return EVT_OTHER_AUTHFAIL;
    }
    return EVT_UNKNOWN;
}

// 점수
int get_base_score(EventType type) {
    switch (type) {
    case EVT_FAILED_PASSWORD:    return 10;
    case EVT_INVALID_USER:       return 12;   // 계정 브루트포스
    case EVT_OTHER_AUTHFAIL:     return 5;
    case EVT_ACCEPTED_PASSWORD:  return -5;   // 의심도 감소
    default:                     return 0;
    }
}

// 이벤트 타입 파악
//
// auth.log 스타일 기준
// - "Failed password" > FAILED_PASSWORD
// - "Invalid user" > INVALID_USER
// - "Accepted password" > ACCEPTED_PASSWORD
// 나머지는 > EVT_OTHER_AUTHFAIL
//
static EventType detect_event_type_from_line(const char *line, char *out_type_str, size_t out_sz) {
    if (!line) {
        if (out_type_str && out_sz > 0) out_type_str[0] = '\0';
        return EVT_UNKNOWN;
    }

    if (strstr(line, "Failed password") != NULL) {
        if (out_type_str && out_sz > 0) {
            strncpy(out_type_str, "FAILED_PASSWORD", out_sz - 1);
            out_type_str[out_sz - 1] = '\0';
        }
        return EVT_FAILED_PASSWORD;
    }

    if (strstr(line, "Invalid user") != NULL) {
        if (out_type_str && out_sz > 0) {
            strncpy(out_type_str, "INVALID_USER", out_sz - 1);
            out_type_str[out_sz - 1] = '\0';
        }
        return EVT_INVALID_USER;
    }

    if (strstr(line, "Accepted password") != NULL) {
        if (out_type_str && out_sz > 0) {
            strncpy(out_type_str, "ACCEPTED_PASSWORD", out_sz - 1);
            out_type_str[out_sz - 1] = '\0';
        }
        return EVT_ACCEPTED_PASSWORD;
    }

    // 다른 auth 실패 패턴 (추가 가능)
    if (strstr(line, "authentication failure") != NULL ||
        strstr(line, "Failed publickey") != NULL) {
        if (out_type_str && out_sz > 0) {
            strncpy(out_type_str, "OTHER_AUTHFAIL", out_sz - 1);
            out_type_str[out_sz - 1] = '\0';
        }
        return EVT_OTHER_AUTHFAIL;
    }

    if (out_type_str && out_sz > 0) out_type_str[0] = '\0';
    return EVT_UNKNOWN;
}

// IP 추출
static int extract_ip_from_line(const char *line, char *out_ip, size_t out_sz) {
    const char *p = strstr(line, " from ");
    if (!p) return 0;

    p += strlen(" from ");
    // 공백 전까지가 IP라 가정
    const char *start = p;
    while (*p && !isspace((unsigned char)*p)) {
        p++;
    }

    size_t len = (size_t)(p - start);
    if (len == 0 || len >= out_sz) return 0;

    memcpy(out_ip, start, len);
    out_ip[len] = '\0';
    return 1;
}

// 공개 API: 한 줄 파싱

int eventscore_parse_line(const char *line, EventScoreResult *out) {
    if (!line || !out) return 0;

    // 작업용 버퍼
    char buf[1024];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *trimmed = trim(buf);

    // 빈 줄이면 패스
    if (*trimmed == '\0') return 0;

    // 이벤트 타입 탐지
    EventType type = detect_event_type_from_line(trimmed,
                                                 out->event_type_str,
                                                 sizeof(out->event_type_str));
    if (type == EVT_UNKNOWN) {
        // 관심 없는 이벤트이면 0 리턴
        return 0;
    }

    // IP 추출
    if (!extract_ip_from_line(trimmed, out->ip, sizeof(out->ip))) {
        // 이벤트는 맞는데 IP가 없으면 스킵
        return 0;
    }

    // 타입 / 점수 채우기
    out->type = type;
    out->base_score = get_base_score(type);

    return 1;
}

// 공개 API: 파일 전체 처리

int eventscore_process_file(
    const char *filename,
    void (*callback)(const EventScoreResult *res, void *user_data),
    void *user_data
) {
    if (!filename || !callback) return -1;

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("[eventScore] fopen");
        return -1;
    }

    char line[2048];
    EventScoreResult res;

    while (fgets(line, sizeof(line), fp)) {
        if (eventscore_parse_line(line, &res)) {
            // 유효한 이벤트/아이피/점수가 파싱된 경우 콜백 호출
            callback(&res, user_data);
        }
    }

    fclose(fp);
    return 0;
}


