#ifndef _SCAN_H_
#define _SCAN_H_

#define MAXTOKENLEN 40  // token的最大长度

extern char tokenString[MAXTOKENLEN+1];  // 每个token的属性

TokenType getToken(void);  // 返回源文件的下一个TOKEN

#endif
