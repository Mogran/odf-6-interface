#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <sys/time.h>
#include <pthread.h>
#include "./include/os_struct.h"

#define MAXFRAME	8
#define INFOSIZE    2530
#define MAXTRAY		6
//#define INFOSIZE    3370
#define INFO_START_INDEX 114

typedef struct {
	pthread_t i3_thid;
	uint8_t  olddat[1100];
	uint8_t  datbuf[2200];
}i3_data_t;

static i3_data_t i3_format;

extern board_info_t boardcfg;
extern jijia_stat_t default_frame;
extern jijia_stat_t current_frame;

struct FramePortInfos {
	uint8_t dyb_portinfo[INFOSIZE*MAXFRAME+INFO_START_INDEX];

	uint8_t zkb_rep0d[MAXFRAME][10]; /* zkb这3个字母是主控板的首字母的缩写，jkb代表接口板，dyb代表单元板的意思*/	
	uint8_t zkb_repA6[MAXFRAME][10]; /* rep这3个字母是英文reply的意思，即代表主控回复接口板的命令数组 */
	uint8_t zkb_rep20[MAXFRAME][10]; /* 0d/20/A6 分别代表了要发送给接口板的数据命令 */
	uint8_t zkb_rxBuf[INFOSIZE];

   uint32_t zkb_stats[MAXFRAME];     /* zkb_stats用来表示主控板中记录着哪些框是要采集的，其中设置1表示要采集，0xdead表示不存在这个框 */	
   uint32_t jkb_stats[MAXFRAME];     /* jkb_stats这个数组用来存放那些要采集的框，在采集进行中的状态变化*/

	struct  timeval timeBegin;
	struct  timeval timeOut;

}PortsInfo;	

extern int uart0fd;
extern OS_UART  uart0;


static unsigned short crc16_ccitt_table[256] ={
	0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
	0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
	0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
	0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
	0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
	0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
	0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
	0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
	0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
	0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
	0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
	0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
	0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
	0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
	0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
	0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
	0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
	0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
	0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
	0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
	0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
	0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
	0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
	0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
	0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
	0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
	0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
	0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
	0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
	0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
	0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
	0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};


/*
 *@brief    : resource_crc16_calc
 *@param[0] : data body
 *@param[1] : data len
 *@return   : uint16_t format data 
 *@description :   data fromat : LSB First 
 *                 Init data   : 0x0000 
 *                 Xor  data   : 0x0000
 * */
static unsigned short resource_crc16_calc(unsigned char* ptr, unsigned int len)
{
    unsigned short crc_reg = 0x0; 
          
    while (len--) 
        crc_reg = (crc_reg >> 8) ^ crc16_ccitt_table[(crc_reg ^ *ptr++) & 0xff];
 
    return crc_reg;
}

/*
 * 该函数只能在资源采集的时候使用
 *
 */
static unsigned char calccrc8(uint8_t* ptr,uint32_t len)
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

/****************************************************************************
   Fuction     : 
   Description : 在主控板向接口板的每个框发起采集命令的时候，提前准备一些数据
   Call        : 不调用其他任何外部函数
   Called by   : 该函数仅被module_mainBoard_resource_handle()这个函数调用
   Input       : 网管发来的采集数据，这里主要区分是一次采集还是二次采集
              
   Output      : 不对外输出
   Return      : 无返回值
                 
   Others      : 无其他说明
 *****************************************************************************/
