#ifndef __LOGROTATE__H__
#define __LOGROTATE__H__

int parse_config_paths(char *pattern, char *dirPath, int dirLen, char *fileName, int fileLen);
int logrotate_file(char *pattern, int rotateCount, int minSize, int debug);

#endif

