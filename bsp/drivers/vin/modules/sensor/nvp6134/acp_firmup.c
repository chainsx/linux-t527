/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * A V4L2 driver for nvp6134 cameras and AHD Coax protocol.
 *
 * Copyright (C) 2016 	NEXTCHIP Inc. All rights reserved.
 * Description	: communicate between Decoder and ISP
 * 				  get information(AGC, motion, FPS) of ISP
 * 				  set NRT/RT
 * 				  upgrade Firmware of ISP
 * Authors:
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "../../../utility/vin_log.h"
#include "common.h"
#include "video.h"
/* #include "eq.h" */
#include "acp.h"
#include "acp_firmup.h"

/*******************************************************************************
* define
*******************************************************************************/

/*******************************************************************************
*  global variable
*******************************************************************************/
extern unsigned int nvp6134_iic_addr[4]; /* Slave address of Chip */
extern unsigned char ACP_RX_D0;		 /* Default read start address */
extern unsigned char ch_mode_status[16]; /* current video mode status */
extern unsigned char ch_vfmt_status[16]; /* NTSC/PAL */
extern unsigned int nvp6134_cnt;	 /* count of CHIP */

/*******************************************************************************
*  static variable
*******************************************************************************/
firmware_update_manager s_firmup_manager; /* firmware update manager */

/*******************************************************************************
* internal functions
*******************************************************************************/
static int __acp_change_mode_command(void *p_param, int curvidmode, int vfmt);
static int __acp_transfer_othervideomode_to_ACP(void *p_param, int curvidmode,
						int vfmt);
static int __acp_firmup_start_command(void *p_param, int curvidmode, int vfmt);
static int __acp_make_protocol_bypass(void *p_param);
static int __acp_send_onepacket_to_isp(void *p_param);
static int __acp_end_command(int send_success, void *p_param);

/*******************************************************************************
*  mode transfer table
*******************************************************************************/
#define MAX_TRANSFER_REG_CNT 12
/***********************************************
*  CHD TRANSFER(register information)
************************************************/
static const int s_acp_cfhd_3025p_table[MAX_TRANSFER_REG_CNT][2] = {
	/* addr, data */
	{0xFF, 0x03},		/* bank3 */
	{0x00, 0x5A},		/* duty */
	{0x03, 0x0F}, {0x05, 0x04}, /* adjust Line number(5) */
	{0x0B, 0x10}, {0x0E, 0x02}, {0x0D, 0xA0}, {0x10, 0x55},
	{0x11, 0x01}, {0x12, 0x00}, {0x13, 0x00}, {0x09, 0x08} };
static const int s_acp_chd_60p_table[MAX_TRANSFER_REG_CNT][2] = {
	/* addr, data */
	{0xFF, 0x03}, {0x00, 0x5A}, {0x03, 0x0F}, {0x05, 0x04},
	{0x0B, 0x10}, {0x0E, 0x00}, {0x0D, 0xA0}, {0x10, 0x55},
	{0x11, 0x01}, {0x12, 0x00}, {0x13, 0x00}, {0x09, 0x08} };
static const int s_acp_chd_50p_table[MAX_TRANSFER_REG_CNT][2] = {
	/* addr, data */
	{0xFF, 0x03}, {0x00, 0x5A}, {0x03, 0x0F}, {0x05, 0x04},
	{0x0B, 0x10}, {0x0E, 0x00}, {0x0D, 0x67}, {0x10, 0x55},
	{0x11, 0x01}, {0x12, 0x00}, {0x13, 0x00}, {0x09, 0x08} };
static const int s_acp_chd_30p_table[MAX_TRANSFER_REG_CNT][2] = {
	/* addr, data */
	{0xFF, 0x03}, {0x00, 0x2D}, {0x03, 0x0F}, {0x05, 0x04},
	{0x0B, 0x10}, {0x0E, 0x00}, {0x0D, 0xD3}, {0x10, 0x55},
	{0x11, 0x01}, {0x12, 0x00}, {0x13, 0x00}, {0x09, 0x08} };
static const int s_acp_chd_25p_table[MAX_TRANSFER_REG_CNT][2] = {
	/* addr, data */
	{0xFF, 0x03}, {0x00, 0x2D}, {0x03, 0x0F}, {0x05, 0x04},
	{0x0B, 0x10}, {0x0E, 0x00}, {0x0D, 0xA3}, {0x10, 0x55},
	{0x11, 0x01}, {0x12, 0x00}, {0x13, 0x00}, {0x09, 0x08} };

/***********************************************
*
*  CHD TRANSFER(register information)
*
************************************************/
static const int s_acp_tfhd_3025p_table[MAX_TRANSFER_REG_CNT][2] = {
	/* addr, data */
	{0xFF, 0x03}, {0x00, 0x5A}, {0x03, 0x09}, {0x05, 0x03}, /* adjust Line */
								/* number(4) */
	{0x0B, 0x10}, {0x0E, 0x02}, {0x0D, 0x90}, {0x10, 0x55}, {0x11, 0x01},
	{0x12, 0x00}, {0x13, 0x00}, {0x09, 0x08} };
