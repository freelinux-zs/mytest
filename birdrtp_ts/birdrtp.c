#include <birdrtplib.h>


static unsigned long tsp_x_last=0,tsp_y_last=0; //È¥¶¶¶¯ÓÃ,ÉÏÒ»´ÎµÄÖµ
static unsigned char TouchScrPressCnt=0; 


void birdrtp_set_xy_volte(int xh,int xl,int yh,int yl)
{
	
	RTP_GPIO_SET_VALUE(birdrtp_ts.RTP_XH_gpio_number,xh);  			
	RTP_GPIO_SET_VALUE(birdrtp_ts.RTP_XL_gpio_number,xl);
	RTP_GPIO_SET_VALUE(birdrtp_ts.RTP_YH_gpio_number,yh);
	RTP_GPIO_SET_VALUE(birdrtp_ts.RTP_YL_gpio_number,yl);
}

void TouchSrcIOPutInit(void)
{
	birdrtp_set_xy_volte(1,1,1,0) ;  /***init panal***/
    	//ÕâÑùÒ»ÉèÖÃ£¬³õÊ¼»¯init×´Ì¬£¬¼ì²éEINTÊÇ·ñ±»µçÆ½£¬¼ì²âÊÇ·ñ°´ÏÂ£¬Ì§Æð
}


void  TouchScr_GetAD_X(TOUCH_SCR_STATUS  *p_status)
{
    unsigned short   databuffer[TOUCH_SCR_NBR_SAMPLES],temp;
    unsigned long   i,j,adc_val;
    
	birdrtp_set_xy_volte(0,1,1,0) ;  /**read x**/
    ndelay(TOUCH_SCR_SETUP_DLY_uS);   //µÈ´ýÉÏÃæµÄÉèÖÃÎÈ¶¨        

	   //ADCÁ¬Ðø×ª»» TOUCH_SCR_NBR_SAMPLES ´Î
    for (i = 0; i < TOUCH_SCR_NBR_SAMPLES; i++) 
    {           
        databuffer [i]= sci_adc_get_value(ADC_RTP_X_CHANNEL,0);
        ndelay(TOUCH_SCR_ADC_DLY_uS);
    }        
    //½«×ª»»½á¹ûÉýÐòÅÅÁÐ
    for (j= 0; j< TOUCH_SCR_NBR_SAMPLES; j++)
    for (i = 0; i < TOUCH_SCR_NBR_SAMPLES; i++) 
    {           
        if(databuffer[i]>databuffer[i+1])//ÉýÐòÅÅÁÐ
				{
					temp=databuffer[i+1];
					databuffer[i+1]=databuffer[i];
					databuffer[i]=temp;
				}  
    } 
		
   //È¡ÖÐ¼äÖµÇóÆ½¾ù
   i=TOUCH_SCR_NBR_SAMPLES>>1;  //³ý2
   adc_val = databuffer[i-1]+ databuffer[i]+ databuffer[i+1]+ databuffer[i+2];
	adc_val = adc_val>>2;  //³ý4
   
   p_status->TouchScrX         = (unsigned short)adc_val;  //¸üÐÂ¼üÖµ
   
    //È¥¶¶¶¯´¦Àí==
    if(tsp_x_last>adc_val)i=tsp_x_last-adc_val;
    else i=adc_val-tsp_x_last;         //µÚÒ»²½Ëã³ö²îÖµ
    if(i>TSP_SAMPLE_NUM)   //ÎÞÐ§,Çå±êÖ¾Î»
    {                
       p_status->TouchScrIsPressed &= ~TOUCH_ACT;
    }
    tsp_x_last = adc_val;  //¸üÐÂÉÏÒ»´ÎµÄÖµ    
    //--    
}


