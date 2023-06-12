
/*****************************************************************************
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of Quectel Co., Ltd. 2013
 *
 *****************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   main.c
 *
 * Project:
 * --------
 *   OpenCPU
 *
 * Description:
 * ------------
 *   This app demonstrates how to send AT command with RIL API, and
 *transparently transfer the response through MAIN UART. And how to use UART
 *port. Developer can program the application based on this example.
 *
 ****************************************************************************/
#ifdef __CUSTOMER_CODE__

/***********************************************************************
 * INCLUDES
 ************************************************************************/
#include "main.h"
#include "lwgps.h"
#include "utf.h"
#include <stdint.h>
/***********************************************************************
 * GLOBAL DATA DEFINITIONS
 ************************************************************************/
// char *__buildDate = __DATE__;
// char *__buildTime = __TIME__;
// char *__device = "LLA1305";
// char *__version = "V1.0.0";

/***********************************************************************
 * MACRO CONSTANT DEFINITIONS
 ************************************************************************/

// const char *Language[3][3] =
// {
//     {"en",             "o`z",              "ру"},
//     {"language",       "til",              "язык"},
//     {"port",           "port",             "порт"},
//     {"on",             "yoq",              "вкл"},
//     {"off",            "o'chir",           "выкл"}
// };

/***********************************************************************
 * STRUCT TYPE DEFINITIONS
 ************************************************************************/
lwgps_t hgps;

/***********************************************************************
 * FUNCTION DECLARATIONS
 ************************************************************************/
static void HAL_NMEA_CallBack(u8 *nmea_buff, u16 len, void *customizePara);
static void HAL_UART_CallBack(Enum_SerialPort port, Enum_UARTEventType msg, bool level, void *customizedPara);
static void Send_SMS_Location(char *strPhNum, ST_Time *PTime, lwgps_t *pLocInfo);
static void New_SMS_Arrived(u32 nIndex);

/***********************************************************************
 * MACRO FUNCTION DEFINITIONS
 ************************************************************************/
void proc_subtask1(s32 taskId)
{
    u16 data[4];
    /* IIC */
    Ql_IIC_Init(0, PINNAME_RI, PINNAME_DCD, 1);
    Ql_IIC_Config(0, TRUE, 0x10, 100);
    data[0] = 2;
    data[1] = 50;
    data[2] = 1000;
    data[3] = 0x00001;
    Ql_IIC_Write(0, 0x10, (u8 *)data, 8);
    data[0] = 0xFFFF;
    data[1] = 0xFFFF;
    data[2] = 0xFFFF;
    data[3] = 0x00002;
    Ql_IIC_Write(0, 0x10, (u8 *)data, 8);
    while (TRUE)
    {
        data[0] = 0xFFFF;
        data[1] = 0xFFFF;
        data[2] = 0xFFFF;
        data[3] = 0x00004;
        Ql_IIC_Write(0, 0x10, (u8 *)data, 8);
        Ql_Sleep(500);
        data[0] = 0x0000;
        data[1] = 0x0000;
        data[2] = 0x0000;
        data[3] = 0x00004;
        Ql_IIC_Write(0, 0x10, (u8 *)data, 8);
        Ql_Sleep(500);
    }
}

