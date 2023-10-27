
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

/***********************************************************************
 * GLOBAL DATA DEFINITIONS
 ************************************************************************/
#define APN "internet.uzmobile.uz"
#define USERID ""
#define PASSWD ""
// const char *__buildDate = __DATE__;
// const char *__buildTime = __TIME__;
// const char *__device = "LLA1305";
// const char *__version = "V1.0.0";

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

const char *pTestInData = "Man Sani Sevaman, Я тебя люблю, I love you, Мен сені жақсы көремін, Мен сени сүйөм, Ман туро дӯст медорам, Mən səni sevirəm, მიყვარხარ, Ես սիրում եմ քեզ, Seni seviyorum🥰😘 - Man Sani Sevaman, Я тебя люблю, I love you, Мен сені жақсы көремін, Мен сени сүйөм, Ман туро дӯст медорам, Mən səni sevirəm, მიყვარხარ, Ես սիրում եմ քեզ, Seni seviyorum🥰😘";
u16 PortStatus;

/***********************************************************************
 * STRUCT TYPE DEFINITIONS
 ************************************************************************/
lwgps_t hgps;

/***********************************************************************
 * FUNCTION DECLARATIONS
 ************************************************************************/
static void LL_NMEA_CallBack(u8 *nmea_buff, u16 len, void *customizePara);
static void LL_UART_CallBack(Enum_SerialPort port, Enum_UARTEventType msg, bool level, void *customizedPara);
s32 New_SMS_Arrived(const char *pNum, const char *pMsg);

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
    PortStatus = 0;
    while (TRUE)
    {
        data[0] = PortStatus;
        data[1] = PortStatus;
        data[2] = PortStatus;
        data[3] = 0x00004;
        Ql_IIC_Write(0, 0x10, (u8 *)data, 8);
        Ql_Sleep(500);
    }
}

void proc_main_task(s32 taskId)
{
    ST_MSG msg;
    /* UART_PORT1 is MAIN UART */
    Ql_UART_Register(UART_PORT1, LL_UART_CallBack, NULL);
    Ql_UART_Open(UART_PORT1, 115200, FC_NONE);
    /* GNSS */
    lwgps_init(&hgps);
    Ql_GNSS_PowerOn(ALL_NMEA_EN, LL_NMEA_CallBack, NULL);
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
            {
                if (SYS_STATE_SMSOK == msg.param2)
                {
                    RIL_SMS_DeleteSMS(0, RIL_SMS_DEL_ALL_MSG);
                }
                break;
            }
            case URC_NEW_SMS_IND:
            {
                ReadUtf16SMS(msg.param2, New_SMS_Arrived);
                RIL_SMS_DeleteSMS(msg.param2, RIL_SMS_DEL_INDEXED_MSG);
                break;
            }
            default:
                break;
            }
            break;
        default:
            break;
        }
    }
}

static void LL_NMEA_CallBack(u8 *nmea_buff, u16 len, void *customizePara)
{
    lwgps_process(&hgps, nmea_buff, len);
}

s32 New_SMS_Arrived(const char *pNum, const char *pMsg)
{
    APP_DEBUG("%s\n\r", pNum);

    APP_DEBUG("%s\n\r", pMsg);
    StrToLwrExt((unsigned char *)pMsg);
    APP_DEBUG("%s\n\r", pMsg);

    if (Ql_strstr(pMsg, "порт вкл"))
    {
        PortStatus = 0xFFFF;
    }
    else if (Ql_strstr(pMsg, "порт выкл"))
    {
        PortStatus = 0x0000;
    }

    return 0;
}

static void LL_UART_CallBack(Enum_SerialPort port, Enum_UARTEventType msg, bool level, void *customizedPara)
{
    switch (msg)
    {

    case EVENT_UART_READY_TO_READ:
    {
        break;
    }

    case EVENT_UART_READY_TO_WRITE:
    {
        break;
    }

    default:
        break;
    }
}

#endif // __CUSTOMER_CODE__
