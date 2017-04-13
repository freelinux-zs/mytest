#include "birdrtplib.h"

#if GTP_ICS_SLOT_REPORT
#include <linux/input/mt.h>
#endif

static const char *birdrtp_ts_name = RTP_NAME;
static struct workqueue_struct *birdrtp_wq;
struct birdrtp_platform_data  birdrtp_ts = {
	.RTP_XH_gpio_number = -1,
	.RTP_YH_gpio_number = -1,
	.RTP_XL_gpio_number = -1,
	.RTP_YL_gpio_number  = -1,
	.irq_gpio_number = -1,
	.RTP_MAX_X = 0,
	.RTP_MAX_Y = 0 ,
};

#ifdef CONFIG_HAS_EARLYSUSPEND
/*******************************************************
Function:
    Early suspend function.
Input:
    h: early_suspend struct.
Output:
    None.
*******************************************************/
static void birdrtp_ts_early_suspend(struct early_suspend *h)
{
		RTP_INFO("birdrtp_ts_early_suspend\n");
        msleep(58);
}


/*******************************************************
Function:
    Late resume function.
Input:
    h: early_suspend struct.
Output:
    None.
*******************************************************/

static void birdrtp_ts_late_resume(struct early_suspend *h)
{
        RTP_INFO("birdrtp_ts_late_resume\n");
}
#endif

/*******************************************************
Function:
    Disable irq function
Input:
    ts: goodix i2c_client private data
Output:
    None.
*********************************************************/
void rtp_irq_disable(struct  birdrtp_ts_data *ts)
{
        unsigned long irqflags;

        RTP_DEBUG_FUNC();

        spin_lock_irqsave(&ts->irq_lock, irqflags);
        if (!ts->irq_is_disable) {
                ts->irq_is_disable = 1;
                disable_irq_nosync(ts->irq);
        }
        spin_unlock_irqrestore(&ts->irq_lock, irqflags);
}

/*******************************************************
Function:
    Enable irq function
Input:
    ts: goodix i2c_client private data
Output:
    None.
*********************************************************/
void rtp_irq_enable(struct birdrtp_ts_data *ts)
{
        unsigned long irqflags = 0;

        RTP_DEBUG_FUNC();

        spin_lock_irqsave(&ts->irq_lock, irqflags);
        if (ts->irq_is_disable) {
                enable_irq(ts->irq);
                ts->irq_is_disable = 0;
        }
        spin_unlock_irqrestore(&ts->irq_lock, irqflags);
}


/*******************************************************
Function:
    Report touch point event
Input:
    ts: goodix i2c_client private data
    id: trackId
    x:  input x coordinate
    y:  input y coordinate
    w:  input pressure
Output:
    None.
*********************************************************/
static void rtp_touch_down(struct birdrtp_ts_data* ts,s32 x,s32 y)
{
		input_report_abs(ts->input_dev, ABS_X, x);
        input_report_abs(ts->input_dev, ABS_Y, y);
        input_report_key(ts->input_dev,BTN_TOUCH,1);
        input_sync(ts->input_dev);

}

/*******************************************************
Function:
    Report touch release event
Input:
    ts: goodix i2c_client private data
Output:
    None.
*********************************************************/
static void rtp_touch_up(struct birdrtp_ts_data* ts)
{
	    input_report_key(ts->input_dev,BTN_TOUCH,0);
        input_sync(ts->input_dev);
}

