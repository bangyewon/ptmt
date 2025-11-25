#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <unistd.h>
#include <poll.h>

#define BUF_CHUNK 4096
#define INOT_BUF  (1024 * (sizeof(struct inotify_event) + 16))

/* 기본 로그 경로 */
static const char *AUTH_LOG = "/var/log/auth.log";
static const char *OUT_LOG  = "./logwatcher/ip-lines.log";

/* 파일 열기: EOF */
static FILE* open_at_end(const char *path, long *pos_out) {
    FILE *f = fopen(path, "r");
    if (!f) return NULL;
	if(fseek(f, 0, SEEK_END) != 0){
		fclose(f);
		return NULL;
	}
    long p = ftell(f);
    if (pos_out) *pos_out = p;
    return f;
}

/* 완성된 라인 그대로 출력 */
static void write_raw_line(const char *line, FILE *out) {
	fprintf(out, "%s\n", line);
	fflush(out);
}


/* 완성된 라인만 처리 */
static void drain_completed_lines(char **buf, size_t *sz, FILE *out) {
    char *start = *buf;
    char *cur = start;
    char *end = start + *sz;

    while (cur < end) {
        char *nl = memchr(cur, '\n', (size_t)(end - cur));
        if (!nl) break; // 미완 라인

		*nl = '\0'; // 개행 제거
        write_raw_line(cur, out) ; // 원문 그대로 출력
        cur = nl + 1; // 다음 라인 이
    }

    /* 남은 로그  앞으로 당기기 */
    size_t remain = (size_t)(end - cur);
    memmove(start, cur, remain);
    *sz = remain;
}

/* 파일 크기 < 현재 위치 :  truncate 감지 */
static int truncated(const char *path, long cur_pos) {
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return (st.st_size < cur_pos);
}

int main(void) {
	FILE *out = NULL;
	FILE *in = NULL;
	int ifd =-1;
	int wd =-1;
	char *buf = NULL;
	size_t cap = 0, sz = 0;
	long cur_pos = 0;

    /* 출력 파일 준비 */
    system("mkdir -p ./logwatcher");
    out = fopen(OUT_LOG, "a");
    if (!out) {
        fprintf(stderr, "[ERROR] open %s: %s\n", OUT_LOG, strerror(errno));
        return 1;
    }

    /* 감시할 auth.log 열기 */
    in = open_at_end(AUTH_LOG, &cur_pos);
	if(!in) {
		fprintf(stderr, "[ERROR] open %s: %s\n", AUTH_LOG, strerror(errno));
		fclose(out);
		return 1;
	}

    fprintf(stderr, "[INFO] Watching %s\n", AUTH_LOG);
    fprintf(stderr, "[INFO] Output   %s\n", OUT_LOG);

    /* inotify 설정 */
    ifd = inotify_init1(IN_NONBLOCK);
    if (ifd < 0) {
        fprintf(stderr, "[ERROR] inotify_init1: %s\n", strerror(errno));
        fclose(in);
		fclose(out);
        return 1;
    }

    uint32_t mask = IN_MODIFY | IN_MOVE_SELF | IN_ATTRIB;
    wd = inotify_add_watch(ifd, AUTH_LOG, mask);
    if (wd < 0) {
        fprintf(stderr, "[ERROR] add_watch: %s\n", strerror(errno));
        close(ifd); 
		fclose(in);
		fclose(out);
        return 1;
    }

    /* 버퍼 초기화 */
    struct pollfd pfd = { .fd = ifd, .events = POLLIN };
    char ibuf[INOT_BUF];

    while (1) {
        int pr = poll(&pfd, 1, 1000);
        if (pr < 0 && errno != EINTR) {
            fprintf(stderr, "[WARN] poll: %s\n", strerror(errno));
        }

        if (pfd.revents & POLLIN) {
            ssize_t r = read(ifd, ibuf, sizeof(ibuf));
            if (r > 0) {
                int need_reopen = 0;
                ssize_t off = 0;

                while (off < r) {
                    struct inotify_event *ev = (struct inotify_event *)(ibuf + off);

                    if (ev->mask & IN_MODIFY) {
                        char tmp[BUF_CHUNK];
                        size_t nread;

						/* 새로 추가된 부분 읽어오기 */
                        while ((nread = fread(tmp, 1, sizeof(tmp), in)) > 0) {
                            if (sz + nread > cap) {
                                size_t newcap = (cap == 0) ? 8192 : cap * 2;
                                while (newcap < sz + nread) newcap *= 2;
                                char *nb = realloc(buf, newcap);
                                if (!nb) {
                                    fprintf(stderr, "[ERROR] out of memory\n");
                                    goto cleanup;
                                }
                                buf = nb; 
								cap = newcap;
                            }
                            memcpy(buf + sz, tmp, nread);
                            sz += nread;
                            cur_pos += (long)nread;
                            drain_completed_lines(&buf, &sz, out);
                        }
                        clearerr(in);
                    }

                    if (ev->mask & IN_MOVE_SELF) 
						need_reopen = 1;

                    if (ev->mask & IN_ATTRIB && truncated(AUTH_LOG, cur_pos))
                        need_reopen = 1;

                    off += sizeof(struct inotify_event) + ev->len;
                }

                if (need_reopen) {
                    inotify_rm_watch(ifd, wd);
                    fclose(in);
                    usleep(200000);

					/* 새 파일 기준 재감시, EOF 위치 이동 */
                    wd = inotify_add_watch(ifd, AUTH_LOG, mask);
                    in = open_at_end(AUTH_LOG, &cur_pos);
                    sz = 0;
					
					if (wd < 0 || !in){
						fprintf(stderr, "[ERROR] reopen %s after rotate/truncate\n", AUTH_LOG);
						goto cleanup;
					}
                    fprintf(stderr, "[INFO] Reopened after rotation/truncate\n");
                }
            }
        }
    }

cleanup:
    if (buf) free(buf);
    if (wd >= 0) inotify_rm_watch(ifd, wd);
    if (ifd >= 0) close(ifd);
    if (in) fclose(in);
    if (out) fclose(out);
    return 0;
}

