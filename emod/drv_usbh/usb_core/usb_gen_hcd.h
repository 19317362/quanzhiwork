/*
*******************************************************************************
*                                            sun project ---- usb host module
*
*                             Copyright(C), 2006-2008, SoftWinners Co., Ltd.
*											       All Rights Reserved
*
* File Name :
*
* Author : GLHuang(HoLiGun)
*
* Version : 1.0
*
* Date : 2008.05.xx
*
* Description :
*
* History :
********************************************************************************************************************
*/

#ifndef	__USB_HCD_H__
#define	__USB_HCD_H__

#include "usb_host_common.h"


//USB Packet IDs (PIDs)

#define USB_PID_UNDEF_0			0xf0
#define USB_PID_OUT				0xe1
#define USB_PID_ACK				0xd2
#define USB_PID_DATA0			0xc3
#define USB_PID_PING			0xb4	/* USB 2.0 */
#define USB_PID_SOF				0xa5
#define USB_PID_NYET			0x96	/* USB 2.0 */
#define USB_PID_DATA2			0x87	/* USB 2.0 */
#define USB_PID_SPLIT			0x78	/* USB 2.0 */
#define USB_PID_IN				0x69
#define USB_PID_NAK				0x5a
#define USB_PID_DATA1			0x4b
#define USB_PID_PREAMBLE		0x3c	/* Token mode */
#define USB_PID_ERR				0x3c	/* USB 2.0: handshake mode */
#define USB_PID_SETUP			0x2d
#define USB_PID_STALL			0x1e
#define USB_PID_MDATA			0x0f	/* USB 2.0 */


/*-------------------------------------------------------------------------*/

/*
 * HCD Root Hub support
 */

//#include "hub.h"

/* (shifted) direction/type/recipient from the USB 2.0 spec, table 9.2 */
#define DeviceRequest \
	((USB_DIR_IN|USB_TYPE_STANDARD|USB_RECIP_DEVICE)<<8)
#define DeviceOutRequest \
	((USB_DIR_OUT|USB_TYPE_STANDARD|USB_RECIP_DEVICE)<<8)

#define InterfaceRequest \
	((USB_DIR_IN|USB_TYPE_STANDARD|USB_RECIP_INTERFACE)<<8)

#define EndpointRequest \
	((USB_DIR_IN|USB_TYPE_STANDARD|USB_RECIP_INTERFACE)<<8)
#define EndpointOutRequest \
	((USB_DIR_OUT|USB_TYPE_STANDARD|USB_RECIP_INTERFACE)<<8)

/* class requests from the USB 2.0 hub spec, table 11-15 */
/* GetBusState and SetHubDescriptor are optional, omitted */
#define ClearHubFeature		(0x2000 | USB_REQ_CLEAR_FEATURE)
#define ClearPortFeature	(0x2300 | USB_REQ_CLEAR_FEATURE)
#define GetHubDescriptor	(0xa000 | USB_REQ_GET_DESCRIPTOR)
#define GetHubStatus		(0xa000 | USB_REQ_GET_STATUS)
#define GetPortStatus		(0xa300 | USB_REQ_GET_STATUS)
#define SetHubFeature		(0x2000 | USB_REQ_SET_FEATURE)
#define SetPortFeature		(0x2300 | USB_REQ_SET_FEATURE)





#define	HC_GEN_DEV___ACTIVE		0x01
#define	HC_GEN_DEV___SUSPEND		0x04
#define	HC_GEN_DEV___TRANSIENT		0x80

#define	HC_GEN_DEV_STATE_HALT		0
#define	HC_GEN_DEV_STATE_RUNNING	(HC_GEN_DEV___ACTIVE)
#define	HC_GEN_DEV_STATE_QUIESCING	(HC_GEN_DEV___SUSPEND|HC_GEN_DEV___TRANSIENT|HC_GEN_DEV___ACTIVE)
#define	HC_GEN_DEV_STATE_RESUMING	(HC_GEN_DEV___SUSPEND|HC_GEN_DEV___TRANSIENT)
#define	HC_GEN_DEV_STATE_SUSPENDED	(HC_GEN_DEV___SUSPEND)
#define	HC_GEN_DEV_IS_RUNNING(state) ((state) & HC_GEN_DEV___ACTIVE)
#define	HC_GEN_DEV_IS_SUSPENDED(state) ((state) & HC_GEN_DEV___SUSPEND)

struct hc_gen_dev
{
    /* usb_bus.hcpriv points to this */
    struct usb_virt_bus		self;		/* hcd is-a bus */

    const char		*product_desc;	/* product/vendor string */
    char			irq_descr[24];	/* driver + bus # */

    //	struct timer_list	rh_timer;	//rh��timer
    USB_OS_KERNEL_SOFT_TIMER *rh_timer;  //�䵽ʱ������Ϊrh_timer_func()
    /* drives root-hub polling */
    struct urb		*status_urb;	//��ѯroot hub status��urb
    /* the current status urb */

    /*
     * hardware info/state
     */
    const struct hc_driver	*driver;	/* hw-specific hooks */
    u8 saw_irq ;
    u8 can_wakeup;		/* hw supports wakeup? */
    u8 remote_wakeup;	/* sw should use wakeup? */
    u8 rh_registered;	/* is root hub registered? */

