/*
********************************************************************************
*                                USB Hid Driver
*
*                (c) Copyright 2006-2010, All winners Co,Ld.
*                        All Right Reserved
*
* FileName		:  UsbMouse_DriftControl.c
*
* Author		:  Javen
*
* Date			:  2010.06.02
*
* Description	:  USB Mouse ȥ�����㷨.
*     1����������¼���Ϣ��ӵ�������
*     2��ÿ��5ms��������е����¼�����Ϣ��
*     3��
*
* Others		:  NULL
*
* History:
*		<time> 		<version >		<author>	 	<desc>
*	   2010.07.16	   1.0			 Javen			build this file
*
********************************************************************************
*/

#include  "usb_host_config.h"
#include  "usb_host_base_types.h"
#include  "usb_os_platform.h"
#include  "error.h"

#include  "HidSpec.h"
#include  "Hid_i.h"
#include  "HidFunDrv.h"

#include  "usbMouse.h"
#include  "UsbMouse_DriftControl.h"

/*
*******************************************************************************
*                     UsbMouse_AddToDriftArray
*
* Description:
*
*
* Parameters:
*
*
* Return value:
*
*
* note:
*
*
*******************************************************************************
*/
static void UsbMouse_DriftTimeOut(void *parg)
{
    UsbMouseDriftControl_t *Drift = (UsbMouseDriftControl_t *)parg;
    __cpu_sr cup_sr	= 0;

    if(Drift == NULL)
    {
        DMSG_PANIC("ERR: input error\n");
        return ;
    }

    /* ����п��ɵĵ���ڣ��Ͱѿ��ɵĵ㷢��ȥ */
    if(Drift->WaitEvent && Drift->DubiousMouseEvent.vaild)
    {
        USB_OS_ENTER_CRITICAL(cup_sr);
        DMSG_MOUSE_TEST("TimeOut: DubiousCoordinate = %x\n", Drift->DubiousCoordinate);
        USB_OS_MEMCPY(&Drift->CurrentMouseEvent, &Drift->DubiousMouseEvent, sizeof(UsbMouseEventUnit_t));
        Drift->CurrentMouseEvent.vaild = 1;
        USB_OS_EXIT_CRITICAL(cup_sr);

        UsbThreadWakeUp(Drift->ThreadSemi);
    }

    return;
}

/*
*******************************************************************************
*                     UsbMouse_IsButtonEvent
*
* Description:
*
*
* Parameters:
*
*
* Return value:
*
*
* note:
*
*
*******************************************************************************
*/
static __u32 UsbMouse_IsButtonEvent(USBHMouseEvent_t *MouseEvent)
{
    return (MouseEvent->Button.LeftButton
            || MouseEvent->Button.RightButton
            || MouseEvent->Button.MiddleButton
            || MouseEvent->Wheel);
}

/*
*******************************************************************************
*                     UsbMouse_IsDubiousEvent
*
* Description:
*
*
* Parameters:
*
*
* Return value:
*
*
* note:
*
*
*******************************************************************************
*/
static __u32 UsbMouse_IsDubiousEvent(USBHMouseEvent_t *Event, UsbMouseDriftControl_t *Drift)
{
    __u32 Dubious = 0;

    /* ����ο���PreMouseEvent������, ��ô�Ͳ��û��ɱ��εĵ� */
    if(Drift->PreMouseEvent.vaild == 0)
    {
        return 0;
    }

    /* ���ǰ������ X �����ֵ����127, ��ô����Ϊ���ε�����Ϊ���ɵ����� */
    if(absolute(Event->X - Drift->PreMouseEvent.MouseEvent.X) > USB_HID_MOUSE_DITHER_AREA)
    {
        set_bit(Drift->DubiousCoordinate, 1);
        Dubious = 1;
    }

    /* ���ǰ������ Y �����ֵ����127, ��ô����Ϊ���ε�����Ϊ���ɵ����� */
    if(absolute(Event->Y - Drift->PreMouseEvent.MouseEvent.Y) > USB_HID_MOUSE_DITHER_AREA)
    {
        set_bit(Drift->DubiousCoordinate, 2);
        Dubious = 1;
    }

    if(Dubious)
    {
        DMSG_MOUSE_TEST("DubiousCoordinate = %x\n", Drift->DubiousCoordinate);
    }

    return Dubious;
}

