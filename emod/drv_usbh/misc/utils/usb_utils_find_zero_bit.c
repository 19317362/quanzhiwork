/*
*******************************************************************************
*                                    usb host module
*
*                             Copyright(C), 2006-2008, SoftWinners Co., Ltd.
*											       All Rights Reserved
*
* File Name :
*
* Author : javen
*
* Version : 1.0
*
* Date : 2008.07.xx
*
* Description :
*
* History :
*******************************************************************************
*/
#include "usb_host_config.h"

#include "usb_host_base_types.h"





#define FIND_NEXT_ZERO_BIT_MAX_SIZE		(128)
#define FIND_NEXT_ZERO_BIT_NOT_FOUND		(0xffffffff)


/*
******************************************************************************
*
*             find_next_zero_bit
*
* Description:
*     �ڵ�ַ������n��int�ֽ��У���ָ��bitλ��ʼ���ҵ�һ��Ϊ0��bit��λ.
*
* Parameters:
*     addr    :   ��ʼ��ַ.
*     size    :   n���ֽڵ��ܳ���,(32 * n).
*     offset  :   ��ʼ���ҵ�bitλ.
*
*  Return value:
*     ���ҳɹ������ز��ҵ��ĵ�һ��Ϊ0��bitλ������ʧ�ܣ��򷵻�0xffffffff��
*
******************************************************************************
*/
u32 find_next_zero_bit(const volatile u32 *addr, u32 size, u32 offset)
{
    u32 *p = (u32 *) addr ;
    u32 bit = FIND_NEXT_ZERO_BIT_NOT_FOUND;
    int k;		//�ֽ��е�ƫ����
    int i, j;
    int n;		//int�ֽڵĸ���
    int m;		//�����ֽڵ�λ��

    n = size / 32;
    k = 32 - (offset % 32);
    m = offset / 32;

    //offsetΪ0 ~ size-1
    if(size > FIND_NEXT_ZERO_BIT_MAX_SIZE || offset >= size || m > n)
        return bit;

    //��offsizeλ����ǰ���ң�ֱ����β
    for(j = 0; j < (n - m); j++)
    {
        if(j == 0) //offsize���ڵ��ֽ�
        {
            for(i = 0; i < k; i++)
            {
                if(((*(p + m + j) >> ((offset % 32) + i))) & 0x01)
                    continue;
                else
                {
                    bit = (offset + i);
                    return bit;
                }
            }
        }
        else  //offsize�����ֽڵ���j���ֽ�
        {
            for(i = 0; i < 32; i++)
            {
                if((*(p + m + j) >> i) & 0x01)
                    continue;
                else
                {
                    //					bit = (offset+(j*32)+i);
                    bit = ((j * 32) + i);
                    return bit;
                }
            }
        }
    }

    //��addrλ����ǰ���ң�ֱ��offsize��ֹͣ
    for(j = 0; j <= m; j++)
    {
        if(j != m) //��addrλ����ǰ����
        {
            for(i = 0; i < 32; i++)
            {
                if(((*(p + j) >> i)) & 0x01)
                    continue;
                else
                {
                    bit = i + (j * 32);
                    return bit;
                }
            }
        }
        else  //offsize���ڵ��ֽ�
        {
            for(i = 0; i < (32 - k); i++)
            {
                if(((*(p + j) >> i)) & 0x01)
                    continue;
                else
                {
                    bit = i + (j * 32);
                    return bit;
                }
            }
        }
    }

    return bit;
}

