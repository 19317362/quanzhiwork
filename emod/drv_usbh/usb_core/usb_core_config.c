/*
*******************************************************************************
*                                              usb host module
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
*			usb �����������Լ������ε����������Ľ���
* History :
********************************************************************************************************************
*/
#include "usb_host_config.h"

#include "usb_os_platform.h"
#include "usb_host_common.h"

#include "usb_host_base_types.h"
#include "usb_list.h"
#include "usb_msg.h"


#define USB_MAXALTSETTING		128	/* Hard limit */
#define USB_MAXENDPOINTS		30	/* Hard limit */

#define USB_MAXCONFIG			8	/* Arbitrary limit */


static inline const char *plural(int n)
{
    return (n == 1 ? "" : "s");
}


/* ����,ֱ������dt1,dt2���͵�desc */
static int _find_next_desc(unsigned char *buffer,
                           int size,
                           int dt1,
                           int dt2,
                           int *num_skipped)
{
    struct usb_descriptor_header *h;
    int n = 0;
    unsigned char *buffer0 = buffer;

    /* Find the next descriptor of type dt1 or dt2 */
    while (size > 0)
    {
        h = (struct usb_descriptor_header *) buffer;
        if (h->bDescriptorType == dt1 || h->bDescriptorType == dt2)
        {
            break;
        }

        buffer += h->bLength;
        size -= h->bLength;
        ++n;
    }

    /* Store the number of descriptors skipped and return the
     * number of bytes skipped */
    if (num_skipped)
    {
        *num_skipped = n;
    }

    return buffer - buffer0;
}

static int _usb_parse_endpoint_desc(int cfgno, int inum,
                                    int asnum,
                                    struct usb_host_virt_interface *ifp,
                                    int num_ep,
                                    u8 *buffer,
                                    int size)
{
    u8 *buffer0 = buffer;
    struct usb_endpoint_descriptor *d = NULL;
    struct usb_host_virt_endpoint *endpoint = NULL;
    int n = 0, i = 0;

    d = (struct usb_endpoint_descriptor *) buffer;
    buffer += d->bLength;
    size -= d->bLength;

    if (d->bLength >= USB_DT_ENDPOINT_AUDIO_SIZE)
        n = USB_DT_ENDPOINT_AUDIO_SIZE;
    else if (d->bLength >= USB_DT_ENDPOINT_SIZE)
        n = USB_DT_ENDPOINT_SIZE;
    else
    {
        DMSG_PANIC( "WARNING : config %d interface %d altsetting %d has an "
                    "invalid endpoint descriptor of length %d, skipping\n",
                    cfgno, inum, asnum, d->bLength);
        goto skip_to_next_endpoint_or_interface_descriptor;
    }

    i = d->bEndpointAddress & ~USB_ENDPOINT_DIR_MASK;
    if (i >= 16 || i == 0)
    {
        DMSG_PANIC( "WARNING : config %d interface %d altsetting %d has an "
                    "invalid endpoint with address 0x%X, skipping\n",
                    cfgno, inum, asnum, d->bEndpointAddress);
        goto skip_to_next_endpoint_or_interface_descriptor;
    }

    /* Only store as many endpoints as we have room for */
    if (ifp->desc.bNumEndpoints >= num_ep)
    {
        goto skip_to_next_endpoint_or_interface_descriptor;
    }

    endpoint = &ifp->endpoint[ifp->desc.bNumEndpoints];
    ++ifp->desc.bNumEndpoints;

    USB_OS_MEMCPY((void *)(&endpoint->desc), (void *)d, n);
    INIT_LIST_HEAD(&endpoint->urb_list);

    /* Skip over any Class Specific or Vendor Specific descriptors;
     * find the next endpoint or interface descriptor */
    endpoint->extra = buffer;
    i = _find_next_desc(buffer, size, USB_DT_ENDPOINT, USB_DT_INTERFACE, &n);
    endpoint->extralen = i;
    if (n > 0)
    {
        DMSG_INFO( "skipped %d descriptor%s after %s\n", n, plural(n), "endpoint");
    }

    return buffer - buffer0 + i;

skip_to_next_endpoint_or_interface_descriptor:
    i = _find_next_desc(buffer, size, USB_DT_ENDPOINT,
                        USB_DT_INTERFACE, NULL);

    return buffer - buffer0 + i;
}


