
#include  "csi_dev_i.h"

#if(CSI_DEV_TYPE == CSI_DEV_TYPE_BIT1605)

#define MIRROR 0//0 = normal, 1 = mirror
#define FLIP   0//0 = normal, 1 = flip

//ÿ��device˽�����Ե����ö��ڴ��ļ����
//����reset pin��power down pin�Ĳ���
//mclk������
//����ģʽ�µ�i2c�����Լ���Ӧ��csi����
//˼·���£�
//ͨ��sensor��������
//1.��ʼ��
//2.���κ�ģʽ��ģʽA
//3.���κ�ģʽ��ģʽB
//...
//��˶��弸��������reg_init�� reg_A��reg_B��...
//��csi_dev_init������һ��������reg_init
//����Ҫ��ʱ����Զ������reg_A��reg_B...

//==================ȫ�ֱ���dev_csi.cҪ����========================
//ÿ��ģ�����Ҫ����Щ����
__bool              bProgressiveSrc = 0;   // Indicating the source is progressive or not
__bool              bTopFieldFirst = 1;    // VPO should check this flag when bProgressiveSrc is FALSE
__u16               eAspectRatio = 1000;     //the source picture aspect ratio

__u32 csi_mclk = 27; //MHz

__csi_mode_t csi_mode;//��ǰʹ��ģʽ
__csi_mode_all_t csi_mode_all;//֧�ֵ�ģʽ��
__csi_conf_t conf;//csi���ò���

__csi_color_effect_t csi_color_effect = CSI_COLOR_EFFECT_NORMAL;   	//��ǰ��csi_color_effect
__csi_awb_t          csi_awb 					= CSI_AWB_AUTO;   						//��ǰ��csi_awb
__csi_ae_t					 csi_ae 					= CSI_AE_0;   								//��ǰ��csi_ae
__csi_bright_t			 csi_bright 			= CSI_BRIGHT_0;   						//��ǰ��csi_bright
__csi_contrast_t		 csi_contrast 		= CSI_CONTRAST_0;   					//��ǰ��csi_contrast
__csi_band_t				 csi_band 				= CSI_BAND_50HZ;   						//��ǰ��csi_band
//==================================================================================

extern ES_FILE   *fiic;//i2c���


//==================��ȫ�ֱ������ڴ��ļ����õ�========================
__u32  last_mode_num = 0xff;
__buf_scrab_mode_t  last_buf_mode = CSI_SC_NA;//���ֵ�˫csi����״̬��last_buf_modeΪ˫���л���reset power����->����reset����->˫resetһ��

//====================================================================
static __u8  da = 0x4c >> 1; //�豸��ַ, DATA_bit7�йأ��ϵ�ʱ���ƽ��0/1 -> 0xB8>>1 / 0xBA>>1
static __u32 reg_step = 2; //һ��д�����������ļĴ�����ַ+�Ĵ���ֵ��byte��
//====================================================================


