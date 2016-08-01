#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define TEST_IOCTL_BASE 99
#define TEST_IOCTL_0    _IO(TEST_IOCTL_BASE,0)
#define TEST_IOCTL_1    _IOW(TEST_IOCTL_BASE,1,int)

int main()
{

      // FILE *fp0 = NULL;
       int fb = 0;
       char Buf[4096];    
       ssize_t ret;
       int a;
       int rest;

       /*初始化Buf*/

    //   strcpy(Buf,"Mem is char dev!");

   //    printf("BUF: %s\n",Buf);

       /*打开设备文件*/

       fb = open("/dev/memdev", O_RDWR);
       if (fb < 0) {
              printf("Open Memdev0 Error!\n");
              return -1;
       }
       
		ret = read(fb,&a,sizeof(int));
		if(ret < 0 )
			printf("read error : ret = %lu\n",ret);
			
		printf("read buf = %d\n",a);
		
		a = 200;
		ret = write(fb,&a,sizeof(int));
		if(ret < 0 )
			printf("write error : ret = %lu\n",ret);
		//printf("write buf = %d\n",a);
		
		rest = ioctl(fb, TEST_IOCTL_1,&a);
			printf("rest = %d\n",rest);

       /*写入设备*/
      // write(Buf, sizeof(Buf), 1, fb);
		//ret = write(fb,Buf,sizeof(Buf));
		//printf("ret = %lu\n",ret);
		//if(ret < 0 )
		//	printf("write error \n");
   

       /*（思考没有该指令，会有何后果)？*/

       //答：因为前面假如写了8个字节，指针就会停在8个字节处，如果要读的话，指针就从8个字节以后开始读，

       //这样就读不到东西，因为8个字节后没写东西

       /*重新定位文件位置*/

       //fseek(fb,0,SEEK_SET);

       /*清除Buf*/
    //   printf("BUF: %s\n",Buf);
	  // strcpy(Buf,"Buf is NULL!");


      
         /*读出设备*/

     //  read(Buf, sizeof(Buf), 1, fb);
		//ret = read(fb,Buf,sizeof(Buf));
		//printf("ret = %lu\n",ret);
	//	if(ret <0 )
		//	printf("write error \n");
      

       /*检测结果*/

     //  printf("BUF: %s\n",Buf);
      close(fb);

        return 0; 

}
