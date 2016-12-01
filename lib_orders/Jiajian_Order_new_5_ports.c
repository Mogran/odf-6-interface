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

static int * communication_sockfd[5];

static uint8_t master_part_1_restructures_the_newbuld_data_routine(uint8_t *src)
{
	uint16_t datalen = 1, ofs = 0;
	uint16_t port_z_index = 0, crc16_val = 0;
	uint8_t  batch_entires = 0,  previous_index = 0;
	uint8_t  find_same_frame = 0, batch_index = 0;
	uint8_t  jianei_batch_num = 1 , jiawai_batch_num = 1;
	uint8_t  jiawai_used_sockfd_num = 0;

	/* calc frame total bytes */
	for(;;){
		if(src[datalen] == 0x7e && src[datalen+1] == 0x0){
			break;
		}else{
			datalen++;
		}
	}


	for(ofs = 0; ofs < 5; ofs++){
		communication_sockfd[ofs] = NULL;
	}

	batch_entires = (datalen - 38)%34;
	batch_entires = batch_entires/6;

	Jiajian_Order.batch_entires = batch_entires;

	for(ofs = 0; ofs < batch_entires; ofs++){
		memcpy(&Jiajian_Order.local_port_info[ofs][0], &src[72+ofs*3], 3);
		port_z_index = 72+3*batch_entires+ofs*37;
		memcpy(&Jiajian_Order.jiajian_port_uuid[ofs][0], &src[port_z_index], 30);
		memcpy(&Jiajian_Order.jiajian_port_ipaddr[ofs][0], &src[port_z_index+30], 4);
		memcpy(&Jiajian_Order.jiajian_port_info[ofs][0], &src[port_z_index+34], 3);
	}

	/* rcombine local ODF's data */
	for(batch_index = 0, ofs = 0; batch_index < batch_entires; batch_index++){
		if(batch_index == 0){
			Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x7e;
			Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
			Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
			Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x0d;
			Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x10;
			Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x07;
			memcpy(&Jiajian_Order.local_lo_temp_data[0][ofs], &src[72], 3);
			ofs += 3;
			Jiajian_Order.local_lo_temp_data[0][ofs++] = jianei_batch_num++;
			Jiajian_Order.local_data_length[0] = ofs;
		}else{

			find_same_frame = 0;
			for(previous_index = 0; previous_index < batch_entires; previous_index++){
				if(src[72+batch_index*3] == Jiajian_Order.local_lo_temp_data[previous_index][6]){
					find_same_frame = 1;
					ofs = Jiajian_Order.local_data_length[previous_index];
					memcpy(&Jiajian_Order.local_lo_temp_data[0][ofs], &src[72+batch_index*3], 3);
					ofs += 3;
					Jiajian_Order.local_lo_temp_data[0][ofs++] = jianei_batch_num++;
					Jiajian_Order.local_data_length[previous_index] = ofs;			
				}
			}

			if(!find_same_frame){
				ofs = 0;
				Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x7e;
				Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x00;
				Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x00;
				Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x0d;
				Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x10;
				Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x07;
				memcpy(&Jiajian_Order.local_lo_temp_data[batch_index][ofs], &src[72+batch_index*3], 3);
				ofs += 3;
				Jiajian_Order.local_lo_temp_data[0][ofs++] = jianei_batch_num++;
				Jiajian_Order.local_data_length[batch_index] = ofs;
			}
		}
	}

	/* calc local ODF's data crc8  */
	for(batch_index = 0; batch_index < batch_entires; batch_index++){
		if(Jiajian_Order.local_data_length[batch_index] > 0){
			ofs = Jiajian_Order.local_data_length[batch_index];
			Jiajian_Order.local_lo_temp_data[batch_index][2] = ofs;
			Jiajian_Order.local_lo_temp_data[batch_index][ofs] = calccrc8(&Jiajian_Order.local_lo_temp_data[batch_index][0], ofs);
			Jiajian_Order.local_lo_temp_data[batch_index][ofs + 1] = 0x5a;
			
//			debug_net_printf(&Jiajian_Order.local_lo_temp_data[batch_index][0], ofs + 2, 60001);
			write(uart0fd, &Jiajian_Order.local_lo_temp_data[batch_index][0], ofs + 2);
			usleep(250000);
		}	
	}


	/* recombine remote ODF's data */

	for(batch_index = 0, ofs = 0; batch_index < batch_entires; batch_index++){
		if(batch_index == 0){
			port_z_index = 72+3*batch_entires;
			Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x7e;
			Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x00;
			Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x10;
			memset(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], 0x0, 14);				
			ofs += 14;
			Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x80;
			Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x11;
			Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0xff;
			Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x30;
			memcpy(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], &src[port_z_index], 37);
			ofs += 37;
			Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = jiawai_batch_num++;
			Jiajian_Order.jiajian_lo_data_length[0] = ofs;
			
			communication_sockfd[0] = &Jiajian_Order.connect_sockfd[0];

		}else{
			find_same_frame = 0;
			port_z_index  = 72+3*batch_entires+batch_index*37;

			for(previous_index = 0; previous_index < batch_index; previous_index++){
				if(myStringcmp(&src[port_z_index], &Jiajian_Order.jiajian_lo_temp_data[previous_index][21], 30)){
					find_same_frame = 1;
					ofs = Jiajian_Order.jiajian_lo_data_length[previous_index];
					memcpy(&Jiajian_Order.jiajian_lo_temp_data[previous_index][ofs], &src[port_z_index+34], 3);
					ofs += 3;
					Jiajian_Order.jiajian_lo_temp_data[previous_index][ofs++] = jiawai_batch_num++;
					Jiajian_Order.jiajian_lo_data_length[previous_index] = ofs;	

					communication_sockfd[batch_index] = &Jiajian_Order.connect_sockfd[previous_index];
				}
			}

			if(!find_same_frame){
				ofs = 0;
				Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x7e;
				Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x00;
				Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x10;
				memset(&Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs], 0x0, 14);				
				ofs += 14;
				Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x80;
				Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x11;
				Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0xff;
				Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x30;
				memcpy(&Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs], &src[port_z_index], 37);
				ofs += 37;
				Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = jiawai_batch_num++;
				jiawai_used_sockfd_num += 1;
				Jiajian_Order.jiajian_lo_data_length[batch_index] = ofs;

				communication_sockfd[batch_index] = &Jiajian_Order.connect_sockfd[jiawai_used_sockfd_num];

			}

		}
	}

	/* calc local ODF's data crc16  */
	for(batch_index = 0; batch_index < batch_entires; batch_index++){
		if(Jiajian_Order.jiajian_lo_data_length[batch_index] > 0){
			ofs = Jiajian_Order.jiajian_lo_data_length[batch_index];
			crc16_val = crc16_calc(&Jiajian_Order.jiajian_lo_temp_data[batch_index][1], ofs - 1);
			Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = crc16_val & 0xff;
			Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = (crc16_val >> 8) & 0xff;
			Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x7e;
			Jiajian_Order.jiajian_lo_data_length[batch_index] = ofs;

			Jiajian_Order.jiajian_i3_data_length[batch_index] = data_format_transform_routine(
					&Jiajian_Order.jiajian_i3_temp_data[batch_index][0],
					&Jiajian_Order.jiajian_lo_temp_data[batch_index][0],
					Jiajian_Order.jiajian_lo_data_length[batch_index] - 1,
					OLD_PROTOCOL_2_I3_PROTOCOL
					);

			
	//		debug_net_printf(&Jiajian_Order.jiajian_i3_temp_data[batch_index][0], Jiajian_Order.jiajian_i3_data_length[batch_index], 60001);

		}	
	}

	printf("Jiajian_Order.communication_sockfd[%d] = %d ....\n",  0, *communication_sockfd[0]);
	printf("Jiajian_Order.communication_sockfd[%d] = %d ....\n",  1, *communication_sockfd[0]);
	printf("Jiajian_Order.communication_sockfd[%d] = %d ....\n",  2, *communication_sockfd[0]);

}