void  TouchScr_GetAD_Y(TOUCH_SCR_STATUS  *p_status)
{

    unsigned short   databuffer[TOUCH_SCR_NBR_SAMPLES],temp;
    unsigned long    i,j,adc_val;
      
	birdrtp_set_xy_volte(1,0,0,1) ;  /**read y**/

    ndelay(TOUCH_SCR_SETUP_DLY_uS);   //µÈ´ýÉÏÃæµÄÉèÖÃÎÈ¶¨   
    
    //ADCÁ¬Ðø×ª»» TOUCH_SCR_NBR_SAMPLES ´Î
    for (i = 0; i < TOUCH_SCR_NBR_SAMPLES; i++) 
    {           
	      databuffer [i]= sci_adc_get_value(ADC_RTP_Y_CHANNEL,0);
	      ndelay(TOUCH_SCR_ADC_DLY_uS);
    }        
    //½«×ª»»½á¹ûÉýÐòÅÅÁÐ
    for (j= 0; j< TOUCH_SCR_NBR_SAMPLES; j++)
    for (i = 0; i < TOUCH_SCR_NBR_SAMPLES; i++) 
    {           
        if(databuffer[i]>databuffer[i+1])//ÉýÐòÅÅÁÐ
				{
						temp=databuffer[i+1];
						databuffer[i+1]=databuffer[i];
						databuffer[i]=temp;
				}  
    } 
	  //´®¿Úµ÷ÊÔ²é¿´½á¹û
	  // for (j= 0; j< TOUCH_SCR_NBR_SAMPLES; j++)
	  //     Usart0_Printf("databuffer[%d]=%d\r\n",j,databuffer[j]);
	   //È¡ÖÐ¼äÖµÇóÆ½¾ù
	   i=TOUCH_SCR_NBR_SAMPLES>>1;  //³ý2
	   adc_val = databuffer[i-1]+ databuffer[i]+ databuffer[i+1]+ databuffer[i+2];
	   adc_val = adc_val>>2;  //³ý4
	   
	   p_status->TouchScrY = (unsigned short)adc_val;//¸üÐÂ¼üÖµ
	   
     //È¥¶¶¶¯´¦Àí
    if(tsp_y_last>adc_val)i=tsp_y_last-adc_val;
    else i=adc_val-tsp_y_last;         //µÚÒ»²½Ëã³ö²îÖµ
    if(i>TSP_SAMPLE_NUM)   //ÎÞÐ§Çå±êÖ¾Î»
    {     
       p_status->TouchScrIsPressed &= ~TOUCH_ACT;
    }
    tsp_y_last = adc_val;  //¸üÐÂÉÏÒ»´ÎµÄÖµ
 
}


void  TouchScrConvert(TOUCH_SCR_STATUS  *p_status)
{
    signed long  x_pixels;
    signed long  y_pixels;
    unsigned char   f_x=0,f_y=0;
    //ÎÞÐ§Öµ£¬Ö±½Ó·µ»Ø==
    //Èç¹û³¬¹ý·¶Î§==
    //x
    if(p_status->TouchScrX<=TOUCH_SCR_MIN_X_ADC)
    {
      p_status->TouchScrX = 0; f_x=1;
    }
    else if(p_status->TouchScrX>=TOUCH_SCR_MAX_X_ADC)
    {
       p_status->TouchScrX = birdrtp_ts.RTP_MAX_X;f_x=1;
    }
    //y
    if(p_status->TouchScrY<=TOUCH_SCR_MIN_Y_ADC)
    {
      p_status->TouchScrY = 0; f_y=1;
    }
    else if(p_status->TouchScrY>=TOUCH_SCR_MAX_Y_ADC)
    {
       p_status->TouchScrY = birdrtp_ts.RTP_MAX_Y; f_y=1;
    }
    //--
    //--
    x_pixels = p_status->TouchScrX;
    y_pixels = p_status->TouchScrY;
    if(f_x==0)
    {  
      x_pixels = (x_pixels - TOUCH_SCR_MIN_X_ADC)*birdrtp_ts.RTP_MAX_X;
      x_pixels /= TOUCH_SCR_DELTA_X_ADC; 
    }
    if(f_y==0)
    {      
        y_pixels  = (y_pixels - TOUCH_SCR_MIN_Y_ADC)  * birdrtp_ts.RTP_MAX_Y;
        y_pixels /= TOUCH_SCR_DELTA_Y_ADC; 
    }
    p_status->TouchScrX = (unsigned short)x_pixels;
    p_status->TouchScrY = (unsigned short)y_pixels;
}


