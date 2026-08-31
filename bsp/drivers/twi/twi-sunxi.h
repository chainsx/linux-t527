/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * SUNXI TWI Controller Register Definition
 *
 */


#ifndef _SUNXI_TWI_H_
#define _SUNXI_TWI_H_

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/clkdev.h>
#include <linux/clk-provider.h>
#include <linux/reset.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/gpio.h>
#include <linux/pinctrl/consumer.h>
#include <linux/pm_runtime.h>
#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>
#include <linux/dmapool.h>
#include <asm/uaccess.h>
#include <linux/time.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/regulator/consumer.h>

#ifdef CONFIG_AW_AMP_SYS_RSC_MANAGER
#include <linux/sunxi_amp_rsc.h>
#endif

/* TWI Register Offset */
/* 31:8bit reserved,7-1bit for slave addr,0 bit for GCE */
#define TWI_ADDR		(0x00)
/* 31:8bit reserved,7-0bit for second addr in 10bit addr */
#define TWI_XADDR		(0x04)
/* 31:8bit reserved, 7-0bit send or receive data byte */
#define TWI_DATA		(0x08)
/* INT_EN,BUS_EN,M_STA,INT_FLAG,A_ACK */
#define TWI_CNTR		(0x0C)
/* 28 interrupt types + 0xF8 normal type = 29  */
#define TWI_STAT		(0x10)
/* 31:7bit reserved,6-3bit,CLK_M,2-0bit CLK_N */
#define TWI_CCR			(0x14)
/* 31:1bit reserved;0bit,write 1 to clear 0. */
#define TWI_SRST		(0x18)
/* 31:2bit reserved,1:0 bit data byte follow read command */
#define TWI_EFR			(0x1C)
/* 31:6bits reserved  5:0bit for sda&scl control */
#define TWI_LCR			(0x20)
/* 23:16 VER_BIG 7:0:VER_SMALL */
#define TWI_VERSION		(0xFC)

#if IS_ENABLED(CONFIG_I2C_SLAVE)
/* TWI_ADDR */
#define TWI_ADDR_SHIFT		(0x1)
#define TWI_ADDR_10_BIT_SHIFT	(0x8)
#define TWI_ADDR_10_BIT_WIDTH	(0x3)
#define TWI_ADDR_10_BIT_MASK	(0x78)

#define TWI_SLAVE_7_BIT_MASK	(0x7f)
#define TWI_SLAVE_10_BIT_MASK	(0xff)
#endif

/* TWI_DATA */
#define TWI_DATA_MASK	(0xff << 0)

/* TWI_CNTR */
#define CLK_COUNT_MODE	(0x1 << 0)
/* set 1 to send A_ACK,then low level on SDA */
#define A_ACK		(0x1 << 2)
/* INT_FLAG,interrupt status flag: set '1' when interrupt coming */
#define INT_FLAG	(0x1 << 3)
/* M_STP,Automatic clear 0 */
#define M_STP		(0x1 << 4)
/* M_STA,atutomatic clear 0 */
#define M_STA		(0x1 << 5)
/* BUS_EN, master mode should be set 1 */
#define BUS_EN		(0x1 << 6)
/* INT_EN, set 1 to enable interrupt */
#define INT_EN		(0x1 << 7)	/* INT_EN */
/* 31:8 bit reserved */

/* TWI_STAT */
/*
 * -------------------------------------------------------------------
 * Code   Status
 * 00h    Bus error
 * 08h    START condition transmitted
 * 10h    Repeated START condition transmitted
 * 18h    Address + Write bit transmitted, ACK received
 * 20h    Address + Write bit transmitted, ACK not received
 * 28h    Data byte transmitted in master mode, ACK received
 * 30h    Data byte transmitted in master mode, ACK not received
 * 38h    Arbitration lost in address or data byte
 * 40h    Address + Read bit transmitted, ACK received
 * 48h    Address + Read bit transmitted, ACK not received
 * 50h    Data byte received in master mode, ACK transmitted
 * 58h    Data byte received in master mode, not ACK transmitted
 * 60h    Slave address + Write bit received, ACK transmitted
 * 68h    Arbitration lost in address as master,
 *	  slave address + Write bit received, ACK transmitted
 * 70h    General Call address received, ACK transmitted
 * 78h    Arbitration lost in address as master,
 *	  General Call address received, ACK transmitted
 * 80h    Data byte received after slave address received, ACK transmitted
 * 88h    Data byte received after slave address received, not ACK transmitted
 * 90h    Data byte received after General Call received, ACK transmitted
 * 98h    Data byte received after General Call received, not ACK transmitted
 * A0h    STOP or repeated START condition received in slave mode
 * A8h    Slave address + Read bit received, ACK transmitted
 * B0h    Arbitration lost in address as master,
 *	  slave address + Read bit received, ACK transmitted
 * B8h    Data byte transmitted in slave mode, ACK received
 * C0h    Data byte transmitted in slave mode, ACK not received
 * C8h    Last byte transmitted in slave mode, ACK received
 * D0h    Second Address byte + Write bit transmitted, ACK received
 * D8h    Second Address byte + Write bit transmitted, ACK not received
 * F8h    No relevant status information or no interrupt
 *--------------------------------------------------------------------------
 */
