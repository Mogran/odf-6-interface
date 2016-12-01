#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>    
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#include "os_struct.h"

extern int uart0fd,alarmfd,uart1fd; 
extern board_info_t boardcfg;

#define MAX_JIAJIAN_ORDER_ENTIRES	5
#define I3_PROTOCOL_2_OLD_PROTOCOL  1
#define OLD_PROTOCOL_2_I3_PROTOCOL  2

#define DATA_SOURCE_UART			1
#define DATA_SOURCE_MOBILE			2
#define DATA_SOURCE_NET				3
#define DATA_SOURCE_JIAJIAN			4

#define THIS_ODF_IS_HOST			2
#define REMOTE_ODF_IS_HOST			3

#define DYB_PORT_IS_EMPTY			4	
#define DYB_PORT_IS_USED			5

typedef struct {
	unsigned long connect_sockfd_ip[5];

	int order_host;
	int connect_sockfd[5];
	int server_sockfd;

	uint8_t order_type;
	uint8_t batch_entires;
	uint8_t upload_entires;
	uint8_t order_result[300];

	uint8_t local_port_info[MAX_JIAJIAN_ORDER_ENTIRES][10];
	uint8_t local_port_eids[MAX_JIAJIAN_ORDER_ENTIRES][40];
	uint8_t local_port_stat[MAX_JIAJIAN_ORDER_ENTIRES*2];
	uint8_t local_port_ipaddr[MAX_JIAJIAN_ORDER_ENTIRES][4];
	uint8_t local_port_uuid[30];

	uint8_t jiajian_port_uuid[MAX_JIAJIAN_ORDER_ENTIRES][30];
	uint8_t jiajian_port_ipaddr[MAX_JIAJIAN_ORDER_ENTIRES][4];
	uint8_t jiajian_port_info[MAX_JIAJIAN_ORDER_ENTIRES][10];
	uint8_t jiajian_port_eids[MAX_JIAJIAN_ORDER_ENTIRES][40];
	uint8_t jiajian_port_stat[MAX_JIAJIAN_ORDER_ENTIRES*2];

	uint8_t  jiajian_i3_temp_data[MAX_JIAJIAN_ORDER_ENTIRES][300];
	uint16_t jiajian_i3_data_length[MAX_JIAJIAN_ORDER_ENTIRES];
	uint8_t  jiajian_lo_temp_data[MAX_JIAJIAN_ORDER_ENTIRES][300];
	uint16_t jiajian_lo_data_length[MAX_JIAJIAN_ORDER_ENTIRES];

	uint8_t  local_i3_temp_data[MAX_JIAJIAN_ORDER_ENTIRES][300];
	uint8_t  local_lo_temp_data[MAX_JIAJIAN_ORDER_ENTIRES][300];
	uint16_t local_data_length[MAX_JIAJIAN_ORDER_ENTIRES];

	struct sockaddr_in remote_addr[5];

	pthread_t thid[10];

}Jiajian_t;



extern void Jiajian_order_reply_routine(uint8_t sub_cmd_code, uint16_t error_code);