//=======================================================================================
static __u8 reg_init[] = //�Ĵ�������ʽΪ��[�Ĵ�����ַ], [�Ĵ���ֵ], (��ַ��ֵ��Ϊ16-bit��Ȼ��byteΪ��λ����λ��ǰ����λ�ں������0xABCDд��0x1234: 0xab, 0xcd, 0x12, 0x34,)
{
    0x00, 0xd4,
    0x01, 0x00,
    0x02, 0x00,
    0x03, 0x00,
    0x04, 0x00,
    0x05, 0x00,
    0x06, 0x00,
    0x07, 0x00,
    0x08, 0x00,
    0x09, 0xb6,
    0x0a, 0xf0,
    0x0b, 0x00,
    0x0c, 0x60,
    0x0d, 0x0d,
    0x0e, 0xc5,
    0x0f, 0x00,

    0x10, 0xa0,
    0x11, 0x91,
    0x12, 0x80,
    0x13, 0x40,
    0x14, 0x00,
    0x15, 0x83,
    0x16, 0x82,
    0x17, 0xc0,
    0x18, 0x40,
    0x19, 0x00,
    0x1a, 0x12,
    0x1b, 0x16,
    0x1c, 0x05,
    0x1d, 0x4a,
    0x1e, 0x28,
    0x1f, 0xf8,

    0x20, 0xe2,
    0x21, 0xc1,
    0x22, 0xf5,
    0x23, 0x03,
    0x24, 0x03,
    0x25, 0xef,
    0x26, 0x81,
    0x27, 0x20,
    0x28, 0x26,
    0x29, 0x99,
    0x2a, 0x91,
    0x2b, 0x90,
    0x2c, 0x00,
    0x2d, 0x92,
    0x2e, 0x08,
    0x2f, 0x00,

    0x30, 0x21,
    0x31, 0x00,
    0x32, 0x03,
    0x33, 0x80,
    0x34, 0x80,
    0x35, 0x50,
    0x36, 0x50,
    0x37, 0xc4,
    0x38, 0x29,
    0x39, 0x38,
    0x3a, 0x00,
    0x3b, 0x08,
    0x3c, 0x00,
    0x3d, 0x00,
    0x3e, 0x20,
    0x3f, 0x28,

    0x40, 0x10,
    0x41, 0x10,
    0x42, 0x0a,
    0x43, 0x07,
    0x44, 0x04,
    0x45, 0xaa,
    0x46, 0x73,
    0x47, 0x18,
    0x48, 0x30,
    0x49, 0x30,
    0x4a, 0x59,
    0x4b, 0x15,
    0x4c, 0x40,
    0x4d, 0x3f,
    0x4e, 0x00,
    0x4f, 0x04,

    0x50, 0x03,
    0x51, 0x00,
    0x52, 0x59,
    0x53, 0x15,
    0x54, 0x40,
    0x55, 0x3f,
    0x56, 0x00,
    0x57, 0x04,
    0x58, 0x00,
    0x59, 0x00,
    0x5a, 0x60,
    0x5b, 0x1f,
    0x5c, 0x0e,
    0x5d, 0x11,
    0x5e, 0x80,
    0x5f, 0x00,

    0x60, 0x00,
    0x61, 0x00,
    0x62, 0x00,
    0x63, 0x00,
    0x64, 0x00,
    0x65, 0xc4,
    0x66, 0x03,
    0x67, 0x00,
    0x68, 0x00,
    0x69, 0x04,
    0x6a, 0x00,
    0x6b, 0x00,
    0x6c, 0x00,
    0x6d, 0x05,
    0x6e, 0x10,
    0x6f, 0x66,

    0x70, 0x25,
    0x71, 0xee,
    0x72, 0x25,
    0x73, 0xcd,
    0x74, 0xb5,
    0x75, 0xff,
    0x76, 0x19,
    0x77, 0x20,
    0x78, 0x0c,
    0x79, 0x07,
    0x7a, 0x1f,
    0x7b, 0x07,
    0x7c, 0x62,

};
static __u8 reg_720_576_25000[] =
{
    0x00, 0xd4,
    0x01, 0x00,
    0x02, 0x00,
    0x03, 0x00,
    0x04, 0x00,
    0x05, 0x00,
    0x06, 0x00,
    0x07, 0x00,
    0x08, 0x00,
    0x09, 0xb6,
    0x0a, 0xf0,
    0x0b, 0x00,
    0x0c, 0x64,
    0x0d, 0x0d,
    0x0e, 0xc5,
    0x0f, 0x00,

    0x10, 0xa0,
    0x11, 0x91,
    0x12, 0x80,
    0x13, 0x40,
    0x14, 0x00,
    0x15, 0x83,
    0x16, 0x82,
    0x17, 0xc0,
    0x18, 0x40,
    0x19, 0x00,
    0x1a, 0x12,
    0x1b, 0x16,
    0x1c, 0x05,
    0x1d, 0x4a,
    0x1e, 0x28,
    0x1f, 0xf8,

    0x20, 0xe2,
    0x21, 0xc1,
    0x22, 0xf5,
    0x23, 0x03,
    0x24, 0x03,
    0x25, 0xef,
    0x26, 0x81,
    0x27, 0x20,
    0x28, 0x26,
    0x29, 0x99,
    0x2a, 0x91,
    0x2b, 0x90,
    0x2c, 0x00,
    0x2d, 0x92,
    0x2e, 0x08,
    0x2f, 0x00,

    0x30, 0x21,
    0x31, 0x00,
    0x32, 0x03,
    0x33, 0x80,
    0x34, 0x80,
    0x35, 0x50,
    0x36, 0x50,
    0x37, 0xc4,
    0x38, 0x29,
    0x39, 0x38,
    0x3a, 0x00,
    0x3b, 0x08,
    0x3c, 0x00,
    0x3d, 0x00,
    0x3e, 0x20,
    0x3f, 0x28,

    0x40, 0x10,
    0x41, 0x10,
    0x42, 0x0a,
    0x43, 0x07,
    0x44, 0x04,
    0x45, 0xaa,
    0x46, 0x73,
    0x47, 0x18,
    0x48, 0x30,
    0x49, 0x30,
    0x4a, 0x59,
    0x4b, 0x15,
    0x4c, 0x40,
    0x4d, 0x3f,
    0x4e, 0x00,
    0x4f, 0x04,

    0x50, 0x03,
    0x51, 0x00,
    0x52, 0x59,
    0x53, 0x15,
    0x54, 0x40,
    0x55, 0x3f,
    0x56, 0x00,
    0x57, 0x04,
    0x58, 0x00,
    0x59, 0x00,
    0x5a, 0x60,
    0x5b, 0x1f,
    0x5c, 0x0e,
    0x5d, 0x11,
    0x5e, 0x80,
    0x5f, 0x00,

    0x60, 0x00,
    0x61, 0x00,
    0x62, 0x00,
    0x63, 0x00,
    0x64, 0x00,
    0x65, 0xc4,
    0x66, 0x03,
    0x67, 0x00,
    0x68, 0x00,
    0x69, 0x04,
    0x6a, 0x00,
    0x6b, 0x00,
    0x6c, 0x00,
    0x6d, 0x05,
    0x6e, 0x10,
    0x6f, 0x66,

    0x70, 0x25,
    0x71, 0xee,
    0x72, 0x25,
    0x73, 0xcd,
    0x74, 0xb5,
    0x75, 0xff,
    0x76, 0x19,
    0x77, 0x20,
    0x78, 0x0c,
    0x79, 0x07,
    0x7a, 0x1f,
    0x7b, 0x07,
    0x7c, 0x62,

};
static __u8 reg_720_480_30000[] =
{
    0x00, 0xd4,
    0x01, 0x00,
    0x02, 0x00,
    0x03, 0x00,
    0x04, 0x00,
    0x05, 0x00,
    0x06, 0x00,
    0x07, 0x00,
    0x08, 0x00,
    0x09, 0xb6,
    0x0a, 0xf0,
    0x0b, 0x00,
    0x0c, 0x64,
    0x0d, 0x0d,
    0x0e, 0xc5,
    0x0f, 0x00,

    0x10, 0xa0,
    0x11, 0x91,
    0x12, 0x80,
    0x13, 0x40,
    0x14, 0x00,
    0x15, 0x83,
    0x16, 0x82,
    0x17, 0xc0,
    0x18, 0x40,
    0x19, 0x00,
    0x1a, 0x12,
    0x1b, 0x16,
    0x1c, 0x05,
    0x1d, 0x4a,
    0x1e, 0x28,
    0x1f, 0xf8,

    0x20, 0xe2,
    0x21, 0xc1,
    0x22, 0xf5,
    0x23, 0x03,
    0x24, 0x03,
    0x25, 0xef,
    0x26, 0x81,
    0x27, 0x20,
    0x28, 0x26,
    0x29, 0x99,
    0x2a, 0x91,
    0x2b, 0x90,
    0x2c, 0x00,
    0x2d, 0x92,
    0x2e, 0x08,
    0x2f, 0x00,

    0x30, 0x21,
    0x31, 0x00,
    0x32, 0x03,
    0x33, 0x80,
    0x34, 0x80,
    0x35, 0x50,
    0x36, 0x50,
    0x37, 0xc4,
    0x38, 0x29,
    0x39, 0x38,
    0x3a, 0x00,
    0x3b, 0x08,
    0x3c, 0x00,
    0x3d, 0x00,
    0x3e, 0x20,
    0x3f, 0x28,

    0x40, 0x10,
    0x41, 0x10,
    0x42, 0x0a,
    0x43, 0x07,
    0x44, 0x04,
    0x45, 0xaa,
    0x46, 0x73,
    0x47, 0x18,
    0x48, 0x30,
    0x49, 0x30,
    0x4a, 0x59,
    0x4b, 0x15,
    0x4c, 0x40,
    0x4d, 0x3f,
    0x4e, 0x00,
    0x4f, 0x04,

    0x50, 0x03,
    0x51, 0x00,
    0x52, 0x59,
    0x53, 0x15,
    0x54, 0x40,
    0x55, 0x3f,
    0x56, 0x00,
    0x57, 0x04,
    0x58, 0x00,
    0x59, 0x00,
    0x5a, 0x60,
    0x5b, 0x1f,
    0x5c, 0x0e,
    0x5d, 0x11,
    0x5e, 0x80,
    0x5f, 0x00,

    0x60, 0x00,
    0x61, 0x00,
    0x62, 0x00,
    0x63, 0x00,
    0x64, 0x00,
    0x65, 0xc4,
    0x66, 0x03,
    0x67, 0x00,
    0x68, 0x00,
    0x69, 0x04,
    0x6a, 0x00,
    0x6b, 0x00,
    0x6c, 0x00,
    0x6d, 0x05,
    0x6e, 0x10,
    0x6f, 0x66,

    0x70, 0x25,
    0x71, 0xee,
    0x72, 0x25,
    0x73, 0xcd,
    0x74, 0xb5,
    0x75, 0xff,
    0x76, 0x19,
    0x77, 0x20,
    0x78, 0x0c,
    0x79, 0x07,
    0x7a, 0x1f,
    0x7b, 0x07,
    0x7c, 0x62,

};

