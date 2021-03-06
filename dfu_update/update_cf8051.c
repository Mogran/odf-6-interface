/*
 *@brief: 接口板软件升级程序
 *@date : 2015-04-21
 *@writer:lu-ming-liang
 *@least update:2015-04-17
 */

#include "dfu_update.h"
#include "os_struct.h"

extern int 		  uart0fd;
extern struct DFU_UPDATE dfu;
extern OS_UART	  uart0;
struct DFU_CF8051 cf8051;

static int find_which_cf8051_board_needs_to_update(uint8_t *buff)
{
	uint8_t inloops = 0;

	for(inloops = 0; inloops < 48; inloops++){
		if(cf8051.updating[inloops] == UPDATE_NOW && cf8051.updating_status[inloops] == UPDATE_NOW)
		{
			buff[0] = inloops/6+1;//find frame
			buff[1] = inloops%6+1;//find tray
			return FIND_SUCCESS;
		}
	}
	
	return FIND_FAILED;
}

static int send_startup_command_to_specific_cf8051_board(void)
{
	int ret = 0;
	unsigned char frame_index = 0, tray_index = 0;	
	unsigned short index, loops, outloops, inloops;
	unsigned char update_cf8051[2];

	memset(update_cf8051, 0x0, 2);
	ret = find_which_cf8051_board_needs_to_update(update_cf8051);
	if(ret == FIND_FAILED){
		return UPDATE_DONE;
	}
	
	frame_index = update_cf8051[0];
	tray_index = update_cf8051[1];
	cf8051.updating_frame_index = frame_index;	
	cf8051.updating_tray_index = tray_index;	

	memset(&uart0.txBuf[frame_index-1][0], 0x0, SZ_600);
	uart0.txBuf[frame_index-1][0] = 0x7e;
	uart0.txBuf[frame_index-1][1] = 0x00;
	uart0.txBuf[frame_index-1][2] = 0x0a;
	uart0.txBuf[frame_index-1][3] = 0x06;/* maincmd */
	uart0.txBuf[frame_index-1][4] = 0x01;/* subcmd:0x1 is symbol for startup */
	uart0.txBuf[frame_index-1][5] = 0x03;/* dfu type: 0x3 is symbol for cf8051 */
	uart0.txBuf[frame_index-1][6] = 0x01;/* dfu number is 1 */
	uart0.txBuf[frame_index-1][7] = frame_index;
	uart0.txBuf[frame_index-1][8] = tray_index;
	uart0.txBuf[frame_index-1][9] = 0x01;/* startup status : 0x01---default failed */
	uart0.txBuf[frame_index-1][10] = dev_update_calccrc8(&uart0.txBuf[frame_index-1][0], uart0.txBuf[frame_index-1][2]);
	uart0.txBuf[frame_index-1][11] = 0x5a;

		/*  send data to specific frame that this cf8051 board whereis */
	for(loops = 0; loops < 3; loops++){
		write(uart0fd, &uart0.txBuf[frame_index-1][0], uart0.txBuf[frame_index-1][2] + 2);
		usleep(500000);

		memset(&uart0.rxBuf[0], 0x0, sizeof(uart0.rxBuf));
		if(read(uart0fd, &uart0.rxBuf[0], 100) > 0){
			if(uart0.rxBuf[3] == 0xa6){
				memset(&uart0.txBuf[frame_index-1][0], 0x0, 12);
				cf8051.update = UPDATE_NOW;
				break;
			}
		}else{
			printf("not recv frame%d tray%d .... startup cmd reply \n", frame_index, tray_index);
		}
	}
	
	if(cf8051.update == UPDATE_NOW){
		printf("finish send startup cmd, update c8051 continue ... \n");
		return UPDATE_CONTINUE;
	}

	/* if startup command send failed , set this frame update fail!!! */
	if(uart0.txBuf[frame_index-1][3] == 0x6){
		for(loops = 0; loops < 6; loops++){
			index = (frame_index-1)*6+loops;
			if(cf8051.updating[index] == UPDATE_NOW){
				cf8051.updating_status[index] = UPDATE_DONE;
				cf8051.updating_result[index] = UPDATE_FAIL;
				cf8051.total_update_nums -= 1;
			}	
		}
	}
	
	memset(&uart0.txBuf[frame_index-1][0], 0x0, 12);

	return FRAME_SEND_FAILED;
}

