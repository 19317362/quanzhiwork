/*
********************************************************************************************************************
*                                              usb host driver
*
*                              (c) Copyright 2007-2010, javen.China
*										All	Rights Reserved
*
* File Name 	: usb_msc_i.h
*
* Author 		: javen
*
* Version 		: 2.0
*
* Date 			: 2010.03.02
*
* Description 	:
*
* History 		:
*
********************************************************************************************************************
*/
#ifndef  __USB_MSC_I_H__
#define  __USB_MSC_I_H__

#include  "usb_list.h"
#include  "list_head_ext.h"

/* Sub Classes */
#define USBMSC_SUBCLASS_RBC				0x01		/* Typically, flash devices */
#define USBMSC_SUBCLASS_8020			0x02		/* CD-ROM 					*/
#define USBMSC_SUBCLASS_QIC				0x03		/* QIC-157 Tapes 			*/
#define USBMSC_SUBCLASS_UFI				0x04		/* Floppy 					*/
#define USBMSC_SUBCLASS_8070			0x05		/* Removable media 			*/
#define USBMSC_SUBCLASS_SCSI			0x06		/* Transparent 				*/
#define USBMSC_SUBCLASS_ISD200    		0x07		/* ISD200 ATA 				*/

#define USBMSC_SUBCLASS_MIN				USBMSC_SUBCLASS_RBC
#define USBMSC_SUBCLASS_MAX				USBMSC_SUBCLASS_ISD200

/* Protocol */
#define USBMSC_INTERFACE_PROTOCOL_CBIT	0x00	/* Control/Bulk/Interrupt 		*/
#define USBMSC_INTERFACE_PROTOCOL_CBT	0x01	/* Control/Bulk w/o interrupt 	*/
#define USBMSC_INTERFACE_PROTOCOL_BOT	0x50	/* bulk only 					*/

/* Ԥ���� */
struct __mscDev;
struct __ScsiCmnd;
struct __mscLun;


#define  MSC_MAX_LUN  			15 		/* mscЭ��涨���֧��15��Lun 		*/
#define  SCSI_INQUERY_LEN   	36 		/* ��׼inquery����صĶ���36�ֽ� 	*/
#define  SCSI_MAX_INQUERY_LEN   44 		/* ��׼inquery����صĶ���36�ֽ� 	*/
#define  MSC_IOBUF_SIZE			64


/* mscDev״̬ */
typedef enum __mscDev_state
{
    MSC_DEV_OFFLINE = 0,			/* mscDev�Ѿ��γ� 		*/
    MSC_DEV_ONLINE,				/* mscDev�Ѿ���� 		*/
    MSC_DEV_DIED,				/* mscDev������ 		*/
    MSC_DEV_RESET				/* mscDev���ڱ�reset 	*/
} mscDev_state_t;

typedef __s32 (* msc_ResetRecovery)(struct __mscDev *mscDev);
typedef __s32 (* msc_Transport)(struct __mscDev *mscDev, struct __ScsiCmnd *ScsiCmnd);
typedef __s32 (* msc_StopTransport)(struct __mscDev *mscDev);
typedef __s32 (* msc_ProtocolHandler)(struct __mscDev *mscDev, struct __ScsiCmnd *ScsiCmnd);


