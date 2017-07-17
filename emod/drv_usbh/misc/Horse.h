/*
********************************************************************************************************************
*
*
*                              (c) Copyright 2007-2010, javen.China
*										All	Rights Reserved
*
* File Name 	: horse.h
*
* Author 		: javen
*
* Version 		: 2.0
*
* Date 			: 2010.03.02
*
* Description 	: �ڴ�Ԥ����
*
* History 		:
*
********************************************************************************************************************
*/
#ifndef  __HORSE_H__
#define  __HORSE_H__

#define  MAX_HORSE_USER_NUM    10   /* ����û����� */


//---------------------------------------------------------------
//
//---------------------------------------------------------------
typedef  struct __HorseBlock
{
    __u32  is_used;			//�Ƿ�ռ��

    void *buff;			//
    __u32  size;			//buffer��С
} __HorseBlock_t;

typedef  struct __HorseHead
{
    __u32 used;				//�Ƿ�ռ��

    __u32 nr;				//block�ĸ���
    __HorseBlock_t *entry;  //block��������
} __HorseHead_t;

typedef struct __HorseMgr
{
    __HorseHead_t Horse[MAX_HORSE_USER_NUM];
} __HorseMgr_t;

//---------------------------------------------------------------
//
//---------------------------------------------------------------
void *HorseHeadInit(__u32 size, __u32 nr);
__s32 HorseHeadExit(void *hdle);
void *HorseBlockMalloc(void *hdle);
__s32 HorseBlockFree(void *hdle, void *buffer_addr);

__s32 HorseInit(void);
__s32 HorseExit(void);

#endif   //__HORSE_H__