static int verify_update_finished_or_not(void)
{
	uint8_t inloops = 0;

	for(inloops = 0; inloops < 96; inloops++){
		if(cf8051.updating[inloops] == UPDATE_NOW && cf8051.updating_status[inloops] == UPDATE_NOW){
			return UPDATE_CONTINUE;
		}
	}
	
	return UPDATE_DONE;
}

static void get_plates_which_needs_to_update(void)
{
	uint8_t ofs = 0, loop = 0; 
	uint8_t frame_stats[30];
	uint8_t cmd_array[20];
	uint8_t reply_array[20];

	/* Judge whether this frame is active */
	memset(cmd_array, 0x0, sizeof(cmd_array));
	memset(frame_stats, 0x0, sizeof(frame_stats));
	cmd_array[0] = 0x7e;		
	cmd_array[1] = 0x00;		
	cmd_array[2] = 0x05;		
	cmd_array[3] = 0x20;
	cmd_array[6] = 0x5a;
	for(ofs = 0; ofs < 8; ofs++){
		cmd_array[4] = ofs + 1;		
		cmd_array[5] = crc8(cmd_array, 5);		

		write(uart0fd, cmd_array, cmd_array[2]+2);
		usleep(400000);
		
		memset(reply_array, 0x0, sizeof(reply_array));
		if(read(uart0fd, reply_array, 7) > 0){
			if(reply_array[3] != 0x0){
				frame_stats[ofs] = ofs + 1;
			}
		}
	}

	debug_net_printf(frame_stats, 10, 60001);

	/* Judge active frames's plate situation */
	for(ofs = 0; ofs < 8; ofs++){
		/* if this frame is active */
		if(frame_stats[ofs] == ofs + 1){
			cmd_array[0] = 0x7e;
			cmd_array[1] = 0x00;
			cmd_array[2] = 0x05;
			cmd_array[3] = 0x03;
			cmd_array[4] = ofs + 1;		
			cmd_array[5] = crc8(cmd_array, 5);		
			cmd_array[6] = 0x5a;

			write(uart0fd, cmd_array, cmd_array[2]+2);
			usleep(400000);

			memset(reply_array, 0x0, sizeof(reply_array));
			if(read(uart0fd, reply_array, 7) > 0){
				if(reply_array[3] == 0xa6){
					frame_stats[ofs] = ofs + 1;
				}
			}else{
				continue;
			}

			cmd_array[0] = 0x7e;
			cmd_array[1] = 0x00;
			cmd_array[2] = 0x05;
			cmd_array[3] = 0x20;
			cmd_array[4] = ofs + 1;		
			cmd_array[5] = crc8(cmd_array, 5);		
			cmd_array[6] = 0x5a;

			write(uart0fd, cmd_array, cmd_array[2]+2);
			usleep(400000);
			memset(reply_array, 0x0, sizeof(reply_array));
			if(read(uart0fd, reply_array, 15) > 0){
				if(reply_array[3] == 0x03){
					cmd_array[0] = 0x7e;
					cmd_array[1] = 0x00;
					cmd_array[2] = 0x07;
					cmd_array[3] = 0xa6;
					cmd_array[4] = 0x03;
					cmd_array[5] = ofs + 1;
					cmd_array[6] = ofs + 1;		
					cmd_array[7] = crc8(cmd_array, 7);		
					cmd_array[8] = 0x5a;

					write(uart0fd, cmd_array, cmd_array[2]+2);
					usleep(400000);

					memcpy(&cf8051.updating[ofs*6],        &reply_array[6], 6);
					memcpy(&cf8051.updating_status[ofs*6], &reply_array[6], 6);
					memcpy(&cf8051.updating_result[ofs*6], &reply_array[6], 6);

					for(loop = 0; loop < 6; loop++){
						if(reply_array[loop + 6] != 0x0){
							cf8051.total_update_nums += 1;
							cf8051.updating[ofs*6 + loop] = UPDATE_NOW;
							cf8051.updating_status[ofs*6 + loop] =  UPDATE_NOW;
							cf8051.updating_result[ofs*6 + loop] =  UPDATE_NOW;
						}
					}
				}
			}

		}
	
	}

}

static void webmaster_data_handle(void)
{
	memset((void*)&cf8051, 0x0, sizeof(struct DFU_CF8051));

	get_plates_which_needs_to_update();

	if(cf8051.total_update_nums > 0){
		cf8051.update = UPDATE_NOW;
	}

//	debug_net_printf(&cf8051.updating[0], 48, 60001);
//	debug_net_printf(&cf8051.updating_status[0], 48, 60001);
//	debug_net_printf(&cf8051.updating_result[0], 48, 60001);

}