static void zkb_txBuf_data_tianchong(unsigned char *src)
{
	unsigned char idex = 0;
	unsigned char zkb_pol[7] = {0x7e, 0x00, 0x05, 0x20, 0x00, 0x00, 0x5a};		/*zkb_pol is short for zkb_poll_cmd */
	unsigned char zkb_col[8] = {0x7e, 0x00, 0x06, 0x0d, 0x01, 0x00, 0x00, 0x5a};/*zkb_col is short for zkb_collect_cmd */	
	unsigned char zkb_rep[9] = {0x7e, 0x00, 0x07, 0xa6, 0x0d, 0x00, 0x00, 0x00, 0x5a};


	memset((void*)&PortsInfo, 0x0, sizeof(struct FramePortInfos));

	/* clear array */
	for(idex = 0; idex < MAXFRAME; idex++){
		memset(&PortsInfo.zkb_rep0d[idex][0], 0, 10);
		memset(&PortsInfo.zkb_repA6[idex][0], 0, 10);
		memset(&PortsInfo.zkb_rep20[idex][0], 0, 10);
	}
	
	/* Judge first time collect or Second time collect*/
	if(0x23 == src[3] || 0x24 == src[3]){
		zkb_col[4] = 0x11;
	}else if(0x20 == src[3]){
		zkb_col[4] = 0x01;
	}

	/* tianchong cmd 0x0d to PortsInfo.zkb_rep0d */
	for(idex = 0; idex < MAXFRAME; idex++){
		zkb_col[5] = idex + 1;
		zkb_col[6] = calccrc8(zkb_col, 6);
		memcpy(&PortsInfo.zkb_rep0d[idex][0], zkb_col, zkb_col[1]<<8|zkb_col[2]+2);
	}

	/* tianchong 0x20 PortsInfo.zkb_rep20 */
	for(idex = 0; idex < MAXFRAME; idex++){
		zkb_pol[4] = idex + 1;
		zkb_pol[5] = calccrc8(zkb_pol, 5);
		memcpy(&PortsInfo.zkb_rep20[idex][0], zkb_pol, zkb_pol[1]<<8|zkb_pol[2]+2);	
	}

	/* set jkb default value 1*/
	memset(&PortsInfo.jkb_stats[0], 0, MAXFRAME);
	for(idex = 0; idex < MAXFRAME; idex++){
		PortsInfo.jkb_stats[idex] = 1;
		PortsInfo.zkb_stats[idex] = 1;
	}

	/* reround time count */
	PortsInfo.timeBegin.tv_sec = 0;
	PortsInfo.timeBegin.tv_usec = 0;
	PortsInfo.timeOut.tv_sec = 0;
	PortsInfo.timeOut.tv_usec = 0;

	/*clear dyb_portinfo*/
	memset(&PortsInfo.dyb_portinfo[0], 0, MAXFRAME*INFOSIZE+INFO_START_INDEX);
	PortsInfo.dyb_portinfo[3] = src[3];
}	
/****************************************************************************
   Fuction     :
   Description : 根据主控板识别的下面挂载的框数量决定采集的超时时间
   Call        : 不调用任何其他函数
   Called by   : 仅在module_resource.c文件中内部调用
   Input       : 
              param[0]: devicesnum, 主控板在上电的时候，识别的外部的框数量
   Return      : 返回计算出来的采集上限时间
                 
   Others      : 无任何其他说明
 *****************************************************************************/
static uint32_t calc_collect_time_deadline(uint8_t devicesnum)
{
	uint32_t time_stamp = 0;

	if(devicesnum == 8){
		time_stamp = 10*1000000;
	}else if(devicesnum == 16){
		time_stamp = 19*1000000;
	}

	return time_stamp;
}

/****************************************************************************
   Fuction     : 
   Description : 资源采集主体函数中的接收并处理接口板发来数据的模块
   Call        : 
   Called by   : 只被 module_mainBoard_resource_handle()函数调用
   Input       : 
              
   Return      : 
                 
   Others      : 
 *****************************************************************************/
static uint32_t zkb_txBuf_data_part(void)
{
	static uint8_t jkb_num = 1;
	uint8_t ret_jkb_num = jkb_num;

	printf("jkb_num = %d, PortsInfo.jkb_stats[%d] = %d..\n", jkb_num, jkb_num - 1, PortsInfo.jkb_stats[jkb_num-1]);

	if(0x01 == PortsInfo.jkb_stats[jkb_num - 1]){
		write(uart0fd, &PortsInfo.zkb_rep0d[jkb_num - 1][0], PortsInfo.zkb_rep0d[jkb_num-1][1]<<8|PortsInfo.zkb_rep0d[jkb_num-1][2]+2);
		usleep(150000);
	}else if(0x02 == PortsInfo.jkb_stats[jkb_num - 1]){
		write(uart0fd, &PortsInfo.zkb_rep20[jkb_num - 1][0], PortsInfo.zkb_rep20[jkb_num-1][1]<<8|PortsInfo.zkb_rep20[jkb_num-1][2]+2);
		usleep(400000);
	}else if(0x03 == PortsInfo.jkb_stats[jkb_num - 1]){
		jkb_num += 1;
		if(jkb_num > MAXFRAME){
			jkb_num = 1;
		}
		return 0;
	}

	jkb_num += 1;
	if(jkb_num > MAXFRAME){
		jkb_num = 1;
	}

	return ret_jkb_num;
}

