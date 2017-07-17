/*
********************************************************************************************************************
*                                              usb host driver
*
*                              (c) Copyright 2007-2010, javen.China
*										All	Rights Reserved
*
* File Name 	: Disk.c
*
* Author 		: javen
*
* Version 		: 2.0
*
* Date 			: 2010.03.02
*
* Description 	:
*
* History 		:
*
********************************************************************************************************************
*/
#include  "usb_host_config.h"
#include  "usb_host_base_types.h"
#include  "usb_os_platform.h"
#include  "error.h"

#include  "usbh_to_usbm.h"

#include  "usb_msc_i.h"
#include  "Scsi2.h"

#include  "LunMgr_i.h"
#include  "BlkDev.h"

/*
*******************************************************************************
*                     DiskMediaChange
*
* Description:
*
*
* Parameters:
*
*
* Return value:
*    ��
*
* note:
*    ��
*
*******************************************************************************
*/
static void DiskMediaChange(__mscLun_t *mscLun)
{
    __UsbBlkDev_t *BlkDev = NULL;
    __s32 ret = 0;

    if(mscLun == NULL)
    {
        DMSG_PANIC("ERR: DiskMediaChange: mscLun == NULL\n");
        return;
    }

    BlkDev = (__UsbBlkDev_t *)mscLun->sdev_data;
    if(BlkDev == NULL)
    {
        DMSG_PANIC("ERR: DiskMediaChange: BlkDev == NULL\n");
        return;
    }

    UsbLock(mscLun->Lock);
    ret = ScsiTestUnitReady(mscLun);
    UsbUnLock(mscLun->Lock);
    switch(ret)
    {
    case USB_STATUS_SUCCESS:
        /* û��ע��disk�豸, ��ȥע��disk�豸 */
        if(!BlkDev->is_RegDisk)
        {
            /* ��ô�����Ϣ */
            GetDiskInfo(BlkDev);

            /* ע����豸 */
            if(mscLun->MediaPresent)
            {
                UsbBlkDevUnReg(BlkDev);
                UsbBlkDevReg(BlkDev, DEV_CLASS_DISK, 1);
            }
        }
        break;

    case USB_STATUS_MEDIA_NOT_PRESENT:
        if(BlkDev->is_RegDisk)
        {
            DMSG_PANIC("ERR: DiskMediaChange: media is change\n");

            UsbBlkDevUnReg(BlkDev);
            UsbBlkDevReg(BlkDev, DEV_CLASS_USERDEF, 0);
        }
        break;

    case USB_STATUS_NOT_READY_TO_READY_TRANSITION:
        DMSG_INFO("[USB Disk]: media not ready to ready transition\n");
        break;

    default:
        BlkDev->ErrCmdNr++;
        if(BlkDev->ErrCmdNr > 3)
        {
            DMSG_PANIC("ERR: too much error during test unit ready\n");
            BlkDev->ErrCmdNr = 0;

            if(BlkDev->is_RegDisk)
            {
                UsbBlkDevUnReg(BlkDev);
                UsbBlkDevReg(BlkDev, DEV_CLASS_USERDEF, 0);
            }
            else
            {
                DMSG_PANIC("ERR: device is not regist a disk device\n");
            }
        }
    }

    return;
}

/*
*******************************************************************************
*                     UsbBlkDevOpen
*
* Description:
*
*
* Parameters:
*
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
__s32 DiskProbe(__mscLun_t *mscLun)
{
    __UsbBlkDev_t *BlkDev = NULL;
    __cpu_sr  cpu_sr;
    __s32 ret = 0;

    if(mscLun == NULL)
    {
        DMSG_PANIC("ERR: DiskProbe: input error\n");
        return -1;
    }

    /* ��ʼ��һ�����豸 */
    BlkDev = UsbBlkDevAllocInit(mscLun);
    if(BlkDev == NULL)
    {
        DMSG_PANIC("ERR: DiskProbe: alloc blkdev failed\n");
        return -1;
    }

    mscLun->sdev_data = (void *)BlkDev;

    if(mscLun->LunNo >= mscLun->mscDev->MaxLun - 1)
    {
        BlkDev->last_lun = 1;
        __wrn("disk, send last lun msg.........\n");
    }
    else
    {
        BlkDev->last_lun = 0;
    }
    __wrn("mscLun->LunNo=%d\n", mscLun->LunNo);
    __wrn("mscLun->mscDev->MaxLun=%d\n", mscLun->mscDev->MaxLun);
    __wrn("BlkDev->last_lun=%d\n", BlkDev->last_lun);

    /* ��ô�����Ϣ */
    GetDiskInfo(BlkDev);

    /* ע����豸 */
    if(!mscLun->MediaPresent)
    {
        ret = UsbBlkDevReg(BlkDev, DEV_CLASS_USERDEF, 0);
    }
    else
    {
        ret = UsbBlkDevReg(BlkDev, DEV_CLASS_DISK, 1);
    }

    if(ret != USB_ERR_SUCCESS)
    {
        DMSG_PANIC("ERR: DiskProbe: UsbBlkDevReg failed\n");
        return USB_ERR_REG_BLK_DEV_FAILED;
    }

    /* ����usb_monitor, scsi diskʶ���豸�ɹ� */
    {
        u32 is_reg = 1;

        usbm_sendcmd(DEV_IOC_USR_HWSC_USBH_MSC_DEV_REG_SET , &is_reg);
    }

    /*
         ֻ�п��ƶ��豸����Ҫ���MediaChange��������SataתUSB�ӿڵ��豸�
     �豸����Լ��ǲ����ƶ��豸����hostû�з�������ʱ���豸�����͹���ģʽ��
     �豸�ĵ����ת�ˡ���host���豸ʱ���豸�ĵ����ʼ�������ڴ�������ʱ������epstall��
     ���豸���и�λ���豸ͻȻ�Ͽ������ֻ��һֱ������test_unit_ready���������
     ����͹���ģʽ��
     */
    USB_OS_ENTER_CRITICAL(cpu_sr);
    mscLun->MediaChange = DiskMediaChange;
    USB_OS_EXIT_CRITICAL(cpu_sr);

    return USB_ERR_SUCCESS;
}

/*
*******************************************************************************
*                     UsbBlkDevOpen
*
* Description:
*
*
* Parameters:
*
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
__s32 DiskRemove(__mscLun_t *mscLun)
{
    __UsbBlkDev_t *BlkDev = NULL;
    __cpu_sr  cpu_sr;

    if(mscLun == NULL)
    {
        DMSG_PANIC("ERR: DiskRemove: input error\n");
        return -1;
    }

    BlkDev = (__UsbBlkDev_t *)mscLun->sdev_data;
    if(BlkDev == NULL)
    {
        DMSG_PANIC("ERR: BlkDev == NULL\n");
        return -1;
    }

    /* ע�����豸 */
    UsbBlkDevUnReg(BlkDev);

    /* ����usb_monitor scsi disk�豸�Ѿ�ע�� */
    {
        u32 is_reg = 0;

        usbm_sendcmd(DEV_IOC_USR_HWSC_USBH_MSC_DEV_REG_SET , &is_reg);
    }

    ShutDown(BlkDev);

    /* media change�߳��Ѿ�ֹͣ��, �������ڿ���ֱ��ɾ��MediaChange */
    USB_OS_ENTER_CRITICAL(cpu_sr);
    mscLun->MediaChange = NULL;
    USB_OS_EXIT_CRITICAL(cpu_sr);

    UsbBlkDevFree(BlkDev);
    mscLun->sdev_data = NULL;

    return USB_ERR_SUCCESS;
}