static const int s_acp_thd_60p_table[MAX_TRANSFER_REG_CNT][2] = {
	/* addr, data */
	{0xFF, 0x03}, {0x00, 0x5A}, {0x03, 0x09}, {0x05, 0x03},
	{0x0B, 0x10}, {0x0E, 0x01}, {0x0D, 0x34}, {0x10, 0x55},
	{0x11, 0x01}, {0x12, 0x00}, {0x13, 0x00}, {0x09, 0x08} };
static const int s_acp_thd_50p_table[MAX_TRANSFER_REG_CNT][2] = {
	/* addr, data */
	{0xFF, 0x03}, {0x00, 0x5A}, {0x03, 0x09}, {0x05, 0x03},
	{0x0B, 0x10}, {0x0E, 0x00}, {0x0D, 0xEC}, {0x10, 0x55},
	{0x11, 0x01}, {0x12, 0x00}, {0x13, 0x00}, {0x09, 0x08} };
static const int s_acp_thd_3025p_Btype_table[MAX_TRANSFER_REG_CNT][2] = {
	/* addr, data */
	{0xFF, 0x03}, {0x00, 0x2C}, {0x03, 0x09}, {0x05, 0x03},
	{0x0B, 0x10}, {0x0E, 0x01}, {0x0D, 0x20}, {0x10, 0x55},
	{0x11, 0x01}, {0x12, 0x00}, {0x13, 0x00}, {0x09, 0x08} };
static const int s_acp_thd_3025p_Atype_table[MAX_TRANSFER_REG_CNT][2] = {
	/* addr, data */
	{0xFF, 0x03}, {0x00, 0x5A}, {0x03, 0x09}, {0x05, 0x03},
	{0x0B, 0x10}, {0x0E, 0x03}, {0x0D, 0x40}, {0x10, 0x55},
	{0x11, 0x01}, {0x12, 0x00}, {0x13, 0x00}, {0x09, 0x08} };

/*******************************************************************************
*
*
*
*	Internal functions
*
*
*
********************************************************************************/
/*******************************************************************************
*  description    : make protocol(FILL)->by pass mode
*  argurments     : p_param( file information )
*  return value   : N/A
*  modify         :
*  warning        :
 *******************************************************************************/
int __acp_make_protocol_bypass(void *p_param)
{
	int i;
	int ch = 0;
	unsigned int low = 0x00;
	unsigned int mid = 0x00;
	unsigned int high = 0x00;
	int byteNumOfPacket = 0;
	/* unsigned int onepacketaddr = 0; */
	unsigned int readsize = 0;
	/* unsigned int packetsum = 0; */
	/* unsigned int filechecksum = 0; */
	/* unsigned int filesize = 0; */
	FIRMWARE_UP_FILE_INFO *pstFileInfo = (FIRMWARE_UP_FILE_INFO *)p_param;

	/* file information */
	ch = pstFileInfo->channel;
	readsize = pstFileInfo->readsize;

	/* fill packet(139bytes), end packet is filled with 0xff */
	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0xff, 0x0c + (ch % 4));
	for (i = 0; i < ONE_PACKET_MAX_SIZE; i++) {
		if (byteNumOfPacket < readsize) {
			gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0x00 + i,
				       pstFileInfo->onepacketbuf[i]);
			byteNumOfPacket++;
		} else if (byteNumOfPacket >=
			   readsize) {
			/* end packet : fill 0xff */
			gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0x00 + i,
				       0xff);
			byteNumOfPacket++;
		}

		if (i == 0)
			low = pstFileInfo->onepacketbuf[i];
		else if (i == 1)
			mid = pstFileInfo->onepacketbuf[i];
		else if (i == 2)
			high = pstFileInfo->onepacketbuf[i];
	}

	/* offset */
	pstFileInfo->currentFileOffset =
	    (unsigned int)((high << 16) & (0xFF0000)) |
	    (unsigned int)((mid << 8) & (0xFF00)) | (unsigned char)(low);

/*
	/* debug message
	for (i = 0; i < ONE_PACKET_MAX_SIZE; i++) {
		if ((i != 0) && ((i%16) == 0))
			printk("\n");
		printk("%02x ", pstFileInfo->onepacketbuf[i]);
	}
	printk("\n");
	printk(">>>>> DRV[%s:%d] CH:%d, offset:0x%x[%d], low:0x%x, mid:0x%x, hight:0x%x\n", \
			__func__, __LINE__, ch, pstFileInfo->currentFileOffset, pstFileInfo->currentFileOffset, low, mid, high);
*/

	return 0;
}

/*******************************************************************************
*  description    : send data and verification(SYNC+ACK)
*  argurments     : p_param( file information )
*  return value   : 0(send:success), -1(send:failed)
*  modify         :
*  warning        :
 *******************************************************************************/
