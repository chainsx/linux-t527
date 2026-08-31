/*
* Allwinner rpmsg-deinterlace driver.
*
* Copyright(c) 2022-2027 Allwinnertech Co., Ltd.
*
* This file is licensed under the terms of the GNU General Public
* License version 2.  This program is licensed "as is" without any
* warranty of any kind, whether express or implied.
*/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/rpmsg.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include "rpmsg_ve.h"

#define VE_PACKET_MAGIC 0x9656787

struct ve_packet {
	u32 magic;
	u32 type;
};

enum ve_packet_type {
	VE_RV_START,
	VE_RV_START_ACK,
	VE_RV_STOP,
	VE_RV_STOP_ACK,
	VE_LINUX_INIT_FINISH,
};

enum control_status {
	CONTROL_BY_RTOS,
	CONTROL_BY_ARM,
};

struct rpmsg_ve_private {
	struct ve_amp_ctrl *ve_ops;
	struct rpmsg_endpoint *ept;
	struct rpmsg_device *rpdev;
	struct mutex lock;
	enum control_status control;
};

struct rpmsg_ve_private *pVe;
#define VE_LOGD(fmt, arg...) printk(KERN_DEBUG"VE: "fmt"\n", ##arg)
#define VE_LOGE(fmt, arg...) printk(KERN_ERR"VE: "fmt"\n", ##arg)

static int amp_ve_rpmsg_send(enum ve_packet_type type)
{
	struct ve_packet packet;
	int ret = 0;

	if (!pVe->ept)
		ret = -1;

	memset(&packet, 0, sizeof(packet));
	packet.magic = VE_PACKET_MAGIC;
	packet.type = type;

	ret = rpmsg_send(pVe->ept, &packet, sizeof(packet));
	VE_LOGD("ve rpmsg send type: %d\n", type);
	return ret;
}

static int rpmsg_ve_cb(struct rpmsg_device *rpdev, void *data, int len,
						void *priv, u32 src)
{
	struct ve_packet *pack;
	VE_LOGD("rpmsg ve cb\n");

	pack = (struct ve_packet *)data;

	if (pack->magic != VE_PACKET_MAGIC || len != sizeof(*pack)) {
		VE_LOGE("ve packet invalid magic or size %d %d %x\n",
				len, (int)sizeof(*pack), pack->magic);
		return 0;
	}

	mutex_lock(&pVe->lock);
	VE_LOGD("pack_type %d\n", pack->type);
	if (pVe->ve_ops == NULL) {
		VE_LOGD("please pass ve_ops to rpmsg\n");
		mutex_unlock(&pVe->lock);
		return 0;
	}

	if (pack->type == VE_RV_START && pVe->control != CONTROL_BY_RTOS) {
		pVe->ve_ops->rv_start();
		amp_ve_rpmsg_send(VE_RV_START_ACK);
		pVe->control = CONTROL_BY_RTOS;
	} else if (pack->type == VE_RV_STOP && pVe->control != CONTROL_BY_ARM) {
		pVe->ve_ops->rv_stop();
		amp_ve_rpmsg_send(VE_RV_STOP_ACK);
		pVe->control = CONTROL_BY_ARM;
	}

	mutex_unlock(&pVe->lock);
	return 0;
}

static int rpmsg_ve_probe(struct rpmsg_device *rpdev)
{
	mutex_lock(&pVe->lock);
	pVe->ept = rpdev->ept;
	pVe->rpdev = rpdev;
	rpdev->announce = rpdev->src != RPMSG_ADDR_ANY;
	mutex_unlock(&pVe->lock);
	return 0;
}

static void rpmsg_ve_remove(struct rpmsg_device *rpdev)
{
	dev_info(&rpdev->dev, "%s is removed\n", dev_name(&rpdev->dev));
	mutex_lock(&pVe->lock);
	pVe->ept = NULL;
	mutex_unlock(&pVe->lock);
}

static struct rpmsg_device_id rpmsg_ve_id_table[] = {
	{ .name	= "sunxi,rpmsg_ve" },
	{ },
};
MODULE_DEVICE_TABLE(rpmsg, rpmsg_ve_id_table);

static struct rpmsg_driver rpmsg_ve_client = {
	.drv.name	= KBUILD_MODNAME,
	.id_table	= rpmsg_ve_id_table,
	.probe		= rpmsg_ve_probe,
	.callback	= rpmsg_ve_cb,
	.remove		= rpmsg_ve_remove,
};

void amp_ve_init(void *ops)
{
	struct rpmsg_ve_private *p;

	p = kzalloc(sizeof(*pVe), GFP_KERNEL);
	mutex_init(&p->lock);
	pVe = p;
	pVe->ve_ops = (struct ve_amp_ctrl *) ops;
	register_rpmsg_driver(&rpmsg_ve_client);
	pVe->control = CONTROL_BY_RTOS;
	VE_LOGD("amp_ve_init");
}

void amp_ve_exit(void)
{
	unregister_rpmsg_driver(&rpmsg_ve_client);
}