#ifndef __BIRD_RTP_LIB_H
#define __BIRD_RTP_LIB_H 

#include <linux/kernel.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <asm/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <mach/gpio.h>
#include <linux/earlysuspend.h>
//#include <mach/board.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/irq.h>

#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <soc/sprd/adi.h>
#include <soc/sprd/adc.h>
#include <soc/sprd/regulator.h>
#include <linux/regulator/consumer.h>
#include <linux/delay.h>


#define RTP_DRIVER_VERSION    "V0.0<2016/05/18>"
#define RTP_NAME          "birdrtp_ts"


//#define TEST_ADC
#define TEST_PRESS
#define Y_FLIP
#define RTP_POLL_TIME         10
#define RTP_DEBUG_FUNC_ON 1
#define RTP_DEBUG_ON 1
#define DEVICE_NAME_SIZE  20

#define  TOUCH_SCR_NBR_SAMPLES                     10                 /* Number of samples for Touch screen measurements */
#define  TOUCH_SCR_SETUP_DLY_uS                   150                /* Setup delay (us) for I/O pins                   */
#define  TOUCH_SCR_ADC_DLY_uS                       3                 /* convertion time (ns) for ADC ADC_clk = 4.5 Mhz  */
#define  TOUCH_SCR_DFLT_DLY_uS                    20                 /* Maximum delay in case the Timer #2 is stopped...*/

#define ADC_RTP_X_CHANNEL			20
#define ADC_RTP_Y_CHANNEL			1

#define TOUCH_SCR_PRESS_CNT   3   //´¥ÃþÆÁÁ¬Ðø¼ì²âµ½TOUCH_SCR_PRESS_CNT´Î°´ÏÂ£¬ÔòÈÏÎªÕæµÄ°´ÏÂ
                                  //´¥ÃþÆÁÁ¬Ðø¼ì²âµ½TOUCH_SCR_PRESS_CNT´ÎËÉ¿ª£¬ÔòÈÏÎªÕæµÄËÉ¿ª
#define TSP_SAMPLE_NUM  5  //Á½´Î×ª»»µÄ²îÖµ
                            //ÒÔ²âÊÔÊ±Îª×¼

#define TOUCH_DOW  (0x1 << 0)   //´¥Ãþ°´ÏÂ
#define TOUCH_ACT  (0x1 << 1)  //´¥Ãþ°´ÏÂÇÒ¼üÖµÓÐÐ§


#define  TOUCH_SCR_MIN_X_ADC                    90
#define  TOUCH_SCR_MIN_Y_ADC                    220   //ÒÔÉÏËÄ¸öÖµÊÇ²âÊÔÊ±µÃ³öµÄ
#define  TOUCH_SCR_MAX_X_ADC                    927
#define  TOUCH_SCR_MAX_Y_ADC                    3970     
#define  TIME_VALUE                  			125//250//500      //Éè¶¨Á½´Î±¨µãµÄÑÓÊ±   ms  

#define TOUCH_SCR_DELTA_X_ADC         TOUCH_SCR_MAX_X_ADC  -   TOUCH_SCR_MIN_X_ADC
#define TOUCH_SCR_DELTA_Y_ADC         TOUCH_SCR_MAX_Y_ADC  -    TOUCH_SCR_MIN_Y_ADC


struct birdrtp_platform_data{
	int RTP_XH_gpio_number;
	int RTP_YH_gpio_number;
	int RTP_XL_gpio_number;
	int RTP_YL_gpio_number;
	int irq_gpio_number;
	int RTP_MAX_X;
	int RTP_MAX_Y;
};


struct birdrtp_ts_data {
        spinlock_t irq_lock;
        struct input_dev  *input_dev;
        struct hrtimer timer;
        struct work_struct  work;
        struct early_suspend early_suspend;
        s32 irq_is_disable;
        s32 use_irq;
        u16 abs_x_max;
        u16 abs_y_max;
        u8  max_touch_num;
        u8  int_trigger_type;
        u8  green_wake_mode;
        u8  enter_update;
        u8  rtp_is_suspend;
        u8  rtp_rawdiff_mode;
        u8  rtp_cfg_len;
        u8  fixed_cfg;
        u8  fw_error;
        u8  pnl_init_error;
		int irq;
		char name[DEVICE_NAME_SIZE];
};