//cfgno	:
//cofnig	:
//buffer	:	ָ��interface�Ŀ�ʼ
static int _usb_parse_interface_desc(int cfgno,
                                     struct usb_host_virt_config *config,
                                     unsigned char *buffer,
                                     int size,
                                     u8 inums[],
                                     u8 nalts[])
{
    unsigned char *buffer0 = buffer;
    struct usb_interface_descriptor	*interface_desc = NULL;
    int inum = 0, asnum = 0;
    struct usb_interface_cache *intfc = NULL;
    struct usb_host_virt_interface *alt = NULL;
    int i = 0, n = 0;
    int len = 0, retval = 0;
    int num_ep = 0, num_ep_orig = 0;

    interface_desc = (struct usb_interface_descriptor *) buffer;
    buffer += interface_desc->bLength;
    size -= interface_desc->bLength;

    if (interface_desc->bLength < USB_DT_INTERFACE_SIZE)
        goto skip_to_next_interface_descriptor;

    //Which interface entry is this?
    //���ݸ�interface��index��
    //��config���ҵ����Ӧ��intf_cache[]�е�entry
    intfc = NULL;
    inum = interface_desc->bInterfaceNumber;
    for (i = 0; i < config->desc.bNumInterfaces; ++i)
    {
        if (inums[i] == inum)
        {
            intfc = config->intf_cache[i];
            break;
        }
    }

    if (!intfc || intfc->num_altsetting >= nalts[i])
    {
        goto skip_to_next_interface_descriptor;
    }

    /* Check for duplicate altsetting entries */
    asnum = interface_desc->bAlternateSetting;
    for ((i = 0, alt = &(intfc->altsetting_array[0])); i < intfc->num_altsetting; (++i, ++alt))
    {
        if (alt->desc.bAlternateSetting == asnum)
        {
            DMSG_PANIC( "WARNING : Duplicate descriptor for config %d "
                        "interface %d altsetting %d, skipping\n",
                        cfgno, inum, asnum);
            goto skip_to_next_interface_descriptor;
        }
    }

    ++intfc->num_altsetting;
    USB_OS_MEMCPY((void *)(&alt->desc), (void *)interface_desc, USB_DT_INTERFACE_SIZE);

    /* Skip over any Class Specific or Vendor Specific descriptors;
     * find the first endpoint or interface descriptor */
    //����endpoint desc
    alt->extra = buffer;
    i = _find_next_desc(buffer, size, USB_DT_ENDPOINT, USB_DT_INTERFACE, &n);
    alt->extralen = i;
    if (n > 0)
    {
        DMSG_INFO( "skipped %d descriptor%s after %s\n", n, plural(n), "interface");
    }
    buffer += i;
    size -= i;

    /* Allocate space for the right(?) number of endpoints */
    num_ep = num_ep_orig = alt->desc.bNumEndpoints;
    alt->desc.bNumEndpoints = 0;		// Use as a counter
    if (num_ep > USB_MAXENDPOINTS)
    {
        DMSG_PANIC( "WARNING : too many endpoints for config %d interface %d "
                    "altsetting %d: %d, using maximum allowed: %d\n",
                    cfgno, inum, asnum, num_ep, USB_MAXENDPOINTS);
        num_ep = USB_MAXENDPOINTS;
    }

    //����endpoint desc�Ŀռ�
    len = sizeof(struct usb_host_virt_endpoint) * num_ep;
    alt->endpoint = USB_OS_MALLOC(len, USB_MEM_FILE_TRIGGER , USB_MEM_LINE_TRIGGER );
    if (!alt->endpoint)
    {
        DMSG_PANIC("ERR: MALLOC failed\n");
        return -ENOMEM;
    }
    USB_OS_MEMSET(alt->endpoint, 0, len);

    /* Parse all the endpoint descriptors */
    n = 0;
    while (size > 0)
    {
        if (((struct usb_descriptor_header *) buffer)->bDescriptorType == USB_DT_INTERFACE)
        {
            break;
        }

        retval = _usb_parse_endpoint_desc(cfgno, inum, asnum, alt, num_ep, buffer, size);
        if (retval < 0)
        {
            return retval;
        }
        ++n;

        buffer += retval;
        size -= retval;
    }

    if (n != num_ep_orig)
    {
        DMSG_PANIC( "WARNING : config %d interface %d altsetting %d has %d "
                    "endpoint descriptor%s, different from the interface "
                    "descriptor's value: %d\n",
                    cfgno, inum, asnum, n, plural(n), num_ep_orig);
    }

    return buffer - buffer0;

skip_to_next_interface_descriptor:
    i = _find_next_desc(buffer, size, USB_DT_INTERFACE, USB_DT_INTERFACE, NULL);

    return buffer - buffer0 + i;
}


