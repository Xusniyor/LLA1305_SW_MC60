#include "util.h"
#include "utf.h"
#include "ql_stdlib.h"
#include "ql_memory.h"
#include "ril_sms.h"
#include "ril.h"
#include <math.h>

#define MAX_UCS2_SMS_SIZE 268

void ConvertByteToHexString(u8 *pInputByte, u8 *pOutputStr, u32 OutputStrSize)
{
    const char *__TemplateHex = TEMPLATE_HEX;
    for (u32 i = 0; i < OutputStrSize; i++)
        pOutputStr[i] = __TemplateHex[(pInputByte[i / 2] >> (4 * (1 - (i % 2)))) & 0x0F];
}

void ConvertWordToHexString(u16 *pInputWord, u8 *pOutputStr, u32 OutputStrSize)
{
    const char *__TemplateHex = TEMPLATE_HEX;
    for (u32 i = 0; i < OutputStrSize; i++)
        pOutputStr[i] = __TemplateHex[(pInputWord[i / 4] >> (4 * (3 - (i % 4)))) & 0x0F];
}

void ConvertDoubleWordToHexString(u32 *pInputDoubleWord, u8 *pOutputStr, u32 OutputStrSize)
{
    const char *__TemplateHex = TEMPLATE_HEX;
    for (u32 i = 0; i < OutputStrSize; i++)
        pOutputStr[i] = __TemplateHex[(pInputDoubleWord[i / 8] >> (4 * (7 - (i % 8)))) & 0x0F];
}

u32 ConvertHexStringToByte(u8 *pInputStr, u8 *pOutputByte, u32 OutputByteSize)
{
    u32 i, j, k;
    for (i = 0; i < OutputByteSize; i++)
    {
        pOutputByte[i] = 0;
        k = (i * 2) + 2;
        for (j = i * 2; j < k; j++)
        {
            pOutputByte[i] <<= 4;

            if ((pInputStr[j] >= '0') && (pInputStr[j] <= '9'))
                pOutputByte[i] |= pInputStr[j] - '0';

            else if ((pInputStr[j] >= 'A') && (pInputStr[j] <= 'F'))
                pOutputByte[i] |= pInputStr[j] + 10 - 'A';

            else if ((pInputStr[j] >= 'a') && (pInputStr[j] <= 'f'))
                pOutputByte[i] |= pInputStr[j] + 10 - 'a';

            else
                return i + 1;
        }
    }
    return OutputByteSize;
}

u32 ConvertHexStringToWord(u8 *pInputStr, u16 *pOutputWord, u32 OutputWordSize)
{
    u32 i, j, k;
    for (i = 0; i < OutputWordSize; i++)
    {
        pOutputWord[i] = 0;
        k = (i * 4) + 4;
        for (j = i * 4; j < k; j++)
        {
            pOutputWord[i] <<= 4;

            if ((pInputStr[j] >= '0') && (pInputStr[j] <= '9'))
                pOutputWord[i] |= pInputStr[j] - '0';

            else if ((pInputStr[j] >= 'A') && (pInputStr[j] <= 'F'))
                pOutputWord[i] |= pInputStr[j] + 10 - 'A';

            else if ((pInputStr[j] >= 'a') && (pInputStr[j] <= 'f'))
                pOutputWord[i] |= pInputStr[j] + 10 - 'a';

            else
                return i + 1;
        }
    }
    return OutputWordSize;
}

u32 ConvertHexStringToDoubleWord(u8 *pInputStr, u32 *pOutputDoubleWord, u32 OutputDoubleWordSize)
{
    u32 i, j, k;
    for (i = 0; i < OutputDoubleWordSize; i++)
    {
        pOutputDoubleWord[i] = 0;
        k = (i * 8) + 8;
        for (j = i * 8; j < k; j++)
        {
            pOutputDoubleWord[i] <<= 4;

            if ((pInputStr[j] >= '0') && (pInputStr[j] <= '9'))
                pOutputDoubleWord[i] |= pInputStr[j] - '0';

            else if ((pInputStr[j] >= 'A') && (pInputStr[j] <= 'F'))
                pOutputDoubleWord[i] |= pInputStr[j] + 10 - 'A';

            else if ((pInputStr[j] >= 'a') && (pInputStr[j] <= 'f'))
                pOutputDoubleWord[i] |= pInputStr[j] + 10 - 'a';

            else
                return i + 1;
        }
    }
    return OutputDoubleWordSize;
}

