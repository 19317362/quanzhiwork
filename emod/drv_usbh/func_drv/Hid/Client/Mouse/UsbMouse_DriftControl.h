/*
********************************************************************************
*                                USB Hid Driver
*
*                (c) Copyright 2006-2010, All winners Co,Ld.
*                        All Right Reserved
*
* FileName		:  UsbMouse_DriftControl.h
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
*	   2010.07.16	   1.0			 Javen			build this file
*
********************************************************************************
*/
#ifndef  __USBMOUSE_DRIFTCONTROL_H__
#define  __USBMOUSE_DRIFTCONTROL_H__

//--------------------------------------------------------
//
//--------------------------------------------------------
typedef struct _UsbMouseEventUnit
{
    __u32 vaild;					/* �Ƿ���Ч */
    USBHMouseEvent_t MouseEvent;	/* ����¼� */
} UsbMouseEventUnit_t;

typedef struct _UsbMouseDriftControl
{
    usbMouse_t *usbMouse;

    USB_OS_KERNEL_SOFT_TIMER *TimerHdle;	/* timer ��� 			*/
    USB_OS_KERNEL_EVENT *ThreadSemi;		/* �߳��ź��� 			*/
    USB_OS_KERNEL_EVENT *notify_complete;	/* �߳�ͬ���ź��� 		*/
    __u8 ThreadId;							/* �߳�ID 				*/

    __u32 ButtonDown;						/* �������� 			*/

    UsbMouseEventUnit_t PreMouseEvent;		/* ��һ�ε�����¼� 	*/
    UsbMouseEventUnit_t DubiousMouseEvent; 	/* ���Ե�����¼� 		*/
    UsbMouseEventUnit_t CurrentMouseEvent; 	/* ��ǰ������¼� 		*/
    __u32 DubiousCoordinate;				/* ��������, bit1:X, bit2:Y, bit3: X & Y */
    __u32 WaitEvent;						/* �ȴ���һ�ε��¼� 	*/
} UsbMouseDriftControl_t;


//--------------------------------------------------------
//
//--------------------------------------------------------
void UsbMouse_AddToDriftArray(usbMouse_t *usbMouse, USBHMouseEvent_t *Event);
__s32 UsbMouse_DriftControl_Init(usbMouse_t *usbMouse);
__s32 UsbMouse_DriftControl_Exit(usbMouse_t *usbMouse);

#endif   //__USBMOUSE_DRIFTCONTROL_H__

