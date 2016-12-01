#include "Jiajian_Order.h"

Jiajian_t Jiajian_Order;


extern pthread_mutex_t mobile_mutex; 

static uint16_t transform_jiajian_i3_protocol_data_to_local_old_protocol_data(
		uint8_t *i3_data_buf, uint8_t *old_data_buf, uint16_t transform_len)
{
	uint16_t i3_data_index = 0, old_data_index = 0;

	old_data_buf[old_data_index++] = 0x7e;

	for(i3_data_index = 1; i3_data_index < transform_len; i3_data_index++){
		if(i3_data_buf[i3_data_index] == 0x7d){
			if(i3_data_buf[i3_data_index + 1] == 0x5e){
				old_data_buf[old_data_index++] = 0x7e;
				i3_data_index++;
			}else if(i3_data_buf[i3_data_index + 1] == 0x5d){
				old_data_buf[old_data_index++] = 0x7d;	
				i3_data_index++;
			}
		}else{
			old_data_buf[old_data_index++] = i3_data_buf[i3_data_index];	
		}
	}

	old_data_buf[old_data_index++] = 0x7e;

	return old_data_index;
}

static uint16_t transform_local_old_protocol_data_to_jiajian_i3_protocol_data(
		uint8_t *old_data_buf, uint8_t *i3_data_buf, uint16_t transform_len)
{
	uint16_t old_data_index = 0, i3_data_index = 0;

	i3_data_buf[i3_data_index++] = 0x7e;

	for(old_data_index = 1; old_data_index < transform_len; old_data_index++){
		if(old_data_buf[old_data_index] == 0x7e){
			i3_data_buf[i3_data_index++] = 0x7d;
			i3_data_buf[i3_data_index++] = 0x5e;
		}else if(old_data_buf[old_data_index] == 0x7d){
			i3_data_buf[i3_data_index++] = 0x7d;
			i3_data_buf[i3_data_index++] = 0x5d;
		}else{
			i3_data_buf[i3_data_index++] = old_data_buf[old_data_index];
		}
	}

	i3_data_buf[i3_data_index++] = 0x7e;

	return i3_data_index;
}

uint16_t data_format_transform_routine(
		uint8_t *i3_protocol_data, uint8_t *old_protocol_data, 
		uint16_t transform_len, uint8_t type)
{
	uint16_t retval = 0;

	if(type == I3_PROTOCOL_2_OLD_PROTOCOL){
		retval = transform_jiajian_i3_protocol_data_to_local_old_protocol_data(
				i3_protocol_data, 
				old_protocol_data, 
				transform_len);
	}else if(type  == OLD_PROTOCOL_2_I3_PROTOCOL){
		retval = transform_local_old_protocol_data_to_jiajian_i3_protocol_data(
				old_protocol_data, 
				i3_protocol_data, 
				transform_len);	
	}

	return retval;
}


