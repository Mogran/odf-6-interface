//最后修改日期:2014-12-10-11:34
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
/* for net */
#include <netinet/in.h>
#include <net/if.h>
#include <net/route.h>
#include <sys/socket.h>
#include <netdb.h>    
/* for signal & time */
#include "os_struct.h"
#include "none-rtl-task.h" 

#define DATA_SOURCE_MOBILE     1
#define DATA_SOURCE_UART	   2	
#define DATA_SOURCE_NET	   	   3		
#define TX_BUFFER_SIZE		  600 
#define SIZE				  100


extern OS_BOARD s3c44b0x;
extern OS_UART  uart0;
extern pthread_mutex_t mobile_mutex;
extern  int   uart0fd,alarmfd,uart1fd; 

static unsigned char calccrc8(unsigned char* ptr,unsigned short len)
{
	unsigned char  i = 0,crc = 0;

	while(len--){
		i = 0x80;
		while(i != 0){
		if(((crc & 0x80) != 0) && (((*ptr) & i) != 0)){
			crc <<= 1;
		}else if(((crc & 0x80)!= 0) && (((*ptr) & i)==0)){
		    crc <<= 1;
		 	crc ^= 0x31;
 		}else if(((crc & 0x80) == 0)&& (((*ptr) & i) !=0)){
       		crc <<= 1;
	 		crc ^= 0x31;
  		}else if(((crc & 0x80) == 0) &&(((*ptr)&i)== 0)){
  		   crc <<= 1;
  		}
  			i >>= 1;
		}
		ptr++;
	}

  return(crc);
} 


/*
 * \breif   : netDatacrccheck
 * \version : v1.0
 */
static unsigned char  netDatacrccheck(unsigned char * srcData,unsigned short datalen)
{
	unsigned short src_len = 0;
	unsigned char  calced_crc = 0;
	unsigned char  src_crc = 0,ret = 0;
	unsigned char  *p = srcData;	 

	src_len = (*(p + 1) << 8) | (*(p + 2));
	calced_crc = calccrc8(srcData, src_len);
	src_crc  = *(srcData + src_len);

	if(calced_crc == src_crc) ret = 1;

	return ret;
}

/*
 * @func          : module_mainBoard_orderOperations_handle
 * @func's own    : (仅供主程序调用)
 * @brief         : 手机端，所有的工单操作集合
 * param[] ...    : param[1] :数据 param[2]:数据来源 1：手机终端 2:通信串口0(UART0)
 * @created  by   : MingLiang.Lu
 * @created date  : 2014-11-10 
 * @modified by   : MingLiang.Lu
 * @modified date : 2014-11-19 
 */
int module_mainBoard_orderOperations_handle(unsigned char *src,
		unsigned char datatype,int sock)
{
	if(src == NULL)//程序健壮性检查，不累赘叙说
		return 0;
	
	if(0x02 == src[4]){		 //单端新建 0x02
		Jianei_Order_new_1_port(src, datatype, sock);
	}else if(0x03 == src[4]){//单端改跳 0x03
		Jianei_Order_exchange_2_ports(src, datatype, sock);
	}else if(0x04 == src[4]){//单端拆除 0x04
		Jianei_Order_del_1_port(src, datatype, sock);
	}else if(0x05 == src[4]){//双端新建 0x05
		Jianei_Order_new_2_ports(src, datatype, sock);
	}else if(0x06 == src[4]){//双端拆除 0x06
		Jianei_Order_del_2_ports(src, datatype, sock);
	}else if(0x07 == src[4]){//批量新建 0x07
		Jianei_Batch_new_5_ports(src, datatype, sock);
	}else if(0x08 == src[4]){//架间新建 0x08
		Jiajian_Order_new_2_ports(src, datatype, sock);
	}else if(0x18 == src[4]){//架间新建 0x18
		Jiajian_Order_new_2_ports(src, datatype, sock);
	}else if(0x09 == src[4]){//架间双端删除 0x09
		Jiajian_Order_del_2_ports(src, datatype, sock);
	}else if(0x19 == src[4]){//架间双端删除 0x19
		Jiajian_Order_del_2_ports(src, datatype, sock);
	}else if(0x10 == src[4]){//架间批量工单 0x10
		Jiajian_Order_new_5_ports(src, datatype, sock);
	}else if(0x30 == src[4]){//架间批量工单 0x30
		Jiajian_Order_new_5_ports(src, datatype, sock);
	}else if(0xee == src[4]){//wrong type mark
		orderOperations_wrong_mark_input(src, datatype, sock);
	}

	return 0;
}
/*
 * @func          : module_mainBoard_orderOperations_handle
 * @func's own    : (仅供主程序调用)
 * @brief         : 手机端，所有的工单操作集合
 * param[] ...    : param[1] :数据 param[2]:数据来源 1：手机终端 2:通信串口0(UART0)
 * @created  by   : MingLiang.Lu
 * @created date  : 2014-11-10 
 * @modified by   : MingLiang.Lu
 * @modified date : 2014-11-19 
 */