//=======================================================================================



__s32  csi_power_on(void)
{
    csi_pwdn(1);//pwdn=1
    csi_rst(1);//rst=1

    csi_pwren(0);//pwr=0
    esKRNL_TimeDly(1);//shut sensor power first in case system already open power before pull pwdn and reset
    csi_pwren(1);//pwr=1
    esKRNL_TimeDly(1);
    csi_pwdn(0);//pwdn=0
    esKRNL_TimeDly(1);
    csi_rst(0);//rst=0
    esKRNL_TimeDly(1);
    csi_rst(1);//rst=1

    return EPDK_OK;
}

#if 0
void init_gpio(void)
{
    __hdle hdl_pdn, hdl_reset;
    user_gpio_set_t  gpio_set[2];

    gpio_set[0].port = 8;
    gpio_set[0].port_num = 8;
    gpio_set[0].mul_sel = 1;
    gpio_set[0].pull = 1;
    gpio_set[0].drv_level = 1;
    gpio_set[0].data = 1;
    hdl_pdn = esPINS_PinGrpReq(&gpio_set[0], 1);

    //
    gpio_set[1].port = 8;
    gpio_set[1].port_num = 9;
    gpio_set[1].mul_sel = 1;
    gpio_set[1].pull = 1;
    gpio_set[1].drv_level = 1;
    gpio_set[1].data = 1;
    hdl_reset = esPINS_PinGrpReq(&gpio_set[1], 1);

    esPINS_WritePinData(hdl_pdn, 1, 0);
    esPINS_WritePinData(hdl_reset, 1, 0);
    esKRNL_TimeDly(1);
    esPINS_WritePinData(hdl_reset, 0, 0);
    esKRNL_TimeDly(2);
    esPINS_WritePinData(hdl_reset, 1, 0);
    esPINS_WritePinData(hdl_pdn, 1, 0);
    esKRNL_TimeDly(1);
}
#endif