void Jiajian_order_reply_routine(uint8_t sub_cmd_code, uint16_t error_code)
{
	uint16_t ofs = 0, crc16_val = 0;
	uint16_t ret = 0, nbytes = 0;

	switch(sub_cmd_code){
	case 0x08:
		ofs = 112;
		Jiajian_Order.order_result[ofs++] = error_code;
		if(error_code == 0x00){
			memcpy(&Jiajian_Order.order_result[ofs], &Jiajian_Order.local_port_eids[0][0], 32);
		}else{
			memset(&Jiajian_Order.order_result[ofs], 0x0, 32);
		}
		ofs += 32;
		crc16_val = crc16_calc(&Jiajian_Order.order_result[1], ofs - 1);				
		Jiajian_Order.order_result[ofs++] = crc16_val & 0xff;
		Jiajian_Order.order_result[ofs++] = (crc16_val >> 8) & 0xff;
		Jiajian_Order.order_result[ofs++] = 0x7e;
		nbytes = data_format_transform_routine(
				&Jiajian_Order.jiajian_i3_temp_data[0][0],
				&Jiajian_Order.order_result[0],
				ofs - 1,
				OLD_PROTOCOL_2_I3_PROTOCOL
				);		
		
		debug_net_printf(&Jiajian_Order.jiajian_i3_temp_data[0][0], nbytes, 60001);

		if(Jiajian_Order.order_type == DATA_SOURCE_NET){
			ret = send(Jiajian_Order.server_sockfd, &Jiajian_Order.jiajian_i3_temp_data[0][0], nbytes, MSG_NOSIGNAL);		
			if(ret < 0){
				if(errno == -EPIPE || errno == -EBADF){
					close(Jiajian_Order.server_sockfd);
				}
			}		

		}else if(Jiajian_Order.order_type == DATA_SOURCE_MOBILE){
			write(uart1fd, &Jiajian_Order.jiajian_i3_temp_data[0][0], nbytes);		
		}

	break;

	case 0x09:
		ofs = 112;
		Jiajian_Order.order_result[ofs++] = error_code;
		crc16_val = crc16_calc(&Jiajian_Order.order_result[1], ofs - 1);				
		Jiajian_Order.order_result[ofs++] = crc16_val & 0xff;
		Jiajian_Order.order_result[ofs++] = (crc16_val >> 8) & 0xff;
		Jiajian_Order.order_result[ofs++] = 0x7e;
		nbytes = data_format_transform_routine(
				&Jiajian_Order.jiajian_i3_temp_data[0][0],
				&Jiajian_Order.order_result[0],
				ofs - 1,
				OLD_PROTOCOL_2_I3_PROTOCOL
				);		
		
		debug_net_printf(&Jiajian_Order.jiajian_i3_temp_data[0][0], nbytes, 60001);

		if(Jiajian_Order.order_type == DATA_SOURCE_NET){
			ret = send(Jiajian_Order.server_sockfd, &Jiajian_Order.jiajian_i3_temp_data[0][0], nbytes, MSG_NOSIGNAL);		
			if(ret < 0){
				if(errno == -EPIPE || errno == -EBADF){
					close(Jiajian_Order.server_sockfd);
				}
			}		

		}else if(Jiajian_Order.order_type == DATA_SOURCE_MOBILE){
			write(uart1fd, &Jiajian_Order.jiajian_i3_temp_data[0][0], nbytes);		
		}
	
	break;

	case 0x10:
		ofs = 112;
		Jiajian_Order.order_result[ofs++] = error_code;
		ofs += 32;
		crc16_val = crc16_calc(&Jiajian_Order.order_result[1], ofs - 1);				
		Jiajian_Order.order_result[ofs++] = crc16_val & 0xff;
		Jiajian_Order.order_result[ofs++] = (crc16_val >> 8) & 0xff;
		Jiajian_Order.order_result[ofs++] = 0x7e;
		nbytes = data_format_transform_routine(
				&Jiajian_Order.jiajian_i3_temp_data[0][0],
				&Jiajian_Order.order_result[0],
				ofs - 1,
				OLD_PROTOCOL_2_I3_PROTOCOL
				);		
		
		debug_net_printf(&Jiajian_Order.jiajian_i3_temp_data[0][0], nbytes, 60001);

		if(Jiajian_Order.order_type == DATA_SOURCE_NET){
			ret = send(Jiajian_Order.server_sockfd, &Jiajian_Order.jiajian_i3_temp_data[0][0], nbytes, MSG_NOSIGNAL);		
			if(ret < 0){
				if(errno == -EPIPE || errno == -EBADF){
					close(Jiajian_Order.server_sockfd);
				}
			}		

		}else if(Jiajian_Order.order_type == DATA_SOURCE_MOBILE){
			write(uart1fd, &Jiajian_Order.jiajian_i3_temp_data[0][0], nbytes);		
		}
	break;
	}
}