#define TWI_STAT_MASK		(0xff)
/* 7:0 bits use only,default is 0xF8 */
#define TWI_STAT_BUS_ERR	(0x00)	/* BUS ERROR */
/* timeout when sending 9th scl clk */
#define TWI_STAT_TIMEOUT_9CLK  (0x01)
/* start can't send out */
#define TWI_STAT_TX_NSTA	(0x02)	/* defined by us not spec */
/* Master mode use only */
#define TWI_STAT_TX_STA		(0x08)	/* START condition transmitted */
/* Repeated START condition transmitted */
#define TWI_STAT_TX_RESTA	(0x10)
/* Address+Write bit transmitted, ACK received */
#define TWI_STAT_TX_AW_ACK	(0x18)
/* Address+Write bit transmitted, ACK not received */
#define TWI_STAT_TX_AW_NAK	(0x20)
/* data byte transmitted in master mode,ack received */
#define TWI_STAT_TXD_ACK	(0x28)
/* data byte transmitted in master mode ,ack not received */
#define TWI_STAT_TXD_NAK	(0x30)
/* arbitration lost in address or data byte */
#define TWI_STAT_ARBLOST	(0x38)
/* Address+Read bit transmitted, ACK received */
#define TWI_STAT_TX_AR_ACK	(0x40)
/* Address+Read bit transmitted, ACK not received */
#define TWI_STAT_TX_AR_NAK	(0x48)
/* data byte received in master mode ,ack transmitted */
#define TWI_STAT_RXD_ACK	(0x50)
/* date byte received in master mode,not ack transmitted */
#define TWI_STAT_RXD_NAK	(0x58)
/* Slave mode use only */
/* Slave address+Write bit received, ACK transmitted */
#define TWI_STAT_RXWS_ACK	(0x60)
#define TWI_STAT_ARBLOST_RXWS_ACK	(0x68)
/* General Call address received, ACK transmitted */
#define TWI_STAT_RXGCAS_ACK		(0x70)
#define TWI_STAT_ARBLOST_RXGCAS_ACK	(0x78)
#define TWI_STAT_RXDS_ACK		(0x80)
#define TWI_STAT_RXDS_NAK		(0x88)
#define TWI_STAT_RXDGCAS_ACK		(0x90)
#define TWI_STAT_RXDGCAS_NAK		(0x98)
#define TWI_STAT_RXSTPS_RXRESTAS	(0xA0)
#define TWI_STAT_RXRS_ACK		(0xA8)
#define TWI_STAT_ARBLOST_SLAR_ACK	(0xB0)
#define TWI_STAT_SLV_TXD_ACK		(0xB8)
#define TWI_STAT_SLV_TXD_NACK		(0xC0)
#define TWI_STAT_SLV_TX_LAST_ACK	(0xC8)
/* 10bit Address, second part of address */
/* Second Address byte+Write bit transmitted,ACK received */
#define TWI_STAT_TX_SAW_ACK		(0xD0)
/* Second Address byte+Write bit transmitted,ACK not received */
#define TWI_STAT_TX_SAW_NAK		(0xD8)
/* No relevant status information,INT_FLAG = 0 */
#define TWI_STAT_IDLE			(0xF8)
/* status erro */
#define TWI_STAT_ERROR			(0xF9)

/* TWI_CCR */
/*
 * Fin is APB CLOCK INPUT;
 * Fsample = F0 = Fin/2^CLK_N;
 *	F1 = F0/(CLK_M+1);
 *
 * Foscl = F1/10 = Fin/(2^CLK_N * (CLK_M+1)*10);
 * Foscl is clock SCL;standard mode:100KHz or fast mode:400KHz
 */