/*******************************************************
Function:
    BirdRTP touchscreen work function
Input:
    work: work struct of BirdRTP_workqueue
Output:
    None.
*********************************************************/
static void birdrtp_ts_work_func(struct work_struct *work)
{
	RTP_INFO("birdrtp_ts_work_func\n");
	struct birdrtp_ts_data *ts = NULL;
	TOUCH_SCR_STATUS     TouchValue;

    ts = container_of(work, struct birdrtp_ts_data, work);
    if (ts->enter_update) {
            return;
    }

#ifdef TEST_ADC
	while(1)
	{
		RtpAdcTest();
	}
#endif


//GetTouchScr(&TouchValue);
#if  1
	GetTouchScr(&TouchValue); //把 TouchScrConvert注释掉就是ad的值，否则就是键值


#ifdef Y_FLIP
	TouchValue.TouchScrY =  birdrtp_ts.RTP_MAX_Y - TouchValue.TouchScrY;
#endif

	RTP_INFO("X=%d,Y=%d",TouchValue.TouchScrX,TouchValue.TouchScrY );

	if(GetTouchScrPressed(&TouchValue)){
		 RTP_INFO("down.................");
		 rtp_touch_down(ts,120,TouchValue.TouchScrY);
		// hrtimer_start(&ts->timer, ktime_set(0, (RTP_POLL_TIME+6)*1000000), HRTIMER_MODE_REL);
		 hrtimer_start(&ts->timer, ktime_set(TIME_VALUE/1000, (TIME_VALUE%1000)*1000000), HRTIMER_MODE_REL);
		 ts->use_irq = 0;
		}else{
		RTP_INFO("up.................");
		rtp_touch_up(ts);
		ts->use_irq = 1;
		} 
#endif


	if (ts->use_irq){
		rtp_irq_enable(ts);
	}
}

/*******************************************************
Function:
    Timer interrupt service routine for polling mode.
Input:
    timer: timer struct pointer
Output:
    Timer work mode.
        HRTIMER_NORESTART: no restart mode
*********************************************************/
static enum hrtimer_restart birdrtp_ts_timer_handler(struct hrtimer *timer)
{
        struct birdrtp_ts_data *ts = container_of(timer, struct birdrtp_ts_data , timer);

        RTP_DEBUG_FUNC();

        queue_work(birdrtp_wq, &ts->work);
       // hrtimer_start(&ts->timer, ktime_set(0, (RTP_POLL_TIME+6)*1000000), HRTIMER_MODE_REL);
        return HRTIMER_NORESTART;
}

/*******************************************************
Function:
    External interrupt service routine for interrupt mode.
Input:
    irq:  interrupt number.
    dev_id: private data pointer
Output:
    Handle Result.
        IRQ_HANDLED: interrupt handled successfully
*********************************************************/
static irqreturn_t birdrtp_ts_irq_handler(int irq, void *dev_id)
{
        struct birdrtp_ts_data *ts = dev_id;

        RTP_DEBUG_FUNC();
		
		rtp_irq_disable(ts);

		//hrtimer_start(&ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
      	queue_work(birdrtp_wq, &ts->work);

        return IRQ_HANDLED;
}


/*******************************************************
Function:
    Initialize rtp.
Input:
    ts: birdrtp private data
Output:
    Executive outcomes.
        0: succeed, otherwise: failed
*******************************************************/
static s32 rtp_init_panel(struct birdrtp_ts_data *ts)
{
        s32 ret = -1;
		
        if ((ts->abs_x_max == 0) && (ts->abs_y_max == 0)) {
                ts->abs_x_max = birdrtp_ts.RTP_MAX_X;
                ts->abs_y_max = birdrtp_ts.RTP_MAX_Y;
                ts->int_trigger_type = RTP_INT_TRIGGER;
        }

		RTP_GPIO_OUTPUT(birdrtp_ts.RTP_XH_gpio_number,1);
		RTP_GPIO_OUTPUT(birdrtp_ts.RTP_XL_gpio_number,1);
		RTP_GPIO_OUTPUT(birdrtp_ts.RTP_YH_gpio_number,1);
		RTP_GPIO_OUTPUT(birdrtp_ts.RTP_YL_gpio_number, 0);  //modif

		RTP_INFO("birdrtp_ts.RTP_XH_gpio_number :%d\n",RTP_GPIO_GET_VALUE(birdrtp_ts.RTP_XH_gpio_number));
		RTP_INFO("birdrtp_ts.RTP_XL_gpio_number :%d\n",RTP_GPIO_GET_VALUE(birdrtp_ts.RTP_XL_gpio_number));
		RTP_INFO("birdrtp_ts.RTP_YH_gpio_number :%d\n",RTP_GPIO_GET_VALUE(birdrtp_ts.RTP_YH_gpio_number));
		RTP_INFO("birdrtp_ts.RTP_YL_gpio_number :%d\n",RTP_GPIO_GET_VALUE(birdrtp_ts.RTP_YL_gpio_number));
		RTP_INFO("birdrtp_ts.irq_gpio_number :%d\n",RTP_GPIO_GET_VALUE(birdrtp_ts.irq_gpio_number));
		RTP_INFO("abs_x_max :%d,  abs_y_max :%d\n", ts->abs_x_max, ts->abs_y_max);
        msleep(10);

        return 0;
}