static uint8_t master_part_1_restructures_the_cancle_data_routine(uint8_t *src)
{
	uint16_t datalen = 1, ofs = 0, c_idex = 0;
	uint16_t port_z_index = 0, crc16_val = 0;
	uint8_t  find_same_frame = 0, batch_index = 0;
	uint8_t  jianei_used_buff_num = 0;
	uint8_t  jiawai_used_buff_num = 0;

	/* calc frame total bytes */
	for(;;){
		if(src[datalen] == 0x7e && src[datalen+1] == 0x0){
			break;
		}else{
			datalen++;
		}
	}

	jianei_used_buff_num = 0;
	jiawai_used_buff_num = 0;
	for(ofs = 23; ofs < datalen - 3; ofs += 38){

		/* rcombine local ODF's data */
		if(myStringcmp(&src[ofs + 4], &boardcfg.box_id[0], 30)){
			if(jianei_used_buff_num == 0){
					c_idex = 0;
					Jiajian_Order.local_lo_temp_data[0][c_idex++] = 0x7e;
					Jiajian_Order.local_lo_temp_data[0][c_idex++] = 0x00;
					Jiajian_Order.local_lo_temp_data[0][c_idex++] = 0x00;
					Jiajian_Order.local_lo_temp_data[0][c_idex++] = 0x0d;
					Jiajian_Order.local_lo_temp_data[0][c_idex++] = 0xa2;
					Jiajian_Order.local_lo_temp_data[0][c_idex++] = 0x01;
					memcpy(&Jiajian_Order.local_lo_temp_data[0][c_idex], &src[ofs], 4);
					c_idex += 4;
					Jiajian_Order.local_data_length[0] = c_idex;
					
					jianei_used_buff_num += 1;
			}else{
				
				find_same_frame = 0;
				for(batch_index = 0; batch_index < jianei_used_buff_num; batch_index++){
					if(src[ofs] == Jiajian_Order.local_lo_temp_data[batch_index][6]){
						find_same_frame = 1;
						c_idex = Jiajian_Order.local_data_length[batch_index];
						memcpy(&Jiajian_Order.local_lo_temp_data[batch_index][c_idex], &src[ofs], 4);
						c_idex += 4;
						Jiajian_Order.local_lo_temp_data[batch_index][5] += 0x01;
						Jiajian_Order.local_data_length[batch_index] = c_idex;				
					}				
				}

				if(!find_same_frame){
					c_idex = 0;
					Jiajian_Order.local_lo_temp_data[batch_index][c_idex++] = 0x7e;
					Jiajian_Order.local_lo_temp_data[batch_index][c_idex++] = 0x00;
					Jiajian_Order.local_lo_temp_data[batch_index][c_idex++] = 0x00;
					Jiajian_Order.local_lo_temp_data[batch_index][c_idex++] = 0x0d;
					Jiajian_Order.local_lo_temp_data[batch_index][c_idex++] = 0xa2;
					Jiajian_Order.local_lo_temp_data[batch_index][c_idex++] = 0x01;
					memcpy(&Jiajian_Order.local_lo_temp_data[batch_index][c_idex], &src[ofs], 4);
					c_idex += 4;
					Jiajian_Order.local_data_length[batch_index] = c_idex;
					
					jianei_used_buff_num += 1;			
				}

			}		

		}else{
			
			if(jiawai_used_buff_num == 0){
					c_idex = 0;
					Jiajian_Order.jiajian_lo_temp_data[0][c_idex++] = 0x7e;
					Jiajian_Order.jiajian_lo_temp_data[0][c_idex++] = 0x00;
					Jiajian_Order.jiajian_lo_temp_data[0][c_idex++] = 0x10;
					memset(&Jiajian_Order.jiajian_lo_temp_data[0][c_idex], 0x0, 14);				
					c_idex += 14;
					Jiajian_Order.jiajian_lo_temp_data[0][c_idex++] = 0x80;
					Jiajian_Order.jiajian_lo_temp_data[0][c_idex++] = 0x11;//main cmd
					Jiajian_Order.jiajian_lo_temp_data[0][c_idex++] = 0xff;//stat code
					Jiajian_Order.jiajian_lo_temp_data[0][c_idex++] = 0x30;//subcmd
					Jiajian_Order.jiajian_lo_temp_data[0][c_idex++] = 0xa3;//nxtcmd
					Jiajian_Order.jiajian_lo_temp_data[0][c_idex++] = 0x01;//cancle numbers
					memcpy(&Jiajian_Order.jiajian_lo_temp_data[0][c_idex], &src[ofs+4], 30);
					c_idex += 30;
					memcpy(&Jiajian_Order.jiajian_lo_temp_data[0][c_idex], &src[ofs+34], 4);
					c_idex += 4;
					memcpy(&Jiajian_Order.jiajian_lo_temp_data[0][c_idex], &src[ofs], 4);
					c_idex += 4;

					Jiajian_Order.jiajian_lo_data_length[0] = c_idex;		
			
					jiawai_used_buff_num += 1;
			}else{
				
				find_same_frame = 0;
				for(batch_index = 0; batch_index < jiawai_used_buff_num; batch_index++){
					if(myStringcmp(&src[ofs+4], &Jiajian_Order.jiajian_lo_temp_data[batch_index][23], 30)){
						find_same_frame = 1;
						c_idex = Jiajian_Order.jiajian_lo_data_length[batch_index];
						memcpy(&Jiajian_Order.jiajian_lo_temp_data[batch_index][c_idex], &src[ofs], 4);
						c_idex += 4;
						Jiajian_Order.jiajian_lo_temp_data[batch_index][22] += 0x01;//cancle numbers
						Jiajian_Order.jiajian_lo_data_length[0] = c_idex;		
					}			
				}
			
				if(!find_same_frame){
					c_idex = 0;
					Jiajian_Order.jiajian_lo_temp_data[batch_index][c_idex++] = 0x7e;
					Jiajian_Order.jiajian_lo_temp_data[batch_index][c_idex++] = 0x00;
					Jiajian_Order.jiajian_lo_temp_data[batch_index][c_idex++] = 0x10;
					memset(&Jiajian_Order.jiajian_lo_temp_data[batch_index][c_idex], 0x0, 14);				
					c_idex += 14;
					Jiajian_Order.jiajian_lo_temp_data[batch_index][c_idex++] = 0x80;
					Jiajian_Order.jiajian_lo_temp_data[batch_index][c_idex++] = 0x11;//main cmd
					Jiajian_Order.jiajian_lo_temp_data[batch_index][c_idex++] = 0xff;//stat code
					Jiajian_Order.jiajian_lo_temp_data[batch_index][c_idex++] = 0x30;//subcmd
					Jiajian_Order.jiajian_lo_temp_data[batch_index][c_idex++] = 0xa3;//nxtcmd
					Jiajian_Order.jiajian_lo_temp_data[batch_index][c_idex++] = 0x01;//cancle numbers
					memcpy(&Jiajian_Order.jiajian_lo_temp_data[batch_index][c_idex], &src[ofs+4], 30);
					c_idex += 30;
					memcpy(&Jiajian_Order.jiajian_lo_temp_data[batch_index][c_idex], &src[ofs+34], 4);
					c_idex += 4;
					memcpy(&Jiajian_Order.jiajian_lo_temp_data[batch_index][c_idex], &src[ofs], 4);
					c_idex += 4;

					Jiajian_Order.jiajian_lo_data_length[batch_index] = c_idex;		
			
					jiawai_used_buff_num += 1;			
				
				}
			
			}

		}
	}

	/* calc local ODF's data crc8  */
	for(batch_index = 0; batch_index < jianei_used_buff_num; batch_index++){
		if(Jiajian_Order.local_data_length[batch_index] > 0){
			c_idex = Jiajian_Order.local_data_length[batch_index];
			Jiajian_Order.local_lo_temp_data[batch_index][2] = c_idex;
			Jiajian_Order.local_lo_temp_data[batch_index][c_idex] = calccrc8(&Jiajian_Order.local_lo_temp_data[batch_index][0], c_idex);
			Jiajian_Order.local_lo_temp_data[batch_index][c_idex + 1] = 0x5a;
		
			write(uart0fd, &Jiajian_Order.local_lo_temp_data[batch_index][0], c_idex + 2);
			usleep(250000);		
		//	debug_net_printf(&Jiajian_Order.local_lo_temp_data[batch_index][0], c_idex + 2, 60001);
		}	
	}


	/* calc local ODF's data crc16  */
	for(batch_index = 0; batch_index < jiawai_used_buff_num; batch_index++){
		if(Jiajian_Order.jiajian_lo_data_length[batch_index] > 0){
			c_idex = Jiajian_Order.jiajian_lo_data_length[batch_index];
			crc16_val = crc16_calc(&Jiajian_Order.jiajian_lo_temp_data[batch_index][1], c_idex - 1);
			Jiajian_Order.jiajian_lo_temp_data[0][c_idex++] = crc16_val & 0xff;
			Jiajian_Order.jiajian_lo_temp_data[0][c_idex++] = (crc16_val >> 8) & 0xff;
			Jiajian_Order.jiajian_lo_temp_data[0][c_idex++] = 0x7e;
			Jiajian_Order.jiajian_lo_data_length[batch_index] = c_idex;


			Jiajian_Order.jiajian_i3_data_length[batch_index] = data_format_transform_routine(
					&Jiajian_Order.jiajian_i3_temp_data[batch_index][0],
					&Jiajian_Order.jiajian_lo_temp_data[batch_index][0],
					Jiajian_Order.jiajian_lo_data_length[batch_index] - 1,
					OLD_PROTOCOL_2_I3_PROTOCOL
					);

			
		//	debug_net_printf(&Jiajian_Order.jiajian_i3_temp_data[batch_index][0], Jiajian_Order.jiajian_i3_data_length[batch_index], 60001);

		}	
	}

}


void *pthread_jiajian_connect_nebuild_routine(void *arg)
{
	int ret = 0, remote_fd = 0;
	uint8_t loop = 0;

	pthread_detach(pthread_self());

	for(loop = 0; loop < 5; loop++){
		if(Jiajian_Order.jiajian_lo_data_length[loop] > 0){
			remote_fd = socket(AF_INET, SOCK_STREAM, 0);
			if(remote_fd > 0){
				Jiajian_Order.remote_addr[loop].sin_family = AF_INET;
				Jiajian_Order.remote_addr[loop].sin_port = htons(60001);
				Jiajian_Order.remote_addr[loop].sin_addr.s_addr =  0x0;
				Jiajian_Order.remote_addr[loop].sin_addr.s_addr |=  Jiajian_Order.jiajian_lo_temp_data[loop][54] << 24;
				Jiajian_Order.remote_addr[loop].sin_addr.s_addr |=  Jiajian_Order.jiajian_lo_temp_data[loop][53] << 16;
				Jiajian_Order.remote_addr[loop].sin_addr.s_addr |=  Jiajian_Order.jiajian_lo_temp_data[loop][52] << 8;
				Jiajian_Order.remote_addr[loop].sin_addr.s_addr |=  Jiajian_Order.jiajian_lo_temp_data[loop][51];
				printf("%s :%d remote s_addr = %lu ...\n", __func__, __LINE__, Jiajian_Order.remote_addr[loop].sin_addr.s_addr); 

				ret = connect(remote_fd, (struct sockaddr*)&Jiajian_Order.remote_addr[loop], sizeof(struct sockaddr_in));
				if(ret == -1){
					 printf("%s :%d jiajian newbuild connect request failed ...\n", __func__, __LINE__); 
				}else{
					 printf("%s :%d jiajian newbuild connect request successed ... ip = %d.%d.%d.%d ....\n", __func__, __LINE__, 
						 Jiajian_Order.jiajian_lo_temp_data[loop][51], Jiajian_Order.jiajian_lo_temp_data[loop][52], 
						 Jiajian_Order.jiajian_lo_temp_data[loop][53], Jiajian_Order.jiajian_lo_temp_data[loop][54]); 
				
					ret = send(remote_fd, &Jiajian_Order.jiajian_i3_temp_data[loop][0], 
							Jiajian_Order.jiajian_i3_data_length[loop], MSG_NOSIGNAL);

					if(ret < 0){
						if(errno == -EPIPE || errno == -EBADF){
							close(Jiajian_Order.connect_sockfd[loop]);
						}
					}else{
						Jiajian_Order.connect_sockfd[loop] = remote_fd;
						printf("%s :%d Jiajian_Order.connect_sockfd[%d] = %d \n", __func__, __LINE__, loop, Jiajian_Order.connect_sockfd[loop]);
					}

				}
			}
		}
	}

	printf("%s :%d pthread_jiajian_newbuild connect_routine exit ..... [done] \n", __func__, __LINE__);

	pthread_exit(0);

}

