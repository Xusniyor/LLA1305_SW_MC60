#ifndef __UTIL_H__
#define __UTIL_H__

#include "ql_type.h"

#define TEMPLATE_HEX (const char *)"0123456789ABCDEF"

typedef struct
{
    u32 SingleBytePoints;
    u32 DoubleBytePoints;
    u32 TripleBytePoints;
    u32 QuadrupleBytePoints;
    u32 AllPoints;
} PointsUtf8;

s32 ReadUtf16SMS(u32 nIndex, s32 f(const char *, const char *));
s32 SendUtf8SMS(const char *pNumber, const char *pMsg);

#endif /* __UTIL_H__ */