/****************************************************************************
   Fuction     : zkb_rxBuf_data_part(uint32_t jkb_num)
   Description : 资源采集过程中，主控板处理接口板发来的数据的处理模块 
   Call        : 该函数没有调用任何其他的函数
   Called by   : 该函数仅被process_main.c调用
   Input       : 
                 param[0]：当前正在巡检的接口板的编号
   Output      : 无对外输出
   Return      : 
                 0：表示收到的接口板发来的数据无效
				 1：表示收到的接口板发来的数据有效，并被正确的解析了。
   Others      : 无其他描述
 *****************************************************************************/
static uint8_t zkb_rxBuf_data_part(uint32_t jkb_num)
{
	int nBytes = 0x0, nlength = 0x0;
	uint32_t dyb_portinfo_storedidex= 0, eidType = 32;
	uint8_t  dbg_buf[10], calced_crc8 = 0;

	memset(&PortsInfo.zkb_rxBuf, 0x0, INFOSIZE);
	nBytes = read(uart0fd, &PortsInfo.zkb_rxBuf, INFOSIZE);
	if(nBytes <= 0){
		return 0;/* msg invalid*/
	}else{
		nlength = PortsInfo.zkb_rxBuf[1]<<8|PortsInfo.zkb_rxBuf[2];
		if(nlength > INFOSIZE){
			printf("%s, %d , msg invaild , length%d is more than limit value\n", __func__, __LINE__, nlength);
			return 0;/* msg invalid */
		}else{
			calced_crc8 = calccrc8(&PortsInfo.zkb_rxBuf[0], nlength);
			if(calced_crc8 == PortsInfo.zkb_rxBuf[nlength]){
				/*crc check PASS !!! */
				if(0xa6 == PortsInfo.zkb_rxBuf[3]){
					PortsInfo.jkb_stats[jkb_num - 1] = 0x2;	
				}else if(0x01 == PortsInfo.zkb_rxBuf[4] || 0x11 == PortsInfo.zkb_rxBuf[4]){
					dyb_portinfo_storedidex = (jkb_num-1)*(((eidType+3)*12)*6) + INFO_START_INDEX;
					memcpy(&PortsInfo.dyb_portinfo[dyb_portinfo_storedidex], &PortsInfo.zkb_rxBuf[6], (eidType+3)*12*MAXTRAY);
					dbg_buf[0] = 0x7e;			
					dbg_buf[1] = 0x00;			
					dbg_buf[2] = 0x07;			
					dbg_buf[3] = 0xa6;			
					dbg_buf[4] = 0x0d;			
					dbg_buf[5] = 0x01;			
					dbg_buf[6] = jkb_num;			
					dbg_buf[7] = calccrc8(dbg_buf, 7);			
					dbg_buf[8] = 0x5a;			
					write(uart0fd, dbg_buf, 9);	
					usleep(150000);
					PortsInfo.jkb_stats[jkb_num - 1] = 0x3;	
					printf("%s : %d frame collect ............ [done]\n", __TIME__, jkb_num);
				}
				return 1;
			}else{
#if 0				
				/* 收到接口发来的数据校验后发现crc不对,因此返回0*/
				dbg_buf[0] = 0x7e;			
				dbg_buf[1] = 0x00;			
				dbg_buf[2] = 0x07;			
				dbg_buf[3] = 0xf6;			
				dbg_buf[4] = 0x0d;			
				dbg_buf[5] = 0x01;			
				dbg_buf[6] = jkb_num;			
				dbg_buf[7] = calccrc8(dbg_buf, 7);			
				dbg_buf[8] = 0x5a;			
				write(uart0fd, dbg_buf, 9);	
				usleep(150000);
#endif				
				return 0;
			}
		}
	}

}