void jiajian_new_or_del_2_ports_order_prepare_cmd_data_routine(uint8_t *src)
{
	uint16_t ofs = 0x0, crc16_val = 0;

	if(myStringcmp(&boardcfg.box_id[0], &src[38], 30)){

		/* Jiajian transmit data recombine*/
		memset(&Jiajian_Order.jiajian_lo_temp_data[0][0], 0x0, 150);
		ofs = 0;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x7e;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x00;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x10;
		memset(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], 0x0, 14);
		ofs += 14;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x80;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x11;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0xff;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x18;
		memcpy(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], &src[75], 30+4+3);
		ofs += 37;
		crc16_val = crc16_calc(&Jiajian_Order.jiajian_lo_temp_data[0][1], ofs - 1);
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = crc16_val & 0xff;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = crc16_val >> 8 & 0xff;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x7e;
	
		Jiajian_Order.jiajian_i3_data_length[0] = data_format_transform_routine(
													&Jiajian_Order.jiajian_i3_temp_data[0][0],
													&Jiajian_Order.jiajian_lo_temp_data[0][0],
													ofs - 1,
													OLD_PROTOCOL_2_I3_PROTOCOL
													);

		/* local frame data recombine */
		memset(&Jiajian_Order.local_lo_temp_data[0][0], 0x0, 150);
		ofs = 0;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x7e;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x0d;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x08;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x05;
		memcpy(&Jiajian_Order.local_lo_temp_data[0][ofs], &src[72], 3);
		ofs += 3;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
		ofs += 4;
		ofs += 32;

		Jiajian_Order.local_lo_temp_data[0][2] = ofs;
		Jiajian_Order.local_lo_temp_data[0][ofs] = calccrc8(&Jiajian_Order.local_lo_temp_data[0][0], ofs);
		Jiajian_Order.local_lo_temp_data[0][ofs+1] = 0x5a;
		Jiajian_Order.local_data_length[0] = ofs + 2;


	}else if(myStringcmp(&boardcfg.box_id[0], &src[75], 30)){
		
		/* Jiajian transmit data recombine*/
		memset(&Jiajian_Order.jiajian_lo_temp_data[0][0], 0x0, 150);
		ofs = 0;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x7e;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x00;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x10;
		memset(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], 0x0, 14);
		ofs += 14;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x80;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x11;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0xff;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x18;
		memcpy(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], &src[38], 30+4+3);
		ofs += 37;
		crc16_val = crc16_calc(&Jiajian_Order.jiajian_lo_temp_data[0][1], ofs - 1);
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = crc16_val & 0xff;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = crc16_val >> 8 & 0xff;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x7e;
	
		Jiajian_Order.jiajian_i3_data_length[0] = data_format_transform_routine(
													&Jiajian_Order.jiajian_i3_temp_data[0][0],
													&Jiajian_Order.jiajian_lo_temp_data[0][0],
													ofs - 1,
													OLD_PROTOCOL_2_I3_PROTOCOL
													);
		
		/* local frame transmit data recombine */
		memset(&Jiajian_Order.local_lo_temp_data[0][0], 0x0, 150);
		ofs = 0;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x7e;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x0d;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x08;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x05;
		memcpy(&Jiajian_Order.local_lo_temp_data[0][ofs], &src[109], 3);
		ofs += 3;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
		ofs += 4;
		ofs += 32;

		Jiajian_Order.local_lo_temp_data[0][2] = ofs;
		Jiajian_Order.local_lo_temp_data[0][ofs] = calccrc8(&Jiajian_Order.local_lo_temp_data[0][0], ofs);
		Jiajian_Order.local_lo_temp_data[0][ofs + 1] = 0x5a;
		Jiajian_Order.local_data_length[0] = ofs + 2;
	}

//	debug_net_printf(&Jiajian_Order.jiajian_i3_temp_data[0][0], Jiajian_Order.jiajian_i3_data_length[0], 60001);
//	debug_net_printf(&Jiajian_Order.local_lo_temp_data[0][0], Jiajian_Order.local_data_length[0], 60001);

}