/*
 *
 *@brief: Part1--cf8051 board, startup service routine .	
 *
 */
static int cf8051_startup_service_routine(void)
{
	int ret = 0;
	unsigned char loops, update_status;

	if(cf8051.total_update_nums == 0)
		return UPDATE_DONE;

	//send
	for(loops = 0; loops < cf8051.total_update_nums; loops++){
		ret = send_startup_command_to_specific_cf8051_board();

		if(ret == UPDATE_DONE){
			update_status = UPDATE_DONE;
			break;
		}else if(ret == FRAME_SEND_FAILED){
			ret = verify_update_finished_or_not();
			if(ret == UPDATE_DONE){
				update_status = UPDATE_DONE;
				break;
			}
		}else if (ret == UPDATE_CONTINUE){
			update_status = UPDATE_CONTINUE;
			break;
		}

	}
	//check update status
	if(update_status == UPDATE_DONE){
		return UPDATE_DONE;
	}

	return UPDATE_CONTINUE;
}

/*
 *@brief: Part2--cf8051 enter bootloader service rotuine .
 *
 */
static int cf8051_enter_bootloader_stage_handle(unsigned char *uart_stream)
{
	int ret = 0;
	unsigned short index = 0;
	unsigned char frame_index = 0, tray_index = 0;
	unsigned char loops = 0;

	printf("\r frame %d Tray %d enter bootloader stage ...\n", cf8051.updating_frame_index, cf8051.updating_tray_index);	
	
	frame_index = cf8051.updating_frame_index;
	tray_index = cf8051.updating_tray_index;
	index = 9 +(tray_index-1)*3;
	if(uart_stream[index] != 0x0){
		cf8051.updating_status[(frame_index-1)*6+(tray_index-1)] = UPDATE_DONE;
		return CF8051_STARTUP_FAILED;
	}

	index = 0;
	uart0.txBuf[frame_index-1][index++] = 0x7e;/* bit0 : */
	uart0.txBuf[frame_index-1][index++] = 0x00;/* bit1 : */
	uart0.txBuf[frame_index-1][index++] = 0x00;/* bit2 : */
	uart0.txBuf[frame_index-1][index++] = 0x06;/* bit3 : maincmd */
	uart0.txBuf[frame_index-1][index++] = 0x02;/* bit4 : sub_cmd */
	uart0.txBuf[frame_index-1][index++] = 0x03;/* bit5 : devtype */
	uart0.txBuf[frame_index-1][index++] = 0x01;/* bit6 : devnums */
	uart0.txBuf[frame_index-1][index++] = frame_index;
	uart0.txBuf[frame_index-1][index++] = tray_index;
	uart0.txBuf[frame_index-1][index++] = 0x0; /* bit9 : package index */
	uart0.txBuf[frame_index-1][index++] = 0x1; /* bit10: package index */
	memcpy(&uart0.txBuf[frame_index-1][index], &data_packet[0][0], SZ_512);
	index += SZ_512;
	uart0.txBuf[frame_index-1][1] = (index>>8)&0xff;
	uart0.txBuf[frame_index-1][2] = index&0xff;
	uart0.txBuf[frame_index-1][index] = dev_update_calccrc8(&uart0.txBuf[frame_index-1][0], index);
	uart0.txBuf[frame_index-1][index+1] = 0x5a;

	/*  send package data to specific frame that this cf8051 board whereis */
	usleep(300000);
	for(loops = 0; loops < 3; loops++){
		write(uart0fd, &uart0.txBuf[frame_index-1][0], uart0.txBuf[frame_index-1][1]<<8|uart0.txBuf[frame_index-1][2]+2);
		usleep(350000);
		memset(uart0.rxBuf, 0x0, sizeof(uart0.rxBuf));
		ret = read(uart0fd, &uart0.rxBuf[0], 100);
		if(ret > 6){
			if(uart0.rxBuf[3] == 0xa6){
				memset(&uart0.txBuf[frame_index-1][0], 0x0, SZ_600);
				return UPDATE_CONTINUE;
			}
		}
	}
	
	/* if package send failed , set this frame update fail!!! */
	if(uart0.txBuf[frame_index-1][3] == 0x6){
		for(loops = 0; loops < 6; loops++){
			index = (frame_index-1)*6+loops;
			if(cf8051.updating[index] == UPDATE_NOW){
				cf8051.updating_status[index] = UPDATE_DONE;
				cf8051.updating_result[index] = UPDATE_FAIL;
			}	
		}
	}
}


