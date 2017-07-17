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
*                              Copyright(C), 2006-2008, SoftWinners Co., Ltd.
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
#ifndef	__LIST_HEAD_UTILS_H__
#define	__LIST_HEAD_UTILS_H__

//��list_header_input��ɾ��ĳ��node
//return :	0	//��ʾ�ҵ��˸�node���ҳɹ�ɾ����
//��list�Ĳ�����critical����
int list_head_ext_remov_node_from_list(void *node_data, struct list_head *list_header_input);

//������init������������������
#define	list_head_ext_for_each_entry_init()		\
	struct list_head * list_head_ext_start_ = NULL;		\
	struct list_head *  list_head_ext_now_ = NULL ;

//����������,��ǰ��Ϊlist_head_ext_now_
#define	list_head_ext_for_each_entry_process( p_start_list_head)		\
	for(list_head_ext_start_ = (p_start_list_head),list_head_ext_now_ = (p_start_list_head)->next;\
		list_head_ext_now_ != list_head_ext_start_ ;\
		list_head_ext_now_ = list_head_ext_now_->next)

/* ����list, ���Ƿ����ֵΪdata��node */
s32 list_node_exist(void *data, struct list_head *list_head);
s32 list_del_node_by_data(void *data, struct list_head  *list);
s32 list_destroy_whole_list(struct list_head  *list);


#endif