/* �ж�X��Y�Ƿ������������Ƿ��Ǹ��� */
static __u32 UsbMouse_IsAccord8(__s8 x, __s8 y)
{
    /* �Ƿ��Ǹ��� */
    if(x <= 0 && y <= 0)
    {
        return 1;
    }

    /* �Ƿ������� */
    if(x >= 0 && y >= 0)
    {
        return 1;
    }

    return 0;
}

/*
*******************************************************************************
*                     UsbMouse_AddToDriftArray
*
* Description:
*     ��3������������ҳ��켣��ͬ������, Ȼ��ȡƽ��ֵ��
*
* Parameters:
*
*
* Return value:
*
*
* note:
*
*******************************************************************************
*/
static __s32 UsbMouse_AdjustCoordinate(USBHMouseEvent_t *Event1,
                                       USBHMouseEvent_t *Event2,
                                       USBHMouseEvent_t *Event3,
                                       USBHMouseEvent_t *OutEvent)
{
    /* Ѱ�� X �����Ϸ���һ�µĵ� */
    if(UsbMouse_IsAccord8(Event1->X, Event2->X)
            && UsbMouse_IsAccord8(Event1->X, Event3->X))  /* 1,2,3��һ�� */
    {
        OutEvent->X = (Event1->X / 3) + (Event2->X / 3) + (Event3->X / 3);
    }
    else if(UsbMouse_IsAccord8(Event1->X, Event2->X))    /* 1,2����һ�µ� */
    {
        OutEvent->X = Event2->X;
    }
    else if(UsbMouse_IsAccord8(Event1->X, Event3->X))    /* 1,3����һ�µ� */
    {
        OutEvent->X = Event3->X;
    }
    else if(UsbMouse_IsAccord8(Event2->X, Event3->X))    /* 2,3����һ�µ� */
    {
        OutEvent->X = (Event2->X / 2) + (Event3->X / 2);
    }
    else    /* 1,2,3������һ�µ� */
    {
        OutEvent->X = (Event1->X / 3) + (Event2->X / 3) + (Event3->X / 3);
    }

    /* Ѱ�� Y �����Ϸ���һ�µĵ� */
    if(UsbMouse_IsAccord8(Event1->Y, Event2->Y)
            && UsbMouse_IsAccord8(Event1->Y, Event3->Y))  /* 1,2,3��һ�� */
    {
        OutEvent->Y = (Event1->Y / 3) + (Event2->Y / 3) + (Event3->Y / 3);
    }
    else if(UsbMouse_IsAccord8(Event1->Y, Event2->Y))    /* 1,2����һ�µ� */
    {
        OutEvent->Y = Event2->Y;
    }
    else if(UsbMouse_IsAccord8(Event1->Y, Event3->Y))    /* 1,3����һ�µ� */
    {
        OutEvent->Y = Event3->Y;
    }
    else if(UsbMouse_IsAccord8(Event2->Y, Event3->Y))    /* 2,3����һ�µ� */
    {
        OutEvent->Y = (Event2->Y / 2) + (Event3->Y / 2);
    }
    else    /* 1,2,3������һ�µ� */
    {
        OutEvent->Y = (Event1->Y / 3) + (Event2->Y / 3) + (Event3->Y / 3);
    }

    return 0;
}