void *pthread_jiajian_connect_cancle_routine(void *arg)
{
	int ret = 0, remote_fd = 0;
	uint8_t loop = 0;

	pthread_detach(pthread_self());

	for(loop = 0; loop < 5; loop++){
		if(Jiajian_Order.jiajian_lo_data_length[loop] > 0){
			remote_fd = socket(AF_INET, SOCK_STREAM, 0);
			if(remote_fd > 0){
				Jiajian_Order.remote_addr[loop].sin_family = AF_INET;
				Jiajian_Order.remote_addr[loop].sin_port = htons(60001);

				Jiajian_Order.remote_addr[loop].sin_addr.s_addr =  0x0;
				Jiajian_Order.remote_addr[loop].sin_addr.s_addr |=  Jiajian_Order.jiajian_lo_temp_data[loop][56] << 24;
				Jiajian_Order.remote_addr[loop].sin_addr.s_addr |=  Jiajian_Order.jiajian_lo_temp_data[loop][55] << 16;
				Jiajian_Order.remote_addr[loop].sin_addr.s_addr |=  Jiajian_Order.jiajian_lo_temp_data[loop][54] << 8;
				Jiajian_Order.remote_addr[loop].sin_addr.s_addr |=  Jiajian_Order.jiajian_lo_temp_data[loop][53];
				printf("%s :%d cancled remote s_addr = %lu ...\n", __func__, __LINE__, Jiajian_Order.remote_addr[loop].sin_addr.s_addr); 

				ret = connect(remote_fd, (struct sockaddr*)&Jiajian_Order.remote_addr[loop], sizeof(struct sockaddr_in));
				if(ret == -1){
					 printf("%s :%d jiajian cancle connect request failed ...\n", __func__, __LINE__); 
				}else{
					 printf("%s :%d jiajian cancle connect request successed ... ip = %d.%d.%d.%d ....\n", __func__, __LINE__, 
						 Jiajian_Order.jiajian_lo_temp_data[loop][51], Jiajian_Order.jiajian_lo_temp_data[loop][52], 
						 Jiajian_Order.jiajian_lo_temp_data[loop][53], Jiajian_Order.jiajian_lo_temp_data[loop][54]); 
				
					ret = send(remote_fd, &Jiajian_Order.jiajian_i3_temp_data[loop][0], 
							Jiajian_Order.jiajian_i3_data_length[loop], MSG_NOSIGNAL);

					if(ret < 0){
						if(errno == -EBADF || errno == -EPIPE){
							close(remote_fd);
						}else{
							sleep(1);
						}
					}
				}
			}
		}
	}

	printf("%s :%d pthread_jiajian_cancle connect_routine exit ..... [done] \n", __func__, __LINE__);

	pthread_exit(0);

}

static int master_part_1_send_jiajian_data(void)
{
	int rc = 0;
	uint8_t loop = 0;
	
	if(Jiajian_Order.jiajian_lo_temp_data[0][21] == 0xa3){
		rc = pthread_create(&Jiajian_Order.thid[loop], NULL, pthread_jiajian_connect_cancle_routine, NULL);
		if(rc == -1){
			printf("%s : %d pthread create failed ...\n", __func__, __LINE__);
		}
	}else{
		rc = pthread_create(&Jiajian_Order.thid[loop], NULL, pthread_jiajian_connect_nebuild_routine, NULL);
		if(rc == -1){
			printf("%s : %d pthread create failed ...\n", __func__, __LINE__);
		}
	
	}

	sleep(1);

	return 0;
}

static uint8_t search_eids_info_table(uint8_t *info, uint8_t info_source)
{
	uint8_t loop_i = 0;

	switch(info_source){
	case DATA_SOURCE_JIAJIAN:
		/* search PORT A part*/
		for(loop_i = 0; loop_i < 5; loop_i++){
			if(myStringcmp(&Jiajian_Order.local_port_eids[loop_i][5], &info[26], 16)){
				return loop_i;
			}			
		}

		/* serach PORT Z part*/
		for(loop_i = 0; loop_i < 5; loop_i++){
			if(myStringcmp(&Jiajian_Order.jiajian_port_eids[loop_i][5], &info[26], 16)){
				return loop_i + 5;
			}			
		}

	break;

	case DATA_SOURCE_UART:
		/* search PORT A part*/
		for(loop_i = 0; loop_i < 5; loop_i++){
			if(myStringcmp(&Jiajian_Order.local_port_eids[loop_i][5], &info[16], 16)){
				return loop_i;
			}			
		}

		/* serach PORT Z part*/
		for(loop_i = 0; loop_i < 5; loop_i++){
			if(myStringcmp(&Jiajian_Order.jiajian_port_eids[loop_i][5], &info[16], 16)){
				return loop_i + 5;
			}			
		}

	break;
	}

	return 0;
}


static uint8_t master_part_1_handle_routine(uint8_t *src, uint8_t data_source, int sockfd)
{
	uint8_t loop = 0;
	uint16_t ofs = 0x0, crc16_val = 0;

	switch(src[21]){
	case 0xa3:
		
		master_part_1_restructures_the_cancle_data_routine(src);

		master_part_1_send_jiajian_data();

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

		memcpy(&Jiajian_Order.order_result[0], src, 38);

		/* check uuid */	
		if(!myStringcmp(&boardcfg.box_id[0], &src[PORT_A_UUID_INDEX], UUID_LENGTH)){
			printf("uuid not match ..... \n");
			Jiajian_order_reply_routine(0x10, 0x01);
			return ;
		}

		Jiajian_Order.order_host = THIS_ODF_IS_HOST;

		/* open order led */
		ioctl(alarmfd, 3, 0);
	
		master_part_1_restructures_the_newbuld_data_routine(src);
		
		master_part_1_send_jiajian_data();

	break;
	}

}