//cfgidx	:	config index
//config	:	Ϊ��config׼����usb_host_virt_config
//buff	:	����config������(�������ε�)
//size	:	����config�ĳ���
static int _usb_parse_config_desc(  int cfgidx,
                                    struct usb_host_virt_config *config,
                                    unsigned char *buffer,
                                    int size)
{
    unsigned char *buffer0 = buffer;
    int cfgno = 0;
    s32 interface_nr = 0; 					//��config��interface��
    s32 nintf_orig = 0;
    int i = 0, j = 0, n = 0;
    struct usb_interface_cache *intfc = NULL;
    u8 *buffer2 = NULL;						//��ʱbuff,
    int size2 = 0;
    struct usb_descriptor_header *header = NULL;
    int len = 0, retval = 0;
    u8 inums[USB_MAXINTERFACES];			//��¼interface�ı��
    u8 alt_setting_nr[USB_MAXINTERFACES];	//��config�и���interface��AlertateSetting����Ŀ

    //--<1>--��config�����浽config->desc
    USB_OS_MEMCPY((void *)(&config->desc), (void *)buffer, USB_DT_CONFIG_SIZE);
    if (config->desc.bDescriptorType != USB_DT_CONFIG ||
            config->desc.bLength < USB_DT_CONFIG_SIZE)
    {
        DMSG_PANIC("PANIC : [hub] :invalid descriptor for config index %d: "
                   "type = 0x%X, length = %d\n", cfgidx,
                   config->desc.bDescriptorType, config->desc.bLength);

        return -EINVAL;
    }
    cfgno = config->desc.bConfigurationValue;	//��config��nr

    //--<2>--����config����
    buffer += config->desc.bLength;
    size -= config->desc.bLength;

    interface_nr = nintf_orig = config->desc.bNumInterfaces;	//��config��interface��
    if (interface_nr > USB_MAXINTERFACES)
    {
        DMSG_PANIC("PANIC : [hub] :config %d has too many interfaces: %d, "
                   "using maximum allowed: %d\n",
                   cfgno, interface_nr, USB_MAXINTERFACES);
        interface_nr = USB_MAXINTERFACES;
    }

    //--<3>--��������config�����µ�desc, ��ʵֻ����Interface��ͳ�Ƹ���interface����Ŀ
    n = 0;
    for ((buffer2 = buffer, size2 = size);
            size2 > 0;
            (buffer2 += header->bLength, size2 -= header->bLength))
    {
        //toutine check
        if (size2 < sizeof(struct usb_descriptor_header))
        {
            DMSG_PANIC("PANIC : [hub] :config %d descriptor has %d excess "
                       "byte%s, ignoring\n",
                       cfgno, size2, plural(size2));
            break;
        }

        header = (struct usb_descriptor_header *) buffer2;
        if ((header->bLength > size2) || (header->bLength < 2))
        {
            DMSG_PANIC("PANIC : [hub] :config %d has an invalid descriptor "
                       "of length %d, skipping remainder of the config\n",
                       cfgno, header->bLength);
            break;
        }

        //--<3_1>--Interface��������
        if (header->bDescriptorType == USB_DT_INTERFACE)
        {
            struct usb_interface_descriptor *d = NULL;
            int inum = 0;

            d = (struct usb_interface_descriptor *) header;
            if (d->bLength < USB_DT_INTERFACE_SIZE)
            {
                DMSG_PANIC("PANIC : [hub] :config %d has an invalid "
                           "interface descriptor of length %d, "
                           "skipping\n", cfgno, d->bLength);
                continue;
            }

            inum = d->bInterfaceNumber;
            if (inum >= nintf_orig)
            {
                DMSG_PANIC("PANIC : [hub] :config %d has an invalid "
                           "interface number: %d but max is %d\n",
                           cfgno, inum, nintf_orig - 1);
            }

            /* Have we already encountered this interface?
             * Count its altsettings */
            for (i = 0; i < n; ++i)
            {
                if (inums[i] == inum)
                {
                    break;
                }
            }

            if (i < n)
            {
                if (alt_setting_nr[i] < 255)
                {
                    ++alt_setting_nr[i];
                }
            }
            else if (n < USB_MAXINTERFACES)
            {
                inums[n] = inum;
                alt_setting_nr[n] = 1;
                ++n;
            }

        }
        else if (header->bDescriptorType == USB_DT_DEVICE ||
                 header->bDescriptorType == USB_DT_CONFIG)
        {
            DMSG_PANIC("PANIC : [hub] :config %d contains an unexpected "
                       "descriptor of type 0x%X, skipping\n",
                       cfgno, header->bDescriptorType);
        }
    }	/* for ((buffer2 = buffer, size2 = size); ...) */
    size = buffer2 - buffer;
    config->desc.wTotalLength = cpu_to_le16(buffer2 - buffer0);

    if (n != interface_nr)
    {
        DMSG_PANIC("PANIC : [hub] :config %d has %d interface%s, different from "
                   "the descriptor's value: %d\n",
                   cfgno, n, plural(n), nintf_orig);
    }
    else if (n == 0)
    {
        DMSG_PANIC("PANIC : [hub] :config %d has no interfaces?\n", cfgno);
    }

    //��¼interface����Ŀ
    config->desc.bNumInterfaces = interface_nr = n;

    /* Check for missing interface numbers */
    for (i = 0; i < interface_nr; ++i)
    {
        for (j = 0; j < interface_nr; ++j)
        {
            if (inums[j] == i)
            {
                break;
            }
        }

        if (j >= interface_nr)
        {
            DMSG_PANIC("PANIC : [hub] :config %d has no interface number "
                       "%d\n", cfgno, i);
        }
    }

    //--<4>--Ϊ����interface����洢�ռ�
    /* Allocate the usb_interface_caches and altsetting arrays */
    for (i = 0; i < interface_nr; ++i)
    {
        //u8 * buff_addr = 0;
        //u32 temp = 0;

        j = alt_setting_nr[i];

        if (j > USB_MAXALTSETTING)
        {
            DMSG_PANIC("PANIC : [hub] :oo many alternate settings for "
                       "config %d interface %d: %d, "
                       "using maximum allowed: %d\n",
                       cfgno, inums[i], j, USB_MAXALTSETTING);
            alt_setting_nr[i] = j = USB_MAXALTSETTING;
        }

        //--<4_1>--����洢usb_interface_cache�Ŀռ�
        len = sizeof(struct usb_interface_cache) ;
        config->intf_cache[i] = intfc = USB_OS_MALLOC(len, USB_MEM_FILE_TRIGGER , USB_MEM_LINE_TRIGGER );
        if (!intfc)
        {
            DMSG_PANIC("ERR: USB_OS_MALLOC failed\n");
            return -ENOMEM;
        }

        USB_OS_MEMSET(intfc, 0, len);

        //--<4_2>--����洢usb_host_virt_interface[]�Ŀռ�
        len =  sizeof(struct usb_host_virt_interface) * j;
        intfc->altsetting_array = USB_OS_MALLOC(len, USB_MEM_FILE_TRIGGER , USB_MEM_LINE_TRIGGER );
        if (!intfc->altsetting_array)
        {
            DMSG_PANIC("ERR: USB_OS_MALLOC failed\n");

            USB_OS_FREE(config->intf_cache[i]);
            config->intf_cache[i] = NULL;

            return -ENOMEM;
        }

        USB_OS_MEMSET(intfc->altsetting_array, 0, len);
    }

    //--<4>--���ƣ�ֱ��Interface desc
    /* Skip over any Class Specific or Vendor Specific descriptors;
     * find the first interface descriptor */
    config->extra = buffer;
    i = _find_next_desc(buffer, size, USB_DT_INTERFACE, USB_DT_INTERFACE, &n);
    config->extralen = i;
    if (n > 0)
    {
        DMSG_INFO("  [hub] :skipped %d descriptor%s after %s\n", n, plural(n), "configuration");
    }
    buffer += i;
    size -= i;

    /* Parse all the interface/altsetting descriptors */
    while (size > 0)
    {
        retval = _usb_parse_interface_desc( cfgno,
                                            config,
                                            buffer,
                                            size,
                                            inums,
                                            alt_setting_nr);
        if(retval < 0)
        {
            DMSG_PANIC("ERR: _usb_parse_interface_desc failed\n");

            return retval;
        }

        buffer += retval;
        size -= retval;
    }

    /* Check for missing altsettings */
    for (i = 0; i < interface_nr; ++i)
    {
        intfc = config->intf_cache[i];

        for (j = 0; j < intfc->num_altsetting; ++j)
        {
            for (n = 0; n < intfc->num_altsetting; ++n)
            {
                if (intfc->altsetting_array[n].desc.bAlternateSetting == j)
                {
                    break;
                }
            }

            if (n >= intfc->num_altsetting)
            {
                DMSG_PANIC("PANIC : [hub] :config %d interface %d has no "
                           "altsetting %d\n", cfgno, inums[i], j);
            }
        }
    }

    return 0;
}