/*******************************************************
Function:
    Request gpio(INT &x1 & x2 & y1 & y2) ports.
Input:
    ts: private data.
Output:
    Executive outcomes.
        >= 0: succeed, < 0: failed
*******************************************************/
static s8 rtp_request_io_port(struct birdrtp_ts_data *ts)
{
        s32 ret = 0;

        RTP_DEBUG_FUNC();
        ret = RTP_GPIO_REQUEST(birdrtp_ts.irq_gpio_number, "RTP_INT_IRQ");
        if (ret < 0) {
                RTP_ERROR("Failed to request GPIO:%d, ERRNO:%d", (s32)birdrtp_ts.irq_gpio_number, ret);
                ret = -ENODEV;
        }else{
				RTP_GPIO_AS_INT(birdrtp_ts.irq_gpio_number);
				ts->irq = RTP_INT_IRQ(birdrtp_ts.irq_gpio_number);
		}

        ret = RTP_GPIO_REQUEST(birdrtp_ts.RTP_XH_gpio_number, "RTP_X1");
        if (ret < 0) {
                RTP_ERROR("Failed to request GPIO:%d, ERRNO:%d",(s32)birdrtp_ts.RTP_XH_gpio_number,ret);
                ret = -ENODEV;
        }

        ret = RTP_GPIO_REQUEST(birdrtp_ts.RTP_YH_gpio_number, "RTP_Y1");
        if (ret < 0) {
                RTP_ERROR("Failed to request GPIO:%d, ERRNO:%d",(s32)birdrtp_ts.RTP_YH_gpio_number,ret);
                ret = -ENODEV;
        }
		
		ret = RTP_GPIO_REQUEST(birdrtp_ts.RTP_XL_gpio_number, "RTP_X2");
        if (ret < 0) {
                RTP_ERROR("Failed to request GPIO:%d, ERRNO:%d",(s32)birdrtp_ts.RTP_XL_gpio_number,ret);
                ret = -ENODEV;
        }
		
		ret = RTP_GPIO_REQUEST(birdrtp_ts.RTP_YL_gpio_number, "RTP_Y2");
        if (ret < 0) {
                RTP_ERROR("Failed to request GPIO:%d, ERRNO:%d",(s32)birdrtp_ts.RTP_YL_gpio_number,ret);
                ret = -ENODEV;
        }



        if(ret < 0) {
                RTP_GPIO_FREE(birdrtp_ts.irq_gpio_number);
                RTP_GPIO_FREE(birdrtp_ts.RTP_XH_gpio_number);	
                RTP_GPIO_FREE(birdrtp_ts.RTP_YH_gpio_number);
                RTP_GPIO_FREE(birdrtp_ts.RTP_XL_gpio_number);
				RTP_GPIO_FREE(birdrtp_ts.RTP_YL_gpio_number);				
        }

        return ret;
}