int __acp_send_onepacket_to_isp(void *p_param)
{
	int i = 0;
	int ch = 0;
	int ret = -1;
	int retrycnt = 0;
	int timeoutcnt = 50;
	/* int onepacketvalue = 0x80; */
	unsigned int onepacketaddr = 0;
	unsigned int receive_addr = 0;
	FIRMWARE_UP_FILE_INFO *pstFileInfo = (FIRMWARE_UP_FILE_INFO *)p_param;

	ch = pstFileInfo->channel;
	onepacketaddr = pstFileInfo->currentFileOffset;

	/* change mode to use Big data */
	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0xFF, 0x03 + ((ch % 4) / 2));
	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0x0b + ((ch % 2) * 0x80),
		       0x30);
	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0x05 + ((ch % 2) * 0x80),
		       0x8A);

	/* TX start */
	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0xFF, 0x03 + ((ch % 4) / 2));
	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0x09 + ((ch % 2) * 0x80),
		       0x08); /* trigger on */

	/* send and verification(10 counts) */
	for (i = 0; i < timeoutcnt; i++) {
		/* If the information received is 0x02(table:F/W update), It was
		 * changed Camera update mode.
		 * 0x50(0x55), 0x51, 0x52, 0x53, 0x54, 0x55, 0x56(0:camera
		 * information, 1:Firmware version, 2:f/W start, 3:error,
		 * 0x57(2:F/W update table) */
		if (gpio_i2c_read(nvp6134_iic_addr[ch / 4],
				  0x57 + ((ch % 2) * 0x80)) == 0x02) {
			/* check ISP status */
			if (gpio_i2c_read(nvp6134_iic_addr[ch / 4],
					  0x56 + ((ch % 2) * 0x80)) == 0x03) {
				ret = -1;
				printk(">>>>> DRV[%s:%d] CH:%d, Failed, error "
				       "status, code=3..................\n",
				       __func__, __LINE__, ch);
				break;
			}

			/* check offset */
			receive_addr =
			    ((gpio_i2c_read(nvp6134_iic_addr[ch / 4],
					    0x53 + ((ch % 2) * 0x80))
			      << 16) +
			     (gpio_i2c_read(nvp6134_iic_addr[ch / 4],
					    0x54 + ((ch % 2) * 0x80))
			      << 8) +
			     gpio_i2c_read(nvp6134_iic_addr[ch / 4],
					   0x55 + ((ch % 2) * 0x80)));
			if (onepacketaddr == receive_addr) {
				printk(">>>>> DRV[%s:%d] CH:%d, Success : "
				       "Camera Firmware write "
				       "addr=[R:0x%06X==T:0x%06X]/0x%06X, "
				       "packet num=%3d/%3d, retrycnt:%d\n",
				       __func__, __LINE__, ch, onepacketaddr,
				       receive_addr, pstFileInfo->filesize,
				       pstFileInfo->currentpacketnum + 1,
				       pstFileInfo->filepacketnum, retrycnt);
				gpio_i2c_write(nvp6134_iic_addr[ch / 4],
					       0x09 + ((ch % 2) * 0x80),
					       0x10); /* rest */
				gpio_i2c_write(nvp6134_iic_addr[ch / 4],
					       0x09 + ((ch % 2) * 0x80),
					       0x00); /* trigger off */
				ret = 0;
				break;
			} else {
				/* printk(">>>>> DRV[%s:%d] CH:%d, Failed  :
				   Camera Firmware write addr=0x%06X/0x%06X,
				   packet num=%3d/%3d\n", \
						__func__, __LINE__, ch,
				   onepacketaddr, pstFileInfo->filesize,
				   pstFileInfo->currentpacketnum+1, \
						pstFileInfo->filepacketnum ); */
				msleep(10);
				retrycnt++;
			}
		} else {
			/* printk(">>>>> DRV[%s:%d] CH:%d, Error Write : Camera
			   Firmware = [0x%x]\n", __func__, __LINE__, \
					ch,
			   gpio_i2c_read(nvp6134_iic_addr[ch/4],
			   0x56+((ch%2)*0x80)));  */
			msleep(10);
		}
	}

	/* time out */
	if (i == timeoutcnt) {
#if 1
		/* debug message */
		for (i = 0; i < ONE_PACKET_MAX_SIZE; i++) {
			if ((i != 0) && ((i % 16) == 0))
				printk("\n");
			printk("%02x ", pstFileInfo->onepacketbuf[i]);
		}
		printk("\n");
#endif

		printk(">>>>> DRV[%s:%d] CH:%d, write timeout:%d, offset of "
		       "file:0x%06X\n",
		       __func__, __LINE__, ch, timeoutcnt, onepacketaddr);
		ret = -1;
	}

	return ret;
}

/*******************************************************************************
*  Description    : transfer other video mode to ACP
*  Argurments     : curvidmode(current video mode), vfmt(NTSC/PAL)
*  Return value   : void
*  Modify         :
*  warning        :
 *******************************************************************************/
int __acp_transfer_othervideomode_to_ACP(void *p_param, int curvidmode,
					 int vfmt)
{
	int i = 0;
	int ch = 0;
	int ret = 0;
	unsigned char addr = 0x00;
	unsigned char data = 0x00;
	unsigned char s_acp_transfer_table[MAX_TRANSFER_REG_CNT][2];
	FIRMWARE_UP_FILE_INFO *pstFileInfo = (FIRMWARE_UP_FILE_INFO *)p_param;
	ch = pstFileInfo->channel;

	/* coax RX sync sel */
	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0xFF, 0x05 + ch % 4);
	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0x7C, 0x01);

	/* setting ACP transfer register */
	for (i = 0; i < MAX_TRANSFER_REG_CNT; i++) {
		addr = s_acp_transfer_table[i][0];
		data = s_acp_transfer_table[i][1];
		if (i == 0) {
			gpio_i2c_write(nvp6134_iic_addr[ch / 4], addr,
				       data + ((ch % 4) / 2));
			printk(">>>>> DRV[%s:%d] CH:%d, %2d(0x%02x, 0x%02x)\n",
			       __func__, __LINE__, ch, i, addr, data);
		} else {
			gpio_i2c_write(nvp6134_iic_addr[ch / 4],
				       addr + ((ch % 2) * 0x80), data);
			printk(">>>>> DRV[%s:%d] CH:%d, %2d(0x%02x, 0x%02x)\n",
			       __func__, __LINE__, ch, i, addr, data);
		}
	}
	msleep(100 * 15);

	return ret;
}

