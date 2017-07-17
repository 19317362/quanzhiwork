/*===============================================================================================*
*                                                                                                *
* MMMMMMMM  MMMMMMM    MMMMMMM  MMMMMMMM    MMMMMMMM       MM      MM    MMMMMMMMMMMMM           *
*    MMM      MMM        MMM      MMM          MM      MM        . MM        MMM        MM       *
*    MMM      MMM        MMM      MMM          MM      MM          MM        MMM        MM.      *
*    MMM      MMM        MMM      MMM          MM    . MM          MM        MMM        MMMM     *
*    MMM      MMM        MMM      MMM          MM      MMMM                  MMM        MM       *
*    MMMMMMMMMMMM        MMM      MMM          MM          MMMM              MMMMMMMMMMM         *
*    MMM      MMM        MMM      MMM          MM              MMMM          MMM          .      *
*    MMM      MMM        MMM      MMM          MM                . MM        MMM          MM     *
*    MMM      MMM        MMM      MMM          .     MM            MMMM      MMM          MMMM   *
*    MMM      MMM        MM        MM                MM            MM.       MMM          MM     *
*    MMM      MMM  .     MM        MM.               MMMM          MM        MMM          MM     *
* MMMMMMMM  MMMMMMM  MM.                MMMMM         MM      MMMM        MMMMMMMMMMMMM.         *
*                                                                                                *
*================================================================================================
*
*                                              usb host module
*
*                             Copyright(C), 2006-2008, SoftWinners Co., Ltd.
*											       All Rights Reserved
*
* File Name :
*
* Author : GLHuang(HoLiGun)
*
* Version : 1.0
*
* Date : 2008.06.12
*
* Description :
*			��list_head��extention
* History :
*================================================================================================
*/
#include "usb_host_config.h"

#include "usb_os_platform.h"
#include "usbh_debug.h"

#include "usb_list.h"
#include "list_head_ext.h"


//��list_header_input��ɾ��ĳ��node
//return :	0	//��ʾ�ҵ��˸�node���ҳɹ�ɾ����
//��list�Ĳ�����critical����
/*
********************************************************************************
*                     list_head_ext_remov_node_from_list
* Description:
*     ��list_header_input������ɾ��ֵΪnode_data��node
*
* Arguments:
*     node_data          : input.  ����Ѱ��node
*     list_header_input  : input.  ���е�ͷ
* Return value:
*     0 : success
*    !0 : fail
* note:
*     ����߳̿���ͬʱ�������list, ����Ҫ���ٽ��������жϡ�
* ע�� : ���������ܻ�Ӱ��ϵͳ����
*********************************************************************************
*/
int   list_head_ext_remov_node_from_list(void *node_data,
        struct list_head *list_header_input)
{
    struct list_head *list_start = NULL;
    struct list_head *list_now   = NULL;
    struct list_head *list_next  = NULL;
    __cpu_sr sr = 0;

    /* check input */
    if(node_data == NULL || list_header_input == NULL)
    {
        DMSG_PANIC("ERR : list_head_ext_remov_node_from_list() input == NULL\n");
        return -1;
    }

    /* list must not empty */
    if(list_empty(list_header_input))
    {
        DMSG_PANIC("ERR : list_head_ext_remov_node_from_list() func_drv->dev_list is empty\n");
        return -1 ;
    }

    USB_OS_ENTER_CRITICAL(sr);

    /* ��������list, Ѱ��Ҫɾ����node, ����ɾ���� */
    list_start = list_header_input;
    list_now = list_start->next;
    while(list_start != list_now)
    {
        list_next = list_now->next;

        if(list_now->data == node_data)
        {
            list_now->data = NULL;
            list_head_unlink_and_del(list_now);
            list_now = NULL;
            USB_OS_EXIT_CRITICAL(sr);
            return 0;
        }

        list_now = list_next;
    }

    USB_OS_EXIT_CRITICAL(sr);

    return -1;
}


