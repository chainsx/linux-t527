/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * The driver of SUNXI SecuritySystem controller.
 *
 * Copyright (C) 2013 Allwinner.
 *
 * Mintow <duanmintao@allwinnertech.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include <crypto/internal/hash.h>
#include <crypto/internal/rng.h>

#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>
#include <linux/dmapool.h>

#include "../sunxi_ce_cdev.h"
#include "../sunxi_ce_proc.h"
#include "sunxi_ce_reg.h"

/* Callback of DMA completion. */
static void ss_dma_cb(void *data)
{
	ss_aes_req_ctx_t *req_ctx = (ss_aes_req_ctx_t *)data;

	SS_DBG("DMA transfer data complete!\n");

	complete(&req_ctx->done);
}

/* request dma channel and set callback function */
static int ss_dma_prepare(sunxi_ce_cdev_t *sss, ss_dma_info_t *info, const char *name)
{
	info->chan = dma_request_chan(&sss->pdev->dev, name);
	if (IS_ERR(info->chan)) {
		SS_ERR("can't request DMA channel\n");
		return PTR_ERR(info->chan);
	}
	return 0;
}

/* set dma start flag, if queue, it will auto restart to transfer next queue */
static void ss_dma_start(ss_dma_info_t *info)
{
	dma_async_issue_pending(info->chan);
}

static int ss_dma_dst_config(sunxi_ce_cdev_t *sss,
		void *ctx, ss_aes_req_ctx_t *req_ctx, int len, int cb)
{
	int nents = 0;
	int npages = 0;
	ss_dma_info_t *info = &req_ctx->dma_dst;
	struct dma_slave_config dma_conf = {0};
	struct dma_async_tx_descriptor *dma_desc = NULL;

	info->dir = DMA_DEV_TO_MEM;
	dma_conf.direction = info->dir;
	dma_conf.src_addr = sss->phy_base_addr + SS_REG_TXFIFO;
	dma_conf.src_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
	dma_conf.dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
	dma_conf.src_maxburst = 1;
	dma_conf.dst_maxburst = 1;
	dmaengine_slave_config(info->chan, &dma_conf);

	npages = ss_sg_cnt(info->sg, len);
	WARN_ON(npages == 0);

	nents = dma_map_sg(&sss->pdev->dev, info->sg, npages, info->dir);
	SS_DBG("npages = %d, nents = %d, len = %d, sg.len = %d\n",
		npages, nents, len, sg_dma_len(info->sg));
	if (!nents) {
		SS_ERR("dma_map_sg() error\n");
		return -EINVAL;
	}

	info->nents = nents;
	dma_desc = dmaengine_prep_slave_sg(info->chan, info->sg, nents,
				info->dir, DMA_PREP_INTERRUPT | DMA_CTRL_ACK);
	if (!dma_desc) {
		SS_ERR("dmaengine_prep_slave_sg() failed!\n");
		return -1;
	}

	if (cb == 1) {
		dma_desc->callback = ss_dma_cb;
		dma_desc->callback_param = (void *)req_ctx;
	}
	dmaengine_submit(dma_desc);
	return 0;
}

