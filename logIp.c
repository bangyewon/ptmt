#include <stdio.h>
#include <unistd.h>

#define MAX_LINE
//whiteList-> ipset의 ip와 ip-lines.log ip비교
// 국가별ip와 ip-lines.log비교후 차단
char *trim(char *str) {
        char *end;

        while(isspace(*str)) {
                str++;
        }
        if (*str) {
                end = str + strlen(str) - 1;
                while(end > str && isspace(*end)) {

                        end--;
                }
                *(end + 1) = '\0';
        }
        return str;
}