static uint8_t order_operation_led_guide(uint8_t *info, uint8_t info_source, uint8_t search_result)
{
	int retval = 0;
	uint16_t ofs = 0, crc16_val = 0;
	uint8_t batch_index = 0, ret = 0;

	switch(info_source){
	case DATA_SOURCE_UART:
		batch_index = info[10] - 1;
		if(search_result < 5){

			if(batch_index != search_result){
				/* send to local ODF */
				ofs = 0;
				Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x7e;			
				Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x00;			
				Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x0a;			
				Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x88;
				memcpy(&Jiajian_Order.local_lo_temp_data[batch_index][ofs], &info[7], 3);
				ofs += 3;
				memset(&Jiajian_Order.local_lo_temp_data[batch_index][ofs], 0x0, 3);
				ofs += 3;

				Jiajian_Order.local_lo_temp_data[batch_index][ofs] = calccrc8(&Jiajian_Order.local_lo_temp_data[batch_index][0], ofs);			
				Jiajian_Order.local_lo_temp_data[batch_index][ofs+1] = 0x5a;			

				write(uart0fd, &Jiajian_Order.local_lo_temp_data[batch_index][0], ofs + 2);
//				usleep(250000);

				/* send to remote ODF */
				ofs = 0;
				Jiajian_Order.jiajian_lo_temp_data[search_result][ofs++] = 0x7e;
				Jiajian_Order.jiajian_lo_temp_data[search_result][ofs++] = 0x00;
				Jiajian_Order.jiajian_lo_temp_data[search_result][ofs++] = 0x10;
				memset(&Jiajian_Order.jiajian_lo_temp_data[search_result][ofs], 0x0, 14);				
				ofs += 14;
				Jiajian_Order.jiajian_lo_temp_data[search_result][ofs++] = 0x80;
				Jiajian_Order.jiajian_lo_temp_data[search_result][ofs++] = 0x11;//main cmd
				Jiajian_Order.jiajian_lo_temp_data[search_result][ofs++] = 0xff;//stat code
				Jiajian_Order.jiajian_lo_temp_data[search_result][ofs++] = 0x30;//subcmd
				Jiajian_Order.jiajian_lo_temp_data[search_result][ofs++] = 0x88;//nxtcmd
				memcpy(&Jiajian_Order.jiajian_lo_temp_data[search_result][ofs], &Jiajian_Order.jiajian_port_info[search_result][0], 3);
				ofs += 3;
				memset(&Jiajian_Order.jiajian_lo_temp_data[search_result][ofs], 0x0, 3);
				ofs += 3;

				crc16_val = crc16_calc(&Jiajian_Order.jiajian_lo_temp_data[search_result][1], ofs - 1);
				Jiajian_Order.jiajian_lo_temp_data[search_result][ofs++] = crc16_val & 0xff;
				Jiajian_Order.jiajian_lo_temp_data[search_result][ofs++] = (crc16_val >> 8) & 0xff;
				Jiajian_Order.jiajian_lo_temp_data[search_result][ofs++] = 0x7e;
				Jiajian_Order.jiajian_lo_data_length[search_result] = ofs;


				Jiajian_Order.jiajian_i3_data_length[search_result] = data_format_transform_routine(
						&Jiajian_Order.jiajian_i3_temp_data[search_result][0],
						&Jiajian_Order.jiajian_lo_temp_data[search_result][0],
						Jiajian_Order.jiajian_lo_data_length[search_result] - 1,
						OLD_PROTOCOL_2_I3_PROTOCOL
						);

				if(*communication_sockfd[search_result] > 0){
					ret = send(*communication_sockfd[search_result],
							&Jiajian_Order.jiajian_i3_temp_data[search_result][0], 
							Jiajian_Order.jiajian_i3_data_length[search_result], 
							MSG_NOSIGNAL);			
					if(ret < 0){
						if(errno == -EBADF || errno == -EPIPE){
							close(*communication_sockfd[search_result]);
							*communication_sockfd[search_result] == -1;
						}
					}
				}	
		
				usleep(250000);

				ret = 1;
			}	

		}else{
			search_result -= 5;
			if(batch_index != search_result){
				ofs = 0;
				if(Jiajian_Order.local_port_info[search_result][0] == Jiajian_Order.local_port_info[batch_index][0]){
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x7e;			
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x00;			
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x0a;			
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x88;
					memcpy(&Jiajian_Order.local_lo_temp_data[batch_index][ofs], &info[7], 3);
					ofs += 3;
					memcpy(&Jiajian_Order.local_lo_temp_data[batch_index][ofs], &Jiajian_Order.local_port_info[search_result][0], 3);
					ofs += 3;

					Jiajian_Order.local_lo_temp_data[batch_index][ofs] = calccrc8(&Jiajian_Order.local_lo_temp_data[batch_index][0], ofs);			
					Jiajian_Order.local_lo_temp_data[batch_index][ofs+1] = 0x5a;				

					write(uart0fd, &Jiajian_Order.local_lo_temp_data[batch_index][0], ofs + 2);
					usleep(200000);
				}else{
					ofs = 0;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x7e;			
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x00;			
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x0a;			
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x88;
					memcpy(&Jiajian_Order.local_lo_temp_data[batch_index][ofs], &info[7], 3);
					ofs += 3;
					memset(&Jiajian_Order.local_lo_temp_data[batch_index][ofs], 0x0, 3);
					ofs += 3;

					Jiajian_Order.local_lo_temp_data[batch_index][ofs] = calccrc8(&Jiajian_Order.local_lo_temp_data[batch_index][0], ofs);			
					Jiajian_Order.local_lo_temp_data[batch_index][ofs+1] = 0x5a;				

					write(uart0fd, &Jiajian_Order.local_lo_temp_data[batch_index][0], ofs + 2);
					usleep(200000);				

					ofs = 0;
					Jiajian_Order.local_lo_temp_data[search_result][ofs++] = 0x7e;			
					Jiajian_Order.local_lo_temp_data[search_result][ofs++] = 0x00;			
					Jiajian_Order.local_lo_temp_data[search_result][ofs++] = 0x0a;			
					Jiajian_Order.local_lo_temp_data[search_result][ofs++] = 0x88;
					memcpy(&Jiajian_Order.local_lo_temp_data[search_result][ofs], &Jiajian_Order.local_port_info[search_result][0], 3);
					ofs += 3;
					memset(&Jiajian_Order.local_lo_temp_data[search_result][ofs], 0x0, 3);
					ofs += 3;

					Jiajian_Order.local_lo_temp_data[search_result][ofs] = calccrc8(&Jiajian_Order.local_lo_temp_data[search_result][0], ofs);			
					Jiajian_Order.local_lo_temp_data[search_result][ofs+1] = 0x5a;	

					write(uart0fd, &Jiajian_Order.local_lo_temp_data[search_result][0], ofs + 2);
					usleep(200000);
				}

				ret = 1;
			}
		}
	break;	
	
	case DATA_SOURCE_JIAJIAN:
		batch_index = info[25] - 1;
		if(search_result < 5){
			if(batch_index != search_result){
				
				if(myStringcmp(&Jiajian_Order.jiajian_port_uuid[batch_index][0], &Jiajian_Order.jiajian_port_uuid[search_result][0], 30)){
					/* send to remote ODF */
					ofs = 0;
					Jiajian_Order.jiajian_lo_temp_data[search_result][ofs++] = 0x7e;
					Jiajian_Order.jiajian_lo_temp_data[search_result][ofs++] = 0x00;
					Jiajian_Order.jiajian_lo_temp_data[search_result][ofs++] = 0x10;
					memset(&Jiajian_Order.jiajian_lo_temp_data[search_result][ofs], 0x0, 14);				
					ofs += 14;
					Jiajian_Order.jiajian_lo_temp_data[search_result][ofs++] = 0x80;
					Jiajian_Order.jiajian_lo_temp_data[search_result][ofs++] = 0x11;//main cmd
					Jiajian_Order.jiajian_lo_temp_data[search_result][ofs++] = 0xff;//stat code
					Jiajian_Order.jiajian_lo_temp_data[search_result][ofs++] = 0x30;//subcmd
					Jiajian_Order.jiajian_lo_temp_data[search_result][ofs++] = 0x88;//nxtcmd
					memcpy(&Jiajian_Order.jiajian_lo_temp_data[search_result][ofs], &Jiajian_Order.jiajian_port_info[search_result][0], 3);
					ofs += 3;
					memcpy(&Jiajian_Order.jiajian_lo_temp_data[search_result][ofs], &Jiajian_Order.jiajian_port_info[batch_index][0], 3);
					ofs += 3;

					crc16_val = crc16_calc(&Jiajian_Order.jiajian_lo_temp_data[search_result][1], ofs - 1);
					Jiajian_Order.jiajian_lo_temp_data[search_result][ofs++] = crc16_val & 0xff;
					Jiajian_Order.jiajian_lo_temp_data[search_result][ofs++] = (crc16_val >> 8) & 0xff;
					Jiajian_Order.jiajian_lo_temp_data[search_result][ofs++] = 0x7e;
					Jiajian_Order.jiajian_lo_data_length[search_result] = ofs;


					Jiajian_Order.jiajian_i3_data_length[search_result] = data_format_transform_routine(
							&Jiajian_Order.jiajian_i3_temp_data[search_result][0],
							&Jiajian_Order.jiajian_lo_temp_data[search_result][0],
								Jiajian_Order.jiajian_lo_data_length[search_result] - 1,
							OLD_PROTOCOL_2_I3_PROTOCOL
							);		

					if(*communication_sockfd[search_result] > 0){
						ret = send(*communication_sockfd[search_result],
								&Jiajian_Order.jiajian_i3_temp_data[search_result][0], 
								Jiajian_Order.jiajian_i3_data_length[search_result], 
								MSG_NOSIGNAL);			
						if(ret < 0){
							if(errno == -EBADF || errno == -EPIPE){
								close(*communication_sockfd[search_result]);
								*communication_sockfd[search_result] == -1;
							}
						}
					}

				}else{

					ofs = 0;
					/* send to remote ODF-1 */
					Jiajian_Order.jiajian_lo_temp_data[search_result][ofs++] = 0x7e;
					Jiajian_Order.jiajian_lo_temp_data[search_result][ofs++] = 0x00;
					Jiajian_Order.jiajian_lo_temp_data[search_result][ofs++] = 0x10;
					memset(&Jiajian_Order.jiajian_lo_temp_data[search_result][ofs], 0x0, 14);				
					ofs += 14;
					Jiajian_Order.jiajian_lo_temp_data[search_result][ofs++] = 0x80;
					Jiajian_Order.jiajian_lo_temp_data[search_result][ofs++] = 0x11;//main cmd
					Jiajian_Order.jiajian_lo_temp_data[search_result][ofs++] = 0xff;//stat code
					Jiajian_Order.jiajian_lo_temp_data[search_result][ofs++] = 0x30;//subcmd
					Jiajian_Order.jiajian_lo_temp_data[search_result][ofs++] = 0x88;//nxtcmd
					memcpy(&Jiajian_Order.jiajian_lo_temp_data[search_result][ofs], &Jiajian_Order.jiajian_port_info[search_result][0], 3);
					ofs += 3;
					memset(&Jiajian_Order.jiajian_lo_temp_data[search_result][ofs], 0x0, 3);
					ofs += 3;

					crc16_val = crc16_calc(&Jiajian_Order.jiajian_lo_temp_data[search_result][1], ofs - 1);
					Jiajian_Order.jiajian_lo_temp_data[search_result][ofs++] = crc16_val & 0xff;
					Jiajian_Order.jiajian_lo_temp_data[search_result][ofs++] = (crc16_val >> 8) & 0xff;
					Jiajian_Order.jiajian_lo_temp_data[search_result][ofs++] = 0x7e;
					Jiajian_Order.jiajian_lo_data_length[search_result] = ofs;


					Jiajian_Order.jiajian_i3_data_length[search_result] = data_format_transform_routine(
							&Jiajian_Order.jiajian_i3_temp_data[search_result][0],
							&Jiajian_Order.jiajian_lo_temp_data[search_result][0],
								Jiajian_Order.jiajian_lo_data_length[search_result] - 1,
							OLD_PROTOCOL_2_I3_PROTOCOL
							);				
				

					if(*communication_sockfd[search_result] > 0){
						ret = send(*communication_sockfd[search_result],
								&Jiajian_Order.jiajian_i3_temp_data[search_result][0], 
								Jiajian_Order.jiajian_i3_data_length[search_result], 
								MSG_NOSIGNAL);			
						if(ret < 0){
							if(errno == -EBADF || errno == -EPIPE){
								close(*communication_sockfd[search_result]);
								*communication_sockfd[search_result] == -1;
							}
						}
					}	


					/* send to remote ODF-2 */
					ofs = 0;
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x7e;
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x00;
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x10;
					memset(&Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs], 0x0, 14);				
					ofs += 14;
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x80;
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x11;//main cmd
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0xff;//stat code
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x30;//subcmd
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x88;//nxtcmd
					memcpy(&Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs], &Jiajian_Order.jiajian_port_info[batch_index][0], 3);
					ofs += 3;
					memset(&Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs], 0x0, 3);
					ofs += 3;

					crc16_val = crc16_calc(&Jiajian_Order.jiajian_lo_temp_data[batch_index][1], ofs - 1);
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = crc16_val & 0xff;
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = (crc16_val >> 8) & 0xff;
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x7e;
					Jiajian_Order.jiajian_lo_data_length[batch_index] = ofs;


					Jiajian_Order.jiajian_i3_data_length[batch_index] = data_format_transform_routine(
							&Jiajian_Order.jiajian_i3_temp_data[batch_index][0],
							&Jiajian_Order.jiajian_lo_temp_data[batch_index][0],
								Jiajian_Order.jiajian_lo_data_length[batch_index] - 1,
							OLD_PROTOCOL_2_I3_PROTOCOL
							);					
					
					if(*communication_sockfd[batch_index] > 0){
						ret = send(*communication_sockfd[batch_index],
								&Jiajian_Order.jiajian_i3_temp_data[batch_index][0], 
								Jiajian_Order.jiajian_i3_data_length[batch_index], 
								MSG_NOSIGNAL);			
						if(ret < 0){
							if(errno == -EBADF || errno == -EPIPE){
								close(*communication_sockfd[batch_index]);
								*communication_sockfd[batch_index] == -1;
							}
						}
					}	

				}

				ret = 1;
			}		
		}else{
			search_result -= 5;
			if(batch_index != search_result){
	
					/* send to remote ODF */
					ofs = 0;
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x7e;
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x00;
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x10;
					memset(&Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs], 0x0, 14);				
					ofs += 14;
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x80;
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x11;//main cmd
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0xff;//stat code
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x30;//subcmd
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x88;//nxtcmd
					memcpy(&Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs], &Jiajian_Order.jiajian_port_info[batch_index][0], 3);
					ofs += 3;
					memset(&Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs], 0x0, 3);
					ofs += 3;

					crc16_val = crc16_calc(&Jiajian_Order.jiajian_lo_temp_data[batch_index][1], ofs - 1);
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = crc16_val & 0xff;
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = (crc16_val >> 8) & 0xff;
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x7e;
					Jiajian_Order.jiajian_lo_data_length[batch_index] = ofs;


					Jiajian_Order.jiajian_i3_data_length[batch_index] = data_format_transform_routine(
							&Jiajian_Order.jiajian_i3_temp_data[batch_index][0],
							&Jiajian_Order.jiajian_lo_temp_data[batch_index][0],
								Jiajian_Order.jiajian_lo_data_length[batch_index] - 1,
							OLD_PROTOCOL_2_I3_PROTOCOL
							);		

					if(*communication_sockfd[batch_index] > 0){
						ret = send(*communication_sockfd[batch_index],
								&Jiajian_Order.jiajian_i3_temp_data[batch_index][0], 
								Jiajian_Order.jiajian_i3_data_length[batch_index], 
								MSG_NOSIGNAL);			
						if(ret < 0){
							if(errno == -EBADF || errno == -EPIPE){
								close(*communication_sockfd[batch_index]);
								*communication_sockfd[batch_index] == -1;
							}
						}
					}		

					/* send to local ODF */
					ofs = 0;
					Jiajian_Order.local_lo_temp_data[search_result][ofs++] = 0x7e;			
					Jiajian_Order.local_lo_temp_data[search_result][ofs++] = 0x00;			
					Jiajian_Order.local_lo_temp_data[search_result][ofs++] = 0x0a;			
					Jiajian_Order.local_lo_temp_data[search_result][ofs++] = 0x88;
					memcpy(&Jiajian_Order.local_lo_temp_data[search_result][ofs], &Jiajian_Order.local_port_info[search_result][0], 3);
					ofs += 3;
					memset(&Jiajian_Order.local_lo_temp_data[search_result][ofs], 0x0, 3);
					ofs += 3;

					Jiajian_Order.local_lo_temp_data[search_result][ofs] = calccrc8(&Jiajian_Order.local_lo_temp_data[search_result][0], ofs);			
					Jiajian_Order.local_lo_temp_data[search_result][ofs+1] = 0x5a;	

					write(uart0fd, &Jiajian_Order.local_lo_temp_data[search_result][0], ofs + 2);
					usleep(200000);

					ret = 1;
			}
		}
	break;
	}
	
	return ret;
}


