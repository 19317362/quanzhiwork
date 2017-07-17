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
* Date : 2008.07.01
*
* Description :
*
* History :
*================================================================================================
*/
#include "usb_host_config.h"

#include "usb_os_platform.h"
#include "usb_host_common.h"

#include "usb_host_base_types.h"
#include "usb_list.h"


#include "list_head_ext.h"


static struct usb_virt_bus  my_usb_virt_bus;	//super bus

/*
********************************************************************************
*                     _usb_virt_bus_remov_dev_from_drv
* Description:
*     ��device��driver��virt_dev_list�б���ɾ����
* Arguments:
*     dev       : input.  ������ɾ����device
*     drv       : input.  ����
* Return value:
*     void
* note:
*     ���ù���device�Ƿ������virt_dev_list�б���
*
*********************************************************************************
*/
static void _usb_virt_bus_remov_dev_from_drv(struct usb_host_virt_sub_dev *dev,
        struct  usb_host_func_drv *drv)
{
    /* check input */
    if(dev == NULL || drv == NULL)
    {
        DMSG_PANIC("PANIC : [VIRR BUS] : _usb_virt_bus_remov_dev_from_drv() input == NULL\n");
        return ;
    }

    /* ����Ϊ��,�Ͳ���ɾ���� */
    if(list_empty(&( drv->virt_dev_list)))
    {
        DMSG_PANIC("PANIC : [VIRR BUS] : _usb_virt_bus_remov_dev_from_drv() func_drv->dev_list is empty\n");
        return ;
    }

    /* del device from driver list */
    list_head_ext_remov_node_from_list(dev, &(drv->virt_dev_list));
}


/*
********************************************************************************
*                     usb_virt_bus_drv_reg
* Description:
*     ��driver��ӵ�������
* Arguments:
*     drv       : input.  ��ע�������
* Return value:
*
* note:
*     һ����������ע������?????????
*********************************************************************************
*/
s32 usb_virt_bus_drv_reg(struct usb_host_func_drv *drv)
{
    struct list_head *list_start = NULL;
    struct list_head *list_now = NULL;
    struct usb_host_virt_sub_dev *dev = NULL;

    if(drv == NULL )
    {
        DMSG_PANIC("PANIC : [VIRR BUS] : usb_virt_bus_drv_reg() input == NULL\n");
        return -1;
    }

    /* ��ע���������û���豸����ƥ��� */
    if(! list_empty(&(drv->virt_dev_list)))
    {
        DMSG_PANIC("PANIC : [VIRR BUS] : usb_virt_bus_drv_reg() drv`s dev_list not empty\n");
        return -1;
    }

    /* bus���Ѿ�����driver, �Ͳ�Ҫ������� */
    if(list_node_exist((void *)drv, &(my_usb_virt_bus.drv_list)) == 0)
    {
        DMSG_PANIC("ERR: drv is already in the list\n");
        return 0;
    }

    UsbLock(my_usb_virt_bus.BusLock);

    /* ��driver��ӵ�bus�� */
    list_head_malloc_and_add(drv, &(my_usb_virt_bus.drv_list));

    /* ��ǰ�����Ѿ���device����bus��, �����ȥѰ��bus����֮ƥ���deivce */
    list_start = &(my_usb_virt_bus.dev_list);
    list_now = list_start->next;
    while(list_start != list_now)
    {
        dev = (struct usb_host_virt_sub_dev *)(list_now->data);
        if(dev == NULL)
        {
            DMSG_PANIC("dev == NULL\n", dev);
            break;
        }

        list_now = list_now->next;

        UsbLock(dev->usb_virt_sub_dev_semi);

        /* �ҵ�û�б�bind��dev */
        if(dev->func_drv == NULL)
        {
            /* ���ȳ���bind��һ�� */
            dev->func_drv = drv;
            if(drv->func_drv_ext.probe_ext(dev) == 0)
            {
                /* device �� driver ƥ��ɹ�, �ͽ� device ��ӵ� driver ��virt_dev_list��*/
                list_head_malloc_and_add(dev, &(drv->virt_dev_list));
            }
            else
            {
                /* ƥ��ʧ��, ����ա�device�������� */
                dev->func_drv = NULL;
            }
        }

        UsbUnLock(dev->usb_virt_sub_dev_semi);
    }

    UsbUnLock(my_usb_virt_bus.BusLock);

    return 0;
}