/****************************************************************************
   Fuction     :
   Description : 检查是不是所有的框都采集完成了
   Call        : 
   Called by   : 
   Input       : 
              
   Output      : 
   Return      : 
                 
   Others      : 
 *****************************************************************************/
static uint32_t zkb_check_collectTask_Done(void)
{
	uint8_t idex = 0 , finishedFrames = 0;

	for(idex = 0; idex < MAXFRAME; idex++){
		if((0x1 == PortsInfo.zkb_stats[idex]) && (0x03 == PortsInfo.jkb_stats[idex])){
			finishedFrames += 1;
		}
	}

	if(finishedFrames == MAXFRAME){
		printf("collect task Done \n");
		return 1;
	}

	return 0;
}

static uint16_t old_version_data_2_new_i3_data_format(uint8_t *olddat_buf, uint16_t olddat_len,
		uint8_t *newdat_buf)
{
	uint16_t loops = 0, new_dat_len = 0;

	for(loops = 0, new_dat_len = 0; loops < olddat_len; loops++){
		if(olddat_buf[loops] == 0x7e){
			newdat_buf[new_dat_len++] = 0x7d;
			newdat_buf[new_dat_len++] = 0x5e;
		}else if(olddat_buf[loops] == 0x7d){
			newdat_buf[new_dat_len++] = 0x7d;
			newdat_buf[new_dat_len++] = 0x5d;	
		}else{
			newdat_buf[new_dat_len++] = olddat_buf[loops];
		}
	}

	return new_dat_len;
}

/****************************************************************************
   Fuction     :
   Description : I3 new protocol needs to send data to 1024/per
   Call        : 
   Called by   : 
   Input       : 
              
   Output      : 
   Return      : 
                 
   Others      : 
 *****************************************************************************/
