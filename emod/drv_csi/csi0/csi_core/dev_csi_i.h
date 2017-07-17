/*
********************************************************************************
*                                    eMOD
*                   the Easy Portable/Player Develop Kits
*                               mod_duckweed sub-system
*                          (module name, e.g.mpeg4 decoder plug-in) module
*
*          (c) Copyright 2010-2012, Allwinner Microelectronic Co., Ltd.
*                              All Rights Reserved
*
* File   : dev_csi_i.h
* Version: V1.0
* By     : Eric_wang
* Date   : 2010-1-4
* Description:
********************************************************************************
*/
#ifndef _DEV_CSI_I_H_
#define _DEV_CSI_I_H_

#include "csi_inc.h"
#include "csi.h"
#include "csi_linklist_manager.h"


typedef struct STRUCT_CSI_DEV
{
    __u32           status;
    __u32						csi_channel;	//ͨ����־
    __krnl_event_t  *csi_lock;    ///��CSI_DEV�������л����semaphore
    ES_FILE         *iic_file;

    __hdle  csi_pin;
    __hdle  csi_ahbclk;
    __hdle  csi_sdramclk;
    __hdle  csi_outclk;


    //__u32 frame_id_lowest = 0;
    __s32   csi_frame_num;  //��ʾ��Ч��csi_frame�ĸ���, <= CSI_BUFFER_NUM
    __csi_frame_t csi_frame[CSI_BUFFER_NUM];
    __csi_frame_t *p_csi_frame[CSI_BUFFER_NUM];
    __s32 frame_id_last;//�����һ��index
    __s32 frame_id_last_b;//
    __csi_linklist_manager_t    *full2; //����֡����,��Ϊװ���֡���Ϳ��е�֡
    __csi_linklist_manager_t    *free2;

    __s64       pts;  //��¼�õ��ĵ�ǰ��PTS
    __u32       pts_flag; //����PTS�Ļ�ȡ��ʽ��aux = 0:CSI�Լ���PTS; 1:ͨ���ص������õ�PTS; 2.����ҪPTS
    __pCBK_t    CB_GetPts;

    __csi_mode_t csi_mode_last;//���һ�ε����ã�ĳЩ������Ҫ�ݴ�����

} __csi_dev_t;

#endif  /* _DEV_CSI_I_H_ */