/* ctx - only used for HASH. */
static int ss_dma_src_config(sunxi_ce_cdev_t *sss,
		void *ctx, ss_aes_req_ctx_t *req_ctx, int len, int cb)
{
	int nents = 0;
	int npages = 0;
	ss_dma_info_t *info = &req_ctx->dma_src;
	struct dma_slave_config dma_conf = {0};
	struct dma_async_tx_descriptor *dma_desc = NULL;
	int compensate_len = 0;
	ss_hash_ctx_t *hash_ctx;

	info->dir = DMA_MEM_TO_DEV;
	dma_conf.direction = info->dir;
	dma_conf.dst_addr = sss->phy_base_addr + SS_REG_RXFIFO;
	dma_conf.src_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
	dma_conf.dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
	dma_conf.src_maxburst = 1;
	dma_conf.dst_maxburst = 1;
	dmaengine_slave_config(info->chan, &dma_conf);

	npages = ss_sg_cnt(info->sg, len);
	WARN_ON(npages == 0);

	if (SS_METHOD_IS_HASH(req_ctx->type)) {
		hash_ctx = (ss_hash_ctx_t *)ctx;
		/* Total len is too small, so there is no data for DMA. */
		if (len < SHA1_BLOCK_SIZE)
			return 1;

		/* The tail part will be handled in ss_hash_padding */
		if (len%SHA1_BLOCK_SIZE != 0) {
			/*
			 * When there are multiple cnts in the last transmission, the part that
			 * will be processed by ss_hash_padding does not need to be transmitted.
			 */
			while (compensate_len + sg_dma_len(&info->sg[npages-1]) < hash_ctx->tail_len) {
				compensate_len += sg_dma_len(&info->sg[npages-1]);
				npages--;
			}

			sg_dma_len(&info->sg[npages-1]) = sg_dma_len(&info->sg[npages-1]) - hash_ctx->tail_len + compensate_len;
		}
	}

	nents = dma_map_sg(&sss->pdev->dev, info->sg, npages, info->dir);
	SS_DBG("npages = %d, nents = %d, len = %d, sg.len = %d\n",
		npages, nents, len, sg_dma_len(info->sg));
	if (!nents) {
		SS_ERR("dma_map_sg() error\n");
		return -EINVAL;
	}

	info->nents = nents;

	dma_desc = dmaengine_prep_slave_sg(info->chan, info->sg, nents,
				info->dir, DMA_PREP_INTERRUPT | DMA_CTRL_ACK);
	if (!dma_desc) {
		SS_ERR("dmaengine_prep_slave_sg() failed!\n");
		return -1;
	}

	if (cb == 1) {
		dma_desc->callback = ss_dma_cb;
		dma_desc->callback_param = (void *)req_ctx;
	}
	dmaengine_submit(dma_desc);
	return 0;
}

/* release dma channel, and set queue status to idle. */
static void ss_dma_release(sunxi_ce_cdev_t *sss, ss_dma_info_t *info)
{
	dma_unmap_sg(&sss->pdev->dev, info->sg, info->nents, info->dir);
	dma_release_channel(info->chan);
}

static int ss_aes_start(sunxi_ce_cdev_t *sss, ss_aes_ctx_t *ctx, ss_aes_req_ctx_t *req_ctx, int len)
{
	int ret = 0;
	int flow = ctx->comm.flow;

	ss_pending_clear(flow);
	ss_dma_enable(flow);
	ss_fifo_init();

	ss_method_set(req_ctx->dir, req_ctx->type);
	ss_aes_mode_set(req_ctx->mode);

	SS_DBG("Flow: %d, Dir: %d, Method: %d, Mode: %d, len: %d\n",
		flow, req_ctx->dir, req_ctx->type, req_ctx->mode, len);

	init_completion(&req_ctx->done);

	if (ss_dma_prepare(ss_dev, &req_ctx->dma_src, "rx"))
		return -EBUSY;
	if (ss_dma_prepare(ss_dev, &req_ctx->dma_dst, "tx"))
		return -EBUSY;
	ss_dma_src_config(ss_dev, ctx, req_ctx, len, 0);
	ss_dma_dst_config(ss_dev, ctx, req_ctx, len, 1);
	ss_reg_wr(SS_REG_CTS_LEN, len);

	ss_dma_start(&req_ctx->dma_src);
	ss_ctrl_start();
	ss_dma_start(&req_ctx->dma_dst);

	ret = wait_for_completion_timeout(&req_ctx->done,
		msecs_to_jiffies(SS_WAIT_TIME));
	if (ret == 0) {
		SS_ERR("Timed out\n");
		ss_reset();
		return -ETIMEDOUT;
	}

	ss_ctrl_stop();
	ss_dma_disable(flow);
	ss_dma_release(ss_dev, &req_ctx->dma_src);
	ss_dma_release(ss_dev, &req_ctx->dma_dst);

	return 0;
}