__s32  csi_power_off(void)
{
    csi_rst(0);//rst=0
    csi_pwren(0);//pwr=0
    return EPDK_OK;
}


__s32  csi_clk_on(void)
{
    csi_mclk_on_off(1);
    return EPDK_OK;
}

__s32  csi_clk_off(void)
{
    csi_mclk_on_off(0);
    return EPDK_OK;
}


__s32 block_write(ES_FILE *hdl_iic, __u8 slv_addr, __u8 *reg, __u32 length)
{
    __u32 i = 0;
    __s32 ret = EPDK_FAIL;

    for (i = 0; i < length; i += reg_step)
    {
        //__u8 iic_val=0;
        //__u8 tmp=0;
        //__inf("8_8\n");
        if(iic_write_8_8(hdl_iic, slv_addr, reg[i], reg[i + 1]) != EPDK_OK)
        {
            __wrn("csi1[%d] iic write fail %x = %x\n", reg[i], reg[i + 1], CSI_PORT);
        }
        if (i == 0)esKRNL_TimeDly(1); //must wait
        //__inf("csi0 iic write %d, %x = %x\n", ret, reg[i],reg[i+1]);
    }

    return ret;
}


//�豸ģʽ���ú����������豸��ͬʱ�����豸������Ӧ��csi����
__s32 csi_dev_set_mode(__u32 mode)
{
    //__u32 i = 0;
    __u8 *reg_table;//����sensor��iic��ȶ���__u8����__u16������
    __u32 reg_table_len;

    if (last_mode_num == mode)
    {
        __inf("no need to change csi mode!\n");
    }

    else
    {
        csi_mode = csi_mode_all.csi_mode[mode];

        switch (mode)
        {
        case 0:
        {
            reg_table = reg_720_576_25000;
            reg_table_len = sizeof(reg_720_576_25000);
            conf.input_fmt  = CSI_CCIR656;
            conf.output_fmt = CSI_FRAME_UV_CB_YUV422;
            conf.field_sel  = CSI_ODD;
            conf.seq        = CSI_YUYV;//����ʵ���޸�
            conf.vref       = CSI_LOW;//����ʵ���޸�
            conf.href       = CSI_LOW;//����ʵ���޸�
            conf.clock      = CSI_RISING;//����ʵ���޸�

            CSI_set_base((CSI_PORT == 0x00) ? 0x00 : 0x01);
            CSI_configure(&conf);
            CSI_set_size(0, csi_mode.size.width * 2, 0, csi_mode.size.height / 2, csi_mode.size.width); //ע����������������sourcΪinterlaced������ƴ��frame����ô�˴���heightӦ��Ϊfield�ĸ߶ȣ���frame�ĸ߶ȵ�һ�룬��progressiveΪframe�ĸ߶Ȳ�һ�����мǣ�
            break;
        }
        case 1:
        {
            reg_table = reg_720_576_25000;
            reg_table_len = sizeof(reg_720_576_25000);
            conf.input_fmt  = CSI_CCIR656;
            conf.output_fmt = CSI_FRAME_UV_CB_YUV420;
            conf.field_sel  = CSI_ODD;
            conf.seq        = CSI_YUYV;//����ʵ���޸�
            conf.vref       = CSI_LOW;//����ʵ���޸�
            conf.href       = CSI_LOW;//����ʵ���޸�
            conf.clock      = CSI_RISING;//����ʵ���޸�

            CSI_set_base((CSI_PORT == 0x00) ? 0x00 : 0x01);
            CSI_configure(&conf);
            CSI_set_size(0, csi_mode.size.width * 2, 0, csi_mode.size.height / 2, csi_mode.size.width); //ע����������������sourcΪinterlaced������ƴ��frame����ô�˴���heightӦ��Ϊfield�ĸ߶ȣ���frame�ĸ߶ȵ�һ�룬��progressiveΪframe�ĸ߶Ȳ�һ�����мǣ�
            break;
        }
        case 2:
        {
            reg_table = reg_720_480_30000;
            reg_table_len = sizeof(reg_720_480_30000);
            conf.input_fmt  = CSI_CCIR656;
            conf.output_fmt = CSI_FRAME_UV_CB_YUV422;
            conf.field_sel  = CSI_ODD;
            conf.seq        = CSI_YUYV;//����ʵ���޸�
            conf.vref       = CSI_LOW;//����ʵ���޸�
            conf.href       = CSI_LOW;//����ʵ���޸�
            conf.clock      = CSI_RISING;//����ʵ���޸�

            CSI_set_base((CSI_PORT == 0x00) ? 0x00 : 0x01);
            CSI_configure(&conf);
            CSI_set_size(0, csi_mode.size.width * 2, 0, csi_mode.size.height / 2, csi_mode.size.width); //ע����������������sourcΪinterlaced������ƴ��frame����ô�˴���heightӦ��Ϊfield�ĸ߶ȣ���frame�ĸ߶ȵ�һ�룬��progressiveΪframe�ĸ߶Ȳ�һ�����мǣ�
            break;
        }
        case 3:
        {
            reg_table = reg_720_480_30000;
            reg_table_len = sizeof(reg_720_480_30000);
            conf.input_fmt  = CSI_CCIR656;
            conf.output_fmt = CSI_FRAME_UV_CB_YUV420;
            conf.field_sel  = CSI_ODD;
            conf.seq        = CSI_YUYV;//����ʵ���޸�
            conf.vref       = CSI_LOW;//����ʵ���޸�
            conf.href       = CSI_LOW;//����ʵ���޸�
            conf.clock      = CSI_RISING;//����ʵ���޸�

            CSI_set_base((CSI_PORT == 0x00) ? 0x00 : 0x01);
            CSI_configure(&conf);
            CSI_set_size(0, csi_mode.size.width * 2, 0, csi_mode.size.height / 2, csi_mode.size.width); //ע����������������sourcΪinterlaced������ƴ��frame����ô�˴���heightӦ��Ϊfield�ĸ߶ȣ���frame�ĸ߶ȵ�һ�룬��progressiveΪframe�ĸ߶Ȳ�һ�����мǣ�
            break;
        }

        }


        //block_write(fiic,da,reg_table,reg_table_len);

        last_buf_mode = csi_mode.csi_buf_scrab_mode;
        last_mode_num = mode;
        __inf("set mode %d finished! buf_scrab_mode=%d (0=CSI0, 2=01LR, 4=01UD, 5/7=TDM_2/4CH)\n", mode, last_buf_mode);
    }
    return EPDK_OK;
}



