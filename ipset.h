#ifndef IPSET_H
#define IPSET_H
#include <stdbool.h>

int registerIp(const char *filename);
int addIp(const char *ip);
char *trim(char *str);
#endif