typedef struct bsp_touch_scr_status {
    unsigned short   TouchScrX;                                   
    unsigned short    TouchScrY;          
    unsigned char  TouchScrIsPressed;  
} TOUCH_SCR_STATUS;

#define RTP_INFO(fmt,arg...)           printk("<<-RTP-INFO->> "fmt"\n",##arg)
#define RTP_ERROR(fmt,arg...)          printk("<<-RTP-ERROR->> "fmt"\n",##arg)
#define RTP_DEBUG(fmt,arg...)          do{\
                                         if(RTP_DEBUG_ON)\
                                         printk("<<-RTP-DEBUG->> [%d]"fmt"\n",__LINE__, ##arg);\
                                       }while(0)
#define RTP_DEBUG_ARRAY(array, num)    do{\
                                         signed long i;\
                                         unsigned char * a = array;\
                                         if(RTP_DEBUG_ARRAY_ON)\
                                         {\
                                            printk("<<-RTP-DEBUG-ARRAY->>\n");\
                                            for (i = 0; i < (num); i++)\
                                            {\
                                                printk("%02x   ", (a)[i]);\
                                                if ((i + 1 ) %10 == 0)\
                                                {\
                                                    printk("\n");\
                                                }\
                                            }\
                                            printk("\n");\
                                        }\
                                       }while(0)
#define RTP_DEBUG_FUNC()               do{\
                                         if(RTP_DEBUG_FUNC_ON)\
                                         printk("<<-RTP-FUNC->> Func:%s@Line:%d\n",__func__,__LINE__);\
                                       }while(0)
#define RTP_SWAP(x, y)                 do{\
                                         typeof(x) z = x;\
                                         x = y;\
                                         y = z;\
                                       }while (0)
                                       
 #define RTP_INT_IRQ(pin)     gpio_to_irq(pin)

#define RTP_GPIO_AS_INPUT(pin)          do{\
                                            gpio_direction_input(pin);\
                                        }while(0)
#define RTP_GPIO_AS_INT(pin)            do{\
                                            RTP_GPIO_AS_INPUT(pin);\
                                        }while(0)
#define RTP_GPIO_GET_VALUE(pin)         gpio_get_value(pin)
#define  RTP_GPIO_SET_VALUE(pin,leval)            gpio_set_value(pin,leval)
#define RTP_GPIO_OUTPUT(pin,level)      gpio_direction_output(pin,level)
#define RTP_GPIO_REQUEST(pin, label)    gpio_request(pin, label)
#define RTP_GPIO_FREE(pin)              gpio_free(pin)
#define RTP_IRQ_TAB                     {IRQ_TYPE_EDGE_RISING, IRQ_TYPE_EDGE_FALLING, IRQ_TYPE_LEVEL_LOW, IRQ_TYPE_LEVEL_HIGH}
#define RTP_INT_TRIGGER   2 //8                                             

extern void birdrtp_set_xy_volte(int xh,int xl,int yh,int yl);
extern void TouchSrcIOPutInit(void);
extern void  TouchScr_GetAD_X(TOUCH_SCR_STATUS  *p_status);
extern void  TouchScr_GetAD_Y(TOUCH_SCR_STATUS  *p_status);
extern void  TouchScrConvert(TOUCH_SCR_STATUS  *p_status);
extern unsigned char GetTouchScrPressed(TOUCH_SCR_STATUS  *p_status);
extern void GetTouchScr(TOUCH_SCR_STATUS  *p_status);
extern int adc_rtp_get_adc(int channel);
extern void RtpAdcTest(void);
extern struct birdrtp_platform_data  birdrtp_ts;

#endif
