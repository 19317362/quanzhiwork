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
* File   : csi_linklist_manager.h
* Version: V1.0
* By     : Eric_wang
* Date   : 2010-1-4
* Description:
********************************************************************************
*/
#ifndef _CSI_LINKLIST_MANAGER_H_
#define _CSI_LINKLIST_MANAGER_H_
//����ʵ�ַ�ʽ2
/*******************************************************************************
���� ����ʵ�ַ�ʽ2 ��˵��:
(1). һ���߳�vdrv_task() ��һ���жϴ������csi_irq_handle()�����2������full2��free2
     �����Ҫ�����⡣
(2). ���ǵ�ISR�ǲ��ᱻ��ϵģ�����ֻ��Ҫ��vdrv_task()�����漰�ĺ��������⴦�������
    full2_insert( isr ), wt
    full2_delete( vdrv_task ), rd
    free2_insert( vdrv_task ), wt
    free2_delete( isr ), rd

    ���ԣ�ֻ��Ҫ��full2_delete()��free2_insert()�����⴦������ˡ���ν���⣬Ҳ����
    �ڴ���ǰ����һЩ���ܻᱻ�ı�ı������������ѡ�
*******************************************************************************/
#define FRMID_CNT (CSI_BUFFER_NUM+1)
typedef enum
{
    CSI_LISTTYPE_FREE = 0,
    CSI_LISTTYPE_FULL = 1,
} CSI_LINKLIST_TYPE;
typedef struct tag_CSI_LINKLIST_MANAGER __csi_linklist_manager_t;
typedef void    (*CSI_LINKLIST_MANAGER_Initial)     (__csi_linklist_manager_t *thiz, CSI_LINKLIST_TYPE type);
typedef __s32   (*CSI_LINKLIST_MANAGER_Insert)      (__csi_linklist_manager_t *thiz, __s32 frame_id);
typedef __s32   (*CSI_LINKLIST_MANAGER_Delete)      (__csi_linklist_manager_t *thiz);
typedef __s32   (*CSI_LINKLIST_MANAGER_Exit)        (__csi_linklist_manager_t *thiz);
typedef struct tag_CSI_LINKLIST_MANAGER
{
    CSI_LINKLIST_TYPE list_type;
    __s32 frmid_array[FRMID_CNT];  //��index�ŵ�����,  index��__csi_frame_t csi_frame[CSI_BUFFER_NUM]��index
    __s32 wt;
    __s32 rd;
    CSI_LINKLIST_MANAGER_Initial    initial;
    CSI_LINKLIST_MANAGER_Insert     insert_element;
    CSI_LINKLIST_MANAGER_Delete     delete_element;
    CSI_LINKLIST_MANAGER_Exit       exit;
} __csi_linklist_manager_t; //ֻ����ʹ��CSI_BUFFER_NUM��Ԫ�أ�����wt,rd�غ�ʱ������(�����ǿ�).
extern __csi_linklist_manager_t *csi_linklist_manager_init(void);

#endif  /* _CSI_LINKLIST_MANAGER_H_ */

