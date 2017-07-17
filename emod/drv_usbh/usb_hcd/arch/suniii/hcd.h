/*
********************************************************************************************************************
*                                              USB Host Drvier
*
*                              (c) Copyright 2007-2009, softwinners.China
*										All	Rights Reserved
*
* File Name 	: hcd.h
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

#ifndef  __HCD__
#define  __HDC__

#include "usb_list.h"
#include "usb_host_common.h"
#include "usb_gen_hcd.h"


#define	 HCD_MOV_DATA_BY_DMA			//����dma����
#define  HCD_FIFO_NUM				0  	//���ڽ���HUB�󣬻����ܶ��豸������USB��FIFO��С���ޣ����ֻ���õ�FIFO
//#define  HCD_INT_ISO_TRANSFER			//�Ƿ񿪷��жϴ��䡢ͬ������



#define  HCD_CONTROLLER_NAME	"usb host controller"


//------------------------------------------------------------------------------
//  USB Host Controller Driver����
//------------------------------------------------------------------------------

#define  FULLSPEED     0
#define  HIGHSPEED     1
#define  LOWSPEED      2

//port change
#define  PORT_C_MASK  	((USB_PORT_STAT_C_CONNECTION \
							| USB_PORT_STAT_C_ENABLE \
							| USB_PORT_STAT_C_SUSPEND \
							| USB_PORT_STAT_C_OVERCURRENT \
							| USB_PORT_STAT_C_RESET) << 16)

#define	 LOG2_PERIODIC_SIZE				5	/* arbitrary; this matches OHCI */
#define	 PERIODIC_SIZE					(1 << LOG2_PERIODIC_SIZE)
#define  MAX_PERIODIC_LOAD				500 //50%

#define  FRAME_TIME_USECS	1000L
#define  BitTime(bytecount)  (7 * 8 * bytecount / 6)  	/* with integer truncation */
/* Trying not to use worst-case bit-stuffing
           of (7/6 * 8 * bytecount) = 9.33 * bytecount */
/* bytecount = data payload byte count */
/*
 * Full/low speed bandwidth allocation constants/support.
 */
#define BW_HOST_DELAY	1000L		/* nanoseconds */
#define BW_HUB_LS_SETUP	333L		/* nanoseconds */
/* 4 full-speed bit times (est.) */

#define FRAME_TIME_BITS         12000L		/* frame = 1 millisecond */
#define FRAME_TIME_MAX_BITS_ALLOC	(90L * FRAME_TIME_BITS / 100L)
#define FRAME_TIME_MAX_USECS_ALLOC	(90L * FRAME_TIME_USECS / 100L)

/*
 * Ceiling [nano/micro]seconds (typical) for that many bytes at high speed
 * ISO is a bit less, no ACK ... from USB 2.0 spec, 5.11.3 (and needed
 * to preallocate bandwidth)
 */
#define USB2_HOST_DELAY	5	/* nsec, guess */
#define HS_NSECS(bytes) ( ((55 * 8 * 2083)/1000) \
							+ ((2083UL * (3167 + BitTime (bytes)))/1000) \
							+ USB2_HOST_DELAY)
#define HS_NSECS_ISO(bytes) ( ((38 * 8 * 2083)/1000) \
							+ ((2083UL * (3167 + BitTime (bytes)))/1000) \
							+ USB2_HOST_DELAY)

/* ep list ���� */
#define	HCD_EP_LIST_MAX_EP_NUM			(1 + USBC_MAX_EP_NUM * 2)
#define	HCD_EP_LIST_EP0_INDEX			0

#define	HCD_EP_LIST_IN_BEGIN			1
#define	HCD_EP_LIST_IN_END				(HCD_EP_LIST_IN_BEGIN + USBC_MAX_EP_NUM - 1)
#define	HCD_EP_LIST_OUT_BEGIN			(HCD_EP_LIST_IN_END + 1)
#define	HCD_EP_LIST_OUT_END				(HCD_EP_LIST_OUT_BEGIN + USBC_MAX_EP_NUM - 1)

