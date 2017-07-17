/*
*******************************************************************************
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
* Date : 2008.06.xx
*
* Description :
*		��������reg/unreg�ӿ�
* History :
********************************************************************************************************************
*/

#include "usb_host_config.h"

#include "usb_os_platform.h"
#include "usb_host_common.h"

#include "usb_host_base_types.h"
#include "usb_list.h"
#include "usb_gen_dev_mod.h"
#include "usb_drv_dev_macth.h"
#include "usb_virt_bus.h"
#include "usb_gen_hcd.h"
#include "usb_msg.h"
#include "usb_core_base.h"

//return
//	0	:	�ɹ�reg
s32 usb_host_func_drv_reg (struct usb_host_func_drv *drv)
{
    if(usb_host_is_enabled() == 0)
    {
        DMSG_PANIC("ERR: usb host is disabled\n");
        return -ENODEV;
    }

    DMSG_INFO("[usb bus]: driver \"%s\" detect usb bus\n", drv->func_drv_name != NULL ? drv->func_drv_name : " ");
    drv->func_drv_ext.probe_ext = _usb_core_func_drv_probe_interface;
    drv->func_drv_ext.disconnect_ext = _usb_core_func_drv_disconncet_interface;

    return usb_virt_bus_drv_reg(drv);
}

//return
//	0	:	�ɹ�unreg
s32 usb_host_func_drv_unreg (struct usb_host_func_drv *drv)
{
    DMSG_INFO("[usb bus]: driver \"%s\" disconnect usb bus\n", drv->func_drv_name != NULL ? drv->func_drv_name : " ");

    return usb_virt_bus_drv_unreg( drv);
}

