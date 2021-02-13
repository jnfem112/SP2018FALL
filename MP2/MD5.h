#ifndef MD5_H
#define MD5_H

#define LENGTH_OF_MD5 16

uint32_t leftrotate(uint32_t target, uint32_t bits);
void str_to_md5(char *str, int len, char *dest);
void getFileMd5(const char *fileName, char *dest);
bool same_MD5(char MD5_1[LENGTH_OF_MD5] , char MD5_2[LENGTH_OF_MD5]);

#endif