    /* The next flag is a stopgap, to be removed when all the HCDs
     * support the new root-hub polling mechanism. */
    u8 uses_new_polling;
    u8 poll_rh;			/* poll for rh status? */
    u8 poll_pending;	/* status has changed? */
    int irq;			/* irq allocated */
    /* memory/io resource length */
    u32 power_budget;	/* in mA, 0 = no limit */
    int state;			//��HC_GEN_DEV_STATE_RUNNING

    /* more shared queuing code would be good; it should support
     * smarter scheduling, handle transaction translators, etc;
     * input size of periodic table to an interrupt scheduler.
     * (ohci 32, uhci 1024, ehci 256/512/1024).
     */

    /* The HC driver's private data is stored at the end of
     * this structure.
     */
    u32 hcd_private[1];
};


#define	HC_DRIVER_FLAG_HCD_MEMORY	0x0001		/* HC regs use memory (else I/O) */
#define	HC_DRIVER_FLAG_HCD_USB11	0x0010		/* USB 1.1 */
#define	HC_DRIVER_FLAG_HCD_USB2		0x0020		/* USB 2.0 */

#define	HUB_EVNET_PLUGIN_COMPLETE	0x01
#define	HUB_EVNET_PLUGOUT_COMPLETE	0x02

//*****************************
//����hc������ע��������
//�����������4��:
//�������²���:
//1���жϴ���
//2��start/stop
//3��queue/dequeue,ep en
//4��hub queue/dequeue ,suspend/resume
//*****************************
struct hc_driver
{
    const char	*description;	/* "ehci-hcd" etc */
    const char	*product_desc;	/* product/vendor string */
    u32 		hcd_priv_size;		//size of private data,��hc_priv�ṹ

    /* irq handler */
    u32	(*irq) (struct hc_gen_dev *hcd); //���Կ��Ƿŵ����棬��ʵ������ν��ֻ�Ǹ�irq����
    s32	flags;			//��HC_DRIVER_FLAG_HCD_USB2

    /* called to init HCD and root hub */
    s32	(*reset) (struct hc_gen_dev *hcd);
    s32	(*start) (struct hc_gen_dev *hcd);

    /* NOTE:  these suspend/resume calls relate to the HC as
     * a whole, not just the root hub; they're for bus glue.
     */
    s32	(*suspend)(struct hc_gen_dev *hcd); 	// called after all devices were suspended
    s32	(*resume)(struct hc_gen_dev *hcd);		// called before any devices get resumed
    void (*stop)(struct hc_gen_dev *hcd);		// cleanly make HCD stop writing memory and doing I/O
    s32	(*get_frame_number) (struct hc_gen_dev *hcd);	// return current frame number
    s32	(*urb_enqueue)(struct hc_gen_dev *hcd,
                       struct usb_host_virt_endpoint *ep,
                       struct urb *urb, unsigned mem_flags);
    //manage i/o requests, device state
    //��hcd_ops_urb_enqueue()

    s32	(*urb_dequeue) (struct hc_gen_dev *hcd, struct urb *urb);
    void (*endpoint_disable)(struct hc_gen_dev *hcd,
                             struct usb_host_virt_endpoint *ep);

    s32 (*hub_status_data) (struct hc_gen_dev *hcd, u8 *buf);
    s32 (*hub_control) (struct hc_gen_dev *hcd,
                        u16 typeReq,
                        u16 wValue,
                        u16 wIndex,
                        u8 *buf,
                        u16 wLength);
    s32	(*hub_suspend)(struct hc_gen_dev *);
    s32 (*hub_resume)(struct hc_gen_dev *);
    s32 (*start_port_reset)(struct hc_gen_dev *, u32 port_num);
    void (*hub_irq_enable)(struct hc_gen_dev *);

    /* Needed only if port-change IRQs are level-triggered */
    s32		(*hub_notify)(struct hc_gen_dev *, u32 event);	//such as HUB_EVNET_PLUGIN_COMPLETE
};

struct hc_gen_dev *usb_create_hc_gen_dev (const struct hc_driver *driver, const char *bus_name);
s32 usb_add_hc_gen_dev(struct hc_gen_dev *hcd, u32  irqnum, u32 irqflags);
void usb_remove_hc_gen_dev(struct hc_gen_dev *hcd);
void usb_hcd_poll_rh_status(struct hc_gen_dev *hcd);
void usb_hcd_giveback_urb (struct hc_gen_dev *hcd, struct urb *urb);

int hcd_ops_get_frame_number (struct usb_host_virt_dev *udev);
int hcd_ops_submit_urb (struct urb *urb, unsigned mem_flags);
int hcd_ops_unlink_urb (struct urb *urb, int status);
void *hcd_ops_buffer_alloc (struct usb_virt_bus  *bus, u32 size, u32 mem_flags, u32 *dma);
void hcd_ops_buffer_free (struct usb_virt_bus 	*bus, u32	size, u32 	*addr, u32 *dma);
void hcd_ops_endpoint_disable (struct usb_host_virt_dev *udev, struct usb_host_virt_endpoint *ep);

void usb_gen_hcd_init(void);
void usb_gen_hcd_exit(void);

#endif