/*******************************************************
Function:
    Request interrupt.
Input:
    ts: private data.
Output:
    Executive outcomes.
        0: succeed, -1: failed.
*******************************************************/
static s8 rtp_request_irq(struct birdrtp_ts_data *ts)
{
        s32 ret = -1;
        const u8 irq_table[] = RTP_IRQ_TAB;

        RTP_DEBUG_FUNC();
        RTP_DEBUG("INT trigger type:%x \n", ts->int_trigger_type);
        
	ret  = request_irq(ts->irq,
                           birdrtp_ts_irq_handler,
                           irq_table[ts->int_trigger_type],
                           ts->name,
                           ts);
        if (ret) {
                RTP_ERROR("Request IRQ failed!ERRNO:%d.", ret);
                RTP_GPIO_AS_INPUT(birdrtp_ts.irq_gpio_number);
                RTP_GPIO_FREE(birdrtp_ts.irq_gpio_number);
                return -1;
        } else {
                rtp_irq_disable(ts);
                ts->use_irq = 1;
                return 0;
        }
}

/*******************************************************
Function:
    Request input device Function.
Input:
    ts:private data.
Output:
    Executive outcomes.
        0: succeed, otherwise: failed.
*******************************************************/
static s8 rtp_request_input_dev(struct birdrtp_ts_data *ts)
{
        s8 ret = -1;
        s8 phys[32];


        RTP_DEBUG_FUNC();

        ts->input_dev = input_allocate_device();
        if (ts->input_dev == NULL) {
                RTP_ERROR("Failed to allocate input device.");
                return -ENOMEM;
        }

        ts->input_dev->evbit[0] = BIT_MASK(EV_SYN) | BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS) ;

        ts->input_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);


#if RTP_CHANGE_X2Y
        RTP_SWAP(ts->abs_x_max, ts->abs_y_max);
#endif

#if 0
        input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, ts->abs_x_max, 0, 0);
        input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, ts->abs_y_max, 0, 0);
        input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, 255, 0, 0);
        input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
        input_set_abs_params(ts->input_dev, ABS_MT_TRACKING_ID, 0, 255, 0, 0);
   #else
		input_set_abs_params(ts->input_dev, ABS_X, 0, ts->abs_x_max, 0, 0);
		input_set_abs_params(ts->input_dev, ABS_Y, 0, ts->abs_y_max, 0, 0); 
  #endif

        sprintf(phys, "input/ts");
        ts->input_dev->name = birdrtp_ts_name;
        ts->input_dev->phys = phys;
        ts->input_dev->id.bustype = BUS_HOST;
        ts->input_dev->id.vendor = 0xDEAD;
        ts->input_dev->id.product = 0xBEEF;
        ts->input_dev->id.version = 10427;

        ret = input_register_device(ts->input_dev);
        if (ret) {
                RTP_ERROR("Register %s input device failed", ts->input_dev->name);
                return -ENODEV;
        }

        return 0;
}




#ifdef CONFIG_OF  
static struct birdrtp_platform_data *birdrtp_ts_parse_dt(struct platform_device  *pdev)
{
	int ret = -1;
	struct birdrtp_platform_data *pdata = NULL;
	struct device_node *np = pdev->dev.of_node;

	pdata = kzalloc(sizeof(*pdata), GFP_KERNEL);
	if (!pdata) {
		dev_err(pdev, "Could not allocate struct birdrtp_ts_platform_data");
		return NULL;
	}
	pdata->RTP_XH_gpio_number = of_get_gpio(np, 0);  /*X+*/
	if(pdata->RTP_XH_gpio_number < 0){
		dev_err(pdev, "fail to get reset_gpio_number\n");
		goto fail;
	}
	pdata->RTP_YH_gpio_number = of_get_gpio(np, 1);  /*Y+*/
	if(pdata->RTP_YH_gpio_number < 0){
		dev_err(pdev, "fail to get irq_gpio_number\n");
		goto fail;
	}
	pdata->RTP_XL_gpio_number = of_get_gpio(np, 2); /*X-*/
	if(pdata->RTP_XL_gpio_number < 0){
		dev_err(pdev, "fail to get irq_gpio_number\n");
		goto fail;
	}
	pdata->RTP_YL_gpio_number = of_get_gpio(np, 3);  /*X+*/
	if(pdata->RTP_YL_gpio_number < 0){
		dev_err(pdev, "fail to get irq_gpio_number\n");
		goto fail;
	}
	pdata->irq_gpio_number = of_get_gpio(np, 4);     /*enit*/
	if(pdata->irq_gpio_number < 0){
		dev_err(pdev, "fail to get irq_gpio_number\n");
		goto fail;
	}

