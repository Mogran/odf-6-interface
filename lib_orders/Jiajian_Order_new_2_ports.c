#include "Jiajian_Order.h"

#define WEBMASTER_DATA_ORDERNUM_INDEX      21
#define WEBMASTER_DATA_ORDERNUM_LENGTH     17

#define UUID_LENGTH			30

#define PORT_A_UUID_INDEX	38
#define PORT_A_IP_INDEX     68
#define PORT_A_PORT_INDEX   72

#define PORT_Z_UUID_INDEX   75
#define PORT_Z_IP_INDEX     105
#define PORT_Z_PORT_INDEX   109

extern Jiajian_t Jiajian_Order;

static uint8_t master_part_1_handle_routine(uint8_t *src, uint8_t data_source, int sockfd)
{

	uint16_t ofs = 0x0, crc16_val = 0;

	switch(src[21]){
	case 0xa3:
		/* cancle order 0xa3 */
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
	

		/* send data */
		jiajian_new_or_del_2_ports_order_send_routine( );
	
		ioctl(alarmfd, 4, 0);
		memset((void*)&Jiajian_Order, 0x0, sizeof(Jiajian_t));

	break;	

	default:
		/* clear data struct */
		memset((void *)&Jiajian_Order, 0x0, sizeof(Jiajian_t));

		if(data_source == DATA_SOURCE_NET){
			Jiajian_Order.order_type = DATA_SOURCE_NET;
			Jiajian_Order.server_sockfd = sockfd;
		}

		memcpy(&Jiajian_Order.order_result[0], src, 112);

		/* check uuid */	
		if(!myStringcmp(&boardcfg.box_id[0],&src[PORT_A_UUID_INDEX], UUID_LENGTH)
				&& !myStringcmp(&boardcfg.box_id[0], &src[PORT_Z_UUID_INDEX], UUID_LENGTH)	
		){
			printf("uuid not match ..... \n");
			Jiajian_order_reply_routine(0x08, 0x01);
			return ;
		}

		Jiajian_Order.order_host = THIS_ODF_IS_HOST;

		/* open order led */
		ioctl(alarmfd, 3, 0);
	
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
	
		/* send data */
		jiajian_new_or_del_2_ports_order_send_routine( );

		memcpy(&Jiajian_Order.local_port_info[0][0], &src[PORT_A_PORT_INDEX], 3);
		memcpy(&Jiajian_Order.jiajian_port_info[0][0], &src[PORT_Z_PORT_INDEX], 3);

	break;
	}

}

static uint8_t slave_part_1_handle_routine(uint8_t *src, uint8_t data_type, int sockfd)
{
	uint16_t ofs = 0;

	switch(src[21]){
	case 0xa3:
		/* prepare data */
		ofs = 0;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x7e;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x0d;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0xa2;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x01;
		memcpy(&Jiajian_Order.local_lo_temp_data[0][ofs], &src[56], 4);
		ofs += 4;

		Jiajian_Order.local_lo_temp_data[0][2] = ofs;
		Jiajian_Order.local_lo_temp_data[0][ofs] = calccrc8(&Jiajian_Order.local_lo_temp_data[0][0], ofs);
		Jiajian_Order.local_lo_temp_data[0][ofs+1] = 0x5a;
		Jiajian_Order.local_data_length[0] = ofs + 2;

		write(uart0fd, &Jiajian_Order.local_lo_temp_data[0][0], Jiajian_Order.local_data_length[0]);
		usleep(200000);

		/* open order led */
		ioctl(alarmfd, 4, 0);

		memset((void *)&Jiajian_Order, 0x0, sizeof(Jiajian_t));
	break;	
	
	default:
		/* clear data struct */
		memset((void *)&Jiajian_Order, 0x0, sizeof(Jiajian_t));

		/* check uuid */	
		if(!myStringcmp(&boardcfg.box_id[0],&src[21], UUID_LENGTH)){
			printf("uuid not match ..... \n");
			return ;
		}

		Jiajian_Order.order_host = REMOTE_ODF_IS_HOST;
		Jiajian_Order.connect_sockfd[0] = sockfd;

		/* open order led */
		ioctl(alarmfd, 3, 0);
	
		/* prepare data */
		ofs = 0;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x7e;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x0d;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x18;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x05;
		memcpy(&Jiajian_Order.local_lo_temp_data[0][ofs], &src[55], 3);
		ofs += 3;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
		ofs += 4;
		ofs += 32;

		Jiajian_Order.local_lo_temp_data[0][2] = ofs;
		Jiajian_Order.local_lo_temp_data[0][ofs] = calccrc8(&Jiajian_Order.local_lo_temp_data[0][0], ofs);
		Jiajian_Order.local_lo_temp_data[0][ofs+1] = 0x5a;
		Jiajian_Order.local_data_length[0] = ofs + 2;

		write(uart0fd, &Jiajian_Order.local_lo_temp_data[0][0], Jiajian_Order.local_data_length[0]);
		usleep(200000);
	break;
	}
}