/* ���confg����������parser֮ */
int usb_get_all_config_desc_and_parser(struct usb_host_virt_dev *dev)
{
    int result 		= -ENOMEM;
    int ncfg 		= dev->descriptor.bNumConfigurations;	//config������Ŀ
    u32 cur_cfg_nr 	= 0;	//��ǰcfg�ı��
    u32 length 		= 0;
    u8 *buffer 		= NULL;
    u8 *bigbuffer 	= NULL;
    struct usb_config_descriptor *desc = NULL;

    if (ncfg > USB_MAXCONFIG)
    {
        DMSG_INFO( "INFO : [hub] too many configurations: %d, "
                   "using maximum allowed: %d\n", ncfg, USB_MAXCONFIG);
        dev->descriptor.bNumConfigurations = ncfg = USB_MAXCONFIG;
    }

    if (ncfg < 1)
    {
        DMSG_PANIC( "PANIC : no configurations\n");
        return -EINVAL;
    }

    length = ncfg * sizeof(struct usb_host_virt_config);
    dev->config = USB_OS_MALLOC(length, USB_MEM_FILE_TRIGGER , USB_MEM_LINE_TRIGGER );
    if (!dev->config)
    {
        DMSG_PANIC("ERR: malloc failed\n");
        goto err2;
    }
    USB_OS_MEMSET(dev->config, 0, length);

    length = ncfg * sizeof(char *);
    dev->rawdescriptors = USB_OS_MALLOC(length, USB_MEM_FILE_TRIGGER , USB_MEM_LINE_TRIGGER );
    if (!dev->rawdescriptors)
    {
        DMSG_PANIC("ERR: malloc failed\n");
        goto err2;
    }
    USB_OS_MEMSET(dev->rawdescriptors, 0, length);

    buffer = USB_OS_MALLOC(USB_DT_CONFIG_SIZE, USB_MEM_FILE_TRIGGER , USB_MEM_LINE_TRIGGER );
    if (!buffer)
    {
        DMSG_PANIC("ERR: malloc failed\n");
        goto err2;
    }
    USB_OS_MEMSET(buffer, 0, USB_DT_CONFIG_SIZE);

    desc = (struct usb_config_descriptor *)buffer;

    //�����ȡconfig,��������config
    for (cur_cfg_nr = 0; cur_cfg_nr < ncfg; cur_cfg_nr++)
    {
        //--<1>--��̽�ԵĻ�ȡconfig����ҪĿ���ǻ�ȡ����config��total len
        result = usb_get_descriptor(dev, USB_DT_CONFIG, cur_cfg_nr, buffer, USB_DT_CONFIG_SIZE);
        if (result < 0)
        {
            DMSG_PANIC("PANIC : [hub] : unable to read config index %d "
                       "descriptor/%s\n", cur_cfg_nr, "start");
            goto err;
        }
        else if (result < 4)
        {
            DMSG_PANIC( "PANIC : [hub] : config index %d descriptor too short "
                        "(expected %i, got %i)\n", cur_cfg_nr,
                        USB_DT_CONFIG_SIZE, result);
            result = -EINVAL;
            goto err;
        }

        length = USB_MAX((int) le16_to_cpu(desc->wTotalLength), USB_DT_CONFIG_SIZE);

        //Now that we know the length, get the whole thing //
        bigbuffer = USB_OS_MALLOC(length, USB_MEM_FILE_TRIGGER , USB_MEM_LINE_TRIGGER );
        if (!bigbuffer)
        {
            DMSG_PANIC("ERR: malloc failed\n");
            result = -ENOMEM;
            goto err;
        }
        USB_OS_MEMSET(bigbuffer, 0, length);

        //--<2>--�����Ļ�ȡconfig
        result = usb_get_descriptor(dev, USB_DT_CONFIG, cur_cfg_nr, bigbuffer, length);
        if (result < 0)
        {
            DMSG_PANIC( "PANIC : [ hub] : unable to read config index %d "
                        "descriptor/%s\n", cur_cfg_nr, "all");
            USB_OS_FREE(bigbuffer);
            bigbuffer = NULL;

            goto err;
        }
        if (result < length)
        {
            DMSG_PANIC ("PANIC : [ hub] : config index %d descriptor too short "
                        "(expected %i, got %i)\n", cur_cfg_nr, length, result);
            length = result;
        }

        dev->rawdescriptors[cur_cfg_nr] = bigbuffer;

        //--<3>--������config
        result = _usb_parse_config_desc(cur_cfg_nr, &dev->config[cur_cfg_nr], bigbuffer, length);
        if (result < 0)
        {
            ++cur_cfg_nr;
            goto err;
        }
    }

    result = 0;

err:
    if(buffer)
    {
        USB_OS_FREE(buffer);
        buffer = NULL;
    }
    else
    {
        DMSG_PANIC("ERR: parameter is NULL, can't free\n");
    }

    dev->descriptor.bNumConfigurations = cur_cfg_nr;

err2:
    if (result == -ENOMEM)
    {
        DMSG_PANIC("out of memory\n");
    }

    return result;
}


