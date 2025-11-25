#ifndef LOGIP_H
#define LOGIP_H
#include<stdbool.h>
char *trim(const char *str);
int isIp(const char *str);
int loadIpset(char ipsetIps[][50], int max);
int extractIp(const char *filename);
int checkCountry(const char *targetIp);

#endif
