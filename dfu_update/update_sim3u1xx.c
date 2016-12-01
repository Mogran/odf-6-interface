/*
 *@brief: 接口板软件升级程序
 *@date : 2015-04-17
 *@writer:lu-ming-liang
 *@least update:2015-04-17
 */

#include "dfu_update.h"
#include "os_struct.h"

struct DFU_SIM3U1XX sim3u146;
extern struct DFU_UPDATE dfu;
extern OS_UART uart0;
extern int uart0fd;

unsigned char reply_to_webmaster_data[100];

/*******************************************************************************
 *
 *	stage1:	Entering prepare data and send to startup cmd ...
 *
 *
 * */

static void get_frames_which_needs_to_update(void)
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
	
	for(ofs = 0; ofs < 8; ofs++){
		if(frame_stats[ofs] != 0x0){
			sim3u146.updating[ofs] = UPDATE_NOW;
			sim3u146.updating_status[ofs] = UPDATE_NOW;
			sim3u146.updating_result[ofs] = UPDATE_NOW;
			sim3u146.total_update_nums += 1;
		}
	}

}

static void startup_stage_step1_download_file( void )
{

	memset((void*)&sim3u146, 0x0, sizeof(struct DFU_SIM3U1XX));
	/* prepare data to distribute update cmd&&data to sim3u146 boards */

	get_frames_which_needs_to_update();

	sim3u146.update = UPDATE_NOW;	
	sim3u146.update_result = UPDATE_FAIL;	
	
	dfu.data_packet_num += 1;

	printf("line :%d ---- %d frame needs to update\n" , __LINE__, sim3u146.total_update_nums);

}

static int find_which_sim3u146_board_needs_to_update(uint8_t *buff)
{
	uint8_t inloops = 0;

	for(inloops = 0; inloops < 8; inloops++){
		if(sim3u146.updating[inloops] == UPDATE_NOW && sim3u146.updating_status[inloops] == UPDATE_NOW)
		{
			buff[0] = inloops+1;//find frame
			printf("find frame %d needs to update! \n", buff[0]);
			return FIND_SUCCESS;
		}
	}
	
	printf("without any frames need to update! \n");
	return FIND_FAILED;
}

/*
 *@brief: Part1--sim3u146 startrup stage handle routine
 *
 * */
static int send_startup_command_to_specific_sim3u146_board(void)
{
	int ret = 0;
	unsigned char boardnum = 0, inloops = 0, nbytes = 0, loops = 0;
	unsigned char framebuff[2];

	memset(framebuff, 0x0, sizeof(framebuff));
	ret = find_which_sim3u146_board_needs_to_update(framebuff);
	if(ret == FIND_FAILED){
		return UPDATE_DONE;
	}

	sim3u146.updating_frame_index = framebuff[0];
	boardnum = sim3u146.updating_frame_index - 1;

	memset(&uart0.txBuf[boardnum][0], 0x0, SZ_600);
	uart0.txBuf[boardnum][0] = 0x7e;
	uart0.txBuf[boardnum][1] = 0x00;
	uart0.txBuf[boardnum][2] = 0x08;
	uart0.txBuf[boardnum][3] = 0x06;
	uart0.txBuf[boardnum][4] = 0x01;//startup
	uart0.txBuf[boardnum][5] = 0x02;//type:interface board
	uart0.txBuf[boardnum][6] = 0x01;//num : 1 ge
	uart0.txBuf[boardnum][7] = boardnum+1;//frameidex 
	uart0.txBuf[boardnum][8] = dev_update_calccrc8(&uart0.txBuf[boardnum][0],8);
	uart0.txBuf[boardnum][9] = 0x5a;	

	for(loops = 0; loops < 3; loops++){
		write(uart0fd, &uart0.txBuf[boardnum][0], uart0.txBuf[boardnum][1]<<8|uart0.txBuf[boardnum][2]+2);
		usleep(300000);
		/* get reply code :0xa6 */
		memset(uart0.rxBuf, 0, sizeof(uart0.rxBuf));
		nbytes = read(uart0fd, uart0.rxBuf, 100);
		if(nbytes > 0 && (0xa6 == uart0.rxBuf[3])){
			memset(&uart0.txBuf[boardnum][0], 0x0, SZ_600);
			sim3u146.updating_status[boardnum] = UPDATE_NOW;
			sim3u146.update = UPDATE_NOW;
			return UPDATE_CONTINUE;
		}
	}

	//try to send this frame 3 times, all failed! mark this board's
	//upgrade stop and clear all stat
	if(uart0.txBuf[boardnum][3] == 0x06){
		sim3u146.updating_status[boardnum] = UPDATE_DONE;
		sim3u146.updating_result[boardnum] = UPDATE_FAIL;
		memset(&uart0.txBuf[boardnum][0], 0x0, SZ_600);
		sim3u146.total_update_nums -= 1;
	}
	
	printf("frame %d startup command send failed!\n", boardnum+1);	
	return FRAME_SEND_FAILED;
}