static uint8_t master_part_2_handle_routine(uint8_t *src, uint8_t data_source, int sockfd)
{
	int ret = 0;
	uint16_t ofs = 0, crc16_val = 0;

	switch(src[21]){

	case 0x08:
			/* if local ODF's port is empty */
			if(isArrayEmpty(&Jiajian_Order.local_port_eids[0][0], 32)){
				/* copy this mark's eids */
				memcpy(&Jiajian_Order.jiajian_port_eids[0][0], &src[22], 32);
				Jiajian_Order.jiajian_port_stat[0] = DYB_PORT_IS_USED;

				/* because remote ODF'port is empty, so we need to send this
				 * mark's eids to remote ODF */
				ofs = 0;
				Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x7e;
				Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
				Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
				Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x0d;
				Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x08;
				Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x41;
				
				/* local ODF's port*/
				memcpy(&Jiajian_Order.local_lo_temp_data[0][ofs], &Jiajian_Order.local_port_info[0][0], 4);
				ofs += 4;
				/* remote ODF's port(not necessary, so we clear with 0x0 */
				memset(&Jiajian_Order.local_lo_temp_data[0][ofs], 0x0, 4);
				ofs += 4;
				memcpy(&Jiajian_Order.local_lo_temp_data[0][ofs], &src[22], 32);
				ofs += 32;
				Jiajian_Order.local_lo_temp_data[0][2] = ofs;
				Jiajian_Order.local_lo_temp_data[0][ofs] = calccrc8(&Jiajian_Order.local_lo_temp_data[0][0], ofs);
				Jiajian_Order.local_lo_temp_data[0][ofs+1] = 0x5a;
				Jiajian_Order.local_data_length[0] = ofs + 2;

				write(uart0fd, &Jiajian_Order.local_lo_temp_data[0][0], Jiajian_Order.local_data_length[0]);
				usleep(200000);

			}else{
				/* remote ODF's port is not empty */
				if(!myStringcmp(&Jiajian_Order.local_port_eids[0][5], &src[27], 16)){
					/* local input mark's eids is not as same as remote ODF mark */
					Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x7e;	
					Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x00;		
					Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x10;	
					memset(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], 0x0, 14);
					ofs += 14;
					Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x80;	
					Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x11;	
					Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0xff;	
					Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x18;	
					Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x28;//0xa3 0x18	0x28 0x8c 0x0a 
					/* remote ODF's port info, index = 22 */
					memcpy(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], &Jiajian_Order.jiajian_port_info[0][0], 4);
					ofs += 4;
					/* local ODF's port info (not necessary)*/
					memset(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], 0x0, 4);
					ofs += 4;
					memcpy(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], &Jiajian_Order.local_port_eids[0][0], 32);
					ofs += 32;
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
					if(Jiajian_Order.connect_sockfd[0] > 0){
						ret = send(Jiajian_Order.connect_sockfd[0],
								&Jiajian_Order.jiajian_i3_temp_data[0][0], 
								Jiajian_Order.jiajian_i3_data_length[0], 
								MSG_NOSIGNAL);			
						if(ret < 0){
							if(errno == -EBADF || errno == -EPIPE){
								close(Jiajian_Order.connect_sockfd[0]);
								Jiajian_Order.connect_sockfd[0] == -1;
							}
						}
					}	
		
				}else{

					Jiajian_Order.jiajian_port_stat[0] = DYB_PORT_IS_USED;
					memcpy(&Jiajian_Order.jiajian_port_eids[0][0], &src[22], 32);
				}
			}

		if(myStringcmp(&Jiajian_Order.local_port_eids[0][5], &Jiajian_Order.jiajian_port_eids[0][5], 16)){
		
					/* send 0x0a to remote ODF */
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
					Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x0a;//0xa3 0x18	
					/* remote ODF's 0x0a cmd, index = 22 */
					memcpy(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], &Jiajian_Order.jiajian_port_info[0][0], 4);
					ofs += 4;
					/* local ODF's port info (not necessary)*/
					memset(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], 0x0, 4);
					ofs += 4;
					memset(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], 0x0, 32);
					ofs += 32;
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
					if(Jiajian_Order.connect_sockfd[0] > 0){
						ret = send(Jiajian_Order.connect_sockfd[0],
								&Jiajian_Order.jiajian_i3_temp_data[0][0], 
								Jiajian_Order.jiajian_i3_data_length[0], 
								MSG_NOSIGNAL);			
						if(ret < 0){
							if(errno == -EBADF || errno == -EPIPE){
								close(Jiajian_Order.connect_sockfd[0]);
								Jiajian_Order.connect_sockfd[0] == -1;
							}
						}
					}	
					
					/* send 0x0a to local ODF  */
					memset(&Jiajian_Order.local_lo_temp_data[0][0], 0x0, 50);
					ofs = 0;
					Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x7e;
					Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
					Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
					Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x0d;
					Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x0a;
					Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x41;
					memcpy(&Jiajian_Order.local_lo_temp_data[0][ofs], &Jiajian_Order.local_port_info[0][0], 4);
					ofs += 4;
					memset(&Jiajian_Order.local_lo_temp_data[0][ofs], 0x0, 4);
					ofs += 4;
					memset(&Jiajian_Order.local_lo_temp_data[0][ofs], 0x0, 32);
					ofs += 32;
					Jiajian_Order.local_lo_temp_data[0][2] = ofs;
					Jiajian_Order.local_lo_temp_data[0][ofs] = calccrc8(&Jiajian_Order.local_lo_temp_data[0][0], ofs);
					Jiajian_Order.local_lo_temp_data[0][ofs + 1] = 0x5a;
					Jiajian_Order.local_data_length[0] = ofs + 2;		

					write(uart0fd, &Jiajian_Order.local_lo_temp_data[0][0], Jiajian_Order.local_data_length[0]);
					usleep(250000);		
		
		}

	break;

	case 0x8c:
		Jiajian_Order.jiajian_port_stat[0] = DYB_PORT_IS_EMPTY;
		memset(&Jiajian_Order.jiajian_port_eids[0][0], 0x0, 32);

		if(Jiajian_Order.local_port_stat[0] == DYB_PORT_IS_EMPTY){

					/* send 0x8c to remote ODF */				
					Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x7e;	
					Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x00;	
					Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x10;	
					memset(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], 0x0, 14);
					ofs += 14;
					Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x80;	
					Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x11;	
					Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0xff;	
					Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x18;	
					Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x8c;//0xa3 0x18	
					/* remote ODF's 0x0a cmd, index = 22 */
					memcpy(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], &Jiajian_Order.jiajian_port_info[0][0], 4);
					ofs += 4;
					memset(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], 0x0, 4);
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
					if(Jiajian_Order.connect_sockfd[0] > 0){
						ret = send(Jiajian_Order.connect_sockfd[0],
								&Jiajian_Order.jiajian_i3_temp_data[0][0], 
								Jiajian_Order.jiajian_i3_data_length[0], 
								MSG_NOSIGNAL);			
						if(ret < 0){
							if(errno == -EBADF || errno == -EPIPE){
								close(Jiajian_Order.connect_sockfd[0]);
								Jiajian_Order.connect_sockfd[0] == -1;
							}
						}
					}	
					
					/* send 0x0a to local ODF  */
					memset(&Jiajian_Order.local_lo_temp_data[0][0], 0x0, 50);
					ofs = 0;
					Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x7e;
					Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
					Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
					Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x0d;
					Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x8c;
					Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x8c;
					memcpy(&Jiajian_Order.local_lo_temp_data[0][ofs], &Jiajian_Order.local_port_info[0][0], 4);
					ofs += 4;
					Jiajian_Order.local_lo_temp_data[0][2] = ofs;
					Jiajian_Order.local_lo_temp_data[0][ofs] = calccrc8(&Jiajian_Order.local_lo_temp_data[0][0], ofs);
					Jiajian_Order.local_lo_temp_data[0][ofs + 1] = 0x5a;
					Jiajian_Order.local_data_length[0] = ofs + 2;		

					write(uart0fd, &Jiajian_Order.local_lo_temp_data[0][0], Jiajian_Order.local_data_length[0]);
					usleep(250000);		

					memset(&Jiajian_Order.local_port_eids[0][0], 0x0, 32);
		}
	break;

	case 0x0a:
		/* prepare data */
		if(src[22] == 0x00)
			Jiajian_Order.jiajian_port_eids[0][39] = 0x0a;
		else
			Jiajian_Order.jiajian_port_eids[0][39] = 0x0e;

		if(Jiajian_Order.local_port_eids[0][39] == 0x0a && Jiajian_Order.jiajian_port_eids[0][39] == 0xa){
			Jiajian_order_reply_routine(0x08, 0x00);
			ioctl(alarmfd, 4, 0);
			memset((void *)&Jiajian_Order, 0x0, sizeof(Jiajian_t));
		}else{
			if(Jiajian_Order.local_port_eids[0][39] == 0x0e || Jiajian_Order.jiajian_port_eids[0][39] == 0x0e){
				Jiajian_order_reply_routine(0x08, 0x01);
				ioctl(alarmfd, 4, 0);
				memset((void *)&Jiajian_Order, 0x0, sizeof(Jiajian_t));
			}
		}
		
	break;
	}

}

