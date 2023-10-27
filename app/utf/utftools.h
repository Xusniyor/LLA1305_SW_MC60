#ifndef __UTFTOOLS_H__
#define __UTFTOOLS_H__

unsigned char *StrToUprExt(unsigned char *pString);
unsigned char *StrToLwrExt(unsigned char *pString);
int StrnCiCmp(const char *s1, const char *s2, unsigned int ztCount);
int StrCiCmp(const char *s1, const char *s2);
char *StrCiStr(const char *s1, const char *s2);

unsigned char *StrToLwrUft8Latin1(unsigned char *pString);
unsigned char *StrToUprUft8Latin1(unsigned char *pString);
int StrnCiCmpLatin1(const char *s1, const char *s2, unsigned int ztCount);
int StrCiCmpLatin1(const char *s1, const char *s2);
char *StrCiStrLatin1(const char *s1, const char *s2);

#endif /* __UTFTOOLS_H__ */