/*
 *@brief:Part3--cf8051 board now writing data to MCU's flash.
 *
 */
static int cf8051_write_nflash_stage_handle(unsigned char *uart_stream)
{
	int ret = 0, status;
	unsigned short index = 0, package_index = 0, nxt_package_index = 0;
	unsigned char frame_index = 0, tray_index = 0, loops = 0;

	frame_index = cf8051.updating_frame_index;
	tray_index = cf8051.updating_tray_index;

	package_index    = uart_stream[26];
	nxt_package_index = package_index;  
	status = nxt_package_index*100 / dfu.data_packet_num;

	printf("frame %d Tray %d package %d , progress rate%d%% ...\r\n", cf8051.updating_frame_index, cf8051.updating_tray_index, 
			nxt_package_index , status);	

	if(nxt_package_index == dfu.data_packet_num){
		/* send restart cmd to specific cf8051 board */
		memset(&uart0.txBuf[frame_index-1][0], 0x0, SZ_600);
		uart0.txBuf[frame_index-1][index++] = 0x7e;/* bit0: */
		uart0.txBuf[frame_index-1][index++] = 0x00;/* bit1: */
		uart0.txBuf[frame_index-1][index++] = 0x00;/* bit2: */
		uart0.txBuf[frame_index-1][index++] = 0x06;/* bit3: maincmd */
		uart0.txBuf[frame_index-1][index++] = 0x03;/* bit4: sub_cmd ---restart cf8051 board */
		uart0.txBuf[frame_index-1][index++] = 0x03;/* bit5: devtype */
		uart0.txBuf[frame_index-1][index++] = 0x01;/* bit6: devnums */
		uart0.txBuf[frame_index-1][index++] = frame_index;
		uart0.txBuf[frame_index-1][index++] = tray_index;
		uart0.txBuf[frame_index-1][index++] = 0x01;/* bit9: restart failed,this is default value. */
		uart0.txBuf[frame_index-1][1] = (index>>8)&0xff;
		uart0.txBuf[frame_index-1][2] = index&0xff;
		uart0.txBuf[frame_index-1][index] = dev_update_calccrc8(&uart0.txBuf[frame_index-1][0], index);
		uart0.txBuf[frame_index-1][index+1] = 0x5a;
	}else{
		memset(&uart0.txBuf[frame_index-1][0], 0x0, SZ_600);
		uart0.txBuf[frame_index-1][index++] = 0x7e;/* bit0: */
		uart0.txBuf[frame_index-1][index++] = 0x00;/* bit1: */
		uart0.txBuf[frame_index-1][index++] = 0x00;/* bit2: */
		uart0.txBuf[frame_index-1][index++] = 0x06;/* bit3: maincmd */
		uart0.txBuf[frame_index-1][index++] = 0x02;/* bit4: sub_cmd */
		uart0.txBuf[frame_index-1][index++] = 0x03;/* bit5: devtype */
		uart0.txBuf[frame_index-1][index++] = 0x01;/* bit6: devnums */
		uart0.txBuf[frame_index-1][index++] = frame_index;
		uart0.txBuf[frame_index-1][index++] = tray_index;
		uart0.txBuf[frame_index-1][index++] = ((nxt_package_index+1)>>8)&0xff;
		uart0.txBuf[frame_index-1][index++] = (nxt_package_index+1)&0xff;
		memcpy(&uart0.txBuf[frame_index-1][index], &data_packet[nxt_package_index][0], SZ_512);
		index += SZ_512;
		uart0.txBuf[frame_index-1][1] = (index>>8)&0xff;
		uart0.txBuf[frame_index-1][2] = index&0xff;
		uart0.txBuf[frame_index-1][index] = dev_update_calccrc8(&uart0.txBuf[frame_index-1][0], index);
		uart0.txBuf[frame_index-1][index+1] = 0x5a;
	}

	usleep(300000);	
	/*  send package data to specific frame that this cf8051 board whereis */
	for(loops = 0; loops < 3; loops++){
		write(uart0fd, &uart0.txBuf[frame_index-1][0], uart0.txBuf[frame_index-1][1]<<8|uart0.txBuf[frame_index-1][2]+2);
		usleep(350000);

		memset(uart0.rxBuf, 0x0, sizeof(uart0.rxBuf));
		if(read(uart0fd, &uart0.rxBuf[0], 100) > 0){
			if(uart0.rxBuf[3] == 0xa6){
				memset(&uart0.txBuf[frame_index-1][0], 0x0, SZ_600);
				return UPDATE_CONTINUE;
			}
		}
	}
	
	/* if package send failed , set this frame update fail!!! */
	if(uart0.txBuf[frame_index-1][3] == 0x6){

		for(loops = 0; loops < 6; loops++){
			index = (frame_index-1)*6+loops;
			if(cf8051.updating[index] == UPDATE_NOW){
				cf8051.updating_status[index] = UPDATE_DONE;
				cf8051.updating_result[index] = UPDATE_FAIL;
				cf8051.total_update_nums -= 1;
			}	
		}

	}

	return FRAME_SEND_FAILED;
}