/*
*******************************************************************************
*                     UsbMouse_AddToDriftArray
*
* Description:
*
*
* Parameters:
*
*
* Return value:
*
*
* note:
*
*	1��ֻ�а�����Ϣ��ֱ�ӷ��͸�app��
*
*	2��ֻ��wheel��Ϣ��ֱ�ӷ��͸�app��
*
*	3��ֻ�����꣬��Ԥ�����Ĺ켣��
*      ������εĵ�A����һ�εĵ�B���̫���ڹ涨ʱ����ȡ��һ�εĵ�C���ο���
*      ���A��C������Ͷ���B�㣬����A���C���app������涨ʱ����û������¼�,�Ͱ�A���B�������app��
*
*	4�����������wheel�����������һ��������������wheel�¼���
*      �ѵ�ǰ���еĵ�ȫ������app�����Ұ���һ�εİ���̧����Ϣ��Ҳ��ʱ�ķ��ͳ�ȥ��
*
*******************************************************************************
*/void UsbMouse_AddToDriftArray(usbMouse_t *usbMouse, USBHMouseEvent_t *Event)
{
    UsbMouseDriftControl_t *Drift = NULL;
    __cpu_sr cup_sr	= 0;

    if(usbMouse == NULL)
    {
        DMSG_PANIC("ERR: input error\n");
        return ;
    }

    Drift = usbMouse->Extern;
    if(Drift == NULL)
    {
        DMSG_PANIC("ERR: Drift == NULL\n");
        return ;
    }

    /* ��ǰ�а�������, ��������̧���, �͸��ѱ�����Ϣ���͸�APP */
    if(Drift->ButtonDown)
    {
        /* ������λ��а�����Ϣ, �ͼ�¼���� */
        if(UsbMouse_IsButtonEvent(Event))
        {
            DMSG_MOUSE_TEST("Had send a button down event, then a new button event come\n");
            Drift->ButtonDown = 1;
        }
        else
        {
            DMSG_MOUSE_TEST("Had send a button down event, then wait for button rise\n");
            Drift->ButtonDown = 0;
        }

        goto SendMsg;
    }

    /* �а����¼����߹����¼����ͻ����߳� */
    if(UsbMouse_IsButtonEvent(Event))
    {
        DMSG_MOUSE_TEST("have a button event\n");

        Drift->ButtonDown = 1;
        goto SendMsg;
    }

    /* ���PreMouseEvent��dubiousMouseEvent����Ч, ��ô��ֱ�ӱȽ� */
    if(Drift->PreMouseEvent.vaild && Drift->DubiousMouseEvent.vaild)
    {
        USB_OS_ENTER_CRITICAL(cup_sr);
        DMSG_MOUSE_TEST("------Pre------\n");
        DMSG_MOUSE_TEST("DubiousCoordinate = %x\n", Drift->DubiousCoordinate);

        DMSG_MOUSE_TEST("Pre Button 1 = %d\n", Drift->PreMouseEvent.MouseEvent.Button.LeftButton);
        DMSG_MOUSE_TEST("Pre Button 2 = %d\n", Drift->PreMouseEvent.MouseEvent.Button.RightButton);
        DMSG_MOUSE_TEST("Pre Button 3 = %d\n", Drift->PreMouseEvent.MouseEvent.Button.MiddleButton);
        DMSG_MOUSE_TEST("Pre Button 4 = %d\n", Drift->PreMouseEvent.MouseEvent.Button.Button4);
        DMSG_MOUSE_TEST("Pre Button 5 = %d\n", Drift->PreMouseEvent.MouseEvent.Button.Button5);
        DMSG_MOUSE_TEST("Pre Button 6 = %d\n", Drift->PreMouseEvent.MouseEvent.Button.Button6);
        DMSG_MOUSE_TEST("Pre Button 7 = %d\n", Drift->PreMouseEvent.MouseEvent.Button.Button7);
        DMSG_MOUSE_TEST("Pre Button 8 = %d\n", Drift->PreMouseEvent.MouseEvent.Button.Button8);

        DMSG_MOUSE_TEST("Pre X        = %d\n", Drift->PreMouseEvent.MouseEvent.X);
        DMSG_MOUSE_TEST("Pre Y        = %d\n", Drift->PreMouseEvent.MouseEvent.Y);
        DMSG_MOUSE_TEST("Pre Wheel    = %d\n", Drift->PreMouseEvent.MouseEvent.Wheel);
        DMSG_MOUSE_TEST("\n");

        DMSG_MOUSE_TEST("\n");
        DMSG_MOUSE_TEST("dubious Button 1 = %d\n", Drift->DubiousMouseEvent.MouseEvent.Button.LeftButton);
        DMSG_MOUSE_TEST("dubious Button 2 = %d\n", Drift->DubiousMouseEvent.MouseEvent.Button.RightButton);
        DMSG_MOUSE_TEST("dubious Button 3 = %d\n", Drift->DubiousMouseEvent.MouseEvent.Button.MiddleButton);
        DMSG_MOUSE_TEST("dubious Button 4 = %d\n", Drift->DubiousMouseEvent.MouseEvent.Button.Button4);
        DMSG_MOUSE_TEST("dubious Button 5 = %d\n", Drift->DubiousMouseEvent.MouseEvent.Button.Button5);
        DMSG_MOUSE_TEST("dubious Button 6 = %d\n", Drift->DubiousMouseEvent.MouseEvent.Button.Button6);
        DMSG_MOUSE_TEST("dubious Button 7 = %d\n", Drift->DubiousMouseEvent.MouseEvent.Button.Button7);
        DMSG_MOUSE_TEST("dubious Button 8 = %d\n", Drift->DubiousMouseEvent.MouseEvent.Button.Button8);

        DMSG_MOUSE_TEST("dubious X        = %d\n", Drift->DubiousMouseEvent.MouseEvent.X);
        DMSG_MOUSE_TEST("dubious Y        = %d\n", Drift->DubiousMouseEvent.MouseEvent.Y);
        DMSG_MOUSE_TEST("dubious Wheel    = %d\n", Drift->DubiousMouseEvent.MouseEvent.Wheel);
        DMSG_MOUSE_TEST("\n");

        DMSG_MOUSE_TEST("\n");
        DMSG_MOUSE_TEST("Event Button 1 = %d\n", Event->Button.LeftButton);
        DMSG_MOUSE_TEST("Event Button 2 = %d\n", Event->Button.RightButton);
        DMSG_MOUSE_TEST("Event Button 3 = %d\n", Event->Button.MiddleButton);
        DMSG_MOUSE_TEST("Event Button 4 = %d\n", Event->Button.Button4);
        DMSG_MOUSE_TEST("Event Button 5 = %d\n", Event->Button.Button5);
        DMSG_MOUSE_TEST("Event Button 6 = %d\n", Event->Button.Button6);
        DMSG_MOUSE_TEST("Event Button 7 = %d\n", Event->Button.Button7);
        DMSG_MOUSE_TEST("Event Button 8 = %d\n", Event->Button.Button8);

        DMSG_MOUSE_TEST("Event X        = %d\n", Event->X);
        DMSG_MOUSE_TEST("Event Y        = %d\n", Event->Y);
        DMSG_MOUSE_TEST("Event Wheel    = %d\n", Event->Wheel);
        DMSG_MOUSE_TEST("------Pre------\n");

        USB_OS_EXIT_CRITICAL(cup_sr);

        /* Ѱ��ͬ����������� */
        /* X������� */
        UsbMouse_AdjustCoordinate(&Drift->PreMouseEvent.MouseEvent,
                                  &Drift->DubiousMouseEvent.MouseEvent,
                                  Event,
                                  Event);

        Drift->DubiousMouseEvent.vaild = 0;
        Drift->PreMouseEvent.vaild     = 0;

        goto SendMsg;
    }
    else
    {
        /* �жϱ��������Ƿ����? */
        if(UsbMouse_IsDubiousEvent(Event, Drift) == 0)
        {
            goto SendMsg;
        }
        else
        {
            DMSG_INFO("a Dubious event\n");

            USB_OS_ENTER_CRITICAL(cup_sr);

            USB_OS_MEMCPY(&Drift->DubiousMouseEvent.MouseEvent, Event, sizeof(USBHMouseEvent_t));
            Drift->DubiousMouseEvent.vaild = 1;
            Drift->WaitEvent = 1;

            USB_OS_EXIT_CRITICAL(cup_sr);
        }
    }

    return;

SendMsg:
    USB_OS_ENTER_CRITICAL(cup_sr);

    USB_OS_MEMCPY(&Drift->CurrentMouseEvent.MouseEvent, Event, sizeof(USBHMouseEvent_t));
    Drift->CurrentMouseEvent.vaild = 1;

    USB_OS_EXIT_CRITICAL(cup_sr);

    UsbThreadWakeUp(Drift->ThreadSemi);
    UsbThreadSleep(Drift->notify_complete);

    return;
}