static uint8_t slave_part_3_handle_routine(uint8_t *uart_stream, uint8_t data_type, int sockfd)
{
	int ret = 0x0;
	uint16_t ofs = 0, crc16_val = 0;

	switch(uart_stream[5]){

	case 0x03:
				memset(&Jiajian_Order.local_lo_temp_data[0][0], 0x0, 80);
				ofs = 0;
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x7e;	
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x00;	
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x10;	
				memset(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], 0x0, 14);
				ofs += 14;
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x80;	
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x11;	
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0xff;	
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x08;	
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x08;//0xa3 0x18	0x8c 0x0a 
				memcpy(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], &uart_stream[10], 32);
				ofs += 32;
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
				if(Jiajian_Order.connect_sockfd[0] > 0){
					ret = send(Jiajian_Order.connect_sockfd[0],
							&Jiajian_Order.jiajian_i3_temp_data[0][0], 
							Jiajian_Order.jiajian_i3_data_length[0], 
							MSG_NOSIGNAL);			
					if(ret < 0){
						if(errno == -EBADF || errno == -EPIPE){
							close(Jiajian_Order.connect_sockfd[0]);
							Jiajian_Order.connect_sockfd[0] == -1;
						}
					}
				}	
	break;

	case 0x8c:

				memset(&Jiajian_Order.local_lo_temp_data[0][0], 0x0, 80);
					/* send 0x8c to remote ODF */				
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x7e;	
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x00;	
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x10;	
				memset(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], 0x0, 14);
				ofs += 14;
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x80;	
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x11;	
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0xff;	
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x08;	
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x8c;//0xa3 0x18	
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
				if(Jiajian_Order.connect_sockfd[0] > 0){
					ret = send(Jiajian_Order.connect_sockfd[0],
							&Jiajian_Order.jiajian_i3_temp_data[0][0], 
							Jiajian_Order.jiajian_i3_data_length[0], 
							MSG_NOSIGNAL);			
					if(ret < 0){
						if(errno == -EBADF || errno == -EPIPE){
							close(Jiajian_Order.connect_sockfd[0]);
							Jiajian_Order.connect_sockfd[0] == -1;
						}
					}
				}	
	break;

	case 0x0a:
				memset(&Jiajian_Order.local_lo_temp_data[0][0], 0x0, 80);
				/* send 0x8c to remote ODF */				
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x7e;	
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x00;	
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x10;	
				memset(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], 0x0, 14);
				ofs += 14;
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x80;	
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x11;	
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0xff;	
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x08;	
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x0a;//0xa3 0x18	
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = uart_stream[11];//success:0x0 fail:0x01
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
				if(Jiajian_Order.connect_sockfd[0] > 0){
					ret = send(Jiajian_Order.connect_sockfd[0],
							&Jiajian_Order.jiajian_i3_temp_data[0][0], 
							Jiajian_Order.jiajian_i3_data_length[0], 
							MSG_NOSIGNAL);			
					if(ret < 0){
						if(errno == -EBADF || errno == -EPIPE){
							close(Jiajian_Order.connect_sockfd[0]);
							Jiajian_Order.connect_sockfd[0] == -1;
						}
					}
				}
				
				sleep(2);
				ioctl(alarmfd, 4, 0);
				memset((void *)&Jiajian_Order, 0x0, sizeof(Jiajian_t));
	break;

	}
}