/* mscDev�ĳ���, ������һ��mscDevӵ�е���Դ */
typedef struct __mscDev
{
    struct usb_host_virt_dev *pusb_dev;	/* mscDev ��Ӧ��Public USB Device 	*/
    struct usb_interface	*pusb_intf;	/* Public usb interface 			*/

    /* device information */
    __u8 InterfaceNo;			/* �ӿں� 								*/
    __u8 *transport_name;		/* ����Э������� 						*/
    __u8 *Protocol_name;		/* ����汾 							*/
    __u8 SubClass; 				/* ���� 								*/
    __u8 Protocol; 				/* ����Э�� 							*/
    __u8 MaxLun;				/* ���Lun���� 							*/
    __u32 mscDevNo; 			/* mscDev�ı��							*/
    __u32 lun_num; 				/* mscDevӵ�е�lun�ĸ���				*/

    /* device manager */
    struct __mscLun *Lun[MSC_MAX_LUN];		/* ������е�Lun 				*/
    struct list_head cmd_list;				/* ������� 					*/
    USB_OS_KERNEL_EVENT *scan_lock;			/* ����ʵ��scan ��remove�Ļ��� 	*/
    USB_OS_KERNEL_EVENT *DevLock;			/* ͬһʱ��ֻ����һ���˷���mscDev*/
    USB_OS_KERNEL_EVENT *ThreadSemi;		/* �߳��ź���		 			*/
    USB_OS_KERNEL_EVENT *notify_complete;	/* ͬ��thread����/ɾ�� 			*/
    mscDev_state_t state;					/* mscDev��ǰ����������״̬ 	*/

    __u8 MainThreadId;						/* ���߳�ID						*/
    __u8 MediaChangeId;						/* media change�����߳�ID		*/

    __u32 SuspendTime;						/* Suspend�豸�����ʱ�� 		*/

    /* command transport */
    __u32 BulkIn;							/* bulk in  pipe 				*/
    __u32 BulkOut;							/* bulk out pipe 				*/
    __u32 CtrlIn; 							/* ctrl in  pipe				*/
    __u32 CtrlOut; 							/* ctrl out pipe				*/
    __u32 IntIn;							/* interrupt in pipe 			*/
    __u8  EpInterval;						/* int ����������ѯ�豸�����ڣ�Bulk������Դ�ֵ */

    __u32 Tag;								/* SCSI-II queued command tag 	*/
    __u32 busy;								/* �����Ƿ����ڴ������� 		*/
    struct __ScsiCmnd *ScsiCmnd;			/* current srb					*/
    struct urb *CurrentUrb;					/* USB requests	 				*/
    USB_OS_KERNEL_EVENT *UrbWait;			/* wait for Urb done 			*/
    struct usb_ctrlrequest *CtrlReq;		/* control requests	 			*/
    __u8 *iobuf;							/* mscDev������, CBW��CSW���õõ�*/

    USB_OS_KERNEL_SOFT_TIMER *TimerHdle;	/* timer ��� 					*/

    msc_ResetRecovery 	ResetRecovery;		/* USB Controller reset 		*/
    msc_Transport    	Transport;			/* msc�豸֧�ֵĴ��䷽ʽ 		*/
    msc_StopTransport 	StopTransport;		/* ��ֹmsc���� 					*/
    msc_ProtocolHandler ProtocolHandler; 	/* msc���� 						*/
} __mscDev_t;

typedef struct __disk_info_t
{
    __u32 capacity;				/* �豸��������������Ϊ��λ			*/
    __u32 sector_size;			/* �����Ĵ�С 						*/
} disk_info_t;

typedef enum __mscLun_state
{
    MSC_LUN_DEL = 0,			/* mscDev������ 	*/
    MSC_LUN_ADD,			/* mscDev�Ѿ���� 	*/
    MSC_LUN_CANCEL			/* mscDev�Ѿ��γ� 	*/
} mscLun_state_t;


typedef __s32 (* LunProbe)(struct __mscLun *);		/* lunʶ�� 	*/
typedef __s32 (* LunRemove)(struct __mscLun *);		/* lunж��  */
typedef void (* LunMediaChange)(struct __mscLun *);	/* media change��Ĵ��� */


