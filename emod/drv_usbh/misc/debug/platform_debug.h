/*
********************************************************************************************************************
*                                              USB Host Drvier
*
*                              (c) Copyright 2007-2009, softwinners.China
*										All	Rights Reserved
*
* File Name 	: platform_debug.h
*
* Author 		: javen
*
* Version 		: 1.0
*
* Date 			: 2009.10.12
*
* Description 	:
*
* History 		:
*
********************************************************************************************************************
*/
#ifndef  __PLATFORM_DEBUG_H__
#define  __PLATFORM_DEBUG_H__

/* ����malloc��free */
typedef struct __memory_debug
{
    __u32 nr;			/* malloc�Ĵ��� */

    __u32 *addr;		/* malloc�ĵ�ַ */
    __u8 file[256];		/* �ļ��� 		*/
    __u32 line;			/* �к� 		*/
} __memory_debug_t;


void *pf_malloc(u32 size, u8 *file_name , u32 line_nr);
void pf_free(void *addr);

__s32 platform_debug_init(void);
__s32 platform_debug_exit(void);

#endif   //__PLATFORM_DEBUG_H__