/*******************************************************************************
*  Description    : change video mode
*  Argurments     : p_param( file information ), curvidmode(Current video mode)
*  					vfmt(NTSC/PAL)
*  Return value   : N/A
*  Modify         :
*  warning        :
 *******************************************************************************/
int __acp_change_mode_command(void *p_param, int curvidmode, int vfmt)
{
	/* int i = 0; */
	int ch = 0;
	int ret = 0;
	/* int TimeOutCnt = 1000; */
	/* unsigned char retval = 0x00; */
	FIRMWARE_UP_FILE_INFO *pstFileInfo = (FIRMWARE_UP_FILE_INFO *)p_param;
	ch = pstFileInfo->channel;

	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0xFF, 0x03 + ((ch % 4) / 2));

	/* Send -> Change camera update mode-1080p@25p */
	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0x09 + ((ch % 2) * 0x80),
		       0x00); /* trigger off */
	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0x0b + ((ch % 2) * 0x80),
		       0x10);

	if (curvidmode == NVP6134_VI_HDEX ||
	    curvidmode == NVP6134_VI_720P_5060 ||
	    curvidmode == NVP6134_VI_1080P_2530 ||
	    curvidmode == NVP6134_VI_3M_NRT || curvidmode == NVP6134_VI_3M ||
	    curvidmode == NVP6134_VI_4M_NRT || curvidmode == NVP6134_VI_4M) {
		gpio_i2c_write(nvp6134_iic_addr[ch / 4],
			       0x10 + ((ch % 2) * 0x80), 0x60); /* Register */
								/* Write Control */
								/* - 17th line */
		gpio_i2c_write(nvp6134_iic_addr[ch / 4],
			       0x11 + ((ch % 2) * 0x80), 0xB0); /* table(Mode */
								/* Change */
								/* Command) */
								/* - 18th line */
		gpio_i2c_write(nvp6134_iic_addr[ch / 4],
			       0x12 + ((ch % 2) * 0x80), 0x02); /* Flash Update */
								/* Mode(big */
								/* data) */
								/* - 19th line */
		gpio_i2c_write(
		    nvp6134_iic_addr[ch / 4], 0x13 + ((ch % 2) * 0x80),
		    0x00); /* Init Value(FW Information Check Mode) - 20th line */

		gpio_i2c_write(nvp6134_iic_addr[ch / 4],
			       0x09 + ((ch % 2) * 0x80), 0x08); /* trigger on */
		msleep(200);
		gpio_i2c_write(nvp6134_iic_addr[ch / 4],
			       0x09 + ((ch % 2) * 0x80), 0x10); /* reset */
		gpio_i2c_write(nvp6134_iic_addr[ch / 4],
			       0x09 + ((ch % 2) * 0x80), 0x00); /* trigger Off */
		printk(">>>>> DRV[%s:%d] CH:%d, ACP firmware update : "
		       "CHANGE!!!!!!!!- AHD\n",
		       __func__, __LINE__, ch);
	} else /* CHD, THD */ {
		/* transfer CHD, THD mode to ACP mode */
		__acp_transfer_othervideomode_to_ACP(pstFileInfo, curvidmode,
						     vfmt);

		/* change to AFHD@25p */
		gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0xFF,
			       0x03 + ((ch % 4) / 2));
		gpio_i2c_write(nvp6134_iic_addr[ch / 4],
			       0x10 + ((ch % 2) * 0x80), 0x55); /* 0x55(header) */
								/* - 16th line */
		gpio_i2c_write(nvp6134_iic_addr[ch / 4],
			       0x11 + ((ch % 2) * 0x80), 0x60); /* Register */
								/* Write Control */
								/* - 17th line */
		gpio_i2c_write(nvp6134_iic_addr[ch / 4],
			       0x12 + ((ch % 2) * 0x80), 0xB0); /* table(Mode */
								/* Change */
								/* Command) */
								/* - 18th line */
		gpio_i2c_write(nvp6134_iic_addr[ch / 4],
			       0x13 + ((ch % 2) * 0x80), 0x02); /* Flash Update */
								/* Mode */
								/* - 19th line */
		gpio_i2c_write(
		    nvp6134_iic_addr[ch / 4], 0x14 + ((ch % 2) * 0x80),
		    0x00); /* Init Value(FW Information Check Mode) - 20th line */

		gpio_i2c_write(nvp6134_iic_addr[ch / 4],
			       0x09 + ((ch % 2) * 0x80), 0x08); /* trigger on */
		msleep(1000);
		gpio_i2c_write(nvp6134_iic_addr[ch / 4],
			       0x09 + ((ch % 2) * 0x80), 0x10); /* reset */
		gpio_i2c_write(nvp6134_iic_addr[ch / 4],
			       0x09 + ((ch % 2) * 0x80), 0x00); /* trigger Off */
		printk(">>>>> DRV[%s:%d] CH:%d, CHD, THD firmware update : "
		       "CHANGE!!!! - CHD, THD\n",
		       __func__, __LINE__, ch);
	}

	return ret;
}