#define TWI_CLK_DUTY		(0x1 << 7)	/* 7bit  */
#define TWI_CLK_DUTY_30		(0x1 << 8)	/* 8bit  */
#define TWI_CLK_DIV_M_OFFSET	3
#define TWI_CLK_DIV_M		(0xf << TWI_CLK_DIV_M_OFFSET)	/* 6:3bit  */
#define TWI_CLK_DIV_N_OFFSET	0
#define TWI_CLK_DIV_N		(0x7 << TWI_CLK_DIV_N_OFFSET)	/* 2:0bit */

/* TWI_SRST */
/* write 1 to clear 0, when complete soft reset clear 0 */
#define TWI_SOFT_RST		(0x1 << 0)

/* TWI_EFR */
/* default -- 0x0 */
/* 00:no,01: 1byte, 10:2 bytes, 11: 3bytes */
#define TWI_EFR_MASK		(0x3 << 0)
#define NO_DATA_WROTE		(0x0 << 0)
#define BYTE_DATA_WROTE		(0x1 << 0)
#define BYTES_DATA1_WROTE	(0x2 << 0)
#define BYTES_DATA2_WROTE	(0x3 << 0)

/* TWI_LCR */
#define SCL_STATE		(0x1 << 5)
#define SDA_STATE		(0x1 << 4)
#define SCL_CTL			(0x1 << 3)
#define SCL_CTL_EN		(0x1 << 2)
#define SDA_CTL			(0x1 << 1)
#define SDA_CTL_EN		(0x1 << 0)

#define TWI_DRV_CTRL	(0x200)
#define TWI_DRV_CFG		(0x204)
#define TWI_DRV_SLV		(0x208)
#define TWI_DRV_FMT		(0x20C)
#define TWI_DRV_BUS_CTRL	(0x210)
#define TWI_DRV_INT_CTRL	(0x214)
#define TWI_DRV_DMA_CFG		(0x218)
#define TWI_DRV_FIFO_CON	(0x21C)
#define TWI_DRV_SEND_FIFO_ACC	(0x300)
#define TWI_DRV_RECV_FIFO_ACC	(0x304)

/* TWI_DRV_CTRL */
/* 0:module disable; 1:module enable; only use in TWI master Mode */
#define TWI_DRV_EN		(0x01 << 0)
/* 0:normal; 1:reset */
#define SOFT_RESET		(0x01 << 1)
#define TIMEOUT_N_8		(0xff)
#define TIMEOUT_N_2		(0x7fff)
#define TWI_DRV_STAT_OFFSET	16
#define TWI_DRV_STAT_MASK	(0xff << TWI_DRV_STAT_OFFSET)

#define TRAN_RESULT	(0x07 << 24)
/* 0:send slave_id + W; 1:do not send slave_id + W */
#define READ_TRAN_MODE	(0x01 << 28)
/* 0:restart; 1:STOP + START */
#define RESTART_MODE	(0x01 << 29)
/* 0:transmission idle; 1:start transmission */
#define START_TRAN	(0x01 << 31)

/* TWI_DRV_CFG */
#define PACKET_CNT_OFFSET	0
#define PACKET_CNT	(0xffff << PACKET_CNT_OFFSET)
#define PACKET_INTERVAL_OFFSET	16
#define PACKET_INTERVAL	(0xffff << PACKET_INTERVAL_OFFSET)

/* TWI_DRV_SLV */
#define SLV_ID_X_OFFSET	0
#define SLV_ID_X	(0xff << SLV_ID_X_OFFSET)
#define CMD		(0x01 << 8)
#define SLV_ID_OFFSET	9
#define SLV_ID		(0x7f << SLV_ID_OFFSET)

/* TWI_DRV_FMT */
/* how many bytes be sent/received as data */
#define DATA_BYTE_OFFSET 0
#define DATA_BYTE	(0xffff << DATA_BYTE_OFFSET)
/* how many btyes be sent as slave device reg address */
#define ADDR_BYTE_OFFSET 16
#define ADDR_BYTE	(0xff << ADDR_BYTE_OFFSET)

/* TWI_DRV_BUS_CTRL */
/* SDA manual output en */
#define SDA_MOE		(0x01 << 0)
/* SCL manual output en */
#define SCL_MOE		(0x01 << 1)
/* SDA manual output value */
#define SDA_MOV		(0x01 << 2)
/* SCL manual output value */
#define SCL_MOV		(0x01 << 3)
/* SDA current status */
#define SDA_STA		(0x01 << 6)
/* SCL current status */
#define SCL_STA		(0x01 << 7)
#define TWI_DRV_CLK_M_OFFSET	8
#define TWI_DRV_CLK_M		(0x0f << TWI_DRV_CLK_M_OFFSET)
#define TWI_DRV_CLK_N_OFFSET	12
#define TWI_DRV_CLK_N		(0x07 << TWI_DRV_CLK_N_OFFSET)
#define TWI_DRV_CLK_DUTY	(0x01 << 15)
#define TWI_DRV_COUNT_MODE	(0x01 << 16)
#define TWI_DRV_CLK_DUTY_30	(0x01 << 17)