int module_mainBoard_Webmaster_Part_orderOperations_handle(unsigned char *src,int sock,
		unsigned char datatype)
{
	if(src == NULL)//程序健壮性检查，不累赘叙说
		return 0;
	
	if(0x02 == src[20]){		 //单端新建 0x02
		Jianei_Order_new_1_port(src, datatype, sock);
	}else if(0x03 == src[20]){//单端改跳 0x03
		Jianei_Order_exchange_2_ports(src, datatype, sock);
	}else if(0x04 == src[20]){//单端拆除 0x04
		Jianei_Order_del_1_port(src, datatype, sock);
	}else if(0x05 == src[20]){//双端新建 0x05
		Jianei_Order_new_2_ports(src, datatype, sock);
	}else if(0x06 == src[20]){//双端拆除 0x06
		Jianei_Order_del_2_ports(src, datatype, sock);
	}else if(0x07 == src[20]){//批量新建 0x07
		Jianei_Batch_new_5_ports(src, datatype, sock);
	}else if(0x08 == src[20]){//架间新建 0x08
		Jiajian_Order_new_2_ports(src, datatype, sock);
	}else if(0x18 == src[20]){//架间新建 0x18
		Jiajian_Order_new_2_ports(src, datatype, sock);
	}else if(0x09 == src[20]){//架间双端删除 0x09
		Jiajian_Order_del_2_ports(src, datatype, sock);
	}else if(0x19 == src[20]){//架间双端删除 0x19
		Jiajian_Order_del_2_ports(src, datatype, sock);
	}else if(0x10 == src[20]){//架间批量工单 0x10
		Jiajian_Order_new_5_ports(src, datatype, sock);
	}else if(0x30 == src[20]){//架间批量工单 0x30
		Jiajian_Order_new_5_ports(src, datatype, sock);
	}

	return 0;
}

/*
 * @func          : order_tasks_to_sim3u1xx_board
 * @func's own    : (仅供本文件内部调用)
 * @brief         : 工单任务派发，专门发送函数
 * param[] ...    : \param[1] :数据 | \param[2]:数据来源 :通信串口0(uart0)
 * @created  by   : MingLiang.Lu
 * @created date  : 2014-11-19 
 * @modified by   : MingLiang.Lu
 * @modified date : 2014-11-19 
 */
static int order_tasks_to_sim3u1xx_board(void)
{
	unsigned short TaskFrameDataLength = 0,nBytes = 0;
	unsigned char  TaskPorts[24] = {'\0'};
	unsigned char  i = 0,j = 0;

	for(;i < s3c44b0x.frameNums;i++){
		if(uart0.txBuf[i][3] != 0x0)TaskPorts[i] = 1;
	}

	for(;j < 3;j++){//重发机制，3次

		for(i = 0; i < s3c44b0x.frameNums;i++){
			if(TaskPorts[i] == 1){
				TaskFrameDataLength = uart0.txBuf[i][1] << 8| uart0.txBuf[i][2] + 2;
				write(uart0fd,&uart0.txBuf[i][0],TaskFrameDataLength);
				memset(uart0.rxBuf,0,sizeof(uart0.rxBuf));
				usleep(200000);
				nBytes = read(uart0fd,uart0.rxBuf,100);
				if(nBytes > 0){
					if(uart0.rxBuf[3] == 0xa6){
						TaskPorts[i] = 0x0;
						memset(&uart0.txBuf[i][0],0,TX_BUFFER_SIZE);
					}
				}
			}
		}
	}

	if(isArrayEmpty(TaskPorts,24) == 1){
		return 1;
	}else{
		return 0;
	}
} 

/*
 * @func          : myStringcmp
 * @func's own    : (仅供本文件内部调用)
 * @brief         : 重写的字符串比较函数
 * param[] ...    : \param[1] :数据 | \param[2]:数据来源 :通信串口0(uart0)
 * @created  by   : MingLiang.Lu
 * @created date  : 2014-11-19 
 * @modified by   : MingLiang.Lu
 * @modified date : 2014-11-19 
 */
static int myStringcmp(unsigned char *src,unsigned char *dst,unsigned short len)
{
	int i = 0;

	if(src == NULL || dst == NULL)
		return 0;
	
	for(;i < len;i++){
		if(*(src+i) != *(dst+i))
			break;
	}

	if(i == len)
		return 1;
	else 
		return 0;

}
/*
 * @func          : isArrayEmpty
 * @func's own    : (仅供本文件内部调用)
 * @brief         : 检查数组是否为空
 * param[] ...    : \param[1] :数据 | \param[2]:数据来源 :通信串口0(uart0)
 * @created  by   : MingLiang.Lu
 * @created date  : 2014-11-19 
 * @modified by   : MingLiang.Lu
 * @modified date : 2014-11-21 
 * @return		  : 为空返回1, 不为空返回0
 */
static int isArrayEmpty(unsigned char *src,unsigned short len)
{
	unsigned int i = 0;
	
	if(src == NULL)
		return 0;

	for(;i < len; i++){
		if(*(src+i) != 0x0)break;	
	}	
	if(i == len)
		return 1;
	else 
		return 0;	
	
}

//--------------------------end of file --------------------------------


