/*
********************************************************************************************************************
*                                              usb host driver
*
*                              (c) Copyright 2007-2010, javen.China
*										All	Rights Reserved
*
* File Name 	: UsbBlkDev.h
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
#ifndef  __BLKDEV_H__
#define  __BLKDEV_H__

#include  "usbh_disk_info.h"

#define	 USB_BLK_DEV_MAGIC 				0x5a13d099
#define  USB_BULK_DISK_MAX_NAME_LEN		32

typedef struct __UsbBlkDev
{
    __u32   last_lun;               //���Ϊ1����ʾ�����һ������
    __u32	Magic;	             	/* ��ʾ�豸�Ƿ�Ϸ�					*/
    __mscLun_t *Lun;          		/* sd������scsi device���е����� 	*/

    /* Disk information */
    __u32 used;                     /* ���豸���� 					*/
    __dev_devop_t DiskOp;			/* �豸�������� 					*/

    /* Disk manager */
    USB_OS_HANDLE DevParaHdle;		/* openʱ�ľ��						*/
    USB_OS_HANDLE DevRegHdle;		/* regʱ�ľ�� 						*/

    __u32 DevNo;								/* ���豸��, ����host_id, target_id, lun ���	*/
    __u8 ClassName[USB_BULK_DISK_MAX_NAME_LEN];	/* �豸����, ��"disk" 				*/
    __u8 DevName[USB_BULK_DISK_MAX_NAME_LEN];	/* ���豸��, ��"SCSI_DISK_000"		*/

    __u32 is_RegDisk;              	/* �Ƿ�ע���disk�豸 							*/
    __u32 ErrCmdNr;           		/* test_unit_ready�ڼ�, δ֪����Ĵ��� 			*/

    void *Extern;					/* ��չ����, ��cd 								*/

    usbh_disk_device_info_t device_info;
} __UsbBlkDev_t;

//------------------------------------------
//
//------------------------------------------
__UsbBlkDev_t *UsbBlkDevAllocInit(__mscLun_t *mscLun);
__s32 UsbBlkDevFree(__UsbBlkDev_t *BlkDev);

void GetDiskInfo(__UsbBlkDev_t *BlkDev);
void ShutDown(__UsbBlkDev_t *BlkDev);

__s32 UsbBlkDevReg(__UsbBlkDev_t *BlkDev, __u8 *ClassName, __u32 RegDisk);
__s32 UsbBlkDevUnReg(__UsbBlkDev_t *BlkDev);


#endif   //__BLKDEV_H__


