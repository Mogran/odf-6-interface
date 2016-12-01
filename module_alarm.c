#include "module_alarm.h"

#define MAX_FRAMES   16

alarm_t 			i3_alarm;
circle_buff_t		ancestor_of_alarm_data; 

unsigned char frame_tray_ports_stats[MAX_FRAMES*100]; 
unsigned int  alarminfo_entires = 0;

static uint16_t old_version_alarm_data_2_new_i3_data_format(
		uint8_t *olddat_buf, 
		uint16_t olddat_len, 
		uint8_t *newdat_buf)
{
	uint16_t loops = 0, new_dat_len = 1;

	newdat_buf[0] = 0x7e;

	for(loops = 1, new_dat_len = 1; loops < olddat_len; loops++){
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

	newdat_buf[new_dat_len++] = 0x7e;

	return new_dat_len;
}

void *pthread_alarm(void *arg)
{
	int  remote_fd = 0,nBytes = 0;
	int  len = 0, loop_i = 0;
	int  flags = 0,status = 0,error = 0;
	unsigned int port_num,server_ip = 0;
	struct sockaddr_in remote_addr;
	struct timeval timeout;
	fd_set rset,wset;
	int rc = 0;
	unsigned char rx_data[30];
	alarm_t alarmdat = i3_alarm;

	pthread_detach(pthread_self());

	port_num = boardcfg.server_portnum[0]| (boardcfg.server_portnum[1] << 8);
	server_ip = boardcfg.server_ip[3] << 24| boardcfg.server_ip[2]<<16| boardcfg.server_ip[1] << 8| boardcfg.server_ip[0];

	for(loop_i = 0; loop_i < 3; loop_i++){
		
		remote_fd = -1;
		if((remote_fd = socket(AF_INET,SOCK_STREAM,0)) == -1){  
			continue;
		}		 
		remote_addr.sin_family = AF_INET;  
    	remote_addr.sin_port = htons(port_num);  
   		remote_addr.sin_addr.s_addr = server_ip;  

	   	bzero(&(remote_addr.sin_zero),8); 	
		/* 将socket句柄重新设置成非阻塞，用于connect连接等待，1秒内没连上，自动断开*/
		if(connect(remote_fd,(struct sockaddr *)&remote_addr,sizeof(struct sockaddr_in)) == -1){
			sleep(300);
			close(remote_fd);
			continue;
		}

//		debug_net_printf(&alarmdat.i3_dat[0], alarmdat.i3_len, 60001);
		nBytes = send(remote_fd, &alarmdat.i3_dat[0], alarmdat.i3_len, 0);
		if(nBytes == -1){
			sleep(300);
			close(remote_fd);
			continue;
		}
	
		memset(rx_data, 0, sizeof(rx_data));
		nBytes = recv(remote_fd,rx_data, 24, 0);
		if(nBytes == -1){
			printf("Can't recv webmaster reply data...\n");
			sleep(300);
			close(remote_fd);
			continue;
		}else if(rx_data[20] == 0x00){
			printf("recv webmaster reply data...\n");
			close(remote_fd);
			break;
		}
	}

	printf("alarm pthread exit.....now \n");

	pthread_exit(0);
}



/*
 *@brief: For use to handle S3C44B0X board's alarminfo.
 *@ret	: none
 *
 *7e 00 00 09 k f p stat k f p stat ... crc 5a.
 *		      4 ...
 * FrameIdx = src[4*n] - 1;
 * TrayIdx  = src[4*n + 1] - 1;
 * PortIdx  = src[4*n + 2] - 1;
 * others :
 *         报警灯跟峰鸣器的操作是： 
 *         写0：红灯亮，峰鸣器响  
 *         写1：红灯灭，峰鸣器不响
 *
 *         托盘上端口的非法插入命令字：0x1 
 *         托盘上端口的非法插入恢复命令字：0x0f
 *
 *         托盘上端口的非法拔出命令字：0x2 
 *         托盘上端口的非法拔出恢复命令字：0x0e
 *
 *         托盘失联命令：0x03 
 *         托盘失联恢复：0x0d
 *
 *         托盘非法插入命令字：0x04 
 *         托盘非法插入恢复命令字：0x0C
 */

void reset_odf_alarm_entires(void)
{
	alarminfo_entires = 0;

	memset(frame_tray_ports_stats, 0, sizeof(frame_tray_ports_stats));
}

uint8_t test_odf_alarm_entires(void)
{
	uint8_t loop_i = 0, loop_j = 0;

	for(loop_i = 0; loop_i < 8; loop_i++){
		for(loop_j = 0; loop_j < 110; loop_j++){
			if(frame_tray_ports_stats[loop_i*110+loop_j] != 0x0){
				return 1;
			}
		}	
	}

	return 0;
}

static uint8_t alarminfo_filter(uint8_t *jkb_upload_info)
{
	uint16_t storage_idex = 0x0;
	uint8_t  storage_vals = 0x0;
	uint8_t  frame_num = jkb_upload_info[0] - 1;
	uint8_t  trays_num = jkb_upload_info[1] - 1;
	uint8_t  ports_num = jkb_upload_info[2] - 1;
	uint8_t  stats_val = jkb_upload_info[3];
	uint8_t  ret = 0;

	switch(stats_val){
	case 0x01:
	case 0x0f:
	case 0x02:
	case 0x0e:
		storage_idex = frame_num*100 + 10 + trays_num*12 + ports_num;
		storage_vals = frame_tray_ports_stats[storage_idex];
	break;	
	case 0x03:
	case 0x0d:
	case 0x04:
	case 0x0C:
		storage_idex = frame_num*100+trays_num;
		storage_vals = frame_tray_ports_stats[storage_idex];
	break;
	default:
		ret = 1;
		return ret;
	}

	switch(stats_val){
	case 0x01:
	case 0x03:
		if(storage_vals & (1 << 0))
			ret = 1;
	break;
	case 0x0d:
	case 0x0f:
		if(!(storage_vals & (1 << 0)))
			ret = 1;
	break;	
	case 0x02:
	case 0x04:
		if(storage_vals & (1 << 1))
			ret = 1;
	break;	
	case 0x0C:
	case 0x0e:
		if(!(storage_vals & (1 << 1)))
			ret = 1;
	break;	
	default:
		ret = 1;
	break;
	}

	return ret;
}

#if 0
int module_mainBoard_abortinfo_handle(unsigned char *src, unsigned char closedalarm)
{
	static int isLighted = 0;
	int status = 0,i = 0, c_idex = 0, rc = 0;
	unsigned int  PortIdx = 0, ofs = 0;
	unsigned short alarmEntires = 0,datalen = 0, crc16_val;
	unsigned char FrameIdx = 0,TrayIdx = 0,PortStat = 0;

	pthread_t pthid;

	if(closedalarm == 1 && src == NULL){
		reset_odf_alarm_entires();
		ioctl(alarmfd, 1, 0);
		isLighted = 0;
		return 0;
	}

	/* 根据接口板发来的告警数据，解析出一共有几条告警信息*/
	alarmEntires  = (*(src + 1) << 8) | *(src + 2);
	datalen = alarmEntires + 2;
	alarmEntires -= 4; 
	alarmEntires  = alarmEntires/4;

	ofs = 0;
	i3_alarm.dat[ofs++] = 0x7e;	
	i3_alarm.dat[ofs++] = 0x00;	
	i3_alarm.dat[ofs++] = 0x10;	
	memset(&i3_alarm.dat[ofs], 0x0, 14);
	ofs += 14;
	i3_alarm.dat[ofs++] = 0x09;
	i3_alarm.dat[ofs++] = 0x11;
	i3_alarm.dat[ofs++] = 0xff;
	i3_alarm.dat[ofs++] = alarmEntires;
	
	for(i = 0; i < alarmEntires; i++){

		if(alarminfo_filter(&src[(i+1)*4])){
			/* discard this alarm info*/
			i3_alarm.dat[20] -= 1;
			printf("filter one alarm entires ...\n");
		}else{
			memcpy(&i3_alarm.dat[ofs], &src[(i+1)*4], 4);
			ofs += 4;
			if(i3_alarm.dat[ofs-1] == 0x1 
				|| i3_alarm.dat[ofs -1] == 0x02 
				|| i3_alarm.dat[ofs-1] == 0x03 
				|| i3_alarm.dat[ofs-1] == 0x04
			)
			{
				i3_alarm.dat[ofs++] = 0x01;
			}else{
				i3_alarm.dat[ofs++] = 0x02;
			}

			if(i3_alarm.dat[ofs-2] == 0x0f){
				i3_alarm.dat[ofs-2] = 0x01;
			}else if(i3_alarm.dat[ofs-2] == 0x0e){
				i3_alarm.dat[ofs-2] = 0x02;
			}else if(i3_alarm.dat[ofs-2] == 0x0d){
				i3_alarm.dat[ofs-2] = 0x03;
			}else if(i3_alarm.dat[ofs-2] == 0x0c){
				i3_alarm.dat[ofs-2] = 0x04;
			}
		}
	}
	
	if(i3_alarm.dat[20] == 0){
		/* if without vaild alarm info, return here*/
		return 0;
	}

	crc16_val = crc16_calc(&i3_alarm.dat[1], ofs - 1);	
	i3_alarm.dat[ofs++] = crc16_val&0xff;
	i3_alarm.dat[ofs++] = (crc16_val >> 8 )&0xff;
	i3_alarm.dat[ofs++] = 0x7e;

//	debug_net_printf(&i3_alarm.dat[0], ofs, 60001);

	i3_alarm.i3_len = old_version_alarm_data_2_new_i3_data_format(&i3_alarm.dat[0], ofs - 1, &i3_alarm.i3_dat[0]);

//	debug_net_printf(&i3_alarm.i3_dat[0], i3_alarm.i3_len, 60001);

	rc = pthread_create(&pthid, NULL, pthread_alarm, (void *)&i3_alarm);
	if(rc == -1){
		printf("alarm pthread routine failed \n");
	}

	alarmEntires  = (*(src + 1) << 8) | *(src + 2);
	datalen = alarmEntires + 2;
	alarmEntires -= 4; 
	alarmEntires  = alarmEntires/4;
	for(i = 0;i < alarmEntires;i++){
		FrameIdx = src[4*i + 4] - 1; 	
		TrayIdx  = src[4*i + 5] - 1;
		PortStat = src[4*i + 7]; 

		switch(PortStat){
		case 0x01:
		case 0x0f:
		case 0x02:
		case 0x0e:
			printf("Port alarm....\n");
			/* 此处是表征"单元板上端口"出现的异常或恢复,+10是因为端口下标都是从每100个字节的第10个字节偏移处开始记录的（留意了！）  */
			PortIdx = FrameIdx*100 + 10 + TrayIdx*12 + src[4*i + 6] - 1;
			break;	
		case 0x03:
		case 0x0d:
		case 0x04:
		case 0x0C:
			printf("Tray alarm info...\n");
			/* 这里表征"单元板"上出现了异常，失联或者失联恢复，非法插入框或者非法插入框恢复*/
			PortIdx = FrameIdx*100+TrayIdx;
			break;	
		}

		if(PortStat == 0x03 || PortStat == 0x01){
			frame_tray_ports_stats[PortIdx] |= (1 << 0);
		}else if(PortStat == 0x04 || PortStat == 0x02){
			frame_tray_ports_stats[PortIdx] |= (1 << 1);
		}else if(PortStat == 0x0d || PortStat == 0x0f){
			frame_tray_ports_stats[PortIdx] &= ~(1 << 0);
		}else if(PortStat == 0x0c || PortStat == 0x0e){
			frame_tray_ports_stats[PortIdx] &= ~(1 << 1);
		}else{
			frame_tray_ports_stats[PortIdx] = PortStat;
		}

		if(PortStat == 0x03 || PortStat == 0x04){
			/* if tray got lost, we should clear all tray's port stats !!! */
			PortIdx = FrameIdx*100 + 10 + TrayIdx*12;
			memset(&frame_tray_ports_stats[PortIdx], 0x0, 12);
		}
	}

	if(test_odf_alarm_entires()){
		if(isLighted == 0){
			printf("alert : alarm !!!, open alarm led! \n");
			ioctl(alarmfd,0, 0);
			isLighted++;
		}
	}else{
		if(isLighted){
			printf("alert : alarm disappear !!! close alarm led !\n");
			ioctl(alarmfd, 1, 0);
			isLighted = 0;		
		}	
	}

	return 0;
}

#else

int module_mainBoard_abortinfo_handle(unsigned char *src, unsigned char closedalarm)
{
	static int isLighted = 0;
	int status = 0,i = 0, c_idex = 0, rc = 0;
	unsigned int  PortIdx = 0, ofs = 0;
	unsigned short alarmEntires = 0,datalen = 0, crc16_val;
	unsigned char FrameIdx = 0,TrayIdx = 0,PortStat = 0;

	pthread_t pthid;

	if(closedalarm == 1 && src == NULL){
		reset_odf_alarm_entires();
		ioctl(alarmfd, 1, 0);
		isLighted = 0;
		return 0;
	}

}

#endif










void initialization_circle(void)
{
	ancestor_of_alarm_data.head = 0x0;
	ancestor_of_alarm_data.tail = 0x0;
	ancestor_of_alarm_data.priv = 0x0;
}


/*
 * @ descriptions: 增加一段告警，有可能是1条，有可能是多条
 *
 * @ ret : 
 *  	0 : add success
 *  	1 : add failed
 * */
int add_one_node(unsigned char *dst, const unsigned char *src, 
		unsigned short len)
{
	unsigned short c_head = ancestor_of_alarm_data.head;
	unsigned short c_tail = ancestor_of_alarm_data.tail;

	if(c_head + len > CIRCLE_SIZE){
		memcpy(&ancestor_of_alarm_data.data[c_head], 
				src, 
				len - (CIRCLE_SIZE - c_head));
		memcpy(&ancestor_of_alarm_data.data[0], 
				&src[len + c_head - CIRCLE_SIZE],
				c_head + len - CIRCLE_SIZE);
		c_head = c_head + len - CIRCLE_SIZE;
	}else{
		memcpy(&ancestor_of_alarm_data.data[c_head], src, len);
		c_head += len;
	}

	ancestor_of_alarm_data.head = c_head;

	return len;
}

/*
 * @ descriptions:
 * 下载一段告警，可能是1条，可能是多条，但最多只提供下载1024个字节
 * 也就是最多只支持256条告警下载	
 * @ ret : 
 *  	0 : add success
 *  	1 : add failed*
 * */
int download_one_node(unsigned char *dst)
{
	unsigned short c_head = ancestor_of_alarm_data.head;
	unsigned short c_tail = ancestor_of_alarm_data.tail;
	int ret = 0;

	if(c_head == c_tail){
		ret = 0;		
	}else if(c_head > c_tail){
		/* 一次最多只能传256条告警，即1024个字节 */
		if((c_head - c_tail) >= 1024){
			memcpy(dst, &ancestor_of_alarm_data.data[c_tail], 1024);
			ancestor_of_alarm_data.priv = c_tail + 1024;
			ret = 1024;
		}else{
			memcpy(dst, &ancestor_of_alarm_data.data[c_tail], c_head - c_tail);
			ancestor_of_alarm_data.priv = c_tail + c_head - c_tail;
			ret = c_head - c_tail;
		}
	}else if(c_head < c_tail){
		if((c_head + CIRCLE_SIZE - c_tail) >=1024){
			if(CIRCLE_SIZE - c_tail >= 1024){
				memcpy(dst, &ancestor_of_alarm_data.data[c_tail], 1024);
				ancestor_of_alarm_data.priv = c_tail + 1024;
				ret = 1024;
			}else{
				memcpy(dst, &ancestor_of_alarm_data.data[c_tail], CIRCLE_SIZE - c_tail);
				memcpy(dst, &ancestor_of_alarm_data.data[c_tail], 1024 - (CIRCLE_SIZE - c_tail));
				ancestor_of_alarm_data.priv = c_head - (1024 - (CIRCLE_SIZE - c_tail));
				ret = 1024 - (CIRCLE_SIZE - c_tail);
			}			
		}else{
				memcpy(dst, &ancestor_of_alarm_data.data[c_tail], CIRCLE_SIZE - c_tail);
				memcpy(dst, &ancestor_of_alarm_data.data[c_tail], c_head - (CIRCLE_SIZE - c_tail));
				ancestor_of_alarm_data.priv = c_head;			
				ret = c_head - (CIRCLE_SIZE - c_tail);
		}		
	}

}

/*
 * @ descriptions:
 * 删除一段已经上传的告警，该告警可能是一条，可能是多条
 *
 * */
int del_one_node(void)
{
	ancestor_of_alarm_data.tail = ancestor_of_alarm_data.priv;

	return 0;
}

