#ifndef LOGIP_H
#define LOGIP_H
#include<stdbool.h>

int isIp(const char *str);
int loadIpset(char ipsetIps[][50], int max);
int extractIp(const char *filename);
unsigned long longIp(int ip[4]);
int checkCountry(const char *targetIp);
int range(int t[4],int s[4],int e[4]);
#endif
