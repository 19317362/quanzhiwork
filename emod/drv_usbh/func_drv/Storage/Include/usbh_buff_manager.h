/*
********************************************************************************************************************
*                                              usb_host
*
*                              (c) Copyright 2007-2009, holi.China
*										All	Rights Reserved
*
* File Name 	: msc_buff_manager.h
*
* Author 		: javen
*
* Version 		: 2.0
*
* Date 			: 2009.08.19
*
* Description 	:
*
* History 		:
*     v1.0  holi  2008.11.22 - ��д�ٶȿ�, ����ֻ֧�ֵ���lun
*     v2.0  javen 2009.08.19 - ֧�ֶ��lun �� �������͵��豸
********************************************************************************************************************
*/
#ifndef	_USBH_BUFF_MANAGER_H_
#define	_USBH_BUFF_MANAGER_H_

#include "usb_host_base_types.h"


/* usb host ��ʱ buffer */
typedef struct tag_usbh_temp_buff
{
    u32 num;          	/* buffer������buffer��������ĺ��� 	*/

    u32 dev_id;       	/* �豸id, ȷ���豸��ݵ�Ψһ��ʶ 		*/
    u32 start_lba;    	/* ��ʼ���� 							*/
    u32 end_lba;      	/* ��������, ���һ����Ч���� 			*/
    u32 sector_size;  	/* ������С 							*/

    u32 used_time;      /* ���ļ�ϵͳʹ�õĴ���               	*/

    void *buff;       	/* ��Ŵ��豸��ȡ������ 				*/
    u32 buff_len;     	/* buffer�Ĵ�С 						*/

    u32 is_valid;    	/* ���buff�������Ƿ���Ч 				*/
    u32 is_busy;      	/* ��ֹbuff���ڽ���device���ݣ�����
	                       �ֱ�usbh_buff_manager����Ϊ��Ч 		*/
} usbh_temp_buff_t;


#define  USBH_TEMP_BUFFER_MAX_NUM    8   			//buffer�ĸ���, �ȷ�4�����������ټ�
#define  USBH_TEMP_BUFFER_MAX_LEN    (32 * 1024)	//32k, ������k�ı���, ��ΪҪpalloc

typedef struct tag_usbh_buff_manager
{
    u32 temp_buff_len;    /* ��ʱbuff�Ĵ�С, ��������usbh_temp_buff_t��buff_len�Ĵ�С */
    u32 temp_buff_nr;  	  /* ��ʱbuff�ĸ��� */

    usbh_temp_buff_t buff_array[USBH_TEMP_BUFFER_MAX_NUM];
} usbh_buff_manager_t;


//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
s32 usbh_msc_special_read(void *pBuffer,
                          __u32 blk,
                          __u32 n,
                          USB_OS_HANDLE hDev,
                          u32 dev_id,
                          u32 secter_size,
                          void *blk_read_entry);

s32 usbh_msc_special_write(void *pBuffer,
                           __u32 blk,
                           __u32 n,
                           USB_OS_HANDLE hDev,
                           u32 dev_id,
                           u32 secter_size,
                           void *blk_write_entry);

s32 set_usbh_temp_buff_invalid_by_dev(u32 dev_id);
s32 set_all_usbh_temp_buff_invalid(void);

s32 init_usbh_buff_manager(void);
s32 exit_usbh_buff_manager(void);

#endif