/* TWI_DRV_INT_CTRL */
#define TRAN_COM_PD	(0x1 << 0)
#define TRAN_ERR_PD	(0x1 << 1)
#define TX_REQ_PD	(0x1 << 2)
#define RX_REQ_PD	(0x1 << 3)
#define SF_EMP		(0x1 << 4)
#define SF_FULL		(0x1 << 5)
#define RF_EMP		(0x1 << 6)
#define RF_FULL		(0x1 << 7)
#define SF_UDF		(0x1 << 8)
#define SF_OVF		(0x1 << 9)
#define RF_UDF		(0x1 << 10)
#define RF_OVF		(0x1 << 11)
#define TRAN_COM_INT_EN	(0x1 << 16)
#define TRAN_ERR_INT_EN	(0x1 << 17)
#define TX_REQ_INT_EN	(0x1 << 18)
#define RX_REQ_INT_EN	(0x1 << 19)
#define SF_EMP_INT_EN	(0x1 << 20)
#define SF_FULL_INT_EN	(0x1 << 21)
#define RF_EMP_INT_EN	(0x1 << 22)
#define RF_FULL_INT_EN	(0x1 << 23)
#define SF_UDF_INT_EN	(0x1 << 24)
#define SF_OVF_INT_EN	(0x1 << 25)
#define RF_UDF_INT_EN	(0x1 << 26)
#define RF_OVF_INT_EN	(0x1 << 27)
#define TWI_DRV_INT_EN_MASK	(0x0fff << 16)
#define TWI_DRV_INT_STA_MASK	(0x0fff << 0)

#define SUNXI_TWI_RECV_INT_ERR	(RF_UDF_INT_EN | RF_OVF_INT_EN)
#define SUNXI_TWI_SEND_INT_ERR	(SF_UDF_INT_EN | SF_OVF_INT_EN)

/* TWI_DRV_DMA_CFG */
#define TX_TRIG_OFFSET 0
#define TX_TRIG		(0x3f << TX_TRIG_OFFSET)
#define DMA_TX_EN	(0x01 << 8)
#define RX_TRIG_OFFSET	16
#define RX_TRIG		(0x3f << RX_TRIG_OFFSET)
#define DMA_RX_EN	(0x01 << 24)
#define TWI_DRQEN_MASK	(DMA_TX_EN | DMA_RX_EN)

/* TWI_DRV_FIFO_CON */
/* the number of data in SEND_FIFO */
#define SEND_FIFO_CONTENT_OFFSET	0
#define SEND_FIFO_CONTENT	(0x3f << SEND_FIFO_CONTENT_OFFSET)
/* Set this bit to clear SEND_FIFO pointer, and this bit cleared automatically */
#define SEND_FIFO_CLEAR		(0x01 << 5)
#define RECV_FIFO_CONTENT_OFFSET	16
#define RECV_FIFO_CONTENT	(0x3f << RECV_FIFO_CONTENT_OFFSET)
#define RECV_FIFO_CLEAR		(0x01 << 22)

/* TWI_DRV_SEND_FIFO_ACC */
#define SEND_DATA_FIFO	(0xff << 0)
/* TWI_DRV_RECV_FIFO_ACC */
#define RECV_DATA_FIFO	(0xff << 0)
/* end of twi regiter offset */

#define LOOP_TIMEOUT	1024
#define TWI_FREQ_100K	100000
#define TWI_FREQ_200K	200000
#define TWI_FREQ_400K	400000
#define TWI_FREQ_1M		1000000
#define APB_CLK_RATE_24M	24000000
#define APB_CLK_RATE_100M	100000000
#define AUTOSUSPEND_TIMEOUT 5000
#define HEXADECIMAL		(0x10)
#define REG_INTERVAL	(0x04)
#define REG_CL			(0x0c)
#define DMA_THRESHOLD	32
#define MAX_FIFO		32
#define DMA_TIMEOUT		1000
#define TWI_READ	true
#define TWI_WRITE	false
#define TWI_TIMEOUT_US	30000
#define TWI_DRV_IRQ			1
#define TWI_ENGINE_IRQ			0

#define WAIT_TIME_CHANGE_RATIO	2