/*******************************************************************************
*  Description    : start firmware update
*  Argurments     : p_param( file information ), curvidmode(Current video mode)
*  					vfmt(NTSC/PAL)
*  Return value   : N/A
*  Modify         :
*  warning        :
 *******************************************************************************/
int __acp_firmup_start_command(void *p_param, int curvidmode, int vfmt)
{
	int ch = 0;
	int ret = 0;
	FIRMWARE_UP_FILE_INFO *pstFileInfo = (FIRMWARE_UP_FILE_INFO *)p_param;
	ch = pstFileInfo->channel;

	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0xFF, 0x03 + ((ch % 4) / 2));
	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0x03 + ((ch % 2) * 0x80),
		       0x0E); /* line */
	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0x10 + ((ch % 2) * 0x80),
		       0x60); /* Register Write Control */
			      /* - 17th line */
	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0x11 + ((ch % 2) * 0x80),
		       0xB0); /* table(Mode Change Command) */
			      /* - 18th line */
	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0x12 + ((ch % 2) * 0x80),
		       0x02); /* Flash Update Mode(big data) */
			      /* - 19th line */
	gpio_i2c_write(
	    nvp6134_iic_addr[ch / 4], 0x13 + ((ch % 2) * 0x80),
	    0x41); /* Start firmware update                 - 20th line */

	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0x09 + ((ch % 2) * 0x80),
		       0x08); /* trigger on */
	msleep(200);
	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0x09 + ((ch % 2) * 0x80),
		       0x10); /* reset */
	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0x09 + ((ch % 2) * 0x80),
		       0x00); /* trigger Off */
	printk(">>>>> DRV[%s:%d] CH:%d, ACP firmware update : START "
	       "COMMAND!!!!!!!!\n",
	       __func__, __LINE__, ch);
	printk(">>>>> DRV[%s:%d] CH:%d, ACP firmware update : START "
	       "COMMAND!!!!!!!!\n",
	       __func__, __LINE__, ch);
	printk(">>>>> DRV[%s:%d] CH:%d, ACP firmware update : START "
	       "COMMAND!!!!!!!!\n",
	       __func__, __LINE__, ch);
	printk(">>>>> DRV[%s:%d] CH:%d, ACP firmware update : START "
	       "COMMAND!!!!!!!!\n",
	       __func__, __LINE__, ch);
	printk(">>>>> DRV[%s:%d] CH:%d, ACP firmware update : START "
	       "COMMAND!!!!!!!!\n",
	       __func__, __LINE__, ch);

	return ret;
}

/*******************************************************************************
*  description    : check F/W start(ready)
*  argurments     : ch(channel)
*  return value   : 0(OK ), -1(failed )
*  modify         :
*  warning        :
 *******************************************************************************/
int __acp_dvr_check_firmupstart(int ch)
{
	int ret = -1;
	unsigned char retval = 0x00;

	/* If the information received is 0x02(table:F/W update), It was changed
	 * Camera update mode.
	 * 0x50(0x55), 0x51, 0x52, 0x53, 0x54, 0x55, 0x56(0:camera information,
	 * 1:Firmware version, 2:update status, 0x57(2:F/W update table) */
	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0xFF, 0x03 + ((ch % 4) / 2));
	retval =
	    gpio_i2c_read(nvp6134_iic_addr[ch / 4], 0x56 + ((ch % 2) * 0x80));
	if (gpio_i2c_read(nvp6134_iic_addr[ch / 4], 0x57 + ((ch % 2) * 0x80)) ==
	    0x02) {
		/* get status, If the status is 0x00(Camera information),
		 * 0x01(Firmware version), 0x02(Firmware start[possible]) */
		retval = gpio_i2c_read(nvp6134_iic_addr[ch / 4],
				       0x56 + ((ch % 2) * 0x80));
		if (retval == 0x02) {
			printk(
			    ">>>>> DRV[%s:%d] CH:%d, Receive, OK F/W start\n",
			    __func__, __LINE__, ch);
			printk(
			    ">>>>> DRV[%s:%d] CH:%d, Receive, OK F/W start\n",
			    __func__, __LINE__, ch);
			printk(
			    ">>>>> DRV[%s:%d] CH:%d, Receive, OK F/W start\n",
			    __func__, __LINE__, ch);
			printk(
			    ">>>>> DRV[%s:%d] CH:%d, Receive, OK F/W start\n",
			    __func__, __LINE__, ch);
			printk(
			    ">>>>> DRV[%s:%d] CH:%d, Receive, OK F/W start\n",
			    __func__, __LINE__, ch);
			ret = 0;
		}
	}

	return ret;
}

/*******************************************************************************
*  Description    : END command : Success or Fail
*  Argurments     : send_success(0:success, -1:failed), p_param( file
*information )
*  Return value   : N/A
*  Modify         :
*  warning        :
 *******************************************************************************/