static uint8_t slave_part_1_handle_routine(uint8_t *src, uint8_t data_type, int sockfd)
{
	uint16_t ofs = 0, datalen = 1, c_idex = 0;
	uint8_t batch_index = 0, previous_index = 0;
	uint8_t find_same_frame = 0, batch_entires = 0;
	uint8_t jianei_used_buff_num = 0;			

	switch(src[21]){
	case 0xa3:
		batch_entires = src[22];
		for(ofs = 57; ofs < 57 + src[22]*4; ofs += 4){

			if(jianei_used_buff_num == 0){
				c_idex = 0;
				Jiajian_Order.local_lo_temp_data[0][c_idex++] = 0x7e;
				Jiajian_Order.local_lo_temp_data[0][c_idex++] = 0x00;
				Jiajian_Order.local_lo_temp_data[0][c_idex++] = 0x00;
				Jiajian_Order.local_lo_temp_data[0][c_idex++] = 0x0d;
				Jiajian_Order.local_lo_temp_data[0][c_idex++] = 0xa2;
				Jiajian_Order.local_lo_temp_data[0][c_idex++] = 0x01;
				memcpy(&Jiajian_Order.local_lo_temp_data[0][c_idex], &src[ofs], 4);
				c_idex += 4;
				Jiajian_Order.local_data_length[0] = c_idex;
					
				jianei_used_buff_num += 1;

			}else{
				
				find_same_frame = 0;
				for(batch_index = 0; batch_index < jianei_used_buff_num; batch_index++){
					if(src[ofs] == Jiajian_Order.local_lo_temp_data[batch_index][6]){
						find_same_frame = 1;
						c_idex = Jiajian_Order.local_data_length[batch_index];
						memcpy(&Jiajian_Order.local_lo_temp_data[batch_index][c_idex], &src[ofs], 4);
						c_idex += 4;
						Jiajian_Order.local_lo_temp_data[batch_index][5] += 0x01;
						Jiajian_Order.local_data_length[batch_index] = c_idex;				
					}				
				}

				if(!find_same_frame){
					c_idex = 0;
					Jiajian_Order.local_lo_temp_data[batch_index][c_idex++] = 0x7e;
					Jiajian_Order.local_lo_temp_data[batch_index][c_idex++] = 0x00;
					Jiajian_Order.local_lo_temp_data[batch_index][c_idex++] = 0x00;
					Jiajian_Order.local_lo_temp_data[batch_index][c_idex++] = 0x0d;
					Jiajian_Order.local_lo_temp_data[batch_index][c_idex++] = 0xa2;
					Jiajian_Order.local_lo_temp_data[batch_index][c_idex++] = 0x01;
					memcpy(&Jiajian_Order.local_lo_temp_data[batch_index][c_idex], &src[ofs], 4);
					c_idex += 4;
					Jiajian_Order.local_data_length[batch_index] = c_idex;
				
					jianei_used_buff_num += 1;			
				}
			}	
		}

		/* calc local ODF's data crc8  */
		for(batch_index = 0; batch_index < batch_entires; batch_index++){
			if(Jiajian_Order.local_data_length[batch_index] > 0){
				ofs = Jiajian_Order.local_data_length[batch_index];
				Jiajian_Order.local_lo_temp_data[batch_index][2] = ofs;
				Jiajian_Order.local_lo_temp_data[batch_index][ofs] = calccrc8(&Jiajian_Order.local_lo_temp_data[batch_index][0], ofs);
				Jiajian_Order.local_lo_temp_data[batch_index][ofs + 1] = 0x5a;
			
	//			debug_net_printf(&Jiajian_Order.local_lo_temp_data[batch_index][0], ofs + 2, 60001);
				write(uart0fd, &Jiajian_Order.local_lo_temp_data[batch_index][0], ofs + 2);
				usleep(250000);
			}	
		}


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
		
		datalen = 1;
		for(;;){
			if(src[datalen] == 0x7e && src[datalen+1] == 0x0){
				break;
			}else{
				datalen++;
			}
		}

		batch_entires =(datalen - 54)/4;
		Jiajian_Order.batch_entires = batch_entires;
		Jiajian_Order.upload_entires = 0x0;
		ofs = 0;
		/* rcombine local ODF's data */
		for(batch_index = 0, ofs = 0; batch_index < batch_entires; batch_index++){
			if(batch_index == 0){
				Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x7e;
				Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
				Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
				Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x0d;
				Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x30;
				Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x07;
				memcpy(&Jiajian_Order.local_lo_temp_data[0][ofs], &src[55], 4);
				ofs += 4;
				Jiajian_Order.local_data_length[0] = ofs;
			}else{

				find_same_frame = 0;
				for(previous_index = 0; previous_index < batch_entires; previous_index++){
					if(src[55+batch_index*4] == Jiajian_Order.local_lo_temp_data[previous_index][6]){
						find_same_frame = 1;
						ofs = Jiajian_Order.local_data_length[previous_index];
						memcpy(&Jiajian_Order.local_lo_temp_data[0][ofs], &src[55+batch_index*4], 4);
						ofs += 4;
						Jiajian_Order.local_data_length[previous_index] = ofs;			
					}
				}

				if(!find_same_frame){
					ofs = 0;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x7e;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x00;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x00;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x0d;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x30;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x07;
					memcpy(&Jiajian_Order.local_lo_temp_data[batch_index][ofs], &src[55+batch_index*4], 4);
					ofs += 4;
					Jiajian_Order.local_data_length[batch_index] = ofs;
				}
			}
		}

		/* calc local ODF's data crc8  */
		for(batch_index = 0; batch_index < batch_entires; batch_index++){
			if(Jiajian_Order.local_data_length[batch_index] > 0){
				ofs = Jiajian_Order.local_data_length[batch_index];
				Jiajian_Order.local_lo_temp_data[batch_index][2] = ofs;
				Jiajian_Order.local_lo_temp_data[batch_index][ofs] = calccrc8(&Jiajian_Order.local_lo_temp_data[batch_index][0], ofs);
				Jiajian_Order.local_lo_temp_data[batch_index][ofs + 1] = 0x5a;
			
	//			debug_net_printf(&Jiajian_Order.local_lo_temp_data[batch_index][0], ofs + 2, 60001);
				write(uart0fd, &Jiajian_Order.local_lo_temp_data[batch_index][0], ofs + 2);
				usleep(250000);
			}	
		}
	
	break;
	}
}