PointsUtf8 CountUtf8Points(const char *pInputUTF8)
{
    PointsUtf8 Result;
    Ql_memset(&Result, 0, sizeof(Result));
    u16 InputUTF8DataLen = Ql_strlen(pInputUTF8);
    u8 Skip = 0;
    for (u16 i = 0; i < InputUTF8DataLen; i++)
    {
        if (~pInputUTF8[i] & 0x80)
        {
            if (Skip)
                break;
            Result.SingleBytePoints++;
        }
        else if ((pInputUTF8[i] >> 5) == 0x06)
        {
            if (Skip)
                break;
            Skip = 1;
            Result.DoubleBytePoints++;
        }
        else if ((pInputUTF8[i] >> 4) == 0x0E)
        {
            if (Skip)
                break;
            Skip = 2;
            Result.TripleBytePoints++;
        }
        else if ((pInputUTF8[i] >> 3) == 0x1E)
        {
            if (Skip)
                break;
            Skip = 3;
            Result.QuadrupleBytePoints++;
        }
        else if ((pInputUTF8[i] >> 6) == 0x02)
        {
            if (!Skip)
                break;
            Skip--;
        }
        else
        {
            break;
        }
    }
    Result.AllPoints = Result.SingleBytePoints + Result.DoubleBytePoints + Result.TripleBytePoints + Result.QuadrupleBytePoints;
    return Result;
}

u32 RequiredMemorySizeForUtf8FromUtf16(const u16 *pInputUTF16)
{
    u32 Result = 0;
    u32 ch;
    for (u32 i = 0; pInputUTF16[i] != 0x0000; i++)
    {
        ch = pInputUTF16[i];
        if (pInputUTF16[i] >= 0xD800 && pInputUTF16[i] <= 0xDBFF)
        {
            i++;
            if (pInputUTF16[i] >= 0xDC00 && pInputUTF16[i] <= 0xDFFF)
            {
                ch = ((ch - 0xD800) << 10) + (pInputUTF16[i] - 0xDC00) + 0x0010000UL;
            }
        }
        if (ch < 0x80)
        {
            Result += 1;
        }
        else if (ch < 0x800)
        {
            Result += 2;
        }
        else if (ch < 0x10000)
        {
            Result += 3;
        }
        else if (ch < 0x110000)
        {
            Result += 4;
        }
        else
        {
            Result += 3;
        }
    }
    return Result;
}

s32 ReadUtf16SMS(u32 nIndex, s32 f(const char *, const char *))
{
    s32 ret;
    ConversionResult ConvertResult;
    u32 RequiredMemorySize;
    UTF16 *pUtf16;
    UTF8 *pUtf8;
    UTF8 *pUtf8Copy;

    ST_RIL_SMS_TextInfo *pTextInfo = (ST_RIL_SMS_TextInfo *)Ql_MEM_Alloc(sizeof(ST_RIL_SMS_TextInfo));
    Ql_memset(pTextInfo, 0x00, sizeof(ST_RIL_SMS_TextInfo));
    RIL_SMS_ReadSMS_Text(nIndex, LIB_SMS_CHARSET_GSM, pTextInfo);

    if (pTextInfo->param.deliverParam.alpha == LIB_SMS_DCS_ALPHA_DEFAULT)
    {
        ret = f((const char *)pTextInfo->param.deliverParam.oa, (const char *)pTextInfo->param.deliverParam.data);
    }
    else if (pTextInfo->param.deliverParam.alpha == LIB_SMS_DCS_ALPHA_UCS2)
    {
        pUtf16 = (UTF16 *)Ql_MEM_Alloc((pTextInfo->param.deliverParam.length / 2) + 2);

        Ql_memset(pUtf16, 0x00, (pTextInfo->param.deliverParam.length / 2) + 2);
        ConvertHexStringToWord(pTextInfo->param.deliverParam.data, (u16 *)pUtf16, pTextInfo->param.deliverParam.length / 2);
        RequiredMemorySize = RequiredMemorySizeForUtf8FromUtf16((const u16 *)pUtf16);

        pUtf8 = (UTF8 *)Ql_MEM_Alloc(RequiredMemorySize + 1);

        pUtf8Copy = pUtf8;
        const UTF16 *pUtf16Copy = pUtf16;
        Ql_memset(pUtf8, 0x00, RequiredMemorySize + 1);
        ConvertResult = ConvertUTF16toUTF8(&pUtf16Copy, pUtf16Copy + (pTextInfo->param.deliverParam.length / 4), &pUtf8Copy, pUtf8Copy + RequiredMemorySize, lenientConversion);

        if (ConvertResult == conversionOK)
            ret = f((const char *)pTextInfo->param.deliverParam.oa, (const char *)pUtf8);
        else
            ret = f((const char *)pTextInfo->param.deliverParam.oa, (const char *)"\n*** ERROR READ UTF-16 SMS ***");

        Ql_MEM_Free(pUtf16);
        Ql_MEM_Free(pUtf8);
    }
    else
    {
        ret = f((const char *)pTextInfo->param.deliverParam.oa, (const char *)"\n*** ERROR READ UTF-16 SMS ***");
    }

    Ql_MEM_Free(pTextInfo);

    return ret;
}