int ss_aes_key_valid(struct crypto_tfm *tfm, int len)
{
	if (unlikely(len > AES_MAX_KEY_SIZE)) {
		SS_ERR("Unsupported key size: %d\n", len);
		return -EINVAL;
	}
	return 0;
}

static int ss_rng_start(ss_aes_ctx_t *ctx, u8 *rdata, unsigned int dlen)
{
	int ret = 0;
	int flow = ctx->comm.flow;

	ss_pending_clear(flow);
	ss_dma_disable(flow);

	ss_method_set(SS_DIR_ENCRYPT, SS_METHOD_PRNG);

	ss_rng_mode_set(SS_RNG_MODE_CONTINUE);

	ss_ctrl_start();
	ret = ss_random_rd(rdata, dlen);

	ss_ctrl_stop();
	return ret;
}

int ss_rng_get_random(struct crypto_rng *tfm, u8 *rdata, u32 dlen, u32 trng)
{
	int ret = 0;
	ss_aes_ctx_t *ctx = crypto_rng_ctx(tfm);

	SS_DBG("flow: %d, rdata: %p, len: %d\n", ctx->comm.flow, rdata, dlen);
	if (ss_dev->suspend) {
		SS_ERR("SS has already suspend.\n");
		return -EAGAIN;
	}

	ss_dev_lock();

	/* Must set the seed addr in PRNG/TRNG. */
	ss_key_set(ctx->key, ctx->key_size);
	ctx->comm.flags &= ~SS_FLAG_NEW_KEY;

	ret = ss_rng_start(ctx, rdata, dlen);
	ss_dev_unlock();

	SS_DBG("Get %d byte random.\n", ret);

	return ret;
}

u32 ss_hash_start(ss_hash_ctx_t *ctx,
		  ss_aes_req_ctx_t *req_ctx, u32 len, u32 last)
{
	int ret = 0;
	int flow = ctx->comm.flow;

	ss_pending_clear(flow);
	ss_dma_enable(flow);
	ss_fifo_init();

	ss_method_set(req_ctx->dir, req_ctx->type);

	SS_DBG("Flow: %d, Dir: %d, Method: %d, Mode: %d, len: %d/%d\n", flow,
		req_ctx->dir, req_ctx->type, req_ctx->mode, len, ctx->cnt);

	SS_DBG("IV address = 0x%p, size = %d\n", ctx->md, ctx->md_size);
	ss_iv_mode_set(SS_IV_MODE_CONSTANT);

	init_completion(&req_ctx->done);

	if (ss_dma_prepare(ss_dev, &req_ctx->dma_src, "rx"))
		return -EBUSY;

	ret = ss_dma_src_config(ss_dev, ctx, req_ctx, len, 1);
	if (ret == 0) {
		ss_ctrl_start();
		ss_dma_start(&req_ctx->dma_src);

		ret = wait_for_completion_timeout(&req_ctx->done,
			msecs_to_jiffies(SS_WAIT_TIME));
		if (ret == 0) {
			SS_ERR("Timed out\n");
			ss_reset();
			return -ETIMEDOUT;
		}

		ss_md_get(ctx->md, NULL, ctx->md_size);
	}

	ss_dma_disable(flow);
	ss_dma_release(ss_dev, &req_ctx->dma_src);

	ctx->cnt += len;
	return 0;
}

/*
 * increment counter (128-bit int) by 1.
 * This function is copied from openssl.
 */
static void ctr128_inc(unsigned char *counter, int c)
{
	u32 n = 16;

	do {
		--n;
		c += counter[n];
		counter[n] = (u8)c;
		c >>= 8;
	} while (n);
}