/*
 *@brief: Part4--cf8051 board now restart finished ..
 *
 */
static int cf8051_enter_reboot_stage_handle(unsigned char *uart_stream)
{
	int ret = 0;
	unsigned short index = 0, loops = 0, update_result_bit = 0;
	unsigned char frame_index = 0, tray_index = 0;
	
	frame_index = cf8051.updating_frame_index;
	 tray_index = cf8051.updating_tray_index;

	update_result_bit = 9 + (tray_index-1)*3;
	index    = (frame_index-1)*6+(tray_index-1);
	if(uart_stream[update_result_bit] == 0x00){
		cf8051.updating_status[index] = UPDATE_DONE;
		cf8051.updating_result[index] = 0x0;
	}else{	
		cf8051.updating_status[index] = UPDATE_DONE;
		cf8051.updating_result[index] = UPDATE_FAIL;
	}

	cf8051.total_update_nums -= 1;

	return 0;
}


/*
 *@brief: update done. handle some data... 	
 *
 */
static void filling_data_and_send_to_webmaster(void)
{
	int loop_var = 0, update_result = 0x0;

	printf("%s : %d cf8051 board update ...... [ Done] \n", __func__, __LINE__);	
	
	for(loop_var = 0; loop_var < 48; loop_var++){
		if(cf8051.updating[loop_var] == UPDATE_NOW && cf8051.updating_result[loop_var] == UPDATE_FAIL){
			update_result = 0x01;
			break;
		}
	}

	memset((void*)&cf8051, 0x0, sizeof(struct DFU_CF8051));

	cf8051.update = UPDATE_DONE;
	cf8051.update_result = update_result;
}

/*
 *@description : cf8051 boards devices update main routine.	
 *
 *
 * */
void update_cf8051(unsigned char data_source, unsigned char *stream, int fd)
{
	int ret = 0, stage_code = 0, loops = 0;

	switch(data_source){
	case DATA_FROM_NET:
		webmaster_data_handle( );
	
		ret = cf8051_startup_service_routine();
		if(ret == UPDATE_DONE){
			goto update_error;
		}
		printf("%s:%d first frame data send done ...\n", __func__, __LINE__);
	break;

	case DATA_FROM_UART:
		stage_code = stream[4];
		switch(stage_code){
		case STAGE1_DEVICE_STARTUP:
			ret = cf8051_enter_bootloader_stage_handle(stream);
			if(ret == CF8051_STARTUP_FAILED){
				ret = cf8051_startup_service_routine();
				if(ret == UPDATE_DONE){
					goto update_error;
				}	
			}
		break;

		case STAGE2_WRITING_NFLASH:
			ret = cf8051_write_nflash_stage_handle(stream);
			if(ret == FRAME_SEND_FAILED){
				ret = cf8051_startup_service_routine();
				if(ret == UPDATE_DONE){
					goto update_error;
				}
			}
		break;

		case STAGE3_DEVICE_RESTART:
			//record cf8051 board update result
			cf8051_enter_reboot_stage_handle(stream);

			//contiue to update other cf8051 board.
			ret = cf8051_startup_service_routine();
			if(ret == UPDATE_DONE){
				//if update done, reply to webmaster
				filling_data_and_send_to_webmaster();
			}
		break;
		}
	
	break;
	}

	return ;

update_error:
	filling_data_and_send_to_webmaster();
	return ;
}
