/*
********************************************************************************
*                                USB Hid Driver
*
*                (c) Copyright 2006-2010, All winners Co,Ld.
*                        All Right Reserved
*
* FileName		:  Hid_i.h
*
* Author		:  Javen
*
* Date			:  2010/06/02
*
* Description	:  Hid Driver�ж�USB�ӿ��豸������
*
* Others		:  NULL
*
* History:
*		<time> 			<author>	 <version >		<desc>
*	   2010.06.02		Javen			1.0			build this file
*
********************************************************************************
*/
#ifndef  __HID_I_H__
#define  __HID_I_H__

//---------------------------------------------------------
//  Ԥ����
//---------------------------------------------------------
struct _HidRequest;
struct _HidDev;
struct _usbHidReport;

//----------------------------------------------------------------------
//
//
//----------------------------------------------------------------------
/* input, output, feature */
#define USB_HID_MAX_FIELDS 		64
typedef struct _usbHidField
{
    /* Field��; */
    __u32 physical;				/* physical usage for this field 					*/
    __u32 logical;				/* logical usage for this field 					*/
    __u32 application;			/* application usage for this field 				*/
    usbHidUsage_t *usage;		/* usage table for this function 					*/
    __u32 maxusage;				/* maximum usage index	 							*/
    __u32 flags;				/* main-item flags (i.e. volatile,array,constant) 	*/
    __u32 report_offset;		/* bit offset in the report 						*/
    __u32 report_size;			/* size of this field in the report 				*/
    __u32 report_count;			/* number of this field in the report 				*/
    __u32 report_type;			/* (input,output,feature) 							*/
    __u32 *value;				/* last known value(s) 								*/
    __s32 logical_minimum;		/* ��С�߼�ֵ 										*/
    __s32 logical_maximum;		/* ����߼�ֵ 										*/
    __s32 physical_minimum;		/* ��С����ֵ 										*/
    __s32 physical_maximum;		/* �������ֵ 										*/
    __s32 unit_exponent;		/* ��λָ�� 										*/
    __u32 unit;					/* ��λ 											*/

    /* Field���� */
    __u32 Index;				/* ndex into report->field[] 						*/
    struct _usbHidReport *HidReport; /* field ������ HID report 					*/
} usbHidField_t;

#define  USB_HID_REPORT_TYPES		3  	/* ��������� 		*/
#define  USB_HID_REPORT_MAX_NUM		256	/* ����������� 	*/

/* �豸���涨�壬��input, output, feature��3�� */
typedef struct _usbHidReport
{
    __u32 Id;									/* id of this report 			*/
    __u32 Type;									/* report type,  				*/

    __u32 Maxfield;								/* maximum valid field index 	*/
    usbHidField_t *Field[USB_HID_MAX_FIELDS];	/* fields of the report 		*/

    __u32 Size;									/* size of the report (bits) 	*/
} usbHidReport_t;

/* �豸�����б��� */
typedef struct _usbHidReportEnum
{
    __u32 numbered;   /* reprot�Ƿ���� */

    __u32 ReportNum;  /* ��Ч��Report�ĸ��� */
    usbHidReport_t *Report[USB_HID_REPORT_MAX_NUM];
} usbHidReportEnum_t;

#define  USB_HID_GLOBAL_STACK_SIZE 			4
#define  USB_HID_COLLECTION_STACK_SIZE 		4
typedef struct _usbHidParser
{
    usbHidGlobalItems_t global;
    usbHidGlobalItems_t global_stack[USB_HID_GLOBAL_STACK_SIZE];
    __u32  global_stack_ptr;

    usbHidLocalItems_t local;

    __u32 collection_stack[USB_HID_COLLECTION_STACK_SIZE];
    __u32 collection_stack_ptr;

    struct _HidDev *HidDev;
} usbHidParser_t;


/* hid�¼���hid DATA���ƫ���� */
typedef struct _usbHidEvnetExcursion
{
    __u32 BitOffset;
    __u32 BitCount;
} usbHidEvnetExcursion_t;

//---------------------------------------------------------
//
//---------------------------------------------------------

/* Hid device state */
typedef enum _HidDev_state
{
    HID_DEV_OFFLINE = 0,			/* HidDev�Ѿ��γ� 		*/
    HID_DEV_ONLINE,				/* HidDev�Ѿ���� 		*/
    HID_DEV_DIED,				/* HidDev������ 		*/
    HID_DEV_RESET				/* HidDev���ڱ�reset 	*/
} HidDev_State_t;

/* USB Hid device type */
//#define  USB_HID_DEVICE_TYPE_UNKOWN		0x00	/* δ֪�豸 */
//#define  USB_HID_DEVICE_TYPE_KEYBOARD	0x01	/* ���� 	*/
//#define  USB_HID_DEVICE_TYPE_MOUSE		0x02	/* ��� 	*/