s32 SendUtf8SMS(const char *pNumber, const char *pMsg)
{
    ST_RIL_SMS_SendExt sExt;
    ConversionResult ConvertResult;
    s32 Result;

    PointsUtf8 pMsgPoints = CountUtf8Points(pMsg);
    u32 pMsgUCS2length = (((pMsgPoints.SingleBytePoints + pMsgPoints.DoubleBytePoints + pMsgPoints.TripleBytePoints) * 2) + (pMsgPoints.QuadrupleBytePoints * 4)) * 2;
    u8 TotalSMSPackages = ceil((double)pMsgUCS2length / MAX_UCS2_SMS_SIZE);
    u8 MsgSeg = 0;

    u8 *pMemUTF16Msg = (u8 *)Ql_MEM_Alloc(MAX_UCS2_SMS_SIZE / 2);
    u8 *pMemUCS2Msg = (u8 *)Ql_MEM_Alloc(MAX_UCS2_SMS_SIZE + 1);

    const UTF8 *pUtf8 = (const UTF8 *)pMsg;
    UTF16 *pUtf16;

    sExt.conPres = TRUE;
    sExt.con.msgType = 0xFF;
    sExt.con.msgRef = 52;
    sExt.con.msgTot = TotalSMSPackages;

    do
    {
        pUtf16 = (UTF16 *)pMemUTF16Msg;

        Ql_memset(pMemUCS2Msg, 0, MAX_UCS2_SMS_SIZE + 1);
        ConvertResult = ConvertUTF8toUTF16(&pUtf8, pUtf8 + Ql_strlen((const char *)pUtf8), &pUtf16, (UTF16 *)(pMemUTF16Msg + (MAX_UCS2_SMS_SIZE / 2)), lenientConversion);

        if ((ConvertResult == sourceExhausted) || (ConvertResult == sourceIllegal))
        {
            while (MsgSeg < TotalSMSPackages)
            {
                sExt.con.msgSeg = ++MsgSeg;
                RIL_SMS_SendSMS_Text_Ext((char *)pNumber, Ql_strlen(pNumber), LIB_SMS_CHARSET_UCS2, (u8 *)"\n*** ERROR SEND UTF-8 SMS ***", 29, NULL, &sExt);
            }
            Ql_MEM_Free(pMemUTF16Msg);
            Ql_MEM_Free(pMemUCS2Msg);
            return RIL_AT_FAILED;
        }

        ConvertWordToHexString((u16 *)pMemUTF16Msg, pMemUCS2Msg, ((u8 *)pUtf16 - pMemUTF16Msg) * 2);
        sExt.con.msgSeg = ++MsgSeg;
        Result = RIL_SMS_SendSMS_Text_Ext((char *)pNumber, Ql_strlen(pNumber), LIB_SMS_CHARSET_UCS2, pMemUCS2Msg, Ql_strlen((const char *)pMemUCS2Msg), NULL, &sExt);

    } while (ConvertResult == targetExhausted);

    Ql_MEM_Free(pMemUTF16Msg);
    Ql_MEM_Free(pMemUCS2Msg);

    return Result;
}