static uint8_t slave_part_2_handle_routine(uint8_t *src, uint8_t data_type, int sockfd)
{
	uint16_t ofs = 0;

	switch(src[21]){
	
	case 0x18:
		/* clear data struct */
		memset((void *)&Jiajian_Order, 0x0, sizeof(Jiajian_t));

		/* check uuid */	
		if(!myStringcmp(&boardcfg.box_id[0],&src[21], UUID_LENGTH)){
			printf("uuid not match ..... \n");
			return ;
		}

		Jiajian_Order.order_host = REMOTE_ODF_IS_HOST;
		Jiajian_Order.connect_sockfd[0] = sockfd;

		/* open order led */
		ioctl(alarmfd, 3, 0);
	
		/* prepare data */
		ofs = 0;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x7e;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x0d;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x18;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x05;
		memcpy(&Jiajian_Order.local_lo_temp_data[0][ofs], &src[55], 3);
		ofs += 3;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
		ofs += 4;
		ofs += 32;

		Jiajian_Order.local_lo_temp_data[0][2] = ofs;
		Jiajian_Order.local_lo_temp_data[0][ofs] = calccrc8(&Jiajian_Order.local_lo_temp_data[0][0], ofs);
		Jiajian_Order.local_lo_temp_data[0][ofs+1] = 0x5a;
		Jiajian_Order.local_data_length[0] = ofs + 2;

		write(uart0fd, &Jiajian_Order.local_lo_temp_data[0][0], Jiajian_Order.local_data_length[0]);
		usleep(200000);

	break;

	case 0x28:
		/* prepare data */
		ofs = 0;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x7e;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x0d;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x18;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x41;
		memcpy(&Jiajian_Order.local_lo_temp_data[0][ofs], &src[22], 32+4+4);
		ofs += 40;
		Jiajian_Order.local_lo_temp_data[0][2] = ofs;
		Jiajian_Order.local_lo_temp_data[0][ofs] = calccrc8(&Jiajian_Order.local_lo_temp_data[0][0], ofs);
		Jiajian_Order.local_lo_temp_data[0][ofs+1] = 0x5a;
		Jiajian_Order.local_data_length[0] = ofs + 2;

		write(uart0fd, &Jiajian_Order.local_lo_temp_data[0][0], Jiajian_Order.local_data_length[0]);
		usleep(200000);
	break;

	case 0x8c:
		/* prepare data */
		ofs = 0;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x7e;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x0d;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x8c;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x8c;
		memcpy(&Jiajian_Order.local_lo_temp_data[0][ofs], &src[22], 8);
		ofs += 8;
		Jiajian_Order.local_lo_temp_data[0][2] = ofs;
		Jiajian_Order.local_lo_temp_data[0][ofs] = calccrc8(&Jiajian_Order.local_lo_temp_data[0][0], ofs);
		Jiajian_Order.local_lo_temp_data[0][ofs+1] = 0x5a;
		Jiajian_Order.local_data_length[0] = ofs + 2;

		write(uart0fd, &Jiajian_Order.local_lo_temp_data[0][0], Jiajian_Order.local_data_length[0]);
		usleep(200000);
	break;

	case 0x0a:
		/* prepare data */
		ofs = 0;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x7e;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x0d;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x0a;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x41;
		memcpy(&Jiajian_Order.local_lo_temp_data[0][ofs], &src[22], 32+4+4);
		ofs += 40;
		Jiajian_Order.local_lo_temp_data[0][2] = ofs;
		Jiajian_Order.local_lo_temp_data[0][ofs] = calccrc8(&Jiajian_Order.local_lo_temp_data[0][0], ofs);
		Jiajian_Order.local_lo_temp_data[0][ofs+1] = 0x5a;
		Jiajian_Order.local_data_length[0] = ofs + 2;

		write(uart0fd, &Jiajian_Order.local_lo_temp_data[0][0], Jiajian_Order.local_data_length[0]);
		usleep(200000);
	
	break;

	case 0xa3:
		/* prepare data */
		ofs = 0;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x7e;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x0d;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0xa2;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x01;
		memcpy(&Jiajian_Order.local_lo_temp_data[0][ofs], &src[56], 4);
		ofs += 4;

		Jiajian_Order.local_lo_temp_data[0][2] = ofs;
		Jiajian_Order.local_lo_temp_data[0][ofs] = calccrc8(&Jiajian_Order.local_lo_temp_data[0][0], ofs);
		Jiajian_Order.local_lo_temp_data[0][ofs+1] = 0x5a;
		Jiajian_Order.local_data_length[0] = ofs + 2;

		write(uart0fd, &Jiajian_Order.local_lo_temp_data[0][0], Jiajian_Order.local_data_length[0]);
		usleep(200000);

		/* open order led */
		ioctl(alarmfd, 4, 0);
		
		/* clear all data struct */
		memset((void *)&Jiajian_Order, 0x0, sizeof(Jiajian_t));

	break;
	
	}


}