	ret = of_property_read_u32(np, "RTP_MAX_X", &pdata->RTP_MAX_X);
	if(ret){
		dev_err(pdev, "fail to get TP_MAX_X\n");
		goto fail;
	}
	ret = of_property_read_u32(np, "RTP_MAX_Y", &pdata->RTP_MAX_Y);
	if(ret){
		dev_err(pdev, "fail to get TP_MAX_Y\n");
		goto fail;
	}
	return pdata;
fail:
	kfree(pdata);
	return NULL;
}
#endif


/*******************************************************
Function:
    I2c probe.
Input:
    client: i2c device struct.
    id: device id.
Output:
    Executive outcomes.
        0: succeed.
*******************************************************/
static int birdrtp_ts_probe(struct platform_device *pdev)
{
		int ret;
		struct device *dev = &pdev->dev;
		struct birdrtp_platform_data* pdata = NULL;	
        struct birdrtp_ts_data *ts;

        RTP_DEBUG_FUNC();

        //do NOT remove these logs
        RTP_INFO("GTP Driver Version: %s", RTP_DRIVER_VERSION);
        RTP_INFO("GTP Driver Built@%s, %s", __TIME__, __DATE__);


#ifdef CONFIG_OF 
	struct device_node *np = dev->of_node;
	
	if(np) {
			pdata = birdrtp_ts_parse_dt(pdev);
			if (pdata == NULL) {
					RTP_INFO("get dts data failed!\n");
					return -ENODEV;
			}
	} else {
			RTP_INFO("dev.of_node is NULL!\n");
			return -ENODEV;
	}
#endif
		birdrtp_ts.RTP_MAX_X =  pdata->RTP_MAX_X;
		birdrtp_ts.RTP_MAX_Y =  pdata->RTP_MAX_Y;
		
		if(pdata->RTP_XH_gpio_number > 0)
			birdrtp_ts.RTP_XH_gpio_number = pdata->RTP_XH_gpio_number;
			
		if(pdata->RTP_XL_gpio_number > 0)
			birdrtp_ts.RTP_XL_gpio_number = pdata->RTP_XL_gpio_number;
			
		if(pdata->RTP_YH_gpio_number > 0)
			birdrtp_ts.RTP_YH_gpio_number = pdata->RTP_YH_gpio_number;
			
		if(pdata->RTP_YL_gpio_number > 0)
			birdrtp_ts.RTP_YL_gpio_number = pdata->RTP_YL_gpio_number;
			
		if(pdata->irq_gpio_number > 0)
			birdrtp_ts.irq_gpio_number = pdata->irq_gpio_number;
			
printk("birdrtp_ts->RTP_XH_gpio_number  = %d, birdrtp_ts->RTP_YH_gpio_number = %d,birdrtp_ts->RTP_XL_gpio_number = %d, birdrtp_ts->RTP_YL_gpio_number = %d,birdrtp_ts.RTP_MAX_X=%d,birdrtp_ts.RTP_MAX_Y=%d\n\r",\
			birdrtp_ts.RTP_XH_gpio_number ,birdrtp_ts.RTP_YH_gpio_number,birdrtp_ts.RTP_XL_gpio_number,birdrtp_ts.RTP_YL_gpio_number,birdrtp_ts.RTP_MAX_X,birdrtp_ts.RTP_MAX_Y);
		

        ts = kzalloc(sizeof(*ts), GFP_KERNEL);
        if (ts == NULL) {
                RTP_ERROR("Alloc GFP_KERNEL memory failed.");
                return -ENOMEM;
        }

        memset(ts, 0, sizeof(*ts));
        INIT_WORK(&ts->work, birdrtp_ts_work_func);
   
        spin_lock_init(&ts->irq_lock);          // 2.6.39 later
        // ts->irq_lock = SPIN_LOCK_UNLOCKED;   // 2.6.39 & before
		
 

        ts->rtp_rawdiff_mode = 0;

        ret = rtp_request_io_port(ts);
        if (ret < 0) {
                RTP_ERROR("RTP request IO port failed.");
                kfree(ts);
                return ret;
        }

        ret = rtp_init_panel(ts);
        if (ret < 0) {
                RTP_ERROR("RTP init panel failed.");
                ts->abs_x_max = birdrtp_ts.RTP_MAX_X;
                ts->abs_y_max = birdrtp_ts.RTP_MAX_Y;
                ts->int_trigger_type = RTP_INT_TRIGGER;
        }

        ret = rtp_request_input_dev(ts);
        if (ret < 0) {
                RTP_ERROR("RTP request input dev failed");
        }

#ifdef CONFIG_HAS_EARLYSUSPEND
        ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
		ts->early_suspend.suspend = birdrtp_ts_early_suspend;
        ts->early_suspend.resume = birdrtp_ts_late_resume;
        register_early_suspend(&ts->early_suspend);
#endif

        ret = rtp_request_irq(ts);
        if (ret < 0) {
                RTP_INFO("RTP works in polling mode.");
        } else {
                RTP_INFO("RTP works in interrupt mode.");
        }

		
        hrtimer_init(&ts->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
        ts->timer.function = birdrtp_ts_timer_handler;
    //    hrtimer_start(&ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);			

        if (ts->use_irq) {
                rtp_irq_enable(ts);
        }

        return 0;
}


/*******************************************************
Function:
    birdrtp touchscreen driver release function.
Input:
    platform_device :  device struct.
Output:
    Executive outcomes. 0---succeed.
*******************************************************/
static int birdrtp_ts_remove(struct birdrtp_ts_data *ts)
{
        RTP_DEBUG_FUNC();

        RTP_INFO("GTP driver removing...");
		input_unregister_device(ts->input_dev);

        return 0;
}



static const struct of_device_id birdrtp_of_match[] = {
		{.compatible = "bird,birdrtp_drv"},
		{ }
}
MODULE_DEVICE_TABLE(of, birdrtp_of_match);

static struct platform_driver birdrtp_device_driver = {
	.probe		= birdrtp_ts_probe,
	.remove     = birdrtp_ts_remove,
	.suspend    = NULL,
	.resume     = NULL,
	.driver = {
		.name = "birdrtp_driver",
		.owner = THIS_MODULE,
		.of_match_table = birdrtp_of_match,
	},
};

/*******************************************************
Function:
    Driver Install function.
Input:
    None.
Output:
    Executive Outcomes. 0---succeed.
********************************************************/
static int /*__devinit*/ birdrtp_ts_init(void)
{
        s32 ret;

        RTP_DEBUG_FUNC();
        RTP_INFO("RTP driver installing...");
        
        
        birdrtp_wq = create_singlethread_workqueue("birdrtp_wq");
        if (!birdrtp_wq) {
                RTP_ERROR("Creat workqueue failed.");
                return -ENOMEM;
        }

        return platform_driver_register(&birdrtp_device_driver);
}

/*******************************************************
Function:
    Driver uninstall function.
Input:
    None.
Output:
    Executive Outcomes. 0---succeed.
********************************************************/
static void __exit birdrtp_ts_exit(void)
{
        RTP_DEBUG_FUNC();
        RTP_INFO("RTP driver exited.");
		platform_driver_unregister(&birdrtp_device_driver);
        if (birdrtp_wq) {
                destroy_workqueue(birdrtp_wq);
        }
}

late_initcall(birdrtp_ts_init);
module_exit(birdrtp_ts_exit);

MODULE_DESCRIPTION("RTP Driver");
MODULE_LICENSE("GPL");