static uint8_t master_part_2_handle_routine(uint8_t *src, uint8_t data_source, int sockfd)
{
	int ret = 0;
	uint16_t ofs = 0, crc16_val = 0;
	uint8_t batch_index = 0;
	

	batch_index = src[25] - 1; 
	switch(src[21]){
	
	case 0x30:
			/* check eids info table */
			ret = search_eids_info_table(src, DATA_SOURCE_JIAJIAN);
			if(ret > 0){
				ret = order_operation_led_guide(src, DATA_SOURCE_JIAJIAN, ret);
				if(ret){
					return 0;
				}
			}

			/* if local ODF's port is empty */
			if(isArrayEmpty(&Jiajian_Order.local_port_eids[batch_index][0], 32)){
				/* copy this mark's eids */
				memcpy(&Jiajian_Order.jiajian_port_eids[batch_index][0], &src[26], 32);
				Jiajian_Order.jiajian_port_stat[batch_index] = DYB_PORT_IS_USED;

				/* because remote ODF'port is empty, so we need to send this
				 * mark's eids to remote ODF */
				ofs = 0;
				Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x7e;
				Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x00;
				Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x00;
				Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x0d;
				Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x10;
				Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x41;
				
				/* local ODF's port*/
				memcpy(&Jiajian_Order.local_lo_temp_data[batch_index][ofs], &Jiajian_Order.local_port_info[batch_index][0], 4);
				ofs += 4;
				/* remote ODF's port(not necessary, so we clear with 0x0 */
				memset(&Jiajian_Order.local_lo_temp_data[batch_index][ofs], 0x0, 4);
				ofs += 4;
				memcpy(&Jiajian_Order.local_lo_temp_data[batch_index][ofs], &src[26], 32);
				ofs += 32;
				Jiajian_Order.local_lo_temp_data[batch_index][2] = ofs;
				Jiajian_Order.local_lo_temp_data[batch_index][ofs] = calccrc8(&Jiajian_Order.local_lo_temp_data[batch_index][0], ofs);
				Jiajian_Order.local_lo_temp_data[batch_index][ofs+1] = 0x5a;
				Jiajian_Order.local_data_length[batch_index] = ofs + 2;

				write(uart0fd, &Jiajian_Order.local_lo_temp_data[batch_index][0], Jiajian_Order.local_data_length[batch_index]);
				usleep(200000);

			}else{
				/* remote ODF's port is not empty */
				if(!myStringcmp(&Jiajian_Order.local_port_eids[batch_index][5], &src[31], 16)){
					/* local input mark's eids is not as same as remote ODF mark */
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x7e;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x00;		
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x10;	
					memset(&Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs], 0x0, 14);
					ofs += 14;
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x80;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x11;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0xff;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x30;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x41;//0xa3 0x18	0x28 0x8c 0x0a 
					/* remote ODF's port info, index = 22 */
					memcpy(&Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs], &Jiajian_Order.jiajian_port_info[batch_index][0], 4);
					ofs += 4;
					/* local ODF's port info (not necessary)*/
					memset(&Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs], 0x0, 4);
					ofs += 4;
					memcpy(&Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs], &Jiajian_Order.local_port_eids[batch_index][0], 32);
					ofs += 32;
					crc16_val = crc16_calc(&Jiajian_Order.jiajian_lo_temp_data[batch_index][1], ofs - 1);	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = crc16_val & 0xff;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = crc16_val >> 8 & 0xff;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x7e;	

					Jiajian_Order.jiajian_i3_data_length[batch_index] = data_format_transform_routine(
													&Jiajian_Order.jiajian_i3_temp_data[batch_index][0],
													&Jiajian_Order.jiajian_lo_temp_data[batch_index][0],
													ofs - 1,
													OLD_PROTOCOL_2_I3_PROTOCOL
													);
					if(*communication_sockfd[batch_index] > 0){
						ret = send(*communication_sockfd[batch_index],
								&Jiajian_Order.jiajian_i3_temp_data[batch_index][0], 
								Jiajian_Order.jiajian_i3_data_length[batch_index], 
								MSG_NOSIGNAL);			
						if(ret < 0){
							if(errno == -EBADF || errno == -EPIPE){
								close(*communication_sockfd[batch_index]);
								*communication_sockfd[batch_index] == -1;
							}
						}
					}	
		
				}else{

					Jiajian_Order.jiajian_port_stat[batch_index] = DYB_PORT_IS_USED;
					memcpy(&Jiajian_Order.jiajian_port_eids[batch_index][0], &src[26], 32);
				}
			}

		if(myStringcmp(&Jiajian_Order.local_port_eids[batch_index][5], &Jiajian_Order.jiajian_port_eids[batch_index][5], 16)){
		
					/* send 0x0a to remote ODF */
					ofs = 0;
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x7e;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x00;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x10;	
					memset(&Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs], 0x0, 14);
					ofs += 14;
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x80;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x11;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0xff;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x30;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x0a;//0xa3 0x18	
					/* remote ODF's 0x0a cmd, index = 22 */
					memcpy(&Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs], &Jiajian_Order.jiajian_port_info[batch_index][0], 4);
					ofs += 4;
					/* local ODF's port info (not necessary)*/
					memset(&Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs], 0x0, 4);
					ofs += 4;
					memset(&Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs], 0x0, 32);
					ofs += 32;
					crc16_val = crc16_calc(&Jiajian_Order.jiajian_lo_temp_data[batch_index][1], ofs - 1);	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = crc16_val & 0xff;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = crc16_val >> 8 & 0xff;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x7e;	

					Jiajian_Order.jiajian_i3_data_length[batch_index] = data_format_transform_routine(
														&Jiajian_Order.jiajian_i3_temp_data[batch_index][0],
														&Jiajian_Order.jiajian_lo_temp_data[batch_index][0],
														ofs - 1,
														OLD_PROTOCOL_2_I3_PROTOCOL
														);
					if(*communication_sockfd[batch_index] > 0){
						ret = send(*communication_sockfd[batch_index],
								&Jiajian_Order.jiajian_i3_temp_data[batch_index][0], 
								Jiajian_Order.jiajian_i3_data_length[batch_index], 
								MSG_NOSIGNAL);			
						if(ret < 0){
							if(errno == -EBADF || errno == -EPIPE){
								close(*communication_sockfd[batch_index]);
								*communication_sockfd[batch_index] == -1;
							}
						}
					}	

					
					/* send 0x0a to local ODF  */
					memset(&Jiajian_Order.local_lo_temp_data[batch_index][0], 0x0, 50);
					ofs = 0;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x7e;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x00;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x00;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x0d;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x0a;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x41;
					memcpy(&Jiajian_Order.local_lo_temp_data[batch_index][ofs], &Jiajian_Order.local_port_info[batch_index][0], 4);
					ofs += 4;
					memset(&Jiajian_Order.local_lo_temp_data[batch_index][ofs], 0x0, 4);
					ofs += 4;
					memset(&Jiajian_Order.local_lo_temp_data[batch_index][ofs], 0x0, 32);
					ofs += 32;
					Jiajian_Order.local_lo_temp_data[batch_index][2] = ofs;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs] = calccrc8(&Jiajian_Order.local_lo_temp_data[batch_index][0], ofs);
					Jiajian_Order.local_lo_temp_data[batch_index][ofs + 1] = 0x5a;
					Jiajian_Order.local_data_length[batch_index] = ofs + 2;		

					write(uart0fd, &Jiajian_Order.local_lo_temp_data[batch_index][0], Jiajian_Order.local_data_length[batch_index]);
					usleep(250000);		
		
		}

	break;

	case 0x8c:
		Jiajian_Order.jiajian_port_stat[batch_index] = DYB_PORT_IS_EMPTY;
		memset(&Jiajian_Order.jiajian_port_eids[batch_index][0], 0x0, 32);

		if(Jiajian_Order.local_port_stat[batch_index] == DYB_PORT_IS_EMPTY){

					/* send 0x8c to remote ODF */				
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x7e;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x00;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x10;	
					memset(&Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs], 0x0, 14);
					ofs += 14;
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x80;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x11;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0xff;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x30;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x8c;	
					/* remote ODF's 0x0a cmd, index = 22 */
					memcpy(&Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs], &Jiajian_Order.jiajian_port_info[batch_index][0], 4);
					ofs += 4;
					memset(&Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs], 0x0, 4);
					ofs += 4;
					crc16_val = crc16_calc(&Jiajian_Order.jiajian_lo_temp_data[batch_index][1], ofs - 1);	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = crc16_val & 0xff;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = crc16_val >> 8 & 0xff;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x7e;	

					Jiajian_Order.jiajian_i3_data_length[0] = data_format_transform_routine(
														&Jiajian_Order.jiajian_i3_temp_data[batch_index][0],
														&Jiajian_Order.jiajian_lo_temp_data[batch_index][0],
														ofs - 1,
														OLD_PROTOCOL_2_I3_PROTOCOL
														);
					if(*communication_sockfd[batch_index] > 0){
						ret = send(*communication_sockfd[batch_index],
								&Jiajian_Order.jiajian_i3_temp_data[batch_index][0], 
								Jiajian_Order.jiajian_i3_data_length[batch_index], 
								MSG_NOSIGNAL);			
						if(ret < 0){
							if(errno == -EBADF || errno == -EPIPE){
								close(*communication_sockfd[batch_index]);
								*communication_sockfd[batch_index] == -1;
							}
						}
					}	

					/* send 0x0a to local ODF  */
					memset(&Jiajian_Order.local_lo_temp_data[batch_index][0], 0x0, 50);
					ofs = 0;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x7e;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x00;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x00;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x0d;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x8c;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x8c;
					memcpy(&Jiajian_Order.local_lo_temp_data[batch_index][ofs], &Jiajian_Order.local_port_info[batch_index][0], 4);
					ofs += 4;
					Jiajian_Order.local_lo_temp_data[batch_index][2] = ofs;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs] = calccrc8(&Jiajian_Order.local_lo_temp_data[batch_index][0], ofs);
					Jiajian_Order.local_lo_temp_data[batch_index][ofs + 1] = 0x5a;
					Jiajian_Order.local_data_length[batch_index] = ofs + 2;		

					write(uart0fd, &Jiajian_Order.local_lo_temp_data[0][0], Jiajian_Order.local_data_length[batch_index]);
					usleep(250000);		

					memset(&Jiajian_Order.local_port_eids[batch_index][0], 0x0, 32);
		}
	break;

	case 0x0a:
		/* prepare data */
		if(src[26] == 0x00)
			Jiajian_Order.jiajian_port_eids[batch_index][39] = 0x0a;
		else
			Jiajian_Order.jiajian_port_eids[batch_index][39] = 0x0e;

		if(Jiajian_Order.local_port_eids[batch_index][39] == 0x0a && Jiajian_Order.jiajian_port_eids[batch_index][39] == 0xa){
			memcpy(&Jiajian_Order.order_result[38], &boardcfg.box_id[0], 30);
			memcpy(&Jiajian_Order.order_result[68], &boardcfg.dev_ip[0], 4);
			memcpy(&Jiajian_Order.order_result[72], &Jiajian_Order.local_port_info[batch_index][0], 3);

			memcpy(&Jiajian_Order.order_result[75],  &Jiajian_Order.jiajian_port_uuid[batch_index][0], 30);
			memcpy(&Jiajian_Order.order_result[105], &Jiajian_Order.jiajian_port_ipaddr[batch_index][0], 4);
			memcpy(&Jiajian_Order.order_result[109], &Jiajian_Order.jiajian_port_info[batch_index][0], 3);

			memcpy(&Jiajian_Order.order_result[113], &Jiajian_Order.local_port_eids[batch_index][0], 32);
			Jiajian_order_reply_routine(0x10, 0x00);
			Jiajian_Order.upload_entires += 1;

			if(Jiajian_Order.upload_entires == Jiajian_Order.batch_entires){
				ioctl(alarmfd, 4, 0);
				memset((void*)&Jiajian_Order, 0x0, sizeof(Jiajian_t));
			}
		}else{
			if(Jiajian_Order.local_port_eids[batch_index][39] == 0x0e || Jiajian_Order.jiajian_port_eids[batch_index][39] == 0x0e){
				memcpy(&Jiajian_Order.order_result[38], &boardcfg.box_id[0], 30);
				memcpy(&Jiajian_Order.order_result[68], &boardcfg.dev_ip[0], 4);
				memcpy(&Jiajian_Order.order_result[72], &Jiajian_Order.local_port_info[batch_index][0], 3);

				memcpy(&Jiajian_Order.order_result[75],  &Jiajian_Order.jiajian_port_uuid[batch_index][0], 30);
				memcpy(&Jiajian_Order.order_result[105], &Jiajian_Order.jiajian_port_ipaddr[batch_index][0], 4);
				memcpy(&Jiajian_Order.order_result[109], &Jiajian_Order.jiajian_port_info[batch_index][0], 3);
				Jiajian_order_reply_routine(0x10, 0x01);
				Jiajian_Order.upload_entires += 1;
	
				if(Jiajian_Order.upload_entires == Jiajian_Order.batch_entires){
					ioctl(alarmfd, 4, 0);
					memset((void*)&Jiajian_Order, 0x0, sizeof(Jiajian_t));
				}
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
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x10;	
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x30;//slaver part input mark
				memcpy(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], &uart_stream[7], 36);
				ofs += 36;
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
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x10;
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x8c;//0xa3 0x18	
				memcpy(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], &uart_stream[7], 4);
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
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x10;	
				Jiajian_Order.jiajian_lo_temp_data[0][ofs++] = 0x0a;//0xa3 0x18	
				memcpy(&Jiajian_Order.jiajian_lo_temp_data[0][ofs], &uart_stream[7], 4);
				ofs += 4;
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
				
				Jiajian_Order.upload_entires += 1; 
				if(Jiajian_Order.upload_entires == Jiajian_Order.batch_entires){
					sleep(2);
					ioctl(alarmfd, 4, 0);
					memset((void *)&Jiajian_Order, 0x0, sizeof(Jiajian_t));
				}
	break;

	}
}

