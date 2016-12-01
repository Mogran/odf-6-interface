#ifndef MODULE_ALARM_H
#define MODULE_ALARM_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
#include <linux/rtc.h>

/* for net */
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>    
#include <net/if.h>

#include "os_struct.h"
#include "board-alarm.h"


/* 告警环形单元的宏设计 */
#define ALARM_MAX_BYTES		(1024)
#define CIRCLE_SIZE 		(24*1024)
#define DATA_SZ			(8192)
/* 
 * g:global uc:unsigned char v:variable
 * 这个数组记录整个odf机架上框、盘、盘上端口所有的告警状态,每个框有100个字节空间，
 * 每100个字节的数据格式如下：
 * ------------------------------------------------------------------------------------------------------------
 * | 框是否失联（1个字节）| 6个盘是否在线（6个字节）| 以后扩展盘用（3个字节） |72个端口状态值| 以后扩展占位符（18个字节）
 * -----------------------------------------------------------------------------------------------------------
 */
typedef struct {
	unsigned short datlen;
	unsigned char  dat[DATA_SZ];

	unsigned short i3_len;
	unsigned char  i3_dat[DATA_SZ];
}alarm_t;

typedef struct {
	unsigned char data[CIRCLE_SIZE];
	unsigned short head;
	unsigned short tail;
	unsigned short priv;
}circle_buff_t;

extern int alarmfd; 
extern pthread_mutex_t alarm_mutex;
extern board_info_t boardcfg;

#endif



