/*
********************************************************************************************************************
*
*
*                              (c) Copyright 2007-2010, javen.China
*										All	Rights Reserved
*
* File Name 	: horse.c
*
* Author 		: javen
*
* Version 		: 2.0
*
* Date 			: 2010.03.02
*
* Description 	: horse��������Ҫ�������ڴ�Ԥ���䣬�����ڴ��СԤ�ȶ���������ڴ��,
*                 ����Ҫmalloc��ʱ�򣬴��ڴ������ѡ���õĿ飬�ָ��û�ʹ�ã���������
*				  ʹ��ϵͳmalloc��
*
* History 		:
*
********************************************************************************************************************
*/
#include "usb_os_platform.h"
#include "usb_host_base_types.h"

#include  "horse.h"

static __HorseMgr_t *Horse_g = NULL;

/*
*******************************************************************************
*                     __HorseHeadExit
*
* Description:
*    �ͷ�HorseHead
*
* Parameters:
*    hdle  	�� input. HorseHead
*
* Return value:
*    0	���ɹ�
*	!0	��ʧ��
*
* note:
*    ��
*
*******************************************************************************
*/
static __s32 __HorseHeadExit(__HorseHead_t *HorseHead)
{
    __u32 i = 0;

    for(i = 0; i < HorseHead->nr; i++)
    {
        if(HorseHead->entry[i].buff)
        {
            USB_OS_FREE(HorseHead->entry[i].buff);
            HorseHead->entry[i].buff = NULL;
        }

        USB_OS_MEMSET(&(HorseHead->entry[i]), 0, sizeof(__HorseBlock_t));
    }

    USB_OS_FREE(HorseHead->entry);
    HorseHead->entry = NULL;

    return 0;
}

/*
*******************************************************************************
*                     __HorseHeadInit
*
* Description:
*    horse�ڴ��Ԥ��������horse�ڴ�鲢�ҳ�ʼ����
*
* Parameters:
*    size  	�� input. �ڴ���С
*	 nr		:  input. �ڴ�����
*
* Return value:
*    �ɹ�����horse head, ���򷵻�NULL.
*
* note:
*    ��
*
*******************************************************************************
*/
static __s32 __HorseHeadInit(__HorseHead_t *HorseHead, __u32 size, __u32 nr)
{
    __u32 i = 0;

    /* ����HorseBlock */
    HorseHead->nr	 = nr;
    HorseHead->entry = (__HorseBlock_t *)USB_OS_MALLOC(sizeof(__HorseBlock_t) * nr,
                       USB_MEM_FILE_TRIGGER ,
                       USB_MEM_LINE_TRIGGER );
    if(HorseHead->entry == NULL)
    {
        DMSG_PANIC("ERR: USB_OS_MALLOC failed\n");
        return -1;
    }

    USB_OS_MEMSET(HorseHead->entry, 0, (sizeof(__HorseBlock_t) * nr));

    /* ��ʼ��ÿһ��HorseBlock */
    for(i = 0; i < nr; i++)
    {
        HorseHead->entry[i].size = size;
        HorseHead->entry[i].buff = (void *)USB_OS_MALLOC(size,
                                   USB_MEM_FILE_TRIGGER,
                                   USB_MEM_LINE_TRIGGER );
        if(HorseHead->entry[i].buff == NULL)
        {
            DMSG_PANIC("ERR: USB_OS_MALLOC failed\n");
            goto failed;
        }

        USB_OS_MEMSET(HorseHead->entry[i].buff, 0, size);
    }

    return 0;

failed:
    __HorseHeadExit(HorseHead);

    return -1;
}

/*
*******************************************************************************
*                     HorseBlockInit
*
* Description:
*    horse�ڴ��Ԥ��������horse�ڴ�鲢�ҳ�ʼ����
*
* Parameters:
*    size  	�� input. �ڴ���С
*	 nr		:  input. �ڴ�����
*
* Return value:
*    �ɹ�����horse head, ���򷵻�NULL.
*
* note:
*    ��
*
*******************************************************************************
*/
void *HorseHeadInit(__u32 size, __u32 nr)
{
    __HorseMgr_t *HorseMgr = Horse_g;
    __u32 i = 0;

    for(i = 0; i < MAX_HORSE_USER_NUM; i++)
    {
        if(HorseMgr->Horse[i].used == 0)
        {
            HorseMgr->Horse[i].used = 1;

            /* ��ʼ��ʧ�ܾ�ȥѰ����һ�� */
            if(__HorseHeadInit(&(HorseMgr->Horse[i]), size, nr) != 0)
            {
                DMSG_PANIC("ERR: init HorseHead[%d] failed\n", i);
                HorseMgr->Horse[i].used = 0;
                continue;
            }

            return (void *) & (HorseMgr->Horse[i]);
        }
    }

    return NULL;
}