static uint8_t master_part_3_handle_routine(uint8_t *uart_stream, uint8_t data_source, int sockfd)
{
	int ret = 0x0;
	uint16_t ofs = 0, crc16_val = 0;;

	switch(uart_stream[5]){
	case 0x03:
			/* remote ODF's port is empty */
			if(isArrayEmpty(&Jiajian_Order.jiajian_port_eids[0][0], 32)){
				/* copy this mark's eids */
				memcpy(&Jiajian_Order.local_port_eids[0][0], &uart_stream[10], 32);
				Jiajian_Order.local_port_stat[0] = DYB_PORT_IS_USED;

				/* because remote ODF'port is empty, so we need to send this
				 * mark's eids to remote ODF */
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x7e;	
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x00;	
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x10;	
				memset(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], 0x0, 14);
				ofs += 14;
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x80;	
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x11;	
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0xff;	
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x18;	
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x28;//0xa3 0x18	0x28 0x8c 0x0a 
				/* remote ODF's port info, index = 22 */
				memcpy(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], &Jiajian_Order.jiajian_port_info[0][0], 4);
				ofs += 4;
				/* local ODF's port info (not necessary)*/
				memset(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], 0x0, 4);
				ofs += 4;
				memcpy(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], &uart_stream[10], 32);
				ofs += 32;
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

				if(Jiajian_Order.connect_sockfd[0] > 0){
					ret = send(Jiajian_Order.connect_sockfd[0],
							&Jiajian_Order.jiajian_i3_temp_data[0][0], 
							Jiajian_Order.jiajian_i3_data_length[0], 
							MSG_NOSIGNAL);			
					if(ret < 0){
						if(errno == -EBADF || errno == -EPIPE){
							close(Jiajian_Order.connect_sockfd[0]);
							Jiajian_Order.connect_sockfd[0] == -1;
						}
					}
				}	

			}else{
				/* remote ODF's port is not empty */
				if(!myStringcmp(&uart_stream[15], &Jiajian_Order.jiajian_port_eids[0][5], 16)){

					/* local input mark's eids is not as same as remote ODF mark */
					memset(&Jiajian_Order.local_port_eids[0][0], 0x0, 32);

					memset(&Jiajian_Order.local_lo_temp_data[0][0], 0x0, 50);
					ofs = 0;
					Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x7e;
					Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
					Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
					Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x0d;
					Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x08;
					Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x41;
					memcpy(&Jiajian_Order.local_lo_temp_data[0][ofs], &Jiajian_Order.local_port_info[0][0], 4);
					ofs += 4;
					memset(&Jiajian_Order.local_lo_temp_data[0][ofs], 0x0, 4);
					ofs += 4;
					memcpy(&Jiajian_Order.local_lo_temp_data[0][ofs], &Jiajian_Order.jiajian_port_info[0][0], 32);
					ofs += 32;
					Jiajian_Order.local_lo_temp_data[0][2] = ofs;
					Jiajian_Order.local_lo_temp_data[0][ofs] = calccrc8(&Jiajian_Order.local_lo_temp_data[0][0], ofs);
					Jiajian_Order.local_lo_temp_data[0][ofs + 1] = 0x5a;
					Jiajian_Order.local_data_length[0] = ofs + 2;		

					write(uart0fd, &Jiajian_Order.local_lo_temp_data[0][0], Jiajian_Order.local_data_length[0]);
					usleep(250000);
		
				}else{

					Jiajian_Order.local_port_stat[0] = DYB_PORT_IS_USED;
					memcpy(&Jiajian_Order.local_port_eids[0][0], &uart_stream[10], 32);
				}

			}

		if(myStringcmp(&Jiajian_Order.local_port_eids[0][5], &Jiajian_Order.jiajian_port_eids[0][5], 16)){
		
					/* send 0x0a to remote ODF */
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
					Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x0a;//0xa3 0x18	
					/* remote ODF's 0x0a cmd, index = 22 */
					memcpy(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], &Jiajian_Order.jiajian_port_info[0][0], 4);
					ofs += 4;
					/* local ODF's port info (not necessary)*/
					memset(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], 0x0, 4);
					ofs += 4;
					memset(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], 0x0, 32);
					ofs += 32;
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
					if(Jiajian_Order.connect_sockfd[0] > 0){
						ret = send(Jiajian_Order.connect_sockfd[0],
								&Jiajian_Order.jiajian_i3_temp_data[0][0], 
								Jiajian_Order.jiajian_i3_data_length[0], 
								MSG_NOSIGNAL);			
						if(ret < 0){
							if(errno == -EBADF || errno == -EPIPE){
								close(Jiajian_Order.connect_sockfd[0]);
								Jiajian_Order.connect_sockfd[0] == -1;
							}
						}
					}	
					
					/* send 0x0a to local ODF  */
					memset(&Jiajian_Order.local_lo_temp_data[0][0], 0x0, 50);
					ofs = 0;
					Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x7e;
					Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
					Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
					Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x0d;
					Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x0a;
					Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x41;
					memcpy(&Jiajian_Order.local_lo_temp_data[0][ofs], &Jiajian_Order.local_port_info[0][0], 4);
					ofs += 4;
					memset(&Jiajian_Order.local_lo_temp_data[0][ofs], 0x0, 4);
					ofs += 4;
					memset(&Jiajian_Order.local_lo_temp_data[0][ofs], 0x0, 32);
					ofs += 32;
					Jiajian_Order.local_lo_temp_data[0][2] = ofs;
					Jiajian_Order.local_lo_temp_data[0][ofs] = calccrc8(&Jiajian_Order.local_lo_temp_data[0][0], ofs);
					Jiajian_Order.local_lo_temp_data[0][ofs + 1] = 0x5a;
					Jiajian_Order.local_data_length[0] = ofs + 2;		

					write(uart0fd, &Jiajian_Order.local_lo_temp_data[0][0], Jiajian_Order.local_data_length[0]);
					usleep(250000);		
		
		}
	break;

	case 0x8c:

		Jiajian_Order.local_port_stat[0] = DYB_PORT_IS_EMPTY;
		memset(&Jiajian_Order.local_port_eids[0][0], 0x0, 32 );		
		
		if(Jiajian_Order.jiajian_port_stat[0] == DYB_PORT_IS_EMPTY){

					/* send 0x8c to remote ODF */				
					Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x7e;	
					Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x00;	
					Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x10;	
					memset(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], 0x0, 14);
					ofs += 14;
					Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x80;	
					Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x11;	
					Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0xff;	
					Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x18;	
					Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x8c;//0xa3 0x18	
					/* remote ODF's 0x0a cmd, index = 22 */
					memcpy(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], &Jiajian_Order.jiajian_port_info[0][0], 4);
					ofs += 4;
					memset(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], 0x0, 4);
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
					if(Jiajian_Order.connect_sockfd[0] > 0){
						ret = send(Jiajian_Order.connect_sockfd[0],
								&Jiajian_Order.jiajian_i3_temp_data[0][0], 
								Jiajian_Order.jiajian_i3_data_length[0], 
								MSG_NOSIGNAL);			
						if(ret < 0){
							if(errno == -EBADF || errno == -EPIPE){
								close(Jiajian_Order.connect_sockfd[0]);
								Jiajian_Order.connect_sockfd[0] == -1;
							}
						}
					}	
					
					/* send 0x0a to local ODF  */
					memset(&Jiajian_Order.local_lo_temp_data[0][0], 0x0, 50);
					ofs = 0;
					Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x7e;
					Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
					Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
					Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x0d;
					Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x8c;
					Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x8c;
					memcpy(&Jiajian_Order.local_lo_temp_data[0][ofs], &Jiajian_Order.local_port_info[0][0], 4);
					ofs += 4;
					Jiajian_Order.local_lo_temp_data[0][2] = ofs;
					Jiajian_Order.local_lo_temp_data[0][ofs] = calccrc8(&Jiajian_Order.local_lo_temp_data[0][0], ofs);
					Jiajian_Order.local_lo_temp_data[0][ofs + 1] = 0x5a;
					Jiajian_Order.local_data_length[0] = ofs + 2;		

					write(uart0fd, &Jiajian_Order.local_lo_temp_data[0][0], Jiajian_Order.local_data_length[0]);
					usleep(250000);	


					memset(&Jiajian_Order.jiajian_port_eids[0][0], 0x0, 32 );		
		
		}
	break;

	case 0x0a:

		if(uart_stream[11] == 0x00)
			Jiajian_Order.local_port_eids[0][39] = 0x0a;
		else
			Jiajian_Order.local_port_eids[0][39] = 0x0e;

		if(Jiajian_Order.local_port_eids[0][39] == 0x0a && Jiajian_Order.jiajian_port_eids[0][39] == 0xa){
			Jiajian_order_reply_routine(0x08, 0x00);
			ioctl(alarmfd, 4, 0);
			memset((void*)&Jiajian_Order, 0x0, sizeof(Jiajian_t));
		}else{
			if(Jiajian_Order.local_port_eids[0][39] == 0x0e || Jiajian_Order.jiajian_port_eids[0][39] == 0x0e){
				Jiajian_order_reply_routine(0x08, 0x01);
				ioctl(alarmfd, 4, 0);
				memset((void*)&Jiajian_Order, 0x0, sizeof(Jiajian_t));
			}
		}

	break;

	}

}


/*
 *	main_cmd_code  = 0x08
 *
 * */
void Jiajian_Order_new_2_ports(unsigned char *src, unsigned char data_source, 
		int sockfd)
{
	printf("Jiajian New 2 ports ....\n");
	
	switch(data_source){
	case DATA_SOURCE_NET:
	case DATA_SOURCE_MOBILE:
		if(src[20] == 0x08){
			master_part_1_handle_routine(src, data_source, sockfd);		
		}else  if(src[20] == 0x18){
			slave_part_1_handle_routine(src, data_source, sockfd);		
		}
	break;

	case DATA_SOURCE_JIAJIAN:
		if(src[20] == 0x08){
			master_part_2_handle_routine(src, data_source, sockfd);		
		}else if(src[20] == 0x18){
			slave_part_2_handle_routine(src, data_source, sockfd);		
		}
	break;

	case DATA_SOURCE_UART:
		if(src[4] == 0x08){
			master_part_3_handle_routine(src, data_source, sockfd);		
		}else if(src[4] == 0x18){
			slave_part_3_handle_routine(src, data_source, sockfd);		
		}
	break;
	}

}