static uint8_t slave_part_2_handle_routine(uint8_t *src, uint8_t data_type, int sockfd)
{
	uint16_t ofs = 0;

	switch(src[21]){
	
	case 0x41:
		/* prepare data */
		ofs = 0;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x7e;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x0d;
		Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x30;
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

	case 0x88:
		if(src[22] == src[25]){
			ofs = 0;
			Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x7e;
			Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
			Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x0a;
			Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x88;
			memcpy(&Jiajian_Order.local_lo_temp_data[0][ofs], &src[22], 6);
			ofs += 6;
			Jiajian_Order.local_lo_temp_data[0][ofs] = calccrc8(&Jiajian_Order.local_lo_temp_data[0][0], ofs);
			Jiajian_Order.local_lo_temp_data[0][ofs+1] = 0x5a;
			Jiajian_Order.local_data_length[0] = ofs + 2;

			write(uart0fd, &Jiajian_Order.local_lo_temp_data[0][0], Jiajian_Order.local_data_length[0]);
			usleep(200000);	
		}else{
			ofs = 0;
			Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x7e;
			Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
			Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x0a;
			Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x88;
			memcpy(&Jiajian_Order.local_lo_temp_data[0][ofs], &src[22], 3);
			ofs += 3;
			memset(&Jiajian_Order.local_lo_temp_data[0][ofs], 0x0, 3);
			ofs += 3;
			Jiajian_Order.local_lo_temp_data[0][ofs] = calccrc8(&Jiajian_Order.local_lo_temp_data[0][0], ofs);
			Jiajian_Order.local_lo_temp_data[0][ofs+1] = 0x5a;
			Jiajian_Order.local_data_length[0] = ofs + 2;

			write(uart0fd, &Jiajian_Order.local_lo_temp_data[0][0], Jiajian_Order.local_data_length[0]);
			usleep(200000);		
			
			if(src[25] > 0){
				ofs = 0;
				Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x7e;
				Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x00;
				Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x0a;
				Jiajian_Order.local_lo_temp_data[0][ofs++] = 0x88;
				memcpy(&Jiajian_Order.local_lo_temp_data[0][ofs], &src[25], 3);
				ofs += 3;
				memset(&Jiajian_Order.local_lo_temp_data[0][ofs], 0x0, 3);
				ofs += 3;
				Jiajian_Order.local_lo_temp_data[0][2] = ofs;
				Jiajian_Order.local_lo_temp_data[0][ofs] = calccrc8(&Jiajian_Order.local_lo_temp_data[0][0], ofs);
				Jiajian_Order.local_lo_temp_data[0][ofs+1] = 0x5a;
				Jiajian_Order.local_data_length[0] = ofs + 2;

				write(uart0fd, &Jiajian_Order.local_lo_temp_data[0][0], Jiajian_Order.local_data_length[0]);
				usleep(200000);	
			}	
		}

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
	
	}


}