/*
*******************************************************************************
*                     UsbMouse_DriftControl
*
* Description:
*    ���ȥ����
*
* Parameters:
*
*
* Return value:
*
*
* note:
*    ��
*
*******************************************************************************
*/
static __s32 UsbMouse_DriftControl(UsbMouseDriftControl_t *Drift)
{
    __cpu_sr cup_sr	= 0;

    /* ����ȴ���־ */
    USB_OS_ENTER_CRITICAL(cup_sr);
    Drift->WaitEvent = 0;
    USB_OS_EXIT_CRITICAL(cup_sr);

    /* sent mouse event to system */
    if(Drift->CurrentMouseEvent.vaild)
    {
        USB_OS_ENTER_CRITICAL(cup_sr);

        USB_OS_MEMCPY(&Drift->usbMouse->MouseEvent, &Drift->CurrentMouseEvent.MouseEvent, sizeof(USBHMouseEvent_t));
        USB_OS_MEMCPY(&Drift->PreMouseEvent, &Drift->CurrentMouseEvent, sizeof(UsbMouseEventUnit_t));

        Drift->PreMouseEvent.vaild     = 1;
        Drift->CurrentMouseEvent.vaild = 0;
        Drift->DubiousMouseEvent.vaild = 0;

        DMSG_MOUSE_TEST("\n");
        DMSG_MOUSE_TEST("msg Button 1 = %d\n", Drift->usbMouse->MouseEvent.Button.LeftButton);
        DMSG_MOUSE_TEST("msg Button 2 = %d\n", Drift->usbMouse->MouseEvent.Button.RightButton);
        DMSG_MOUSE_TEST("msg Button 3 = %d\n", Drift->usbMouse->MouseEvent.Button.MiddleButton);
        DMSG_MOUSE_TEST("msg Button 4 = %d\n", Drift->usbMouse->MouseEvent.Button.Button4);
        DMSG_MOUSE_TEST("msg Button 5 = %d\n", Drift->usbMouse->MouseEvent.Button.Button5);
        DMSG_MOUSE_TEST("msg Button 6 = %d\n", Drift->usbMouse->MouseEvent.Button.Button6);
        DMSG_MOUSE_TEST("msg Button 7 = %d\n", Drift->usbMouse->MouseEvent.Button.Button7);
        DMSG_MOUSE_TEST("msg Button 8 = %d\n", Drift->usbMouse->MouseEvent.Button.Button8);

        DMSG_MOUSE_TEST("msg X        = %d\n", Drift->usbMouse->MouseEvent.X);
        DMSG_MOUSE_TEST("msg Y        = %d\n", Drift->usbMouse->MouseEvent.Y);
        DMSG_MOUSE_TEST("msg Wheel    = %d\n", Drift->usbMouse->MouseEvent.Wheel);
        DMSG_MOUSE_TEST("\n");

        USB_OS_EXIT_CRITICAL(cup_sr);

        if(Drift->usbMouse->CallBack)
        {
            USB_OS_esKRNL_CallBack((__pCBK_t)Drift->usbMouse->CallBack, (void *)&Drift->usbMouse->MouseEvent);
        }
    }

    UsbThreadWakeUp(Drift->notify_complete);

    return USB_ERR_SUCCESS;
}