/* ep list �� ep index �� ���� ep ��ת�� */
#define ep_index_list_2_phy(is_out, list_index)   	(is_out ? (list_index - HCD_EP_LIST_IN_END) : (list_index))
#define ep_index_phy_2_list(is_out, phy_index) 		(is_out ? (phy_index + HCD_EP_LIST_IN_END) : (phy_index))


#define  EP_PRIV_MASK_DIR_IN			0x10 	//��ʵ���Ǹ�4bit����ʾ����
#define  EP_PRIV_MASK_DIR_OUT 			0x00
#define  get_phy_ep_index(ep_priv)		((ep_priv)->mask_dir_local_ep_nr & 0x0f)
#define  is_in_ep(ep_priv)				((ep_priv)->mask_dir_local_ep_nr & EP_PRIV_MASK_DIR_IN)

//ctrl data�׶εķ���
#define  EP_PRIV_EP_DATA_DIR_NULL		0x00
#define  EP_PRIV_EP_DATA_DIR_IN			0x01
#define  EP_PRIV_EP_DATA_DIR_OUT		0x02

enum rh_state
{
    RH_POWEROFF = 0,
    RH_POWERED,
    RH_ATTACHED,
    RH_NOATTACHED,
    RH_RESET,
    RH_ENABLE,
    RH_DISABLE,
    RH_SUSPEND,
    RH_ERR
};


/* ����ep����, ��������һ������ep. ͬһʱ��һ��ep��Ӧһ��urb */
typedef struct ep_private
{
    struct usb_host_virt_endpoint *hep;	/* core���ep 								*/
    struct usb_host_virt_dev	*udev;	/* core���device 							*/
    struct hc_private *hc;				/* ep��Ӧ��host controller					*/
    struct urb *current_urb;	/* ��ǰ���ڱ������urb, ��������ʱ��Ѹ�ٴ���urb	*/

    u8  ep_list_index;			/* ����ep��Ϣ����hc_priv->ep_list[]�е�index 		*/
    u8	mask_dir_local_ep_nr;	/* ����ep��Ϣ�� [4:0]: ep index, [7]: ep direction 	*/
    u8  defctrl;				/* Ĭ�Ͽ��� 									*/

    u32	maxpacket;				/* ep ���� 									*/
    u8	nextpid;				/* flag. ��¼��һ��ep�Ķ��� 					*/
    u8	stopped;				/* flag. ep �Ƿ�ֹͣ����						*/
    u16	error_count;			/* ep ����Ĵ��� 								*/
    u16	nak_count;				/* NAK �Ĵ��� 									*/
    u32	length;					/* ep һ�δ���ĳ���, ��Ŀ�����ж��Ƿ���ڶ̰�	*/

    /* ep ��������Ϣ */
    __u8  ep_index;				/* local ep point index 						*/
    __u8  ep_num;				/* target device ep point index 				*/
    __u8  ep_ts_type;			/* ep ��������. ctrl, iso, int, bulk 			*/
    __u32 ep_fifo_start;		/* ep fifo start address 						*/
    __u32 ep_fifo_size;			/* ep fifo�Ĵ�С 								*/
    __u32 ep_fun_addr;			/* ep function address 							*/
    __u32 ep_target_muiltTT;	/* ep��Ӧ���豸��muiltTT HUB 					*/
    __u32 ep_hub_addr;			/* ep hub address 								*/
    __u32 ep_hub_port;			/* ep hub port address 							*/
    __u32 ep_interval;			/* {rx,tx} interval register 					*/
    //	__u32 ep_host_speed;		/* ep ��Ӧ�� host �˵��ٶ� 						*/
    //	__u32 ep_target_speed;		/* ep ��Ӧ�� �豸 �˵��ٶ� 						*/

    //periodic schedule//
    u16	period;					/*  											*/
    u16	branch;					/*  											*/
    u16	load;					/*  											*/
    u8	ep0_data_dir;			/* ep0 ���ݽ׶εķ��� 							*/
    struct ep_private *next;	/*  											*/

    //async schedule //
    u8	is_added_schedule;		/*  											*/

    volatile u8	dma_working;	/* flag. DMA is working? 						*/
    u32	dma_bytes;				/* DMA��������ݳ���  							*/

#ifdef HCD_INT_ISO_TRANSFER
    __u16 periodic_done_counter;  /* high bandwidth 							*/
#endif

    __u32 busy;					/* ep���ڱ����� 								*/
} ep_private_t;