/*
*******************************************************************************
*                     HorseHeadExit
*
* Description:
*    �ͷ�HorseHead
*
* Parameters:
*    hdle  	�� input. HorseHead
*
* Return value:
*    0	���ɹ�
*	!0	��ʧ��
*
* note:
*    ��
*
*******************************************************************************
*/
__s32 HorseHeadExit(void *hdle)
{
    __HorseHead_t *HorseHead = (__HorseHead_t *)hdle;

    return __HorseHeadExit(HorseHead);
}

/*
*******************************************************************************
*                     HorseBlockMalloc
*
* Description:
*    �����ڴ�
*
* Parameters:
*    hdle  	�� input. HorseHead
*
* Return value:
*    �ɹ�����horse block buffer��ַ, ���򷵻�NULL.
*
* note:
*    ��
*
*******************************************************************************
*/
void *HorseBlockMalloc(void *hdle)
{
    __HorseHead_t *HorseHead = (__HorseHead_t *)hdle;
    __u32 i = 0;
    __cpu_sr cup_sr = 0;

    for(i = 0; i < HorseHead->nr; i++)
    {
        if((HorseHead->entry[i].is_used == 0) && (HorseHead->entry[i].buff != NULL))
        {
            USB_OS_ENTER_CRITICAL(cup_sr);
            HorseHead->entry[i].is_used = 1;
            USB_OS_EXIT_CRITICAL(cup_sr);

            return (void *)HorseHead->entry[i].buff;
        }
    }

    return NULL;
}

/*
*******************************************************************************
*                     HorseBlockFree
*
* Description:
*    �ͷ��ڴ�
*
* Parameters:
*    hdle  	�� input. HorseHead
*
* Return value:
*    0	���ɹ�
*	!0	��ʧ��
*
* note:
*    ��
*
*******************************************************************************
*/
__s32 HorseBlockFree(void *hdle, void *buffer_addr)
{
    __HorseHead_t *HorseHead = (__HorseHead_t *)hdle;
    __u32 i = 0;
    __cpu_sr cup_sr = 0;

    for(i = 0; i < HorseHead->nr; i++)
    {
        if(HorseHead->entry[i].buff == buffer_addr)
        {
            if(HorseHead->entry[i].is_used)
            {
                USB_OS_ENTER_CRITICAL(cup_sr);
                HorseHead->entry[i].is_used = 0;
                USB_OS_EXIT_CRITICAL(cup_sr);

                return 0;
            }
            else
            {
                DMSG_PANIC("ERR: the HorseBlock had already free\n");
                return 0;
            }
        }
    }

    DMSG_PANIC("ERR: HorseBlockFree failed, the buffer_addr is not belong to HorseBlock\n");

    return -1;
}

/*
*******************************************************************************
*                     HorseInit
*
* Description:
*
*
* Parameters:
*
* Return value:
*
* note:
*
*******************************************************************************
*/
__s32 HorseInit(void)
{
    Horse_g = (__HorseMgr_t *)USB_OS_MALLOC(sizeof(__HorseMgr_t),
                                            USB_MEM_FILE_TRIGGER,
                                            USB_MEM_LINE_TRIGGER );
    if(Horse_g == NULL)
    {
        DMSG_PANIC("ERR: USB_OS_MALLOC failed\n");
        return -1;
    }

    USB_OS_MEMSET(Horse_g, 0, sizeof(__HorseMgr_t));

    return 0;
}

/*
*******************************************************************************
*                     HorseExit
*
* Description:
*
*
* Parameters:
*
* Return value:
*
* note:
*
*******************************************************************************
*/
__s32 HorseExit(void)
{
    if(Horse_g)
    {
        USB_OS_FREE(Horse_g);
        Horse_g = NULL;

        return 0;
    }

    return -1;
}