/*
*******************************************************************************
*                     UsbMouse_DriftThread
*
* Description:
*
*
* Parameters:
*
*
* Return value:
*
*
* note:
*    ��
*
*******************************************************************************
*/
static void UsbMouse_DriftThread(void *p_arg)
{
    UsbMouseDriftControl_t *Drift = (UsbMouseDriftControl_t *)p_arg;

    if(Drift == NULL)
    {
        DMSG_PANIC("ERR: input error\n");
        return ;
    }


    USB_OS_SemPost(Drift->notify_complete);

    while(1)
    {
        //--<1>--ɱ���߳�
        TryToKillThreadSelf("UsbMouse_DriftThread");

        /* sleep */
        UsbThreadSleep(Drift->ThreadSemi);

        UsbMouse_DriftControl(Drift);
    }
}

/*
*******************************************************************************
*                     UsbMouse_DriftControl_Init
*
* Description:
*
*
* Parameters:
*
*
* Return value:
*
*
* note:
*
*
*******************************************************************************
*/
__s32 UsbMouse_DriftControl_Init(usbMouse_t *usbMouse)
{
    __s32 status = 0;
    UsbMouseDriftControl_t *Drift;
    __u8 err = 0;

    Drift = USB_OS_MALLOC(sizeof(UsbMouseDriftControl_t), USB_MEM_FILE_TRIGGER, USB_MEM_LINE_TRIGGER);
    if(Drift == NULL)
    {
        DMSG_PANIC("ERR: USB_OS_MALLOC failed\n");
        return USB_ERR_MALLOC_FAILED;
    }

    USB_OS_MEMSET(Drift, 0, sizeof(UsbMouseDriftControl_t));

    /* create thread */
    Drift->ThreadSemi = USB_OS_SemCreate(0);
    if(Drift->ThreadSemi == NULL)
    {
        DMSG_PANIC("ERR: USB_OS_SemCreate ThreadSemi failed\n");
        status = USB_ERR_CREATE_SIME_FAILED;
        goto err0;
    }

    Drift->notify_complete = USB_OS_SemCreate(0);
    if(Drift->notify_complete == NULL)
    {
        DMSG_PANIC("ERR: USB_OS_SemCreate notify_complete failed\n");
        status = USB_ERR_CREATE_SIME_FAILED;
        goto err1;
    }

    /* Mouse Drift thread */
    Drift->ThreadId = UsbCreateThread((void *)UsbMouse_DriftThread,
                                      (void *)Drift,
                                      0x800,
                                      THREAD_LEVEL_HID_CLIENT);
    if(Drift->ThreadId == USB_OS_NO_ERR)
    {
        DMSG_PANIC("ERR: create MainThreadId failed\n");
        status = USB_ERR_CREATE_THREAD_FAILED;
        goto err2;
    }

    USB_OS_SemPend(Drift->notify_complete, 0, &err);

    /* create timer */
    Drift->TimerHdle = USB_esKRNL_TmrCreate(4 , /* 4ms */
                                            USB_TIMER_PERIOD_MORE,
                                            (USB_TIMER_CALLBACK)UsbMouse_DriftTimeOut,
                                            (void *)Drift);
    if(Drift->TimerHdle == NULL)
    {
        DMSG_PANIC("ERR: create timer failed\n");
        status = USB_ERR_CREATE_TIMER_FAILED;
        goto err3;
    }

    USB_esKRNL_TmrStart(Drift->TimerHdle);

    /*  */
    Drift->usbMouse = usbMouse;
    usbMouse->Extern = Drift;

    return USB_ERR_SUCCESS;


err3:
    UsbKillThread(Drift->ThreadId, NULL);

err2:
    USB_OS_SemDel(Drift->notify_complete, &err);
    Drift->notify_complete = NULL;

err1:
    USB_OS_SemDel(Drift->ThreadSemi, &err);
    Drift->ThreadSemi = NULL;

err0:
    USB_OS_FREE(Drift);

    return status;
}