int __acp_end_command(int send_success, void *p_param)
{
	int ch = 0;
	int ret = 0;
	FIRMWARE_UP_FILE_INFO *pstFileInfo = (FIRMWARE_UP_FILE_INFO *)p_param;
	ch = pstFileInfo->channel;

	/* adjust line */
	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0xff, 0x03 + ((ch % 4) / 2));
	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0x0b + ((ch % 2) * 0x80),
		       0x10);
	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0x05 + ((ch % 2) * 0x80),
		       0x03); /* 3 line number */
	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0x0A + ((ch % 2) * 0x80),
		       0x03); /* 3 line number */

	/* END command */
	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0x10 + ((ch % 2) * 0x80),
		       0x60);
	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0x11 + ((ch % 2) * 0x80),
		       0xb0);
	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0x12 + ((ch % 2) * 0x80),
		       0x02);
	if (send_success == -1) {
		gpio_i2c_write(nvp6134_iic_addr[ch / 4],
			       0x13 + ((ch % 2) * 0x80), 0xE0 /* 0xC0 */);
		printk(">>>>> DRV[%s:%d] CH:%d, Camera UPDATE error signal. "
		       "send Abnormal ending!\n",
		       __func__, __LINE__, ch);
		printk(">>>>> DRV[%s:%d] CH:%d, Camera UPDATE error signal. "
		       "send Abnormal ending!\n",
		       __func__, __LINE__, ch);
		printk(">>>>> DRV[%s:%d] CH:%d, Camera UPDATE error signal. "
		       "send Abnormal ending!\n",
		       __func__, __LINE__, ch);
		printk(">>>>> DRV[%s:%d] CH:%d, Camera UPDATE error signal. "
		       "send Abnormal ending!\n",
		       __func__, __LINE__, ch);
		printk(">>>>> DRV[%s:%d] CH:%d, Camera UPDATE error signal. "
		       "send Abnormal ending!\n",
		       __func__, __LINE__, ch);
	} else {
		printk(">>>>> DVR[%s:%d] CH:%d, Camera UPDATE ending signal. "
		       "wait please!\n",
		       __func__, __LINE__, ch);
		printk(">>>>> DVR[%s:%d] CH:%d, Camera UPDATE ending signal. "
		       "wait please!\n",
		       __func__, __LINE__, ch);
		printk(">>>>> DVR[%s:%d] CH:%d, Camera UPDATE ending signal. "
		       "wait please!\n",
		       __func__, __LINE__, ch);
		printk(">>>>> DVR[%s:%d] CH:%d, Camera UPDATE ending signal. "
		       "wait please!\n",
		       __func__, __LINE__, ch);
		printk(">>>>> DVR[%s:%d] CH:%d, Camera UPDATE ending signal. "
		       "wait please!\n",
		       __func__, __LINE__, ch);
		gpio_i2c_write(nvp6134_iic_addr[ch / 4],
			       0x13 + ((ch % 2) * 0x80), 0x80 /* 0x60 */);
	}

	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0x09 + ((ch % 2) * 0x80),
		       0x08);
	msleep(200);
	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0x09 + ((ch % 2) * 0x80),
		       0x00);

	if (send_success == 0) {
		printk(">>>>> DVR[%s:%d] CH:%d, Camera UPDATE mode finish!\n",
		       __func__, __LINE__, ch);
		printk(">>>>> DVR[%s:%d] CH:%d, Camera UPDATE mode finish!\n",
		       __func__, __LINE__, ch);
		printk(">>>>> DVR[%s:%d] CH:%d, Camera UPDATE mode finish!\n",
		       __func__, __LINE__, ch);
		printk(">>>>> DVR[%s:%d] CH:%d, Camera UPDATE mode finish!\n",
		       __func__, __LINE__, ch);
		printk(">>>>> DVR[%s:%d] CH:%d, Camera UPDATE mode finish!\n",
		       __func__, __LINE__, ch);
	}

	return ret;
}

/*******************************************************************************
*
*
*	Extern functions
*
*
********************************************************************************/
/*******************************************************************************
*  description    : set firmware update status
*  argurments     : ch(channel)
*  return value   : N/A
*  modify         :
*  warning        :
 *******************************************************************************/
void acp_dvr_set_firmupstatus(int ch, int flag)
{
	s_firmup_manager.firmware_status[ch] = flag;
}

/*******************************************************************************
*  description    : get firmware update status
*  argurments     : ch(channel)
*  return value   : current F/W upgade status
*  modify         :
*  warning        :
 *******************************************************************************/
int acp_dvr_get_firmupstatus(int ch)
{
	return s_firmup_manager.firmware_status[ch];
}

/*******************************************************************************
*  description    : set current video mode, video format
*  argurments     : ch(channel), curvidmode(current video mode), vfmt(video
*format)
*  return value   : N/A
*  modify         :
*  warning        :
 *******************************************************************************/
int acp_dvr_set_curvideomode(int ch, int curvidmode, int vfmt)
{
	s_firmup_manager.curvidmode[ch] = curvidmode;
	s_firmup_manager.curvideofmt[ch] = vfmt;

	return 0;
}

/*******************************************************************************
*  description    : get current video mode, video format
*  argurments     : ch(channel), curvidmode(current video mode), vfmt(video
*format)
*  return value   : N/A
*  modify         :
*  warning        :
 *******************************************************************************/
int acp_dvr_get_curvideomode(int ch, int *pcurvidmode, int *pvfmt)
{
	*pcurvidmode = s_firmup_manager.curvidmode[ch];
	*pvfmt = s_firmup_manager.curvideofmt[ch];

	return 0;
}