void jiajian_new_or_del_2_ports_order_cancle_routine(uint8_t *src)
{
	uint16_t ofs = 0x0, crc16_val = 0;

	if(myStringcmp(&boardcfg.box_id[0], &src[27], 30)){

		/* Jiajian transmit data recombine*/
		memset(&Jiajian_Order.jiajian_lo_temp_data[0][0], 0x0, 150);
		ofs = 0;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x7e;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x00;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x10;
		memset(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], 0x0, 14);
		ofs += 14;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x80;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x11;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0xff;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x18;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0xa3;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x01;
		memcpy(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], &src[65], 30);
		ofs += 30;
		memcpy(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], &src[95], 4);
		ofs += 4;
		memcpy(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], &src[61], 4);
		ofs += 4;

		crc16_val = crc16_calc(&Jiajian_Order.jiajian_lo_temp_data[0][1], ofs - 1);
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = crc16_val & 0xff;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = crc16_val >> 8 & 0xff;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x7e;
	
		Jiajian_Order.jiajian_i3_data_length[0] = data_format_transform_routine(
													&Jiajian_Order.jiajian_i3_temp_data[0][0],
													&Jiajian_Order.jiajian_lo_temp_data[0][0],
													ofs - 1,
													OLD_PROTOCOL_2_I3_PROTOCOL
													);

		/* local frame data recombine */
		memset(&Jiajian_Order.local_lo_temp_data[0][0], 0x0, 150);
		ofs = 0;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x7e;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x0d;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0xa2;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x01;

		memcpy(&Jiajian_Order.local_lo_temp_data[0][ofs], &src[23], 4);
		ofs += 4;

		Jiajian_Order.local_lo_temp_data[0][2] = ofs;
		Jiajian_Order.local_lo_temp_data[0][ofs] = calccrc8(&Jiajian_Order.local_lo_temp_data[0][0], ofs);
		Jiajian_Order.local_lo_temp_data[0][ofs+1] = 0x5a;
		Jiajian_Order.local_data_length[0] = ofs + 2;


	}else if(myStringcmp(&boardcfg.box_id[0], &src[65], 30)){
		
		/* Jiajian transmit data recombine*/
		memset(&Jiajian_Order.jiajian_lo_temp_data[0][0], 0x0, 150);
		ofs = 0;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x7e;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x00;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x10;
		memset(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], 0x0, 14);
		ofs += 14;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x80;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x11;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0xff;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x18;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0xa3;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x01;
		memcpy(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], &src[27], 30);
		ofs += 30;
		memcpy(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], &src[57], 4);
		ofs += 4;
		memcpy(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], &src[23], 4);
		ofs += 4;

		crc16_val = crc16_calc(&Jiajian_Order.jiajian_lo_temp_data[0][1], ofs - 1);
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = crc16_val & 0xff;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = crc16_val >> 8 & 0xff;
		Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x7e;
	
		Jiajian_Order.jiajian_i3_data_length[0] = data_format_transform_routine(
													&Jiajian_Order.jiajian_i3_temp_data[0][0],
													&Jiajian_Order.jiajian_lo_temp_data[0][0],
													ofs - 1,
													OLD_PROTOCOL_2_I3_PROTOCOL
													);
		
		/* local frame transmit data recombine */
		memset(&Jiajian_Order.local_lo_temp_data[0][0], 0x0, 150);
		ofs = 0;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x7e;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x0d;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0xa2;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x01;
		memcpy(&Jiajian_Order.local_lo_temp_data[0][ofs], &src[61], 4);
		ofs += 4;

		Jiajian_Order.local_lo_temp_data[0][2] = ofs;
		Jiajian_Order.local_lo_temp_data[0][ofs] = calccrc8(&Jiajian_Order.local_lo_temp_data[0][0], ofs);
		Jiajian_Order.local_lo_temp_data[0][ofs + 1] = 0x5a;
		Jiajian_Order.local_data_length[0] = ofs + 2;
	}

//	debug_net_printf(&Jiajian_Order.jiajian_i3_temp_data[0][0], Jiajian_Order.jiajian_i3_data_length[0], 60001);
//	debug_net_printf(&Jiajian_Order.local_lo_temp_data[0][0], Jiajian_Order.local_data_length[0], 60001);

	ioctl(alarmfd, 4, 0);

}