void *pthread_collect_data_upload_to_webmaster(void *socket_fd)
{
   int sockfd  = *(int*)socket_fd;
   int total_frame_bytes = 0;
   uint16_t total_packet_num = 0, packet_num = 0;
   uint16_t data_index = 0, ssize = 0, crc16_val = 0;
   uint8_t  rxbuf[30];	
   int flags = 0, nbytes = 0;	

   pthread_detach(pthread_self());	

   /* we should set sockfd's attribution BLOCK Mode */
   flags = fcntl(sockfd, F_GETFL, 0);	
   fcntl(sockfd, F_SETFL, flags&~O_NONBLOCK);

   total_frame_bytes = 35*12*MAXTRAY*MAXFRAME + 114;
   total_packet_num  = total_frame_bytes / 1024;
   if(total_frame_bytes%1024){
      total_packet_num += 1;
   }     

   for(packet_num = 0; packet_num < total_packet_num; ){

      if(packet_num == 0){
         data_index = 0;
            /* frame begin */
         i3_format.olddat[data_index++] = 0x7e;
            /* protocol version number */
         i3_format.olddat[data_index++] = 0x00;
         i3_format.olddat[data_index++] = 0x10;
		    /* packet num */
         i3_format.olddat[data_index++] = 0x00;
         i3_format.olddat[data_index++] = 0x00;
		    /* translate total frame num to i3 new socket protocol*/
         i3_format.olddat[data_index++] = total_packet_num&0xff;
         i3_format.olddat[data_index++] = (total_packet_num>>8)&0xff;
		    /* manufactor reserved */
         memset(&i3_format.olddat[data_index], 0x0, 10);	
         data_index += 10;
		    /* command code */
         i3_format.olddat[data_index++] = 0x20;
         i3_format.olddat[data_index++] = 0x11;
		    /* status code */
	     i3_format.olddat[data_index++] = 0x00;

		    /* translate maf_id to i3 new socket protocol*/
		 memcpy(&i3_format.olddat[data_index], &boardcfg.maf_id[0], 3);
         data_index += 3;
		    /* translate box_id to i3 new socket protocol*/
         memcpy(&i3_format.olddat[data_index], &boardcfg.box_id[0], 30);		
         data_index += 30;
		    /* sort style */
         i3_format.olddat[data_index++] = 0x01;		 
		    /* translate device name  to i3 new socket protocol*/
	     memcpy(&i3_format.olddat[data_index], &boardcfg.device_name[0], 80);		
         data_index += 80;	
		    /* translate unit board (cf8051 board's EID info) to i3 new socket protocol  910 = 1024 - 114*/
         memcpy(&i3_format.olddat[data_index], &PortsInfo.dyb_portinfo[INFO_START_INDEX], 910); 		
         data_index += 910;
		 nbytes = 1024;

		 crc16_val = resource_crc16_calc(&i3_format.olddat[1], data_index - 1);
         i3_format.olddat[data_index++] = crc16_val & 0xff;		 
         i3_format.olddat[data_index++] = (crc16_val >> 8) & 0xff;		 
         i3_format.olddat[data_index++] = 0x7e;;		 

         ssize = old_version_data_2_new_i3_data_format(&i3_format.olddat[1], (data_index - 2), &i3_format.datbuf[1]);
         i3_format.datbuf[0]         = 0x7e;
		 i3_format.datbuf[1 + ssize] = 0x7e;

        memset(rxbuf, 0xff, sizeof(rxbuf));
         //send data 
        printf("now send packet%d, total %d ... datalen = %d\n", packet_num, total_packet_num, nbytes);

        if(send(sockfd, &i3_format.datbuf[0], ssize+2, 0) == -1){
           /* SOCKET_ERROR */
			printf("%d cannot send data to webmaster, because socket is closed ...\n", __LINE__);
			break;
		}

        if(recv(sockfd, rxbuf, 24, 0) == -1){
			printf("cannot recv ACK singal form webmaster ...\n");	
	        /* SOCKET_ERROR */
			break;
		}else{
			printf("recv ACK singal form webmaster ... success !\n");		
			if(rxbuf[20] == 0x00){
			   packet_num += 1;
			}else{
			   
			}
		} 

      }else{
         data_index = 0;
            /* frame begin */
         i3_format.olddat[data_index++] = 0x7e;
	        /* protocol version number */
         i3_format.olddat[data_index++] = 0x00;
         i3_format.olddat[data_index++] = 0x10;
		    /* packet num */
         i3_format.olddat[data_index++] = packet_num & 0xff;
         i3_format.olddat[data_index++] = (packet_num >> 8) & 0xfff;
		    /* translate total frame num to i3 new socket protocol*/
         i3_format.olddat[data_index++] = total_packet_num&0xff;
         i3_format.olddat[data_index++] = (total_packet_num>>8)&0xff;
		    /* manufactor reserved */
         memset(&i3_format.olddat[data_index], 0x0, 10);	
         data_index += 10;
		    /* command code */
         i3_format.olddat[data_index++] = 0x20;
	     i3_format.olddat[data_index++] = 0x11;
		    /* status code */
	     i3_format.olddat[data_index++] = 0x00;
			
		    /* translate unit board (cf8051 board's EID info) to i3 new socket protocol  */
         if(packet_num == total_packet_num - 1){
			nbytes = total_frame_bytes - packet_num*1024;
            memcpy(&i3_format.olddat[data_index], &PortsInfo.dyb_portinfo[packet_num*1024], nbytes); 		
            data_index += nbytes;   

         }else{
            memcpy(&i3_format.olddat[data_index], &PortsInfo.dyb_portinfo[packet_num*1024], 1024); 		
            data_index += 1024;
			nbytes = 1024;
         }

	    crc16_val = resource_crc16_calc(&i3_format.olddat[1], data_index - 1);
        i3_format.olddat[data_index++] = crc16_val & 0xff;		 
        i3_format.olddat[data_index++] = (crc16_val >> 8) & 0xff;		 
        i3_format.olddat[data_index++] = 0x7e;;		 

        ssize = old_version_data_2_new_i3_data_format(&i3_format.olddat[1], (data_index - 2), &i3_format.datbuf[1]);
        i3_format.datbuf[0]         = 0x7e;
		i3_format.datbuf[1 + ssize] = 0x7e;

		memset(rxbuf, 0xff, sizeof(rxbuf));
        //send data
        printf("now send packet%d, total %d ... datalen = %d\n", packet_num, total_packet_num, nbytes);

        if(send(sockfd, &i3_format.datbuf[0], ssize+2, 0) == -1){
            /* SOCKET_ERROR */         	      	
			printf("%d cannot send data to webmaster, because socket is closed ...\n", __LINE__);
			break;		
		}
        if(recv(sockfd, rxbuf, 24, 0) == -1){
			/*SOCKET_ERROR*/
			printf("cannot recv ACK singal form webmaster ...\n");		
		}else{
			printf("recv ACK singal form webmaster ... success !\n");		
			if(rxbuf[20] == 0x00){
			   packet_num += 1;
			}else{
			   
			}
		}

     }

   }//end of for loop

   printf("Collect .........................[Done]\n");

   pthread_exit(0);	

}