/*
********************************************************************************
*                     usb_virt_bus_drv_unreg
* Description:
*     ��driver��������ժ��
* Arguments:
*     drv       : input.
* Return value:
*
* note:
*     ע����ڼ䣬dev����ʹ�ã�����dev->usb_virt_dev_semi������
* ��Ϊ����ڼ�dev�����ٽ����򣬴��ڻ���״̬

* ������func_driver��֧�ֵ�scsi_device list,��scsi_func_driver->scsi_dev_list
* ���dev->drv == drv(��dev������ == ������)�������²���:
*		1,��drv->scsi_dev_list��ɾ����dev
*		2,��dev->drv == NULL
*		3,������drv->scsi_remove
*********************************************************************************
*/
s32 usb_virt_bus_drv_unreg(struct usb_host_func_drv *drv)
{
    struct list_head *list_start = NULL;
    struct list_head *list_now = NULL;
    struct usb_host_virt_sub_dev *dev = NULL;

    if(drv == NULL )
    {
        DMSG_PANIC("PANIC : [VIRR BUS] : usb_virt_bus_drv_unreg() input == NULL\n");
        return -1;
    }

    UsbLock(my_usb_virt_bus.BusLock);

    /* ��unbind��drv����֮������dev */
    if(list_empty(&(drv->virt_dev_list)))
    {
        /* ��Ȼbus�ϵ�driver���ж�����, Ҳ��û�б�Ҫȥɾ��driver�� */
    }
    else
    {
        /* ����driver��virt_dev_list, ɾ�����е�device */
        list_start = &(drv->virt_dev_list);
        list_now = list_start->next;
        while(list_start != list_now)
        {
            dev = (struct usb_host_virt_sub_dev *)(list_now->data);
            if(dev == NULL)
            {
                DMSG_PANIC("dev == NULL\n", dev);
                break;
            }

            list_now = list_now->next;

            UsbLock(dev->usb_virt_sub_dev_semi);

            if(dev)
            {
                if(drv->func_drv_ext.disconnect_ext)
                {
                    if(drv->func_drv_ext.disconnect_ext(dev) == NULL)
                    {
                        //remove each other
                        dev->func_drv = NULL;
                        _usb_virt_bus_remov_dev_from_drv(dev, drv);
                    }
                    else
                    {
                        DMSG_PANIC("PANIC : [VIRR BUS] : disconnect fail\n");
                    }
                }
            }

            UsbUnLock(dev->usb_virt_sub_dev_semi);
        }
    }

    /* disconnectʧ�ܵ�device, Ӧ��ǿ��ɾ�� */
    if(! list_empty(&(drv->virt_dev_list)))
    {
        DMSG_PANIC("PANIC : [VIRR BUS] : usb_virt_bus_drv_unreg() drv->dev_list not empty\n");
    }

    /* ��supper bus��ɾ����drv */
    list_head_ext_remov_node_from_list(drv, &(my_usb_virt_bus.drv_list));

    UsbUnLock(my_usb_virt_bus.BusLock);

    return 0;
}


/*
********************************************************************************
*                     usb_virt_bus_dev_add
* Description:
*     ��device��ӵ�������
* Arguments:
*     dev  : input.
* Return value:
*
* note:
*     һ���豸����ע������?????????
*********************************************************************************
*/
s32 usb_virt_bus_dev_add(struct usb_host_virt_sub_dev *dev)
{
    __cpu_sr sr = 0;

    if(!dev || !(dev->father_dev))
    {
        DMSG_PANIC("PANIC : [VIRR BUS] : usb_virt_bus_dev_add() : dev == NULL\n");
        return -1;
    }


    DMSG_PANIC("//usb_virt_bus_dev_add --<1>\n");


    /* bus���Ѿ�����deivce, �Ͳ�Ҫ������� */
    if(list_node_exist((void *)dev, &(my_usb_virt_bus.dev_list)) == 0)
    {
        DMSG_PANIC("ERR: device is already in the list\n");
        return 0;
    }


    DMSG_PANIC("//usb_virt_bus_dev_add --<2>\n");

    UsbLock(my_usb_virt_bus.BusLock);

    if(dev->func_drv != NULL)
    {
        DMSG_PANIC("PANIC : [VIRR BUS] : dev->drv != NULL,new dev has been bind to drv\n");
        UsbUnLock(my_usb_virt_bus.BusLock);
        return -1;
    }
    else
    {
        /* ��ǰ�����Ѿ���driver����bus��, �����ȥѰ��bus����֮ƥ���driver */
        struct list_head *list_start  = NULL;
        struct list_head *list_now = NULL;

        UsbLock(dev->usb_virt_sub_dev_semi);

        list_start = &(my_usb_virt_bus.drv_list);
        list_now = list_start->next;
        while(list_start != list_now)
        {
            struct usb_host_func_drv *func_drv = (struct usb_host_func_drv *)list_now->data;
            list_now = list_now->next;

            /* ���ȳ���bind��һ�� */
            dev->func_drv = func_drv;
            if(func_drv->func_drv_ext.probe_ext(dev) == 0)
            {
                /* add ��drv�� dev list�� */
                USB_OS_ENTER_CRITICAL(sr);
                list_head_malloc_and_add(dev, &(func_drv->virt_dev_list));
                USB_OS_EXIT_CRITICAL(sr);

                break;	/* ������һ��macth��drv�ͽ����� */
            }
            else
            {
                dev->func_drv = NULL; /* ʧ������� */
            }
        }

        UsbUnLock(dev->usb_virt_sub_dev_semi);
    }


    DMSG_PANIC("//usb_virt_bus_dev_add --<3>\n");


    /* ��dev��ӵ�supper bus */
    USB_OS_ENTER_CRITICAL(sr);
    list_head_malloc_and_add(dev, &(my_usb_virt_bus.dev_list));
    USB_OS_EXIT_CRITICAL(sr);

    UsbUnLock(my_usb_virt_bus.BusLock);

    return 0;
}