/* Lun�ĳ���, ������һ��Lunӵ�е���Դ */
typedef struct __mscLun
{
    __mscDev_t *mscDev;				/* Lun������mscDev 						*/
    __u32 LunNo; 					/* lun�ı��							*/

    /* device information */
    __u32 DeviceType;				/* �豸������ 							*/
    __u32 MediumType;				/* �������ͣ�CD_ROMö��ʱ���� 			*/
    __u8  DiskSubClass;				/* ��������						 		*/
    __u32 ScsiLevel;				/* scsi����汾 						*/
    __u8 Inquiry[SCSI_MAX_INQUERY_LEN];	/* ���inquire�����õ��豸��Ϣ 		*/
    __u8 *Vendor;					/* ��Ӧ�� 								*/
    __u8 *Product;			    	/* ��Ʒ��Ϣ								*/
    __u8 *Revision;					/* ��Ʒ���к� 							*/

    /* Lun���� */
    disk_info_t disk_info;			/* ������Ϣ 							*/

    __u32 WriteProtect: 1;			/* �Ƿ��ǿ�д�豸	 					*/
    __u32 RemoveAble: 1;				/* �Ƿ��ǿ��ƶ��豸 					*/
    __u32 Changed: 1;				/* �豸�Ƿ�ı�� 						*/
    __u32 MediaPresent: 1;			/* �豸�����Ƿ���� 					*/
    __u32 WCE: 1;					/* cacheд���� 							*/
    __u32 RCD: 1;					/* cache������ 							*/
    __u32 use_10_for_rw: 1;			/* first try 10-byte read / write 		*/
    __u32 use_10_for_ms: 1; 			/* first try 10-byte mode sense/select	*/
    __u32 skip_ms_page_8: 1;			/* do not use MODE SENSE page 0x08 		*/
    __u32 skip_ms_page_3f: 1;		/* do not use MODE SENSE page 0x3f 		*/
    __u32 use_192_bytes_for_3f: 1; 	/* ask for 192 bytes from page 0x3f 	*/

    USB_OS_KERNEL_EVENT *Lock;   	/* �ź�����ÿ��ֵ��֤һ�����ڶ���д */

    LunProbe   		Probe;
    LunRemove  		Remove;
    LunMediaChange 	MediaChange;

    void *sdev_data;				/* ָ��top level��scsi disk�����ݽṹ	*/
} __mscLun_t;

/* �������ݵķ��� */
typedef enum scsi_data_direction
{
    DATA_NONE = 0,			/* No Data 					*/
    DATA_TO_DEVICE,			/* Data Out. host->device 	*/
    DATA_FROM_DEVICE,		/* Data in. device->host 	*/
} scsi_data_direction_t;

/* command block�ĳ��� */
typedef struct __scsi_transport_cmd
{
    scsi_data_direction_t data_direction;	/* IN - DATA_IN or DATA_OUT 		*/
    __u32 Tag;						/* SCSI-II queued command tag 				*/
    __u32 Timeout;      			/* IN - Timeout for this command Block 		*/
    __u32 dwLun;        			/* IN - Logical Number for Logic Device. 	*/
    __u32 CBLen;					/* The length of command block				*/
    void *CommandBlock; 			/* IN - Pointer to the command block buffer.*/
} __scsi_transport_cmd_t;

typedef void (*LunDone) (struct __ScsiCmnd *);	/* mscDev���ã�Lun cmd�������	*/

/* ScsiCmnd�ĳ���, ������һ��ScsiCmndӵ�е���Դ */
typedef struct __ScsiCmnd
{
    __mscLun_t *sc_lun;						/* �����������豸 						*/
    //	struct list_head list;      			/* ��������device��cmd_list�е�λ�� 	*/

    __scsi_transport_cmd_t cmnd;			/* Command Block 				*/
    __u32 retries;							/* ��ǰretry�Ĵ��� 				*/
    __u32 allowed;             				/* ����retry�Ĵ��� 				*/

    void *buffer;							/* Data buffer 					*/
    __u32 DataTransferLength;				/* Size of data buffer 			*/
    __u32 ActualLength;						/* actual transport length		*/

    USB_OS_KERNEL_EVENT *complete;			/* �ȴ�Lun cmd������� 			*/
    LunDone Done;

    __u32 Result;							/* �����ִ�н��				*/
} __ScsiCmnd_t;

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------
#define  USB_STOR_TRANSPORT_GOOD		0   /* û�д��� 				*/
#define  USB_STOR_TRANSPORT_FAILED		1   /* ����ɹ�������ִ��ʧ�� 	*/
#define  USB_STOR_TRANSPORT_ERROR		2   /* ����ʧ�� 				*/


//-------------------------------------------------------------------
//
//-------------------------------------------------------------------
__s32 mscDevQueueCmnd(__mscLun_t *mscLun, __ScsiCmnd_t *ScsiCmnd);
__u32 mscDevOnline(__mscDev_t *mscDev);

#endif   //__USB_MSC_I_H__