static uint8_t arrayncmp(uint8_t *array, uint16_t array_len, uint8_t cmp_val)
{
	uint16_t ofs = 0;
	uint8_t  ret = 1;//default ret 1, it means array's data both all as same as the compare value

	for(ofs = 0; ofs < array_len; ofs++){
		if(array[ofs] != cmp_val){
			break;
		}
	}

	if(ofs != array_len)
		ret = 0;

	return ret;
}

/****************************************************************************
   Fuction     :
   Description : 最后一步骤，组织数据并发送给网管
   Call        : 
   Called by   : 
   Input       : 
              
   Output      : 
   Return      : 
                 
   Others      : 
 *****************************************************************************/
static int zkb_upload_portinfo_to_webmaster(int fd)
{
	int rc = 0;
	uint16_t ofs = 0, loop = 0;
	uint8_t frame_idex = 0, tray_idex = 0, port_idex = 0;

	printf("collect resource done ...\n");

#if 1
	rc = pthread_create(&i3_format.i3_thid, NULL, pthread_collect_data_upload_to_webmaster, (void *)&fd);
    if(rc == -1){
		printf("cannot create pthread_collect_data_upload_to_webmaster , error val = %d \n", rc);
	}else{
		printf("create pthread_collect_data_upload_to_webmaster , success! \n", rc);
	}
#else 

	debug_net_printf(&PortsInfo.dyb_portinfo[0], 34*1024, 60001);	

#endif

	return rc;
}

/****************************************************************************
   Description : 资源采集主体函数
   Input       : 
   Output      : 
   Return      : 
                 
   Others      : 
 *****************************************************************************/
int module_mainBoard_resource_handle(unsigned char *src,int client_fd)
{
	volatile uint32_t totaltimeuse = 0x0, timeDeadline = 0x0;
	volatile uint32_t c_frame = 0x0, retval = 0x0;

   printf("Collect .........................[start]\n");

	zkb_txBuf_data_tianchong(src);

	timeDeadline = calc_collect_time_deadline(MAXFRAME);

	printf("it will take about %d seconds \n", (int)timeDeadline/1000000);

	gettimeofday(&PortsInfo.timeBegin, NULL);
	while(totaltimeuse < timeDeadline)
	{
		c_frame = zkb_txBuf_data_part();
		if(0x0 != c_frame){
			zkb_rxBuf_data_part(c_frame);
			retval = zkb_check_collectTask_Done();
			if(retval){
				zkb_upload_portinfo_to_webmaster(client_fd);
			}
		}	
		
		gettimeofday(&PortsInfo.timeOut,NULL);
		totaltimeuse = (1000000*(PortsInfo.timeOut.tv_sec - PortsInfo.timeBegin.tv_sec) + PortsInfo.timeOut.tv_usec - PortsInfo.timeBegin.tv_usec);	
//		printf("totaltimeuse = %d .... \n", totaltimeuse);
	}


	printf("collect timeout ...[done]");	
	/*it's so sorry enter here*/	
	zkb_upload_portinfo_to_webmaster(client_fd);

	return 0;
}


/****************************************************************************
   Fuction     :
   Description : 二次采集最后的覆盖模块
   Call        : 
   Called by   : 
   Input       : main_cmd = 0x0d sub_cmd = 0x0c
              
   Output      : 
   Return      : 
                 
   Others      : 
 *****************************************************************************/