int ss_aes_one_req(sunxi_ce_cdev_t *sss, struct skcipher_request *req)
{
	int ret = 0;
	struct crypto_skcipher *tfm = NULL;
	ss_aes_ctx_t *ctx = NULL;
	ss_aes_req_ctx_t *req_ctx = NULL;

	SS_ENTER();
	if (!req->src || !req->dst) {
		SS_ERR("Invalid sg: src = %p, dst = %p\n", req->src, req->dst);
		return -EINVAL;
	}

	ss_dev_lock();

	tfm = crypto_skcipher_reqtfm(req);
	req_ctx = skcipher_request_ctx(req);
	ctx = crypto_skcipher_ctx(tfm);

	/* A31 SS need update key each cycle in decryption. */
	if ((ctx->comm.flags & SS_FLAG_NEW_KEY)
		|| (req_ctx->dir == SS_DIR_DECRYPT)) {
		SS_DBG("KEY address %p, size %d\n", ctx->key, ctx->key_size);
		ss_key_set(ctx->key, ctx->key_size);
		ctx->comm.flags &= ~SS_FLAG_NEW_KEY;
	}

#ifdef SS_CTS_MODE_ENABLE
	if (((req_ctx->mode == SS_AES_MODE_CBC)
		|| (req_ctx->mode == SS_AES_MODE_CTS))
		&& (req->iv != NULL)) {
#else
	if ((req_ctx->mode == SS_AES_MODE_CBC) && (req->iv != NULL)) {
#endif
		SS_DBG("IV address = %p, size = %d\n",
			req->iv, crypto_skcipher_ivsize(tfm));
		ss_iv_set(req->iv, crypto_skcipher_ivsize(tfm));
	}

#ifdef SS_CTR_MODE_ENABLE
	if (req_ctx->mode == SS_AES_MODE_CTR) {
		SS_DBG("Cnt address = %p, size = %d\n",
			req->iv, crypto_skcipher_ivsize(tfm));
		if (ctx->cnt == 0)
			memcpy(ctx->iv,
				req->iv, crypto_skcipher_ivsize(tfm));

		SS_DBG("CNT: %08x %08x %08x %08x\n", *(int *)&ctx->iv[0],
			*(int *)&ctx->iv[4], *(int *)&ctx->iv[8],
			*(int *)&ctx->iv[12]);
		ss_cnt_set(ctx->iv, crypto_skcipher_ivsize(tfm));
	}
#endif

	req_ctx->dma_src.sg = req->src;
	req_ctx->dma_dst.sg = req->dst;

	ret = ss_aes_start(sss, ctx, req_ctx, req->cryptlen);
	if (ret < 0)
		SS_ERR("ss_aes_start fail(%d)\n", ret);

	ss_dev_unlock();

#ifdef SS_CTR_MODE_ENABLE
	if (req_ctx->mode == SS_AES_MODE_CTR) {
		ss_cnt_get(ctx->comm.flow,
			ctx->iv, crypto_skcipher_ivsize(tfm));
		/* update iv */
		ctr128_inc((unsigned char *)ctx->iv, (int)req->cryptlen/crypto_skcipher_ivsize(tfm));
		SS_DBG("CNT: %08x %08x %08x %08x\n", *(int *)&ctx->iv[0],
			*(int *)&ctx->iv[4], *(int *)&ctx->iv[8],
			*(int *)&ctx->iv[12]);
	}
#endif

	ctx->cnt += req->cryptlen;
	return ret;
}

irqreturn_t sunxi_ss_irq_handler(int irq, void *dev_id)
{
	sunxi_ce_cdev_t *sss = (sunxi_ce_cdev_t *)dev_id;
	unsigned long flags = 0;
	int pending = 0;

	spin_lock_irqsave(&sss->lock, flags);

	pending = ss_pending_get();
	SS_DBG("SS pending %#x\n", pending);
	spin_unlock_irqrestore(&sss->lock, flags);

	return IRQ_HANDLED;
}