void proc_main_task(s32 taskId)
{
    ST_MSG msg;
    /* UART_PORT1 is MAIN UART */
    Ql_UART_Register(UART_PORT1, HAL_UART_CallBack, NULL);
    Ql_UART_Open(UART_PORT1, 115200, FC_NONE);
    /* GNSS */
    lwgps_init(&hgps);
    Ql_GNSS_PowerOn(ALL_NMEA_EN, HAL_NMEA_CallBack, NULL);
    /* loop */
    while (TRUE)
    {
        Ql_OS_GetMessage(&msg);
        switch (msg.message)
        {
        case MSG_ID_RIL_READY:
            Ql_RIL_Initialize();
            break;
        case MSG_ID_URC_INDICATION:
            switch (msg.param1)
            {
            case URC_SYS_INIT_STATE_IND:
                if (SYS_STATE_SMSOK == msg.param2)
                {
                    RIL_SMS_DeleteSMS(0, RIL_SMS_DEL_ALL_MSG);
                }
                break;
            case URC_NEW_SMS_IND:
                New_SMS_Arrived(msg.param2);
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }
}

static void HAL_NMEA_CallBack(u8 *nmea_buff, u16 len, void *customizePara)
{
    lwgps_process(&hgps, nmea_buff, len);
}

static void New_SMS_Arrived(u32 nIndex)
{
    ST_RIL_SMS_TextInfo pTextInfo;
    ST_Time TimeLocal;
    Ql_memset(&pTextInfo, 0x00, sizeof(ST_RIL_SMS_TextInfo));
    Ql_memset(&TimeLocal, 0x00, sizeof(ST_Time));
    RIL_SMS_ReadSMS_Text(nIndex, LIB_SMS_CHARSET_GSM, &pTextInfo);
    Ql_GetLocalTime(&TimeLocal);

    Send_SMS_Location("+998992372727", &TimeLocal, &hgps);
    RIL_SMS_DeleteSMS(nIndex, RIL_SMS_DEL_INDEXED_MSG);
}

static void HAL_UART_CallBack(Enum_SerialPort port, Enum_UARTEventType msg, bool level, void *customizedPara)
{
}

void ConvertUTF8toUCS2(
    const char *a_pInputData,
    unsigned int a_uInputDataLen,
    char *a_pOutputData,
    unsigned int a_uOutputDataSize)
{
    static const char *__hex = "0123456789ABCDEF";
    unsigned int len;
    unsigned int index;
    const UTF8 *pUtf8 = (const UTF8 *)a_pInputData;
    UTF16 *pUtf16 = (UTF16 *)Ql_MEM_Alloc(a_uOutputDataSize / 4);
    UTF16 *rUtf16 = pUtf16;
    ConvertUTF8toUTF16(&pUtf8, pUtf8 + a_uInputDataLen, &pUtf16, pUtf16 + (a_uOutputDataSize / 4), lenientConversion);
    len = pUtf16 - rUtf16 - 1;
    index = 0;
    Ql_memset(a_pOutputData, 0x00, a_uOutputDataSize);
    for (unsigned int i = 0; i < len; i++)
    {
        a_pOutputData[index] = __hex[(rUtf16[i] >> 12) & 0xF];
        index++;
        a_pOutputData[index] = __hex[(rUtf16[i] >> 8) & 0xF];
        index++;
        a_pOutputData[index] = __hex[(rUtf16[i] >> 4) & 0xF];
        index++;
        a_pOutputData[index] = __hex[rUtf16[i] & 0xF];
        index++;
    }
    Ql_MEM_Free(rUtf16);
}

static void Send_SMS_Location(char *strPhNum, ST_Time *pTime, lwgps_t *pLocInfo)
{
    ST_RIL_SMS_SendExt sExt;
    char sTextBuf[100];
    char sMsgUCS2[512];

    Ql_memset(&sExt, 0x00, sizeof(sExt));
    Ql_memset(sTextBuf, 0x00, sizeof(sTextBuf));

    sExt.conPres = TRUE;
    sExt.con.msgType = 0xFF;
    sExt.con.msgRef = 52;
    sExt.con.msgTot = 2;

    sExt.con.msgSeg = 1;
    Ql_sprintf(sTextBuf, "📅 День: %02d.%02d.%04d\n⏰ Время: %02d:%02d\n🗺 Я здесь:\n\0\0", pTime->day, pTime->month, pTime->year, pTime->hour, pTime->minute);
    ConvertUTF8toUCS2(sTextBuf, Ql_strlen(sTextBuf) + 1, sMsgUCS2, sizeof(sMsgUCS2));
    RIL_SMS_SendSMS_Text_Ext(strPhNum, Ql_strlen(strPhNum), LIB_SMS_CHARSET_UCS2, (u8 *)sMsgUCS2, Ql_strlen(sMsgUCS2), NULL, &sExt);

    sExt.con.msgSeg = 2;
    Ql_sprintf(sTextBuf, "www.google.com/maps/place/%.6f,%.6f\0\0", pLocInfo->latitude, pLocInfo->longitude);
    ConvertUTF8toUCS2(sTextBuf, Ql_strlen(sTextBuf) + 1, sMsgUCS2, sizeof(sMsgUCS2));
    RIL_SMS_SendSMS_Text_Ext(strPhNum, Ql_strlen(strPhNum), LIB_SMS_CHARSET_UCS2, (u8 *)sMsgUCS2, Ql_strlen(sMsgUCS2), NULL, &sExt);
}

#endif // __CUSTOMER_CODE__