static int verify_update_finished_or_not(void)
{
	uint8_t inloops = 0;

	for(inloops = 0; inloops < 16; inloops++){
		if(sim3u146.updating[inloops] == UPDATE_NOW && sim3u146.updating_status[inloops] == UPDATE_NOW){
			return UPDATE_CONTINUE;
		}
	}
	
	return UPDATE_DONE;
}

static int sim3u146_startup_service_routine(void)
{
	int ret = 0;
	unsigned char loops, update_status;

	if(sim3u146.total_update_nums == 0)
		return UPDATE_DONE;

	//send
	for(loops = 0; loops < sim3u146.total_update_nums; loops = 0){
		ret = send_startup_command_to_specific_sim3u146_board();
		if(ret == UPDATE_DONE){
			update_status = UPDATE_DONE;
			break;
		}else if(ret == FRAME_SEND_FAILED){
			ret = verify_update_finished_or_not();
			if(ret == UPDATE_DONE){
				update_status = UPDATE_DONE;
				break;
			}
		}else if(ret == UPDATE_CONTINUE){
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


/*******************************************************************************
 *
 *	stage2: Enter writing data to NandFlash .
 *
 *
 *
 * */
static int sim3u146_enter_bootloader_stage_handle(unsigned char *uart_stream)
{
	int ret = 0;
	unsigned char frame_index = 0, package_index = 0;
	unsigned char loops = 0, nbytes = 0;

	printf("frame %d is updating, stage One, package 13 ,progress rate %d%% \n",  sim3u146.updating_frame_index, 1300/dfu.data_packet_num);
	frame_index = uart_stream[7];
	package_index = 17;

	if(sim3u146.updating[frame_index-1]){
		memset(&uart0.txBuf[frame_index-1][0],0,SZ_600);
		uart0.txBuf[frame_index-1][0] = 0x7e;
		uart0.txBuf[frame_index-1][1] = 0x02;
		uart0.txBuf[frame_index-1][2] = 0x0c;
		uart0.txBuf[frame_index-1][3] = 0x06;
		uart0.txBuf[frame_index-1][4] = 0x02;//transfer data packet
		uart0.txBuf[frame_index-1][5] = 0x02;//type:interface board
		uart0.txBuf[frame_index-1][6] = 0x01;//num : 1 ge
		uart0.txBuf[frame_index-1][7] = frame_index;//frameidex 
		uart0.txBuf[frame_index-1][8] = 0x00;//pan hao(zhanweifu)
		uart0.txBuf[frame_index-1][9] = 0x01;//success || Failed
		uart0.txBuf[frame_index-1][10] = package_index >> 8 & 0xff;//xuhao1
		uart0.txBuf[frame_index-1][11] = package_index;//xuhao1 
		memcpy(&uart0.txBuf[frame_index-1][12],&data_packet[package_index-1][0],512);
		uart0.txBuf[frame_index-1][524] = dev_update_calccrc8(&uart0.txBuf[frame_index-1][0],524);
		uart0.txBuf[frame_index-1][525] = 0x5a;
	}	

	usleep(300000);
	for(loops = 0; loops < 3; loops++){	
		/* transmit command data */
		if(sim3u146.updating[frame_index-1] && (0x06 == uart0.txBuf[frame_index-1][3])){
			write(uart0fd, &uart0.txBuf[frame_index-1][0], 
					uart0.txBuf[frame_index-1][1]<<8|uart0.txBuf[frame_index-1][2]+2);	
			usleep(500000);
		}

		/* get reply code :0xa6 */
		memset(uart0.rxBuf, 0, sizeof(uart0.rxBuf));
		nbytes = read(uart0fd, uart0.rxBuf, 100);
		if(nbytes > 0 && (0xa6 == uart0.rxBuf[3])){
			memset(&uart0.txBuf[frame_index-1][0], 0x0, SZ_600);
			sim3u146.updating_status[frame_index-1] = UPDATE_NOW;
			return UPDATE_CONTINUE;
		}
	}

	if(0x06 == uart0.txBuf[frame_index-1][3]){
		sim3u146.updating_status[frame_index-1] = UPDATE_DONE;
		sim3u146.updating_result[frame_index-1] = UPDATE_FAIL;
		sim3u146.total_update_nums -= 1;
	}

	return FRAME_SEND_FAILED;
}


/*******************************************************************************
 *
 *	stage3: Enter writing data to NandFlash .
 *
 *
 *
 * */
static int sim3u146_write_nflash_stage_handle(unsigned char *uart_stream)
{
	int status = 0;
	unsigned char frame_index = 0, package_index = 0;
	unsigned char loops = 0, nbytes = 0;

	frame_index = uart_stream[7];
	package_index = uart_stream[11] + 1;
	status = package_index*100 / dfu.data_packet_num;

	printf("frame %d is updating, stage Two, package %d ,progress rate %d%% \n", 
			sim3u146.updating_frame_index, package_index, status);

	if(package_index == dfu.data_packet_num){
		/* package transmit over! send restart command */
		if(sim3u146.updating[frame_index-1] > 0){
			memset(&uart0.txBuf[frame_index-1][0],0,SZ_600);
			uart0.txBuf[frame_index-1][0] = 0x7e;
			uart0.txBuf[frame_index-1][1] = 0x00;
			uart0.txBuf[frame_index-1][2] = 0x10;
			uart0.txBuf[frame_index-1][3] = 0x06;
			uart0.txBuf[frame_index-1][4] = 0x03;//transfer data packet
			uart0.txBuf[frame_index-1][5] = 0x02;//type:interface board
			uart0.txBuf[frame_index-1][6] = 0x01;//num : 1 ge
					
			uart0.txBuf[frame_index-1][7] = frame_index;//frameidex 
			uart0.txBuf[frame_index-1][8] = 0x00;//pan hao(zhanweifu)
			uart0.txBuf[frame_index-1][9] = 0x01;//success || Failed
			uart0.txBuf[frame_index-1][10] = 0xff;//hardware version high bit
			uart0.txBuf[frame_index-1][11] = 0xff;//hardware version low bit
			uart0.txBuf[frame_index-1][12] = 0x00;//Software version high bit
			uart0.txBuf[frame_index-1][13] = 0x01;//Software version low bit 

			uart0.txBuf[frame_index-1][14] = 0xff;//xuhao1
			uart0.txBuf[frame_index-1][15] = 0xff;//xuhao1 
			uart0.txBuf[frame_index-1][16] = crc8(&uart0.txBuf[frame_index-1][0],16);
			uart0.txBuf[frame_index-1][17] = 0x5a;
		}	

	}else{
		/* package not transmit over, continue send package data !*/
		memset(&uart0.txBuf[frame_index-1][0],0,SZ_600);
		uart0.txBuf[frame_index-1][0] = 0x7e;
		uart0.txBuf[frame_index-1][1] = 0x02;
		uart0.txBuf[frame_index-1][2] = 0x0c;
		uart0.txBuf[frame_index-1][3] = 0x06;
		uart0.txBuf[frame_index-1][4] = 0x02;//transfer data packet
		uart0.txBuf[frame_index-1][5] = 0x02;//type:interface board
		uart0.txBuf[frame_index-1][6] = 0x01;//num : 1 ge
		uart0.txBuf[frame_index-1][7] = frame_index;//frameidex 
		uart0.txBuf[frame_index-1][8] = 0x00;//pan hao(zhanweifu)
		uart0.txBuf[frame_index-1][9] = 0x01;//success || Failed
		uart0.txBuf[frame_index-1][10] = package_index >> 8 & 0xff;//xuhao1
		uart0.txBuf[frame_index-1][11] = package_index;//xuhao1 
		memcpy(&uart0.txBuf[frame_index-1][12],&data_packet[package_index-1][0],512);
		uart0.txBuf[frame_index-1][524] = dev_update_calccrc8(&uart0.txBuf[frame_index-1][0],524);
		uart0.txBuf[frame_index-1][525] = 0x5a;
	}	

	usleep(300000);
	for(loops = 0; loops < 3; loops++){	
		/* transmit command data */
		if(sim3u146.updating[frame_index-1] && (0x06 == uart0.txBuf[frame_index-1][3])){
			write(uart0fd, &uart0.txBuf[frame_index-1][0], 
					uart0.txBuf[frame_index-1][1]<<8|uart0.txBuf[frame_index-1][2]+2);	
			usleep(500000);
		}

		/* get reply code :0xa6 */
		memset(uart0.rxBuf, 0, sizeof(uart0.rxBuf));
		nbytes = read(uart0fd, uart0.rxBuf, 100);
		if(nbytes > 0 && (0xa6 == uart0.rxBuf[3])){
			memset(&uart0.txBuf[frame_index-1][0], 0x0, SZ_600);
			sim3u146.updating_status[frame_index-1] = UPDATE_NOW;
			return UPDATE_CONTINUE;
		}
	}

	if(0x06 == uart0.txBuf[frame_index-1][3]){
		sim3u146.updating_status[frame_index-1] = UPDATE_DONE;
		sim3u146.updating_result[frame_index-1] = UPDATE_FAIL;
		sim3u146.total_update_nums -= 1;
	}

	return FRAME_SEND_FAILED;
}


/*******************************************************************************
 *
 *	stage4: Record sim3u146 DFU results ...
 *
 *
 *
 * */

static int sim3u146_enter_reboot_stage_handle(unsigned char *uart_stream)
{
	unsigned char frame_index = 0, updating_status = 0;
	unsigned char loops = 0;

	frame_index     = uart_stream[7];
	updating_status = uart_stream[9];

	sim3u146.updating_status[frame_index-1] = UPDATE_DONE;
	sim3u146.updating_result[frame_index-1] = updating_status;

	sim3u146.total_update_nums -= 1;
	
	printf("frame%d software update done! remaining %d frames\n", sim3u146.updating_frame_index, sim3u146.total_update_nums);

	return 0;
}


static void filling_data_and_send_to_webmaster(void)
{
	int loop_var = 0, update_result = 0x0;

	printf("%s : %d .... sim3u146 update done ...\n", __func__, __LINE__);	
	
	for(loop_var = 0; loop_var < 8; loop_var++){
		if(sim3u146.updating[loop_var] == UPDATE_NOW && sim3u146.updating_result[loop_var] == UPDATE_FAIL){
			update_result = 0x01;
			break;
		}
	}

	memset((void*)&sim3u146, 0x0, sizeof(struct DFU_SIM3U1XX));

	sim3u146.update = UPDATE_DONE;
	sim3u146.update_result = update_result;
}

void update_sim3u146(unsigned char dt, unsigned char *stream, int fd)
{
	int ret = 0, stage_code = 0;

	switch(dt){
	case DATA_FROM_NET:
		startup_stage_step1_download_file( );

		ret = sim3u146_startup_service_routine();
		if(UPDATE_DONE == ret){
			goto update_error;
		}			
	break;

	case DATA_FROM_UART:
		stage_code = stream[4];			
		switch(stage_code){

		case STAGE1_DEVICE_STARTUP:
			ret = sim3u146_enter_bootloader_stage_handle(stream);
			if(FRAME_SEND_FAILED == ret){
				ret = sim3u146_startup_service_routine();
				if(ret == UPDATE_DONE){
					goto update_error;
				}				
			}
		break;

		case STAGE2_WRITING_NFLASH:
			ret = sim3u146_write_nflash_stage_handle(stream);
			if(FRAME_SEND_FAILED == ret){
				ret = sim3u146_startup_service_routine();
				if(ret == UPDATE_DONE){
					goto update_error;
				}	
			}
		break;

		case STAGE3_DEVICE_RESTART:
			sim3u146_enter_reboot_stage_handle(stream);
			ret = sim3u146_startup_service_routine();
			if(UPDATE_DONE == ret){
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