/*
********************************************************************************
*                     usb_virt_bus_dev_add
* Description:
*     ��device��ӵ�������
* Arguments:
*     dev  : input.
* Return value:
*
* note:
*     ע����ڼ䣬dev����ʹ�ã�����dev->scsi_dev_semi������
* ��Ϊ����ڼ�dev�����ٽ����򣬴��ڻ���״̬��dev->usb_virt_dev_semi����
*    ��dev->drv��ɾ����dev
*    ����drv->scsi_remove
*    dev->drv == NULL
*********************************************************************************
*/
s32 usb_virt_bus_dev_del(struct usb_host_virt_sub_dev *dev)
{
    struct usb_host_func_drv *func_driver = NULL;

    if(dev == NULL)
    {
        DMSG_PANIC("PANIC : [VIRR BUS] : usb_virt_bus_dev_del() input == NULL\n");
        return -1;
    }

    UsbLock(my_usb_virt_bus.BusLock);

    func_driver = dev->func_drv;

    UsbLock(dev->usb_virt_sub_dev_semi);
    if(func_driver)
    {
        if(func_driver->func_drv_ext.disconnect_ext)
        {
            if(func_driver->func_drv_ext.disconnect_ext(dev) == NULL)
            {
                //remove each other
                dev->func_drv = NULL;
                _usb_virt_bus_remov_dev_from_drv(dev, func_driver);
            }
            else
            {
                DMSG_PANIC("PANIC : [VIRR BUS] :  disconnect fail\n");
            }
        }
    }
    else
    {
        DMSG_PANIC("PANIC : [VIRR BUS] : usb_virt_bus_dev_del() dev->drv == NULL\n");
    }
    UsbUnLock(dev->usb_virt_sub_dev_semi);

    //del from supper bus
    list_head_ext_remov_node_from_list(dev, &(my_usb_virt_bus.dev_list));

    UsbUnLock(my_usb_virt_bus.BusLock);

    return 0;
}

/*
********************************************************************************
*                     usb_virt_bus_init
* Description:
*     bus�ĳ�ʼ��ֻ�ǳ�ʼ��device list��driver list
* Arguments:
*     void
* Return value:
*     void
* note:
*     void
*********************************************************************************
*/
s32 usb_virt_bus_init(void )
{
    USB_OS_MEMSET(&my_usb_virt_bus, 0, sizeof(struct usb_virt_bus));

    INIT_LIST_HEAD(& (my_usb_virt_bus.dev_list));
    INIT_LIST_HEAD(&(my_usb_virt_bus.drv_list));

    my_usb_virt_bus.BusLock = USB_OS_SemCreate(1);
    if(my_usb_virt_bus.BusLock == NULL)
    {
        DMSG_PANIC("ERR: usb_virt_bus_init: my_usb_virt_bus.BusLock == NULL\n");
        return -1;
    }

    return 0;
}

/*
********************************************************************************
*                     usb_virt_bus_exit
* Description:
*
* Arguments:
*     void
* Return value:
*     void
* note:
*     void
*********************************************************************************
*/
s32 usb_virt_bus_exit(void )
{
    __u8 err = 0;

    if(my_usb_virt_bus.BusLock)
    {
        USB_OS_SemDel(my_usb_virt_bus.BusLock, &err);
        my_usb_virt_bus.BusLock = NULL;
    }
    else
    {
        DMSG_PANIC("ERR: usb_virt_bus_exit: my_usb_virt_bus.BusLock == NULL\n");
        return -1;
    }

    USB_OS_MEMSET(&my_usb_virt_bus, 0, sizeof(struct usb_virt_bus));

    INIT_LIST_HEAD(& (my_usb_virt_bus.dev_list));
    INIT_LIST_HEAD(&(my_usb_virt_bus.drv_list));

    return 0;
}