/* twi transfer status twi->status */
enum SUNXI_TWI_XFER_STATUS {
	/* For master mode */
	SUNXI_TWI_XFER_STATUS_ERROR	= -1,
	SUNXI_TWI_XFER_STATUS_IDLE	= 0,
	SUNXI_TWI_XFER_STATUS_RUNNING,
	SUNXI_TWI_XFER_STATUS_SHUTDOWN,
	SUNXI_TWI_XFER_STATUS_COMPLETE,
#if IS_ENABLED(CONFIG_I2C_SLAVE)
	/* For slave mode */
	SUNXI_TWI_XFER_STATUS_SLAVE_IDLE,
	SUNXI_TWI_XFER_STATUS_SLAVE_SADDR,
	SUNXI_TWI_XFER_STATUS_SLAVE_WDATA,
	SUNXI_TWI_XFER_STATUS_SLAVE_RDATA,
	SUNXI_TWI_XFER_STATUS_SLAVE_ERROR,
#endif
};

enum sunxi_twi_clk_quirk_flags {
	HAS_CLK_DUTY_30 = BIT(0),	/* Bits with 30% duty when the freq is 400K*/
	USE_CLK_DUTY_30 = BIT(1),	/* Use 30% duty when the freq is 400K */
	HAS_CLK_FREQ_1M = BIT(2),	/* Platform support 1M freq */
};

struct sunxi_twi_dma {
	struct dma_chan *chan;
	dma_addr_t dma_buf;
	unsigned int dma_len;
	enum dma_transfer_direction dma_transfer_dir;
	enum dma_data_direction dma_data_dir;
};

struct sunxi_twi_hw_data {
	u32 clk_quirk_flag;
	bool slave_func_fixed; /* fixed the sda/sck issue under slave mode */
	u8 dvfs_valid_chan_nums; /*dvfs available channel numbers*/
	unsigned int drv_timeout_n_offest; /* TWI_DRV_CTRL TIMEOUT_N offest bit */
};

struct sunxi_twi {
	/* twi framework datai */
	struct i2c_adapter adap;
#if IS_ENABLED(CONFIG_I2C_SLAVE)
	struct i2c_client *slave;
#endif
	struct platform_device *pdev;
	struct device *dev;
	struct i2c_msg *msg;
	/* the total num of msg */
	unsigned int msg_num;
	/* the current msg index -> msg[msg_idx] */
	unsigned int msg_idx;
	/* the current msg's buf data index -> msg->buf[buf_idx] */
	unsigned int buf_idx;
	/* for twi core bus lock */
	struct mutex bus_lock;

	/* dts data */
	struct resource *res;
	void __iomem *base_addr;
	struct clk *bus_clk;
	struct clk *apb_clk;
	struct reset_control    *reset;
	unsigned int bus_freq;
	struct regulator *regulator;
	struct pinctrl *pctrl;
	int irq;
	int irq_flag;
	unsigned int twi_drv_used;
	unsigned int no_suspend;
	unsigned int pkt_interval;
	struct sunxi_twi_dma *dma_tx;
	struct sunxi_twi_dma *dma_rx;
	u8 *dma_buf;
	u32 vol;  /* the twi io voltage */

#if IS_ENABLED(CONFIG_AW_TWI_DVFS)
	/* dvfs data */
	bool twi_dvfs_enable;
	struct sunxi_twi_dvfs_device *dvfs_devices;
#endif
	bool clk_duty_30_enable;

	/* other data */
	int bus_num;
	enum SUNXI_TWI_XFER_STATUS status; /* error, idle, running, shutdown */
	unsigned int debug_state; /* log the twi machine state */
	const struct sunxi_twi_hw_data *data;

	spinlock_t lock; /* syn */
	struct mutex thread_lock;
	wait_queue_head_t wait;
	struct completion cmd_complete;

	unsigned int reg1[16]; /* store the twi engined mode resigter status */
	unsigned int reg2[16]; /* store the twi drv mode regiter status */
#if IS_ENABLED(CONFIG_AW_TWI_DELAYINIT)
	const char *rproc_ser_name;
	char rproc_device_name[16];
	bool delay_init_done;
#endif /* CONFIG_AW_TWI_DELAYINIT */

#ifdef CONFIG_AW_AMP_SYS_RSC_MANAGER
	sunxi_amp_rsc_t amp_rsc; /* resource manager data */
#endif
};

void sunxi_twi_drv_set_slave_addr(struct sunxi_twi *twi, struct i2c_msg *msgs);
void sunxi_twi_soft_reset(struct sunxi_twi *twi);

#if IS_ENABLED(CONFIG_AW_TWI_DVFS)
#include "dvfs/twi-sunxi-dvfs.h"
#endif

#endif /* _SUNXI_TWI_H_ */