void *pthread_jiajian_connect_routine(void *arg)
{
	int ret = 0, remote_fd = 0;

	pthread_detach(pthread_self());

	remote_fd = socket(AF_INET, SOCK_STREAM, 0);

	if(remote_fd < 0){

		/* printf("cannot create socket ....\n"); */

	}else{

		Jiajian_Order.remote_addr[0].sin_family = AF_INET;
		Jiajian_Order.remote_addr[0].sin_port = htons(60001);
		if(Jiajian_Order.jiajian_lo_temp_data[0][21] != 0xa3){
			Jiajian_Order.remote_addr[0].sin_addr.s_addr =  0x0;
			Jiajian_Order.remote_addr[0].sin_addr.s_addr |=  Jiajian_Order.jiajian_lo_temp_data[0][54] << 24;
			Jiajian_Order.remote_addr[0].sin_addr.s_addr |=  Jiajian_Order.jiajian_lo_temp_data[0][53] << 16;
			Jiajian_Order.remote_addr[0].sin_addr.s_addr |=  Jiajian_Order.jiajian_lo_temp_data[0][52] << 8;
			Jiajian_Order.remote_addr[0].sin_addr.s_addr |=  Jiajian_Order.jiajian_lo_temp_data[0][51];

			 printf("%s :%d remote s_addr = %lu ...\n", __func__, __LINE__, Jiajian_Order.remote_addr[0].sin_addr.s_addr); 

		}else{
			Jiajian_Order.remote_addr[0].sin_addr.s_addr =  0x0;
			Jiajian_Order.remote_addr[0].sin_addr.s_addr |=  Jiajian_Order.jiajian_lo_temp_data[0][55] << 24;
			Jiajian_Order.remote_addr[0].sin_addr.s_addr |=  Jiajian_Order.jiajian_lo_temp_data[0][54] << 16;
			Jiajian_Order.remote_addr[0].sin_addr.s_addr |=  Jiajian_Order.jiajian_lo_temp_data[0][53] << 8;
			Jiajian_Order.remote_addr[0].sin_addr.s_addr |=  Jiajian_Order.jiajian_lo_temp_data[0][52];

			 printf("%s :%d remote s_addr = %lu ...\n", __func__, __LINE__, Jiajian_Order.remote_addr[0].sin_addr.s_addr); 

		}

		ret = connect(remote_fd, (struct sockaddr*)&Jiajian_Order.remote_addr[0], sizeof(struct sockaddr_in));
		if(ret == -1){
			 printf("%s :%d jiajian connect request failed ...\n", __func__, __LINE__); 
		}else{

			 printf("%s :%d jiajian connect request successed ... ip = %d.%d.%d.%d ....\n", __func__, __LINE__, 
					 Jiajian_Order.jiajian_lo_temp_data[0][51], Jiajian_Order.jiajian_lo_temp_data[0][52], 
					 Jiajian_Order.jiajian_lo_temp_data[0][53], Jiajian_Order.jiajian_lo_temp_data[0][54]); 
			
			ret = send(remote_fd, &Jiajian_Order.jiajian_i3_temp_data[0][0], 
					Jiajian_Order.jiajian_i3_data_length[0], MSG_NOSIGNAL);

			if(ret < 0){
				if(errno == -EPIPE || errno == -EBADF){
					close(Jiajian_Order.connect_sockfd[0]);
				}
			}else{

				Jiajian_Order.connect_sockfd[0] = remote_fd;
				printf("%s :%d Jiajian_Order.connect_sockfd[0] = %d \n", __func__, __LINE__, Jiajian_Order.connect_sockfd[0]);
			}

		}

	}

	printf("%s :%d pthread_jiajian_connect_routine exit ..... [done] \n", __func__, __LINE__);
	pthread_exit(0);

}

int test_pthread_is_alive(pthread_t thid)
{
	int ret = 0;

	ret = pthread_kill(&thid, 0);
	if(ret == ESRCH){
		/* printf("pthread is not alive ..\n"); */
	}else if(ret == EINVAL){
		/* printf("pthread signal is invaild ...\n"); */
	}else{
		/* printf("pthread is alive ...\n"); */
	}
	
	return ret;
}