static uint8_t master_part_3_handle_routine(uint8_t *uart_stream, uint8_t data_source, int sockfd)
{
	int ret = 0x0;
	uint16_t ofs = 0, crc16_val = 0;;
	uint8_t  batch_index = 0;

	batch_index = uart_stream[10] - 1;	
	switch(uart_stream[5]){
	case 0x03:
			/* search eids table */
			ret = search_eids_info_table(uart_stream, DATA_SOURCE_UART);
			if(ret > 0){
				ret = order_operation_led_guide(uart_stream, DATA_SOURCE_UART, ret);
				if(ret){
					return 0;
				}
			}

			/* remote ODF's port is empty */
			if(isArrayEmpty(&Jiajian_Order.jiajian_port_eids[batch_index][0], 32)){
				/* copy this mark's eids */
				memcpy(&Jiajian_Order.local_port_eids[batch_index][0], &uart_stream[11], 32);
				Jiajian_Order.local_port_stat[batch_index] = DYB_PORT_IS_USED;

				/* because remote ODF'port is empty, so we need to send this
				 * mark's eids to remote ODF */
				Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x7e;	
				Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x00;	
				Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x10;	
				memset(&Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs], 0x0, 14);
				ofs += 14;
				Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x80;	
				Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x11;	
				Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0xff;	
				Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x30;	
				Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x41;//0xa3 0x30 0x28 0x8c 0x0a 
				/* remote ODF's port info, index = 22 */
				memcpy(&Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs], &Jiajian_Order.jiajian_port_info[batch_index][0], 4);
				ofs += 4;
				/* local ODF's port info (not necessary)*/
				memset(&Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs], 0x0, 4);
				ofs += 4;
				memcpy(&Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs], &uart_stream[11], 32);
				ofs += 32;
				crc16_val = crc16_calc(&Jiajian_Order.jiajian_lo_temp_data[batch_index][1], ofs - 1);	
				Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = crc16_val & 0xff;	
				Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = crc16_val >> 8 & 0xff;	
				Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x7e;	

				Jiajian_Order.jiajian_i3_data_length[batch_index] = data_format_transform_routine(
													&Jiajian_Order.jiajian_i3_temp_data[batch_index][0],
													&Jiajian_Order.jiajian_lo_temp_data[batch_index][0],
													ofs - 1,
													OLD_PROTOCOL_2_I3_PROTOCOL
													);
					
				if(*communication_sockfd[batch_index] > 0){
					ret = send(*communication_sockfd[batch_index],
							&Jiajian_Order.jiajian_i3_temp_data[batch_index][0], 
							Jiajian_Order.jiajian_i3_data_length[batch_index], 
							MSG_NOSIGNAL);			
					if(ret < 0){
						if(errno == -EBADF || errno == -EPIPE){
							close(*communication_sockfd[batch_index]);
							*communication_sockfd[0] == -1;
						}
					}
				}	

			}else{
				/* remote ODF's port is not empty */
				if(!myStringcmp(&uart_stream[16], &Jiajian_Order.jiajian_port_eids[batch_index][5], 16)){

					/* local input mark's eids is not as same as remote ODF mark */
					memset(&Jiajian_Order.local_port_eids[batch_index][0], 0x0, 32);

					memset(&Jiajian_Order.local_lo_temp_data[batch_index][0], 0x0, 50);
					ofs = 0;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x7e;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x00;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x00;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x0d;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x10;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x41;
					memcpy(&Jiajian_Order.local_lo_temp_data[batch_index][ofs], &Jiajian_Order.local_port_info[batch_index][0], 4);
					ofs += 4;
					memset(&Jiajian_Order.local_lo_temp_data[batch_index][ofs], 0x0, 4);
					ofs += 4;
					memcpy(&Jiajian_Order.local_lo_temp_data[batch_index][ofs], &Jiajian_Order.jiajian_port_info[batch_index][0], 32);
					ofs += 32;
					Jiajian_Order.local_lo_temp_data[batch_index][2] = ofs;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs] = calccrc8(&Jiajian_Order.local_lo_temp_data[batch_index][0], ofs);
					Jiajian_Order.local_lo_temp_data[batch_index][ofs + 1] = 0x5a;
					Jiajian_Order.local_data_length[batch_index] = ofs + 2;		

					write(uart0fd, &Jiajian_Order.local_lo_temp_data[batch_index][0], Jiajian_Order.local_data_length[batch_index]);
					usleep(250000);
		
				}else{

					Jiajian_Order.local_port_stat[batch_index] = DYB_PORT_IS_USED;
					memcpy(&Jiajian_Order.local_port_eids[batch_index][0], &uart_stream[11], 32);
				}

			}

		if(myStringcmp(&Jiajian_Order.local_port_eids[batch_index][5], &Jiajian_Order.jiajian_port_eids[batch_index][5], 16)){
		
					/* send 0x0a to remote ODF */
					ofs = 0;
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x7e;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x00;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x10;	
					memset(&Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs], 0x0, 14);
					ofs += 14;
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x80;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x11;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0xff;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x30;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x0a;//0xa3 0x18	
					/* remote ODF's 0x0a cmd, index = 22 */
					memcpy(&Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs], &Jiajian_Order.jiajian_port_info[batch_index][0], 4);
					ofs += 4;
					/* local ODF's port info (not necessary)*/
					memset(&Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs], 0x0, 4);
					ofs += 4;
					memset(&Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs], 0x0, 32);
					ofs += 32;
					crc16_val = crc16_calc(&Jiajian_Order.jiajian_lo_temp_data[batch_index][1], ofs - 1);	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = crc16_val & 0xff;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = crc16_val >> 8 & 0xff;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x7e;	

					Jiajian_Order.jiajian_i3_data_length[batch_index] = data_format_transform_routine(
														&Jiajian_Order.jiajian_i3_temp_data[batch_index][0],
														&Jiajian_Order.jiajian_lo_temp_data[batch_index][0],
														ofs - 1,
														OLD_PROTOCOL_2_I3_PROTOCOL
														);

					if(*communication_sockfd[batch_index] > 0){
						ret = send(*communication_sockfd[batch_index],
								&Jiajian_Order.jiajian_i3_temp_data[batch_index][0], 
								Jiajian_Order.jiajian_i3_data_length[batch_index], 
								MSG_NOSIGNAL);			
						if(ret < 0){
							if(errno == -EBADF || errno == -EPIPE){
								close(*communication_sockfd[batch_index]);
								*communication_sockfd[0] == -1;
							}
						}
					}	


					
					/* send 0x0a to local ODF  */
					memset(&Jiajian_Order.local_lo_temp_data[batch_index][0], 0x0, 50);
					ofs = 0;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x7e;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x00;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x00;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x0d;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x0a;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x41;
					memcpy(&Jiajian_Order.local_lo_temp_data[batch_index][ofs], &Jiajian_Order.local_port_info[batch_index][0], 4);
					ofs += 4;
					memset(&Jiajian_Order.local_lo_temp_data[batch_index][ofs], 0x0, 4);
					ofs += 4;
					memset(&Jiajian_Order.local_lo_temp_data[batch_index][ofs], 0x0, 32);
					ofs += 32;
					Jiajian_Order.local_lo_temp_data[batch_index][2] = ofs;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs] = calccrc8(&Jiajian_Order.local_lo_temp_data[batch_index][0], ofs);
					Jiajian_Order.local_lo_temp_data[batch_index][ofs + 1] = 0x5a;
					Jiajian_Order.local_data_length[batch_index] = ofs + 2;		

					write(uart0fd, &Jiajian_Order.local_lo_temp_data[batch_index][0], Jiajian_Order.local_data_length[batch_index]);
					usleep(250000);		
		
		}
	break;

	case 0x8c:

		Jiajian_Order.local_port_stat[batch_index] = DYB_PORT_IS_EMPTY;
		memset(&Jiajian_Order.local_port_eids[batch_index][0], 0x0, 32 );		
		
		if(Jiajian_Order.jiajian_port_stat[batch_index] == DYB_PORT_IS_EMPTY){

					/* send 0x8c to remote ODF */				
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x7e;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x00;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x10;	
					memset(&Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs], 0x0, 14);
					ofs += 14;
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x80;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x11;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0xff;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x30;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x8c;//0xa3 0x18	
					/* remote ODF's 0x0a cmd, index = 22 */
					memcpy(&Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs], &Jiajian_Order.jiajian_port_info[batch_index][0], 4);
					ofs += 4;
					memset(&Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs], 0x0, 4);
					ofs += 4;
					crc16_val = crc16_calc(&Jiajian_Order.jiajian_lo_temp_data[batch_index][1], ofs - 1);	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = crc16_val & 0xff;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = crc16_val >> 8 & 0xff;	
					Jiajian_Order.jiajian_lo_temp_data[batch_index][ofs++] = 0x7e;	

					Jiajian_Order.jiajian_i3_data_length[batch_index] = data_format_transform_routine(
														&Jiajian_Order.jiajian_i3_temp_data[batch_index][0],
														&Jiajian_Order.jiajian_lo_temp_data[batch_index][0],
														ofs - 1,
														OLD_PROTOCOL_2_I3_PROTOCOL
														);
					if(*communication_sockfd[batch_index] > 0){
						ret = send(*communication_sockfd[batch_index],
								&Jiajian_Order.jiajian_i3_temp_data[batch_index][0], 
								Jiajian_Order.jiajian_i3_data_length[batch_index], 
								MSG_NOSIGNAL);			
						if(ret < 0){
							if(errno == -EBADF || errno == -EPIPE){
								close(*communication_sockfd[batch_index]);
								*communication_sockfd[0] == -1;
							}
						}
					}	
					
					/* send 0x0a to local ODF  */
					memset(&Jiajian_Order.local_lo_temp_data[batch_index][0], 0x0, 50);
					ofs = 0;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x7e;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x00;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x00;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x0d;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x8c;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs++] = 0x8c;
					memcpy(&Jiajian_Order.local_lo_temp_data[batch_index][ofs], &Jiajian_Order.local_port_info[batch_index][0], 4);
					ofs += 4;
					Jiajian_Order.local_lo_temp_data[batch_index][2] = ofs;
					Jiajian_Order.local_lo_temp_data[batch_index][ofs] = calccrc8(&Jiajian_Order.local_lo_temp_data[batch_index][0], ofs);
					Jiajian_Order.local_lo_temp_data[batch_index][ofs + 1] = 0x5a;
					Jiajian_Order.local_data_length[batch_index] = ofs + 2;		

					write(uart0fd, &Jiajian_Order.local_lo_temp_data[batch_index][0], Jiajian_Order.local_data_length[batch_index]);
					usleep(250000);	
		
		}
	break;

	case 0x0a:

		if(uart_stream[11] == 0x00)
			Jiajian_Order.local_port_eids[batch_index][39] = 0x0a;
		else
			Jiajian_Order.local_port_eids[batch_index][39] = 0x0e;

		if(Jiajian_Order.local_port_eids[batch_index][39] == 0x0a && Jiajian_Order.jiajian_port_eids[batch_index][39] == 0xa){
			memcpy(&Jiajian_Order.order_result[38], &boardcfg.box_id[0], 30);
			memcpy(&Jiajian_Order.order_result[68], &boardcfg.dev_ip[0], 4);
			memcpy(&Jiajian_Order.order_result[72], &Jiajian_Order.local_port_info[batch_index][0], 3);

			memcpy(&Jiajian_Order.order_result[75],  &Jiajian_Order.jiajian_port_uuid[batch_index][0], 30);
			memcpy(&Jiajian_Order.order_result[105], &Jiajian_Order.jiajian_port_ipaddr[batch_index][0], 4);
			memcpy(&Jiajian_Order.order_result[109], &Jiajian_Order.jiajian_port_info[batch_index][0], 3);

			memcpy(&Jiajian_Order.order_result[113], &Jiajian_Order.local_port_eids[batch_index][0], 32);

			Jiajian_order_reply_routine(0x10, 0x00);
			Jiajian_Order.upload_entires += 1;

			if(Jiajian_Order.upload_entires == Jiajian_Order.batch_entires){
				ioctl(alarmfd, 4, 0);
				memset((void*)&Jiajian_Order, 0x0, sizeof(Jiajian_t));
			}
		}else{
			if(Jiajian_Order.local_port_eids[batch_index][39] == 0x0e || Jiajian_Order.jiajian_port_eids[batch_index][39] == 0x0e){
				memcpy(&Jiajian_Order.order_result[38], &boardcfg.box_id[0], 30);
				memcpy(&Jiajian_Order.order_result[68], &boardcfg.dev_ip[0], 4);
				memcpy(&Jiajian_Order.order_result[72], &Jiajian_Order.local_port_info[batch_index][0], 3);

				memcpy(&Jiajian_Order.order_result[75],  &Jiajian_Order.jiajian_port_uuid[batch_index][0], 30);
				memcpy(&Jiajian_Order.order_result[105], &Jiajian_Order.jiajian_port_ipaddr[batch_index][0], 4);
				memcpy(&Jiajian_Order.order_result[109], &Jiajian_Order.jiajian_port_info[batch_index][0], 3);
				Jiajian_order_reply_routine(0x10, 0x01);
				Jiajian_Order.upload_entires += 1;

				if(Jiajian_Order.upload_entires == Jiajian_Order.batch_entires){
					ioctl(alarmfd, 4, 0);
					memset((void*)&Jiajian_Order, 0x0, sizeof(Jiajian_t));
				}
			}
		}

	break;

	}

}


/*
 *	main_cmd_code  = 0x08
 *
 * */
void Jiajian_Order_new_5_ports(unsigned char *src, unsigned char data_source, 
		int sockfd)
{
	printf("Jiajian New 5 ports ....\n");
	
	switch(data_source){
	case DATA_SOURCE_NET:
	case DATA_SOURCE_MOBILE:
		if(src[20] == 0x10){
			master_part_1_handle_routine(src, data_source, sockfd);		
		}else  if(src[20] == 0x30){
			slave_part_1_handle_routine(src, data_source, sockfd);		
		}
	break;

	case DATA_SOURCE_JIAJIAN:
		if(src[20] == 0x10){
			master_part_2_handle_routine(src, data_source, sockfd);		
		}else if(src[20] == 0x30){
			slave_part_2_handle_routine(src, data_source, sockfd);		
		}
	break;

	case DATA_SOURCE_UART:
		if(src[4] == 0x10){
			master_part_3_handle_routine(src, data_source, sockfd);		
		}else if(src[4] == 0x30){
			slave_part_3_handle_routine(src, data_source, sockfd);		
		}
	break;
	}

}
