#ifndef RISK_ENGINE_H
#define RISK_ENGINE_H

#include "eventScore.h"

// riskEngine에 넘겨줄 최소 이벤트 
typedef struct {
    const char *event_type_str; 
    const char *ip;
    int base_score;    
} ParsedEvent;

// 초기화
void risk_engine_init(void);

// 정석 API
void risk_engine_process_event(const ParsedEvent *ev);

// EVENT IP 한 줄 형식일 경우
void risk_engine_process_log_line(const char *line);

#endif