unsigned char GetTouchScrPressed(TOUCH_SCR_STATUS  *p_status)
{
	unsigned char TouchScrPressFlg;  //´¥ÃþÆÁ°´ÏÂ±êÖ¾
	TouchSrcIOPutInit(); //³õÊ¼»¯ÎªEINT MODE£¬ 

	//TouchScrPressFlg = GetTouchScrY1(); //²»ÄÜÕâÑùÐ´,Î»¿í²»Ò»ÖÂ
	if(RTP_GPIO_GET_VALUE(birdrtp_ts.irq_gpio_number))
	 TouchScrPressFlg = 0;
	else
	 TouchScrPressFlg = 1;
	//==
	if(TouchScrPressFlg)  //Îª¸ß±íÊ¾°´ÏÂ
	{	
		if(TouchScrPressCnt<TOUCH_SCR_PRESS_CNT)
			     TouchScrPressCnt++;	 //°´ÏÂ¼ÆÊý
	}
	else
	{
	 if(TouchScrPressCnt>0)
			     TouchScrPressCnt--;	 //ËÉ¿ª¼ÆÊý	
	}
	//--	
	if(TouchScrPressCnt==TOUCH_SCR_PRESS_CNT)p_status->TouchScrIsPressed = 1;
	if(TouchScrPressCnt==0)p_status->TouchScrIsPressed = 0;
		
	return  TouchScrPressFlg;
}

void GetTouchScr(TOUCH_SCR_STATUS  *p_status)
{
	unsigned char flg = GetTouchScrPressed(p_status);  
	// step1£ºÃæ°åµÄ°´ÏÂÓëÊÍ·ÅÏû¶¶¶¯==
	if((flg==0) || (p_status->TouchScrIsPressed == 0))
	{
		return;  //Èç¹ûÓÐ¶¶¶¯Ôò²»×÷AD×ª»»Ö±½Ó·µ»Ø,±£ÁôÉÏÒ»´ÎµÄÖµ
	}		  	                            
	//--
	//step2: AD×ª»»µÃµ½ADÖµ==
	p_status->TouchScrIsPressed |= TOUCH_ACT;  //Ä¬ÈÏÏÂÃæ»ñµÃµÄ¼üÖµÊÇÓÐÐ§µÄ
	TouchScr_GetAD_X(p_status);
	TouchScr_GetAD_Y(p_status);

	//TouchSrcIOPutInit(); //Õâ¸öº¯ÊýÒ»¶¨Òª¼ÇµÃµ÷ÓÃ---->¼ì²âÍêÊÇ·ñ°´ÏÂÌ§Æðºóµ÷ÓÃ´Ëº¯Êý¡£
	//--

	//step3: ×ª»»ADÖµÎª¼üÖµ==
	TouchScrConvert(p_status);
	  //--
}

void RtpAdcTest(void)
{
		unsigned long   test_adc_x,test_adc_y;
		birdrtp_set_xy_volte(0,1,1,0) ;  /**read y**/
		ndelay(300);
		test_adc_x = sci_adc_get_value(ADC_RTP_X_CHANNEL,0);
		ndelay(10000);
		birdrtp_set_xy_volte(1,0,0,1) ;  /**read y**/
		ndelay(300);
		test_adc_y = sci_adc_get_value(ADC_RTP_Y_CHANNEL,0);
		ndelay(10000);
		RTP_INFO("RTP  test adc X=%d ,  Y = %d",test_adc_x,test_adc_y);
}