typedef __s32 (* Hid_SoftReset)(struct _HidDev *HidDev);
typedef __s32 (* Hid_ResetRecovery)(struct _HidDev *HidDev);
typedef __s32 (* Hid_Transport)(struct _HidDev *HidDev, struct _HidRequest *HidReq);
typedef __s32 (* Hid_StopTransport)(struct _HidDev *HidDev);

typedef __s32 (* HidClientProbe)(struct _HidDev *);
typedef __s32 (* HidClientRemove)(struct _HidDev *);

/* ����USB�ӿڵ���Ϣ */
typedef struct _HidDev
{
    struct usb_host_virt_dev *pusb_dev;		/* mscDev ��Ӧ��Public USB Device 	*/
    struct usb_interface	 *pusb_intf;	/* Public usb interface 			*/

    /* device information */
    __u8 InterfaceNo;						/* �ӿں� 							*/
    __u8 SubClass; 							/* ���� 							*/
    __u8 Protocol; 							/* ����Э�� 						*/
    __u32 DevType;							/* �豸���� 						*/
    __u32 DevNo; 							/* �豸�� hid �����еı��			*/

    /* device manager */
    HidDev_State_t State;					/* Dev��ǰ����������״̬ 			*/

    __u8 *ReportDesc;						/* װ����Hid�豸��report������ 		*/
    __u32 ReportSize;						/* report�������Ĵ�С 				*/

    usbHidCollectionItems_t *collection;		/* List of HID collections 				*/
    unsigned collection_size;					/* Number of allocated hid_collections 	*/
    unsigned maxcollection;						/* Number of parsed collections 		*/
    unsigned maxapplication;					/* Number of applications 				*/
    usbHidReportEnum_t HidReportEnum[USB_HID_REPORT_TYPES];		/* �豸�ı�����Ϣ 		*/

    /* transport */
    __u32 CtrlIn; 							/* ctrl in  pipe					*/
    __u32 CtrlOut; 							/* ctrl out pipe					*/
    __u32 IntIn;							/* interrupt in pipe 				*/
    __u8  EpInterval;						/* int ����������ѯ�豸������   	*/
    __u32 OnceTransferLength;				/* �ж�ep����������С 			*/

    __u32 busy;								/* �����Ƿ����ڴ������� 			*/
    struct urb *CurrentUrb;					/* USB requests	 					*/
    USB_OS_KERNEL_EVENT *UrbWait;			/* wait for Urb done 				*/
    struct usb_ctrlrequest *CtrlReq;		/* control requests	 				*/

    /* USB�ӿڲ��� */
    Hid_SoftReset 	  SoftReset;			/* ��λ��ֻ����� hid device ��״̬ */
    Hid_ResetRecovery ResetRecovery;		/* reset device 					*/
    Hid_Transport 	  Transport;			/* ���� 							*/
    Hid_StopTransport StopTransport;		/* ��ֹ���� 						*/

    /* Hid�豸���� */
    HidClientProbe  ClientProbe;
    HidClientRemove ClientRemove;

    void *Extern;							/* ��Ӧ�����hid�豸, ��mouse, keyboard */
} HidDev_t;

typedef void (* HidClientDone)(struct _HidRequest *);

/* Hid �������� */
typedef struct _HidRequest
{
    HidDev_t *HidDev;

    void *buffer;							/* Data buffer 					*/
    __u32 DataTransferLength;				/* Size of data buffer 			*/
    __u32 ActualLength;						/* actual transport length		*/

    HidClientDone Done;
    __u32 Result;							/* ִ�н��						*/

    void *Extern;							/* ��Ӧ�����hid�豸, ��mouse, keyboard */
} HidRequest_t;

//-----------------------------------------------------
//  Hid ������
//-----------------------------------------------------
#define  USB_HID_TRANSPORT_SUCCESS				0x00  /* ����ɹ� 			*/

#define  USB_HID_TRANSPORT_DEVICE_DISCONNECT	0x01  /* �豸�Ͽ� 			*/
#define  USB_HID_TRANSPORT_DEVICE_RESET			0x02  /* �豸��λ 			*/
#define  USB_HID_TRANSPORT_PIPE_HALT			0x03  /* ����ܵ��쳣 		*/
#define  USB_HID_TRANSPORT_CANCEL_CMD			0x04  /* �����ֹ�˴δ��� 	*/

#define  USB_HID_TRANSPORT_UNKOWN_ERR			0xFF  /* δ֪���� 			*/


//-----------------------------------------------------
//
//-----------------------------------------------------
__s32 HidSentRequest(HidRequest_t *HidReq);
__s32 HidGetInputReport(HidDev_t *HidDev, __u32 Usagepage, __u32 Usage, __u32 *BitOffset, __u32 *BitCount);


#endif   //__HID_I_H__

