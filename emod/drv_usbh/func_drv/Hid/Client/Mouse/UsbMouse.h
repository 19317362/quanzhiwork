/*
********************************************************************************
*                                USB Hid Driver
*
*                (c) Copyright 2006-2010, All winners Co,Ld.
*                        All Right Reserved
*
* FileName		:  usbMouse.h
*
* Author		:  Javen
*
* Date			:  2010.06.02
*
* Description	:  USB Mouse
*
* Others		:  NULL
*
* History:
*		<time> 		<version >		<author>	 	<desc>
*	   2010.06.02	   1.0			 Javen			build this file
*
********************************************************************************
*/
#ifndef  __USBMOUSE_H__
#define  __USBMOUSE_H__

#define  USB_HID_MOUSE_DATA_LEN		64    	/* ������ݵ���󳤶� 	*/
#define  USB_HID_MOUSE_DITHER_AREA	127  	/* ���ȥ������Χ 		*/

/* mouse report */
#define  USB_HID_MOUSE_REPORT_BUTTON_LEFT_BIT	0	/* ������ */
#define  USB_HID_MOUSE_REPORT_BUTTON_RIGHT_BIT	1	/* ����Ҽ� */
#define  USB_HID_MOUSE_REPORT_BUTTON_MIDDLE_BIT	2	/* ����м����м�������²����İ��� */
#define  USB_HID_MOUSE_REPORT_BUTTON_SIDE_BIT	3
#define  USB_HID_MOUSE_REPORT_BUTTON_EXTRA_BIT	4


/* �����ȡ�������ݵĶ��� */
typedef struct _usbMouseDataDef
{
    usbHidEvnetExcursion_t Button;	/* button 	*/
    usbHidEvnetExcursion_t X;		/* X 		*/
    usbHidEvnetExcursion_t Y;		/* Y 		*/
    usbHidEvnetExcursion_t Wheel;	/* Wheel 	*/
} usbMouseDataDef_t;

#define  USB_HID_DEV_MAGIC    0x5a13d099
typedef struct _usbMouse
{
    __u32 Magic;	             	/* ��ʾ�豸�Ƿ�Ϸ�					*/

    HidDev_t *HidDev;
    __u32 DevNo;					/* ���豸��							*/
    __u8  ClassName[32];			/* �豸����, ��"disk" 				*/
    __u8  DevName[32];				/* ���豸��, ��"SCSI_DISK_000"		*/

    /* Disk information */
    __u32 used;                     /* ���豸���� 					*/
    __dev_devop_t MouseOp;			/* �豸�������� 					*/

    /* Disk manager */
    USB_OS_HANDLE MouseParaHdle;	/* openʱ�ľ��						*/
    USB_OS_HANDLE MouseRegHdle;		/* regʱ�ľ�� 						*/

    __u8 Vendor[256];
    __u8 Product[256];
    __u8 Serial[32];

    __u8 MouseThdId;						/* ��������¼��̵߳�ID 			*/
    USB_OS_KERNEL_EVENT *MouseThreadSemi;
    USB_OS_KERNEL_EVENT *notify_complete;	/* ͬ��thread����/ɾ�� 				*/

    HidRequest_t HidReq;					/* hid�������� 						*/
    __u8 Data[USB_HID_MOUSE_DATA_LEN];		/* ������귵�ص����� 				*/
    usbMouseDataDef_t DataDef;				/* ������ݵĶ��� 					*/
    __u32 StopWork;							/* ���ֹͣ��������closeʱ���� 		*/

    /* test */
    __u32 USBMouseTestFlag;					/* ���Ա�־ */
    USBHMouseTest_t *USBHMouseTest;

    USBHMouseEvent_t MouseEvent;	  		/* ��ǰ����app������¼� */
    USBHMouse_CallBack CallBack;

    void *Extern;							/* ��չ�ṹ����MouseDriftControl_t */
} usbMouse_t;

#endif   //__USBMOUSE_H__