void usb_release_interface_cache(struct usb_interface_cache *intfc )
{
    s32 j = 0;

    if(intfc == NULL)
    {
        DMSG_PANIC("ERR: usb_release_interface_cache: input = NULL\n");
        return ;
    }

    for (j = 0; j < intfc->num_altsetting; j++)
    {
        if(intfc->altsetting_array[j].endpoint)
        {
            USB_OS_FREE(intfc->altsetting_array[j].endpoint);
            intfc->altsetting_array[j].endpoint = NULL;
        }
        else
        {
            DMSG_PANIC("ERR: parameter is NULL, can't free\n");
        }
    }

    USB_OS_FREE(intfc);
    intfc = NULL;

    return ;
}

// hub-only!! ... and only exported for reset/reinit path.
// otherwise used internally on disconnect/destroy path
void usb_destroy_configuration(struct usb_host_virt_dev *dev)
{
    int c = 0, i = 0;

    if (!dev->config)
    {
        DMSG_PANIC("ERR: dev is not config\n");
        return;
    }

    if (dev->rawdescriptors)
    {
        for (i = 0; i < dev->descriptor.bNumConfigurations; i++)
        {
            if(dev->rawdescriptors[i])
            {
                USB_OS_FREE(dev->rawdescriptors[i]);
                dev->rawdescriptors[i] = NULL;
            }
            else
            {
                DMSG_PANIC("ERR: parameter is NULL, can't free\n");
            }
        }

        if(dev->rawdescriptors)
        {
            USB_OS_FREE(dev->rawdescriptors);
            dev->rawdescriptors = NULL;
        }
        else
        {
            DMSG_PANIC("ERR: parameter is NULL, can't free\n");
        }
    }

    for (c = 0; c < dev->descriptor.bNumConfigurations; c++)
    {
        struct usb_host_virt_config *cf = &dev->config[c];

        if(cf->string)
        {
            USB_OS_FREE(cf->string);
            cf->string = NULL;
        }
        else
        {
            DMSG_PANIC("ERR: parameter is NULL, can't free\n");
        }

        for (i = 0; i < cf->desc.bNumInterfaces; i++)
        {
            if (cf->intf_cache[i])
            {
                usb_release_interface_cache(cf->intf_cache[i]);
            }
        }
    }

    if(dev->config)
    {
        USB_OS_FREE(dev->config);
        dev->config = NULL;
    }
    else
    {
        DMSG_PANIC("ERR: parameter is NULL, can't free\n");
    }

    return ;
}