/*
*******************************************************************************
*                     UsbMouse_DriftControl_Exit
*
* Description:
*
*
* Parameters:
*
*
* Return value:
*
*
* note:
*
*
*******************************************************************************
*/
__s32 UsbMouse_DriftControl_Exit(usbMouse_t *usbMouse)
{
    UsbMouseDriftControl_t *Drift = NULL;
    __u8 err = 0;

    if(usbMouse == NULL)
    {
        DMSG_PANIC("ERR: input error\n");
        return USB_ERR_BAD_ARGUMENTS;
    }

    Drift = usbMouse->Extern;
    if(Drift == NULL)
    {
        DMSG_PANIC("ERR: Drift == NULL\n");
        return USB_ERR_BAD_ARGUMENTS;
    }

    /* stop and kill timer */
    USB_esKRNL_TmrStop(Drift->TimerHdle);
    USB_esKRNL_TmrDel(Drift->TimerHdle);
    Drift->TimerHdle = NULL;

    /* kill thread */
    UsbKillThread(Drift->ThreadId, Drift->ThreadSemi);

    USB_OS_SemDel(Drift->ThreadSemi, &err);
    Drift->ThreadSemi = NULL;

    USB_OS_SemDel(Drift->notify_complete, &err);
    Drift->notify_complete = NULL;

    usbMouse->Extern = NULL;
    USB_OS_FREE(Drift);

    return USB_ERR_SUCCESS;
}