/*
********************************************************************************
*                     list_node_exist
* Description:
*     �ж�list���Ƿ����ֵΪdata��node
*
* Arguments:
*     node_data          : input.
*     list_header_input  : input.
* Return value:
*     0 : success
*    !0 : fail
* note:
*
*********************************************************************************
*/
s32 list_node_exist(void *data, struct list_head *list_head)
{
    struct list_head *list_start = NULL;
    struct list_head *list_now = NULL;
    __cpu_sr cup_sr	= 0;

    /* check input */
    if(data == NULL || list_head == NULL)
    {
        DMSG_PANIC("ERR : list_node_exist: input == NULL\n");
        return -1;
    }

    USB_OS_ENTER_CRITICAL(cup_sr);

    /* ��������list, Ѱ��ֵΪdata��node */
    list_start = list_head;
    list_now = list_start->next;
    while(list_now != list_start)
    {
        if(list_now->data == data)
        {
            USB_OS_EXIT_CRITICAL(cup_sr);
            return 0;
        }

        list_now = list_now->next;
    }

    USB_OS_EXIT_CRITICAL(cup_sr);

    return -1;
}

/*
*****************************************************************************
*                     del_node_from_list
*
* Description:
*     ��������list, ɾ������data���ݵ�����node
* Arguments:
*
* Returns:
*
* note:
*
*
*****************************************************************************
*/
s32 list_del_node_by_data(void *data, struct list_head  *list)
{
    struct list_head *list_now  = NULL;
    struct list_head *list_next = NULL;
    u32  is_find = 0;
    __cpu_sr cup_sr	= 0;

    if(data == NULL || list == NULL)
    {
        DMSG_PANIC("ERR: list_del_node_by_data: input == NULL\n");
        return -1;
    }

    if(list_empty(list))
    {
        DMSG_PANIC("ERR: list_del_node_by_data: list is already empty\n");
        return -1;
    }

    USB_OS_ENTER_CRITICAL(cup_sr);

    list_now = list->next;
    list_next = NULL;

    //��������ͷ���˳�
    while(list_now != list)
    {
        list_next = list_now->next;

        //--<1>--�ҵ�req�ͽ����������ɾ��
        if(list_now->data == data)
        {
            /* �����������ȡ���� */
            list_del_init(list_now);

            /* �ͷ�list�ṹ */
            _list_head_free(list_now);

            list_now = NULL;

            is_find = 1;
        }

        list_now = list_next;
    }

    USB_OS_EXIT_CRITICAL(cup_sr);

    if(is_find)
    {
        return 0;
    }
    else
    {
        //DMSG_PANIC("ERR: list_del_node_by_data: del failed\n");
        return -1;
    }
}

/*
*****************************************************************************
*                     list_destroy_whole_list
*
* Description:
*     ɾ�� list �����е�node, �����ͷ�����Դ
* Arguments:
*
* Returns:
*
* note:
*
*
*****************************************************************************
*/
s32 list_destroy_whole_list(struct list_head  *list)
{
    struct list_head *list_now  = NULL;
    struct list_head *list_next = NULL;
    __cpu_sr cup_sr	= 0;

    if(list == NULL)
    {
        DMSG_PANIC("ERR: input == NULL\n");
        return -1;
    }

    if(list_empty(list))
    {
        DMSG_PANIC("ERR: list is already empty\n");
        return -1;
    }

    USB_OS_ENTER_CRITICAL(cup_sr);

    list_now  = list->next;
    list_next = NULL;

    //��������ͷ���˳�
    while(list_now != list)
    {
        //--<1>--Ԥ��ȡ����һ��node
        list_next = list_now->next;

        //--<2>--ɾ����ǰnode, �����ͷ�����Դ
        list_del_init(list_now);
        _list_head_free(list_now);
        list_now = NULL;

        //--<3>--ȡ����һ��node
        list_now = list_next;
    }

    USB_OS_EXIT_CRITICAL(cup_sr);

    return 0;
}