/*******************************************************************************
*  description    : Check whether firmware update can update or not
*  argurments     : p_param( file information)
*  return value   : 0(possible to update), -1(impossible to update)
*  modify         :
*  warning        :
 *******************************************************************************/
int acp_dvr_ispossible_update(void *p_param)
{
	int ch = 0;
	int ret = -1;
	int timeout = 20;
	unsigned char val = 0x00;
	int curvideomode = 0;
	int curvideofmt = 0;
	FIRMWARE_UP_FILE_INFO *pstFileInfo = (FIRMWARE_UP_FILE_INFO *)p_param;
	ch = pstFileInfo->channel;

	/* start A-CP firmware update(management) */
	acp_dvr_set_firmupstatus(ch, ACP_FIRMWARE_UP_START);
	acp_dvr_set_curvideomode(ch, ch_mode_status[ch], ch_vfmt_status[ch]);

	/* set register */
	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0xFF, 0x03 + ((ch % 4) / 2));
	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0x50 + ((ch % 2) * 0x80),
		       0x05);
	msleep(1000);

	while (timeout != 0) {
		/* If the header is (0x50=>0x55) and chip information is
		 * (0x51=>0x3X, 0x4X, 0x5X ), it can update firmware */
		if (gpio_i2c_read(nvp6134_iic_addr[ch / 4],
				  0x50 + ((ch % 2) * 0x80)) == 0x55) {
			val = gpio_i2c_read(nvp6134_iic_addr[ch / 4],
						 0x51 + ((ch % 2) * 0x80));
			if (val >= 0x06) {
				printk(">>>>> DRV[%s:%d] CH:%d, this camera "
				       "can update, please, wait! = 0x%x\n",
				       __func__, __LINE__, ch,
				       gpio_i2c_read(nvp6134_iic_addr[ch / 4],
						     0x51 + ((ch % 2) * 0x80)));
			} else {
				printk(">>>>> DRV[%s:%d] CH:%d, this camera "
				       "can update, but This cam don't support "
				       "FW[0x%X]\n",
				       __func__, __LINE__, ch, val);
				return -1;
			}
			break;
		} else {
			mdelay(40);
			/* printk(">>>>> DRV[%s:%d] check ACP_STATUS_MODE\n", */
			/* __func__, __LINE__ ); */
		}
		timeout--;
	}

	if (timeout == 0) {
		/* printk(">>>>> DRV[%s:%d] failed It is impossible to update */
		/* camera!\n", __func__, __LINE__ ); */
		return -1;
	}

	/* change video mode(TX:1080p@25p and Black RX:1080p@25p)[command] */
	acp_dvr_get_curvideomode(ch, &curvideomode, &curvideofmt);
	ret = __acp_change_mode_command(pstFileInfo, curvideomode, curvideofmt);
	if (ret == -1) {
		printk(">>>>> DRV[%s:%d] CH:%d, failed to change video "
		       "mdoe[1080p@25p]\n",
		       __func__, __LINE__, ch);
		return -1;
	}

	return ret;
}

/*******************************************************************************
*  description    : check ips status
*  argurments     : p_param( file information)
*  return value   : 0(OK ), -1(failed )
*  modify         :
*  warning        :
 *******************************************************************************/