int jiajian_new_or_del_2_ports_order_send_routine(void)
{
	int ret = 0;
	uint8_t jkb_index = 0x0;

	/* send data to remote ODF */
	if(Jiajian_Order.connect_sockfd[0] < 0 || Jiajian_Order.connect_sockfd[0] == 0){
			
		ret = pthread_create(&Jiajian_Order.thid[0], NULL, pthread_jiajian_connect_routine, NULL);
		if(ret < 0){
			printf("%s : %d pthread_jiajian_connect_routine failed ...\n", __func__, __LINE__);

			return ret;
		}

	}else{

		printf("%s :%d socket alive ! Jiajian_Order.connect_sockfd[0] = %d \n", __func__, __LINE__, Jiajian_Order.connect_sockfd[0]);
		ret = send(Jiajian_Order.connect_sockfd[0], &Jiajian_Order.jiajian_i3_temp_data[0][0], 
				Jiajian_Order.jiajian_i3_data_length[0], MSG_NOSIGNAL);

		if(ret < 0){
			if(errno == -EPIPE || errno == -EBADF){
				close(Jiajian_Order.connect_sockfd[0]);
			}
		}
	
	}

	/* send data to local  ODF */
	jkb_index = Jiajian_Order.local_lo_temp_data[0][6];
		
	if(jkb_index > 0 && jkb_index < 17){
		write(uart0fd, &Jiajian_Order.local_lo_temp_data[0][0], Jiajian_Order.local_data_length[0]);
		usleep(200000);
	}

	if(Jiajian_Order.local_lo_temp_data[0][4] == 0xa2){
		sleep(1);
		printf("clear Jiajian Order struct ...\n");
		memset((void *)&Jiajian_Order, 0x0, sizeof(Jiajian_t));
	}

}



void *pthread_interleave_service_routine(void *arg)
{
	fd_set rset;
	struct timeval tm;
	int  maxfd = 0, retval = 0, nbytes = 0;
	uint8_t idex = 0, loop = 0;

	for(;;){
		/* clear read set */
		maxfd = 0;
		FD_ZERO(&rset);

		/* check read set */
		for(idex = 0; idex < 5; idex++){

			if(Jiajian_Order.connect_sockfd[idex] > 0){
				if(Jiajian_Order.connect_sockfd[idex] > maxfd){
					maxfd = Jiajian_Order.connect_sockfd[idex];
				}
				FD_SET(Jiajian_Order.connect_sockfd[idex], &rset);
			}

		}		
		
		/* Judge if need to select some interleave service */
		if(maxfd > 0){

			tm.tv_sec = 1;
			tm.tv_usec = 0;
			retval = select(maxfd + 1, &rset, NULL, NULL, &tm);
			if( retval == -1 ){
				printf("select error ...\n");
			}else if(retval){

				printf("Jiajian Data Coming ...\n");
				/* check read set */
				for(loop = 0; loop < 5; loop++){

					if(FD_ISSET(Jiajian_Order.connect_sockfd[loop], &rset)){
						memset(&Jiajian_Order.jiajian_i3_temp_data[idex][0], 0x0, 250);
						nbytes = recv(Jiajian_Order.connect_sockfd[loop], &Jiajian_Order.jiajian_i3_temp_data[idex][0], 250, 0);
						if(nbytes < 0){
							/*recv action generate SOCKET_ERROR , we must handle this fault*/
							if(errno == -EBADF || errno == -ECONNREFUSED){
								Jiajian_Order.connect_sockfd[idex] = -1;
							}
						}else if(nbytes == 0){
							/* remote server may be connectless */
							Jiajian_Order.connect_sockfd[idex] = -1;
						}else{
							/* recv data coming .....*/

							/* data format transform */
							retval = data_format_transform_routine(
									&Jiajian_Order.jiajian_i3_temp_data[idex][0], 
									&Jiajian_Order.jiajian_lo_temp_data[idex][0], 
									nbytes - 1, 
									I3_PROTOCOL_2_OLD_PROTOCOL);

							if(Jiajian_Order.order_host == THIS_ODF_IS_HOST){
								debug_net_printf(&Jiajian_Order.jiajian_i3_temp_data[idex][0], retval, 60001);
							}

							if(Jiajian_Order.order_host == REMOTE_ODF_IS_HOST){
								debug_net_printf(&Jiajian_Order.jiajian_i3_temp_data[idex][0], retval, 60002);
							}
							
							pthread_mutex_lock(&mobile_mutex);
							/* call local service routine */
							module_mainBoard_Webmaster_Part_orderOperations_handle(
									&Jiajian_Order.jiajian_lo_temp_data[idex][0], 
									Jiajian_Order.connect_sockfd[idex], 
									4);
							pthread_mutex_unlock(&mobile_mutex);
						}
					}

				}	

			}else{
				printf("No data within 1 seconds... Jiajian_Order.connect_fd[0] = %d \n", Jiajian_Order.connect_sockfd[0]);
			}

		}else{

			sleep(1);
		}
	
	}

}