__s32  csi_dev_init(void)
{
    //	__u32 i,clock = 0;
    __u8 *reg_table;
    __u32 reg_table_len;

    //init_gpio();
    iic_set_clock(fiic, 200 * 1000); //200*1000=200kHz

    __inf("csi_dev_init......\n");

    //��д���豸֧�ֵ�ģʽ
    csi_mode_all.number = 4;//��֧��ģʽ����
    //��Щ���ö���ָ��������ݲ�����Ϊ�˴��ݸ���ʾ���߱���ʱ��֡����
    //---------------------------------------------------------------------------------------------------------------
    //PAL
    csi_mode_all.csi_mode[0].color_format = PIXEL_YUV422;//��ʽ
    csi_mode_all.csi_mode[0].component_seq = YUV_SEQ_UVUV;//����˳��
    csi_mode_all.csi_mode[0].store_mode = YUV_MOD_UV_NON_MB_COMBINED;//�洢��ʽ
    csi_mode_all.csi_mode[0].size.width = 720;//��
    csi_mode_all.csi_mode[0].size.height = 480;//��
    csi_mode_all.csi_mode[0].frame_rate = 25000;//֡�ʣ���λHz
    csi_mode_all.csi_mode[0].frame_period = 40000;//֡����=1/֡�ʣ���λus��
    csi_mode_all.csi_mode[0].color_space = BT601;//ɫ�ʿռ�
    csi_mode_all.csi_mode[0].csi_buf_scrab_mode = CSI0_FULL;
    //PAL
    csi_mode_all.csi_mode[1].color_format = PIXEL_YUV420;//��ʽ
    csi_mode_all.csi_mode[1].component_seq = YUV_SEQ_UVUV;//����˳��
    csi_mode_all.csi_mode[1].store_mode = YUV_MOD_UV_NON_MB_COMBINED;//�洢��ʽ
    csi_mode_all.csi_mode[1].size.width = 720;//��
    csi_mode_all.csi_mode[1].size.height = 480;//��
    csi_mode_all.csi_mode[1].frame_rate = 25000;//֡�ʣ���λHz
    csi_mode_all.csi_mode[1].frame_period = 40000;//֡����=1/֡�ʣ���λus��
    csi_mode_all.csi_mode[1].color_space = BT601;//ɫ�ʿռ�
    csi_mode_all.csi_mode[1].csi_buf_scrab_mode = CSI0_FULL;
    //NTSC
    csi_mode_all.csi_mode[2].color_format = PIXEL_YUV422;//��ʽ
    csi_mode_all.csi_mode[2].component_seq = YUV_SEQ_UVUV;//����˳��
    csi_mode_all.csi_mode[2].store_mode = YUV_MOD_UV_NON_MB_COMBINED;//�洢��ʽ
    csi_mode_all.csi_mode[2].size.width = 720;//��
    csi_mode_all.csi_mode[2].size.height = 480;//��
    csi_mode_all.csi_mode[2].frame_rate = 30000;//֡�ʣ���λHz
    csi_mode_all.csi_mode[2].frame_period = 33333;//֡����=1/֡�ʣ���λus��
    csi_mode_all.csi_mode[2].color_space = BT601;//ɫ�ʿռ�
    csi_mode_all.csi_mode[2].csi_buf_scrab_mode = CSI0_FULL;
    //NTSC
    csi_mode_all.csi_mode[3].color_format = PIXEL_YUV420;//��ʽ
    csi_mode_all.csi_mode[3].component_seq = YUV_SEQ_UVUV;//����˳��
    csi_mode_all.csi_mode[3].store_mode = YUV_MOD_UV_NON_MB_COMBINED;//�洢��ʽ
    csi_mode_all.csi_mode[3].size.width = 720;//��
    csi_mode_all.csi_mode[3].size.height = 480;//��
    csi_mode_all.csi_mode[3].frame_rate = 30000;//֡�ʣ���λHz
    csi_mode_all.csi_mode[3].frame_period = 33333;//֡����=1/֡�ʣ���λus��
    csi_mode_all.csi_mode[3].color_space = BT601;//ɫ�ʿռ�
    csi_mode_all.csi_mode[3].csi_buf_scrab_mode = CSI0_FULL;
    //-----------------------------------------------------------------------------------------------------------

    __inf("bit1605 init...\n");
    //�����豸�ĳ�ʼ������
    block_write(fiic, da, reg_init, sizeof(reg_init)); //��sensor�����ÿ��Ƿ���Ҫ����ע��size�ǰ�__u8����__u16������

    csi_dev_set_mode(1);//��Ҫ��������һ��Ĭ��ģʽ

    return EPDK_OK;
}

__s32  csi_dev_exit(void)
{
    csi_clk_off();
    csi_power_off();

    return EPDK_OK;
}


//�豸color effect���ú���
__s32 csi_dev_set_color_effect(__csi_color_effect_t color_effect)
{

    return EPDK_OK;
}
//�豸awb���ú���
__s32 csi_dev_set_awb(__csi_awb_t awb)
{

    return EPDK_OK;
}
//�豸ae���ú���
__s32 csi_dev_set_ae(__csi_ae_t ae)
{

    return EPDK_OK;
}
//�豸bright���ú���
__s32 csi_dev_set_bright(__csi_bright_t bright)
{

    return EPDK_OK;
}

//�豸contrast���ú���
__s32 csi_dev_set_contrast(__csi_contrast_t contrast)
{

    return EPDK_OK;
}

//�豸band���ú���
__s32 csi_dev_set_band(__csi_band_t band)
{

    return EPDK_OK;
}


#endif
