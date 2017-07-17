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
* File   : csi_linklist_manager.c
* Version: V1.0
* By     : Eric_wang
* Date   : 2010-1-4
* Description:
********************************************************************************
*/
#include "csi_inc.h"
#include "csi_linklist_manager.h"

extern __s32 verify_frame_id(__s32 frame_id);

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
void Impl_initial_csi_linklist_manager(__csi_linklist_manager_t *thiz, CSI_LINKLIST_TYPE type)
{
    //csi_memset(pManager, 0, sizeof(__csi_linklist_manager_t));
    thiz->list_type = type;
    thiz->wt = thiz->rd = 0;
    return;
}
__s32 Impl_csi_linklist_manager_exit(__csi_linklist_manager_t *thiz)
{
    csi_memset(thiz, 0, sizeof(__csi_linklist_manager_t));
    csi_free(thiz);
    return 0;
}

/*******************************************************************************
Function name: full2_insert
Description:
    ���װ��֡�����������������
    isr����
    �޸�wt
    rd��vdrv_task()�޸�
Parameters:
    1. idx:��������__csi_frame_t csi_frame[CSI_BUFFER_NUM]�� frame_id
Return:

Time: 2010/7/12
*******************************************************************************/
__s32 Impl_csi_linklist_manager_insert(__csi_linklist_manager_t *thiz, __s32 frame_id)
{
    //__u32 cpu_sr;
    //__s32 full2_wt;
    //__s32 full2_rd;
    //__s32 uTmpRd;
    //__s32 uNextRd;
    __s32 uTmpWt = thiz->wt + 1;

    uTmpWt %= FRMID_CNT;
    if(thiz->rd == uTmpWt)
    {
        __wrn("fatal error, [%d] array full!\n", thiz->list_type);
    }
    if(EPDK_OK != verify_frame_id(frame_id))
    {
        __wrn("full2_insert error\n");
        return EPDK_FAIL;
    }
    thiz->frmid_array[thiz->wt] = frame_id;
    thiz->wt = uTmpWt;

    return EPDK_OK;
}
/*******************************************************************************
Function name: full2_delete
Description:
    ȡһ��Ԫ�س�����
    vdrv_task()���á����ܻ�û��Ԫ�ء�
    �޸�rd,
    wt��ISR�޸�
Parameters:

Return:
    1.���û��Ԫ��, ����-1
    2.����У�����id�š�
Time: 2010/7/12
*******************************************************************************/
__s32 Impl_csi_linklist_manager_delete(__csi_linklist_manager_t *thiz)
{
    __s32 frame_id;
    __s32 nTmpRd = thiz->rd;
    if(thiz->rd == thiz->wt)
    {
        return -1;
    }
    else
    {
        //__u32 cpu_sr;
        //__s32 full2_wt;
        //__s32 full2_rd;
        frame_id = thiz->frmid_array[nTmpRd++];
        nTmpRd %= FRMID_CNT;
        thiz->rd = nTmpRd;

        return frame_id;
    }
}
__csi_linklist_manager_t *csi_linklist_manager_init()
{
    __csi_linklist_manager_t *p = (__csi_linklist_manager_t *)csi_malloc(sizeof(__csi_linklist_manager_t));
    if(NULL == p)
    {
        __wrn("malloc __csi_linklist_manager_t fail\n");
        return NULL;
    }
    csi_memset(p, 0, sizeof(__csi_linklist_manager_t));
    p->initial = Impl_initial_csi_linklist_manager;
    p->insert_element = Impl_csi_linklist_manager_insert;
    p->delete_element = Impl_csi_linklist_manager_delete;
    p->exit = Impl_csi_linklist_manager_exit;
    return p;
}