int acp_dvr_check_ispstatus(void *p_param)
{
	int ret = -1;
	int ch = 0;
	int timeout_cnt = 0;
	unsigned char retval = 0x00;
	unsigned char retval2 = 0x00;
	FIRMWARE_UP_FILE_INFO *pstFileInfo = (FIRMWARE_UP_FILE_INFO *)p_param;
	ch = pstFileInfo->channel;

	/* If the information received is 0x02(table:F/W update), It was changed
	 * Camera update mode.
	 * 0x50(0x55), 0x51, 0x52, 0x53, 0x54, 0x55, 0x56(0:camera information,
	 * 1:Firmware version, 2:F/W update status, 0x57(2:F/W update table) */
	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0xFF, 0x03 + ((ch % 4) / 2));
	/* test */
	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0x62 + ((ch % 2) * 0x80),
		       0x06); /* line */
	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0x63 + ((ch % 2) * 0x80),
		       0x01); /* common on */
	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0x64 + ((ch % 2) * 0x80),
		       0x00); /* Delay count */
	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0x66 + ((ch % 2) * 0x80),
		       0x80);
	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0x67 + ((ch % 2) * 0x80),
		       0x01); /* RX(1:interrupt enable), (0:interrupt disable) */
	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0x68 + ((ch % 2) * 0x80),
		       0x70); /* [7:4] read number */
	gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0x69 + ((ch % 2) * 0x80),
		       0x80);

	timeout_cnt = 0;
	while (timeout_cnt++ < 20) {
		retval = gpio_i2c_read(nvp6134_iic_addr[ch / 4],
				       0x56 + ((ch % 2) * 0x80));
		retval2 = gpio_i2c_read(nvp6134_iic_addr[ch / 4],
					0x57 + ((ch % 2) * 0x80));
		if (retval2 == 0x02) {
			/* get status, If the status is 0x00(Camera
			 * information), 0x01(Firmware version */
			/* retval = gpio_i2c_read(nvp6134_iic_addr[ch/4], */
			/* 0x56+((ch%2)*0x80)); */
			if (retval == 0x00) /*|| retval == 0x01 ) */ {
				printk(">>>>> DRV[%s:%d] CH:%d, Complete "
				       "Changing F/W update\n",
				       __func__, __LINE__, ch);
				printk(">>>>> DRV[%s:%d] CH:%d, Complete "
				       "Changing F/W update\n",
				       __func__, __LINE__, ch);
				printk(">>>>> DRV[%s:%d] CH:%d, Complete "
				       "Changing F/W update\n",
				       __func__, __LINE__, ch);
				printk(">>>>> DRV[%s:%d] CH:%d, Complete "
				       "Changing F/W update\n",
				       __func__, __LINE__, ch);
				printk(">>>>> DRV[%s:%d] CH:%d, Complete "
				       "Changing F/W update\n",
				       __func__, __LINE__, ch);
				ret = 0;
				break;
			}
		} else {
			printk(">>>>> DRV[%s:%d] stage1(CH:%d), retry : "
			       "Receive ISP status[F/W table], cnt:%d\n",
			       __func__, __LINE__, ch, timeout_cnt);
			if (timeout_cnt == 20) {
				ret = -1;
				break;
			}
			gpio_i2c_write(nvp6134_iic_addr[ch / 4], 0xFF,
				       0x03 + ((ch % 4) / 2));
			gpio_i2c_write(nvp6134_iic_addr[ch / 4],
				       0x09 + ((ch % 2) * 0x80), 0x08);
			msleep(500);
			gpio_i2c_write(nvp6134_iic_addr[ch / 4],
				       0x09 + (((ch) % 2) * 0x80), 0x10);
			gpio_i2c_write(nvp6134_iic_addr[ch / 4],
				       0x09 + (((ch) % 2) * 0x80), 0x00);
			msleep(300);
			gpio_i2c_write(nvp6134_iic_addr[ch / 4],
				       0x3A + (((ch) % 2) * 0x80), 0x01);
			msleep(100);
			gpio_i2c_write(nvp6134_iic_addr[ch / 4],
				       0x3A + (((ch) % 2) * 0x80), 0x00);
			msleep(100);
			printk("test here!!!  set again\n");

			msleep(1000);
		}
	}

	return ret;
}

/*******************************************************************************
*  description    : start F/W
*  argurments     : p_param( file information)
*  return value   : 0(OK ), -1(failed )
*  modify         :
*  warning        :
 *******************************************************************************/
int acp_dvr_start_command(void *p_param)
{
	int ret = -1;
	int ch = 0;
	int s_cnt = 0;
	int curvideomode = 0;
	int curvideofmt = 0;
	/* unsigned char retval = 0x00; */
	FIRMWARE_UP_FILE_INFO *pstFileInfo = (FIRMWARE_UP_FILE_INFO *)p_param;
	ch = pstFileInfo->channel;

	/* start F/W  */
	acp_dvr_get_curvideomode(ch, &curvideomode, &curvideofmt);
	__acp_firmup_start_command(pstFileInfo, curvideomode, curvideofmt);

	/* CHECK : is it possible to start F/W */
	while (s_cnt++ < 15) {
		ret = __acp_dvr_check_firmupstart(ch);
		if (ret == 0) {
			printk(">>>>> DRV[%s:%d] CH:%d, OK  : Receive Firmup "
			       "Start[F/W table], cnt:%d\n",
			       __func__, __LINE__, ch, s_cnt);
			break;
		} else {
			printk(">>>>> DRV[%s:%d] CH:%d, Failed : Receive "
			       "Firmup Start[F/W table], cnt:%d\n",
			       __func__, __LINE__, ch, s_cnt);
		}
		msleep(1000);
	}

	return ret;
}

/*******************************************************************************
*  description    : firmware update( fill, send, verification )
*  argurments     : p_param( file information)
*  return value   : 0(send/receive OK ), -1(send/receive failed )
*  modify         :
*  warning        :
 *******************************************************************************/
int acp_dvr_firmware_update(void *p_param)
{
	int ret = -1;
	int ch = 0;
	FIRMWARE_UP_FILE_INFO *pstFileInfo = (FIRMWARE_UP_FILE_INFO *)p_param;
	ch = pstFileInfo->channel;

	/* make protocol(FILL) */
	__acp_make_protocol_bypass(pstFileInfo);

	/* send and verification */
	ret = __acp_send_onepacket_to_isp(pstFileInfo);

	return ret;
}

/*******************************************************************************
*  Description    : END command : Success or Fail
*  Argurments     : send_success(0:success, -1:failed), p_param( file
*information)
*  Return value   : void
*  Modify         :
*  warning        :
 *******************************************************************************/
int acp_dvr_end_command(int send_success, void *p_param)
{
	int ret = 0;
	int ch = 0;
	FIRMWARE_UP_FILE_INFO *pstFileInfo = (FIRMWARE_UP_FILE_INFO *)p_param;
	ch = pstFileInfo->channel;

	__acp_end_command(send_success, pstFileInfo);

	/* STOP A-CP firmware update */
	acp_dvr_set_firmupstatus(ch, ACP_FIRMWARE_UP_STOP);

	return ret;
}

/*******************************************************************************
* End of file
*******************************************************************************/