/* fifo  ����  */
typedef struct tag_fifo_map
{
    u32 start;
    u32 end;
} fifo_map_t;

/* USB Host Controller */
#define	RHUB_EVNET_NULL			0x00
#define	RHUB_EVNET_PLUGIN_ACT	0x01
#define	RHUB_EVNET_PLUGOUT_ACT	0x02


/* ��host������������ */
typedef struct hc_private
{
    __cpu_sr lock;
    struct hc_gen_dev *point_gen_dev;	/* ָ��hc_gen_dev						*/

    //ͳ����Ϣ
    u32	stat_insrmv;					/*	����γ�							*/
    u32	stat_sof;						/*										*/
    u32	stat_lost;						/*										*/
    u8	speed;							/*										*/
    u32 port;							/* ��16Ϊport change, ��16Ϊport status	*/

    enum rh_state  rhstate; 			/* root hub state						*/

    struct ep_private *ep_list[HCD_EP_LIST_MAX_EP_NUM];

    u32	fifo_point;						/* ��Ŀǰfifo�Ѿ������offset			*/
    u32 fifo_total_size;                /* fifo��С								*/
    fifo_map_t fifo_alloc_map[HCD_EP_LIST_MAX_EP_NUM ];  /* ������¼����ep��fifo������� */

    struct list_head async; 			/* ����async�������, ���ep_private	*/

    struct ep_private *curr_async;		/* current ep_private 					*/
    struct list_head *curr_aync_node;	/* ָ��curr_aync�ڵ�,����Ѱ����һ��node	*/

    u32 root_hub_plug_event;			/* such as RHUB_EVNET_PLUGIN_ACT 		*/
    u32	plug_event_is_proc_comp;		/* �Ƿ������ 						*/
    u32 last_not_proc_event;			/*										*/

#ifdef HCD_INT_ISO_TRANSFER
    struct ep_private *next_periodic;
    struct ep_private *next_async;
    struct ep_private *sof_periodic_entry;
    u16 load[PERIODIC_SIZE];
    __u32 periodic_count;
    __u32 frame;
    __u32 stat_overrun;
    __u32 urb_counter;		/* for sof enable or disable */

#endif
    struct ep_private *periodic[PERIODIC_SIZE];
} hc_private_t;



static inline struct hc_private *hcd_to(struct hc_gen_dev *hcd)
{
    return (struct hc_private *) (hcd->hcd_private);
}

static inline struct hc_gen_dev *to_hcd(struct hc_private *hc_priv)
{
    return (hc_priv != NULL) ? hc_priv->point_gen_dev : NULL;
}

typedef struct tag_hc_int_pendding
{
    u32 int_pendding_tx;
    u32 int_pendding_rx;
    u32 int_pendding_misc;
} hc_int_pendding_t;

/* �Ƿ����õ�ַ�ɹ� */
#define	HAS_SET_ADDR_HAPPEND_FAIL		0x00
#define	HAS_SET_ADDR_HAPPEND_TRUE		0x01

typedef struct hc_priv_env_s
{
    hc_int_pendding_t int_pendding;
    u32 has_set_addr_happend;
    u32 new_addr;
} hc_priv_env_t;



#endif   //__HDC__