int second_resource_collect_recover_task(unsigned char *src,int sockfd)
{
	int  nbyte = 0;
	unsigned short current_array_index[16], current_index = 0;
	unsigned short datlen = 0;
	unsigned char  outloops, inloops;
	unsigned char  frame_index, tray_index, port_index;

	memset(&current_array_index[0], 0x0, 16);
	memset((void*)&uart0, 0x0, sizeof(OS_UART));

	/* calc data len  */
	datlen = src[1]<<8|src[2];

	/* prepare data */
	for(outloops = 21; outloops < datlen; outloops += 3){
		frame_index = src[outloops+0];	
		tray_index  = src[outloops+1];
		port_index  = src[outloops+2];	
		if(uart0.txBuf[frame_index-1][0] == 0x7e){
			current_index = current_array_index[frame_index-1];		
			uart0.txBuf[frame_index-1][current_index++] = frame_index;
			uart0.txBuf[frame_index-1][current_index++] = tray_index;
			uart0.txBuf[frame_index-1][current_index++] = port_index;	
			current_array_index[frame_index-1] = current_index; 		
		}else{
			uart0.txBuf[frame_index-1][0] = 0x7e;
			uart0.txBuf[frame_index-1][1] = 0x00;
			uart0.txBuf[frame_index-1][2] = 0x00;
			uart0.txBuf[frame_index-1][3] = 0x0d;
			uart0.txBuf[frame_index-1][4] = 0x0c;
			uart0.txBuf[frame_index-1][5] = frame_index;
			uart0.txBuf[frame_index-1][6] = tray_index;
			uart0.txBuf[frame_index-1][7] = port_index;
			current_array_index[frame_index-1] = 8; 		
		}		
	}

	/* calc prepare data's crc value */
	for(outloops = 0; outloops < 16; outloops ++){
		current_index = current_array_index[frame_index-1];		
		uart0.txBuf[frame_index-1][1] = (current_index>>8)&0xff;
		uart0.txBuf[frame_index-1][2] = current_index & 0xff;
		uart0.txBuf[frame_index-1][current_index] = calccrc8(&uart0.txBuf[frame_index-1][0], current_index);
		uart0.txBuf[frame_index-1][current_index+1] = 0x5a;
	}

	/* transmit resource polling's recover action to sim3u146 boards */
	for(outloops = 0; outloops < 3; outloops++){
		for(inloops = 0; inloops < 16; inloops++){
			if(uart0.txBuf[inloops][3] == 0x0d){
				write(uart0fd, &uart0.txBuf[inloops][0], uart0.txBuf[inloops][1]<<8|uart0.txBuf[inloops][2]+2);
				usleep(250000);
				memset(&uart0.rxBuf[0], 0x0, 100);
				nbyte = read(uart0fd, &uart0.rxBuf[0], 100);
				if(nbyte > 6){
					if(uart0.rxBuf[3] == 0xa6){
						memset(&uart0.txBuf[inloops][0], 0x0, 600);
					}
				}
			}else{
				continue;
			}
		}
	}

	return 0;
}


/*
 *@brief    : get EID info from ODF frame
 *@param[0] : webmaster's request data. 
 *@param[1] : current socket connect file descriptor value
 *
 *
 * */
int cmd_0x1120_get_resource(unsigned char *dat, int sockfd)
{
	uint8_t cmd_get_resource[20]  = {0x7e, 0x00, 0x05, 0x20, 0x00, 0x00, 0x5a};
	uint8_t cmd_poll_resource[20] = {0x7e, 0x00, 0x05, 0x24, 0x00, 0x00, 0x5a};

	printf("%s , %d", __func__, __LINE__);	
	if(dat[17] == 0x20 && dat[18] == 0x11){
		printf("%s , %d", __func__, __LINE__);	
		module_mainBoard_abortinfo_handle(NULL, 1);
		module_mainBoard_resource_handle(cmd_get_resource, sockfd);
	}else if(dat[17] == 0x23 && dat[18] == 0x11){
		second_resource_collect_recover_task(cmd_poll_resource, sockfd);
	}else if(dat[17] == 0x24 && dat[18] == 0x11){
		second_resource_collect_recover_task(cmd_poll_resource, sockfd);
	}
}

