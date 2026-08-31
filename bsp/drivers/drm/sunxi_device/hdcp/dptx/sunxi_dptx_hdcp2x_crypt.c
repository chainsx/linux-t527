// SPDX-License-Identifier: GPL-2.0
/*
 * Xilinx HDCP2X Cryptography driver
 *
 * Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
 *
 * Author: Lakshmi Prasanna Eachuri <lakshmi.prasanna.eachuri@amd.com>
 *
 * This driver provides Xilinx HDCP 2X transmitter cryptographic functionality.
 *
 * References:
 *
 * http://www.citi.umich.edu/projects/nfsv4/rfc/pkcs-1v2-1.pdf
 * https://www.cryptrec.go.jp/cryptrec_03_spec_cypherlist_files/PDF/pkcs-1v2-12.pdf
 * https://www.digital-cp.com/sites/default/files/HDCP%20on%20DisplayPort%20Specification%20Rev2_3.pdf
 */

#include <crypto/aes.h>
#include <crypto/sha2.h>
#include <crypto/sha256_base.h>
#include <asm/unaligned.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include "sunxi_dptx_hdcp2x_crypt.h"
#include "sunxi_dptx_hdcp_bigdigits.h"
#include "sunxi_dptx_hdcp.h"

#define BD_MAX_MOD_SIZE  (HDCP2X_TX_CERT_RSA_PARAMETER_SIZE / sizeof(u32))

#define XHDCP2X_TX_SHA256_SIZE		(256 / 8)
#define XHDCP2X_TX_INNER_PADDING_BYTE	0x36
#define XHDCP2X_TX_OUTER_PADDING_BYTE	0x5C

#define XHDCP2X_TX_MAX_ALLOWED_STREAM_MANAGE_CHECKS	128
#define XHDCP2X_TX_LC128_SIZE				16
#define XHDCP2X_TX_MAX_STORED_PAIRINGINFO		2
#define XHDCP2X_TX_TS_WAIT_FOR_STREAM_TYPE		0xFD
#define XHDCP2X_TX_TS_WAIT_FOR_CIPHER			GENMASK(7, 1)
#define XHDCP2X_TX_TS_RX_REAUTH_CHECK			GENMASK(7, 0)
#define XHDCP2X_TX_RXSTATUS_REAUTH_REQ_MASK		BIT(11)
#define XHDCP2X_TX_RXSTATUS_READY_MASK			BIT(10)
#define XHDCP2X_TX_RXSTATUS_AVAIL_BYTES_MASK		GENMASK(9, 0)
#define XHDCP2X_TX_SRM_RCVID_SIZE		HDCP2X_RECEIVER_ID_SIZE
#define XHDCP2X_TX_SRM_SIGNATURE_SIZE		384
#define XHDCP2X_TX_MAX_MESSAGE_SIZE		(1 + 534)
#define XHDCP2X_TX_INVALID_RXSTATUS		GENMASK(15, 0)
#define XHDCP2X_TX_KPUB_DCP_LLC_N_SIZE		384
#define XHDCP2X_TX_KPUB_DCP_LLC_E_SIZE		1
#define XHDCP2X_TX_LC128_SIZE			16
#define XHDCP2X_TX_SRM_SIZE			396
#define XHDCP2X_TX_SHA_SIZE			256
#define XHDCP2X_TX_SHA_KEY_LENGTH		64
#define XHDCP2X_TX_RXCAPS_MASK			0x02
#define XHDCP2X_TX_CLKDIV_MHZ			1000000
#define XHDCP2X_TX_CLKDIV_HZ			1000

#define XHDCP2X_TX_TIMER_CNTR_0			0
#define XHDCP2X_TX_TIMER_CNTR_1			1
#define XHDCP2X_TX_TS_UNDEFINED			0



#define HDCP2X_TX_CERT_PUB_KEY_N_SIZE		128
#define HDCP2X_TX_CERT_PUB_KEY_E_SIZE		3
#define HDCP2X_TX_CERT_RSVD_SIZE		2
#define HDCP2X_TX_CERT_RSA_PARAMETER_SIZE		384
#define HDCP2X_TX_CERT_SIGNATURE_SIZE		384
#define HDCP2X_TX_CERT_PADDING_BYTES		330
#define HDCP2X_TX_CERT_PADDING_END_DELIMITER	332
#define HDCP2X_TX_CERT_PADDING_TI_IDENTIFIER	333
#define HDCP2X_TX_CERT_PADDING_T_HASH		352
#define HDCP2X_TX_SRM_SIGNATURE_SIZE		384
#define HDCP2X_TX_CERT_TI_IDENTIFIER_SIZE 19
#define HDCP2X_TX_CERT_T_HASH_SIZE 19
#define HDCP2X_TX_DKEY				15



/*
 * =================== important !!! ==================================
 * copy from kernel/linux-6.6/lib/crypto/sha256.c
 * use self-define to avoid that GKI without some sha256 config enable
 * ====================================================================
 */
static const u32 SHA256_K[] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
	0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
	0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
	0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
	0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
	0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
	0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
	0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
	0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};

static inline u32 Ch(u32 x, u32 y, u32 z)
{
	return z ^ (x & (y ^ z));
}

static inline u32 Maj(u32 x, u32 y, u32 z)
{
	return (x & y) | (z & (x | y));
}

#define e0(x)       (ror32(x, 2) ^ ror32(x, 13) ^ ror32(x, 22))
#define e1(x)       (ror32(x, 6) ^ ror32(x, 11) ^ ror32(x, 25))
#define s0(x)       (ror32(x, 7) ^ ror32(x, 18) ^ (x >> 3))
#define s1(x)       (ror32(x, 17) ^ ror32(x, 19) ^ (x >> 10))

static inline void LOAD_OP(int I, u32 *W, const u8 *input)
{
	W[I] = get_unaligned_be32((__u32 *)input + I);
}

static inline void BLEND_OP(int I, u32 *W)
{
	W[I] = s1(W[I-2]) + W[I-7] + s0(W[I-15]) + W[I-16];
}

#define SHA256_ROUND(i, a, b, c, d, e, f, g, h) do {		\
	u32 t1, t2;						\
	t1 = h + e1(e) + Ch(e, f, g) + SHA256_K[i] + W[i];	\
	t2 = e0(a) + Maj(a, b, c);				\
	d += t1;						\
	h = t1 + t2;						\
} while (0)

static void sha256_transform(u32 *state, const u8 *input, u32 *W)
{
	u32 a, b, c, d, e, f, g, h;
	int i;

	/* load the input */
	for (i = 0; i < 16; i += 8) {
		LOAD_OP(i + 0, W, input);
		LOAD_OP(i + 1, W, input);
		LOAD_OP(i + 2, W, input);
		LOAD_OP(i + 3, W, input);
		LOAD_OP(i + 4, W, input);
		LOAD_OP(i + 5, W, input);
		LOAD_OP(i + 6, W, input);
		LOAD_OP(i + 7, W, input);
	}

	/* now blend */
	for (i = 16; i < 64; i += 8) {
		BLEND_OP(i + 0, W);
		BLEND_OP(i + 1, W);
		BLEND_OP(i + 2, W);
		BLEND_OP(i + 3, W);
		BLEND_OP(i + 4, W);
		BLEND_OP(i + 5, W);
		BLEND_OP(i + 6, W);
		BLEND_OP(i + 7, W);
	}

	/* load the state into our registers */
	a = state[0];  b = state[1];  c = state[2];  d = state[3];
	e = state[4];  f = state[5];  g = state[6];  h = state[7];

	/* now iterate */
	for (i = 0; i < 64; i += 8) {
		SHA256_ROUND(i + 0, a, b, c, d, e, f, g, h);
		SHA256_ROUND(i + 1, h, a, b, c, d, e, f, g);
		SHA256_ROUND(i + 2, g, h, a, b, c, d, e, f);
		SHA256_ROUND(i + 3, f, g, h, a, b, c, d, e);
		SHA256_ROUND(i + 4, e, f, g, h, a, b, c, d);
		SHA256_ROUND(i + 5, d, e, f, g, h, a, b, c);
		SHA256_ROUND(i + 6, c, d, e, f, g, h, a, b);
		SHA256_ROUND(i + 7, b, c, d, e, f, g, h, a);
	}

	state[0] += a; state[1] += b; state[2] += c; state[3] += d;
	state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

static void sha256_transform_blocks(struct sha256_state *sctx,
				    const u8 *input, int blocks)
{
	u32 W[64];

	do {
		sha256_transform(sctx->state, input, W);
		input += SHA256_BLOCK_SIZE;
	} while (--blocks);

	memzero_explicit(W, sizeof(W));
}

static void sha256_update_ext(struct sha256_state *sctx, const u8 *data, unsigned int len)
{
	lib_sha256_base_do_update(sctx, data, len, sha256_transform_blocks);
}

static void __sha256_final_ext(struct sha256_state *sctx, u8 *out, int digest_size)
{
	lib_sha256_base_do_finalize(sctx, sha256_transform_blocks);
	lib_sha256_base_finish(sctx, out, digest_size);
}

static void sha256_final_ext(struct sha256_state *sctx, u8 *out)
{
	__sha256_final_ext(sctx, out, 32);
}

static void sha256_ext(const u8 *data, unsigned int len, u8 *out)
{
	struct sha256_state sctx;

	sha256_init(&sctx);
	sha256_update_ext(&sctx, data, len);
	sha256_final_ext(&sctx, out);
}
/* end of kernel/linux-6.6/lib/crypto/sha256.c copy */



/*
 * DER encoding T of the Digestinfo value is equal to this hash values
 * Reference: http://www.citi.umich.edu/projects/nfsv4/rfc/pkcs-1v2-1.pdf
 * Section 8.2.2 and 9.2.
 */
static u8 ti_identifier[] = {0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86,
			     0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05,
			     0x00, 0x04, 0x20};

/* RSA OAEP masking function */
static void sunxi_hdcp2x_tx_mg_f1(const u8 *seed, unsigned int seedlen,
				 u8 *mask, unsigned int mask_len)
{
	u8  hash_data[HDCP2X_TX_CERT_PUB_KEY_N_SIZE] = {0};
	u8  tx_cert_key[HDCP2X_TX_CERT_PUB_KEY_N_SIZE] = {0};
	u8  hash[HDCP2X_TX_SHA256_HASH_SIZE] = {0};
	u32 i;

	memcpy(hash_data, seed, seedlen);

	/*
	 * Reference: https://www.cryptrec.go.jp/cryptrec_03_spec_cypherlist_files/PDF/pkcs-1v2-12.pdf
	 * Section: 7.1
	 */
	for (i = 0; (i * HDCP2X_TX_SHA256_HASH_SIZE) < mask_len; i++) {
		u32 counter;

		counter = ntohl(i);
		memcpy(hash_data + seedlen, &counter, HDCP2X_TX_CERT_PUBLIC_EXPONENT_E);
		sha256_ext(hash_data, seedlen + HDCP2X_TX_CERT_PUBLIC_EXPONENT_E, hash);
		memcpy(tx_cert_key + (i * HDCP2X_TX_SHA256_HASH_SIZE), hash,
		       HDCP2X_TX_SHA256_HASH_SIZE);
	}
	memcpy(mask, tx_cert_key, mask_len);
}

static void sunxi_hdcp2x_tx_memxor(u8 *out, const u8 *inputparam1,
				  const u8 *inputparam2, u32 size)
{
	u32 i;

	for (i = 0; i < size; i++)
		out[i] = inputparam1[i] ^ inputparam2[i];
}

/* Reference: PKCS#1 v2.1, Section 7.1.1, Part 2 */
static void sunxi_hdcp2x_tx_pkcs1_eme_oaep_encode(const u8 *message, const u32 message_length,
						 const u8 *masking_seed, u8 *encoded_msg)
{
	u8  db_mask[HDCP2X_TX_CERT_PUB_KEY_N_SIZE - HDCP2X_TX_SHA256_HASH_SIZE - 1] = {0};
	u8  db[HDCP2X_TX_CERT_PUB_KEY_N_SIZE - HDCP2X_TX_SHA256_HASH_SIZE - 1] = {0};
	u8  seed_mask[HDCP2X_TX_SHA256_HASH_SIZE] = {0};
	u8  l_hash[HDCP2X_TX_SHA256_HASH_SIZE] = {0};
	u8  seed[HDCP2X_TX_SHA256_HASH_SIZE] = {0};
	u32 offset;

	/* Step 2a: l_hash is the empty string */
	sha256_ext(NULL, 0, l_hash);

	/* Step 2b: Generate PS by initializing DB to zeros */
	memcpy(db, l_hash, HDCP2X_TX_SHA256_HASH_SIZE);

	/* Step 2c: Generate DB = lHash || PS || 0x01 || M */
	db[HDCP2X_TX_CERT_PUB_KEY_N_SIZE - message_length -
	   HDCP2X_TX_SHA256_HASH_SIZE - 2] = 0x01;

	/*
	 * Step 2d: Generate random seed of length hLen
	 * The random seed is passed in as an argument to this function.
	 */
	offset = HDCP2X_TX_CERT_PUB_KEY_N_SIZE - message_length -
	       HDCP2X_TX_SHA256_HASH_SIZE - 1;
	memcpy(db + offset,
	       message, message_length);

	/* Step 2e: Generate dbMask = MGF1(seed, length(DB)) */
	sunxi_hdcp2x_tx_mg_f1(masking_seed,
			     HDCP2X_TX_SHA256_HASH_SIZE, db_mask,
			     HDCP2X_TX_CERT_PUB_KEY_N_SIZE - HDCP2X_TX_SHA256_HASH_SIZE
			     - 1);

	/* Step 2f: Generate maskedDB = DB xor dbMask */
	sunxi_hdcp2x_tx_memxor(db, db, db_mask,
			      HDCP2X_TX_CERT_PUB_KEY_N_SIZE - HDCP2X_TX_SHA256_HASH_SIZE - 1);

	/* Step 2g: Generate seedMask = MGF(maskedDB, length(seed)) */
	sunxi_hdcp2x_tx_mg_f1(db,
			     HDCP2X_TX_CERT_PUB_KEY_N_SIZE - HDCP2X_TX_SHA256_HASH_SIZE - 1,
			     seed_mask, HDCP2X_TX_SHA256_HASH_SIZE);

	/* Step 2h: Generate maskedSeed = seed xor seedMask */
	sunxi_hdcp2x_tx_memxor(seed, masking_seed, seed_mask,
			      HDCP2X_TX_SHA256_HASH_SIZE);

	/* Step 2i: Form encoded message EM = 0x00 || maskedSeed || maskedDB */
	memset(encoded_msg, 0, HDCP2X_TX_CERT_PUB_KEY_N_SIZE);
	memcpy(encoded_msg + 1, seed, HDCP2X_TX_SHA256_HASH_SIZE);
	memcpy(encoded_msg + 1 + HDCP2X_TX_SHA256_HASH_SIZE, db,
	       HDCP2X_TX_CERT_PUB_KEY_N_SIZE - HDCP2X_TX_SHA256_HASH_SIZE - 1);
}

static int sunxi_hdcp2x_tx_rsa_encrypt(const u8 *rsa_public_key, int public_key_size,
				      const u8 *exponent_key, int exponent_key_size,
				      const u8 *msg, int msg_size, u8 *encrypted_msg)
{
	unsigned int n[BD_MAX_MOD_SIZE], e[BD_MAX_MOD_SIZE],
			m[BD_MAX_MOD_SIZE], s[BD_MAX_MOD_SIZE];
	unsigned int mod_size = public_key_size / sizeof(unsigned int);

	if (msg_size != public_key_size)
		return -EINVAL;

	mp_conv_from_octets(n, mod_size, rsa_public_key, public_key_size);
	mp_conv_from_octets(e, mod_size, exponent_key, exponent_key_size);

	mp_conv_from_octets(m, mod_size, msg, msg_size);
	mp_mod_exp(s, m, e, n, mod_size);
	mp_conv_to_octets(s, mod_size, encrypted_msg, msg_size);

	return 0;
}

/* Reference: PKCS#1 v2.1, Section 7.1. */
int sunxi_hdcp2x_tx_rsa_oae_encrypt(const u8 *rsa_public_key,
					  int public_key_size,
					  const u8 *exponent_key, int exponent_key_size,
					  const u8 *message, const u32 message_length,
					  const u8 *masking_seed, u8 *encrypted_msg)
{
	int status;
	u8 encrypt_msg[HDCP2X_TX_CERT_PUB_KEY_N_SIZE];

	/* Step 1: Length checking */
	if (message_length > (HDCP2X_TX_CERT_PUB_KEY_N_SIZE -
			2 * HDCP2X_TX_SHA256_HASH_SIZE - 2))
		return -EINVAL;
	/* Step 2: EME-OAEP Encoding */
	sunxi_hdcp2x_tx_pkcs1_eme_oaep_encode(message, message_length,
					     masking_seed, encrypt_msg);

	/* Step 3: RSA encryption */
	status = sunxi_hdcp2x_tx_rsa_encrypt(rsa_public_key, public_key_size,
					    exponent_key, exponent_key_size,
					    encrypt_msg, public_key_size,
					    encrypted_msg);
	if (status)
		return -EINVAL;

	return 0;
}

/* Reference: PKCS#1 v2.1, Section 8.2.2 and Section 9.2 */
static int sunxi_hdcp2x_tx_rsa_signature_verify(const u8 *msg_ptr, int msg_size,
					       const u8 *signature,
					       const u8 *dcp_cert_nvalue, int dcp_cert_nsize,
					       const u8 *dcp_cert_evalue, int dcp_cert_esize)
{
	u8 encrypted_msg[HDCP2X_TX_CERT_SIGNATURE_SIZE];
	u8 t_hash[HDCP2X_TX_SHA256_HASH_SIZE];
	u8 *encrypted_msg_ptr = NULL;
	int i;
	int result = 0;

	sha256_ext(msg_ptr, msg_size, t_hash);

	result = sunxi_hdcp2x_tx_rsa_encrypt(dcp_cert_nvalue, dcp_cert_nsize,
					    dcp_cert_evalue, dcp_cert_esize,
					    signature, HDCP2X_TX_CERT_SIGNATURE_SIZE,
					    encrypted_msg);
	if (result)
		return -EINVAL;

	if (encrypted_msg[0] != 0 || encrypted_msg[1] != 1)
		return -EFAULT;

	encrypted_msg_ptr = &encrypted_msg[HDCP2X_TX_CERT_RSVD_SIZE];
	for (i = 0; i < HDCP2X_TX_CERT_PADDING_BYTES; i++) {
		if (encrypted_msg_ptr[i] != GENMASK(7, 0))
			return -EFAULT;
	}

	encrypted_msg_ptr = &encrypted_msg[HDCP2X_TX_CERT_PADDING_END_DELIMITER];
	if (encrypted_msg_ptr[0])
		return -EFAULT;

	encrypted_msg_ptr = &encrypted_msg[HDCP2X_TX_CERT_PADDING_TI_IDENTIFIER];
	if (memcmp(ti_identifier, encrypted_msg_ptr, HDCP2X_TX_CERT_TI_IDENTIFIER_SIZE))
		return -EFAULT;

	encrypted_msg_ptr = &encrypted_msg[HDCP2X_TX_CERT_PADDING_T_HASH];
	if (memcmp(t_hash, encrypted_msg_ptr, HDCP2X_TX_CERT_T_HASH_SIZE))
		return -EFAULT;

	return result;
}

int sunxi_hdcp2x_tx_verify_certificate(const struct sunxi_dptx_hdcp2_certrx *rx_certificate,
				      const u8 *dcp_cert_nvalue, int dcp_cert_nsize,
				      const u8 *dcp_cert_evalue, int dcp_cert_esize)
{
	return sunxi_hdcp2x_tx_rsa_signature_verify((u8 *)rx_certificate,
						  (sizeof(struct sunxi_dptx_hdcp2_certrx) -
						  sizeof(rx_certificate->signature)),
						  rx_certificate->signature,
						  dcp_cert_nvalue, dcp_cert_nsize,
						  dcp_cert_evalue, dcp_cert_esize);
}

int sunxi_hdcp2x_verify_srm(const u8 *srm, int srm_size,
			   const u8 *dcp_cert_nvalue, int dcp_cert_nsize,
			   const u8 *dcp_cert_evalue, int dcp_cert_esize)
{
	return sunxi_hdcp2x_tx_rsa_signature_verify((u8 *)srm,
						  srm_size - HDCP2X_TX_SRM_SIGNATURE_SIZE,
						  srm + (srm_size - HDCP2X_TX_SRM_SIGNATURE_SIZE),
						  dcp_cert_nvalue, dcp_cert_nsize,
						  dcp_cert_evalue, dcp_cert_esize);
}

static void sunxi_hdcp2x_tx_aes128_encrypt(const u8 *data, const u8 *key, u8 *output)
{
	struct crypto_aes_ctx ctx;

	aes_expandkey(&ctx, key, HDCP2X_TX_AES128_SIZE);
	aes_encrypt(&ctx, output, data);
	memzero_explicit(&ctx, sizeof(ctx));
}

/*
 * This function implements the HMAC Hash message for Authentication.
 * Reference: http://www.citi.umich.edu/projects/nfsv4/rfc/pkcs-1v2-1.pdf
 */
static int sunxi_hdcp2x_cmn_hmac_sha256_hash(const u8 *data, int data_size, const u8 *key,
					    int key_size, u8  *hashed_data)
{
	u8 buffer_in[XHDCP2X_TX_SHA_SIZE] = {0};
	u8 buffer_out[XHDCP2X_TX_SHA_SIZE] = {0};
	u8 ktemp[XHDCP2X_TX_SHA256_SIZE] = {0};
	u8 ktemp2[XHDCP2X_TX_SHA256_SIZE] = {0};
	u8 ipad[XHDCP2X_TX_SHA_KEY_LENGTH + 1] = {0};
	u8 opad[XHDCP2X_TX_SHA_KEY_LENGTH + 1] = {0};
	int i;

	if (data_size + XHDCP2X_TX_SHA_KEY_LENGTH >  XHDCP2X_TX_SHA_SIZE)
		return -EINVAL;

	if (key_size > XHDCP2X_TX_SHA_KEY_LENGTH) {
		sha256_ext(key, key_size, ktemp);
		key     = ktemp;
		key_size = XHDCP2X_TX_SHA256_SIZE;
	}

	memcpy(ipad, key, key_size);
	memcpy(opad, key, key_size);

	for (i = 0; i < XHDCP2X_TX_SHA_KEY_LENGTH; i++) {
		ipad[i] ^= XHDCP2X_TX_INNER_PADDING_BYTE;
		opad[i] ^= XHDCP2X_TX_OUTER_PADDING_BYTE;
	}

	memcpy(buffer_in, ipad, XHDCP2X_TX_SHA_KEY_LENGTH);
	memcpy(buffer_in + XHDCP2X_TX_SHA_KEY_LENGTH, data, data_size);
	sha256_ext(buffer_in, XHDCP2X_TX_SHA_KEY_LENGTH + data_size, ktemp2);

	memcpy(buffer_out, opad, XHDCP2X_TX_SHA_KEY_LENGTH);
	memcpy(buffer_out + XHDCP2X_TX_SHA_KEY_LENGTH, ktemp2, XHDCP2X_TX_SHA256_SIZE);
	sha256_ext(buffer_out, XHDCP2X_TX_SHA_KEY_LENGTH + XHDCP2X_TX_SHA256_SIZE, (u8 *)hashed_data);

	return 0;
}

void sunxi_hdcp2x_tx_compute_hprime(const u8 *r_rx, const u8 *rxcaps,
				   const u8 *r_tx, const u8 *txcaps,
				   const u8 *km, u8 *hprime)
{
	u8 kd[HDCP2X_TX_DKEY_SIZE * HDCP2X_TX_AES128_SIZE] = {0};
	u8 aes_iv[HDCP2X_TX_AES128_SIZE] = {0};
	u8 aes_key[HDCP2X_TX_AES128_SIZE] = {0};
	u8 hash_input[HDCP2X_RTX_SIZE + HDCP2X_RXCAPS_SIZE +
				HDCP2X_TXCAPS_SIZE] = {0};
	int idx = 0;

	memcpy(aes_key, km, HDCP2X_TX_KM_SIZE);
	memcpy(aes_iv, r_tx, HDCP2X_RTX_SIZE);
	memcpy(&aes_iv[HDCP2X_RTX_SIZE], r_rx, HDCP2X_RRX_SIZE);

	sunxi_hdcp2x_tx_aes128_encrypt(aes_iv, aes_key, kd);

	/* Determine dkey1, counter is 1: Rrx | 0x01. */

	/*
	 * Reference: Section 2.7.1: Key derivation
	 * https://www.digital-cp.com/sites/default/files/HDCP%20on%20DisplayPort%20Specification%20Rev2_3.pdf
	 */
	aes_iv[HDCP2X_TX_DKEY] ^= HDCP2X_TX_DKEY_CTR1;

	sunxi_hdcp2x_tx_aes128_encrypt(aes_iv, aes_key, &kd[HDCP2X_TX_KM_SIZE]);
	memcpy(hash_input, r_tx, HDCP2X_RTX_SIZE);
	idx += HDCP2X_RTX_SIZE;
	memcpy(&hash_input[idx], rxcaps, HDCP2X_RXCAPS_SIZE);
	idx += HDCP2X_RXCAPS_SIZE;
	memcpy(&hash_input[idx], txcaps, HDCP2X_TXCAPS_SIZE);

	sunxi_hdcp2x_cmn_hmac_sha256_hash(hash_input, sizeof(hash_input), kd,
					 sizeof(kd), hprime);
}

void sunxi_hdcp2x_tx_compute_edkey_ks(const u8 *rn, const u8 *km, const u8 *ks,
				     const u8 *r_rx, const u8 *r_tx,
				     u8 *encrypted_ks)
{
	u8 aes_iv[HDCP2X_TX_AES128_SIZE] = {0};
	u8 aes_key[HDCP2X_TX_AES128_SIZE] = {0};
	u8 dkey2[HDCP2X_TX_AES128_SIZE] = {0};

	memcpy(&aes_key[HDCP2X_RN_SIZE], rn, HDCP2X_RN_SIZE);

	sunxi_hdcp2x_tx_memxor(aes_key, aes_key, km, HDCP2X_TX_KM_SIZE);

	/* Determine dkey2. */
	/* Add m = Rtx || Rrx. */
	memcpy(aes_iv, r_tx, HDCP2X_RTX_SIZE);
	memcpy(&aes_iv[HDCP2X_RTX_SIZE], r_rx, HDCP2X_RRX_SIZE);

	aes_iv[HDCP2X_TX_DKEY] ^= HDCP2X_TX_DKEY_CTR2;

	sunxi_hdcp2x_tx_aes128_encrypt(aes_iv, aes_key, dkey2);

	/* EdkeyKs = Ks XOR (Dkey2 XOR Rrx). */
	/* Rrx XOR Dkey2. */
	memset(encrypted_ks, 0, HDCP2X_EDKEY_KS_SIZE);

	memcpy(&encrypted_ks[HDCP2X_EDKEY_KS_SIZE - HDCP2X_RRX_SIZE], r_rx,
	       HDCP2X_RRX_SIZE);

	sunxi_hdcp2x_tx_memxor(encrypted_ks, encrypted_ks, dkey2, HDCP2X_TX_AES128_SIZE);
	sunxi_hdcp2x_tx_memxor(encrypted_ks, encrypted_ks, ks, HDCP2X_TX_KS_SIZE);
}

void sunxi_hdcp2x_tx_compute_lprime(const u8 *rn, const u8 *km,
				   const u8 *r_rx, const u8 *r_tx,
				   u8 *lprime)
{
	u8 hash_key[HDCP2X_TX_SHA256_HASH_SIZE] = {0};
	u8 aes_iv[HDCP2X_TX_AES128_SIZE] = {0};
	u8 aes_key[HDCP2X_TX_AES128_SIZE] = {0};
	u8 kd[HDCP2X_TX_DKEY_SIZE * HDCP2X_TX_AES128_SIZE] = {0};

	memcpy(aes_key, km, HDCP2X_TX_KM_SIZE);
	memcpy(aes_iv, r_tx, HDCP2X_RTX_SIZE);
	memcpy(&aes_iv[HDCP2X_RTX_SIZE], r_rx, HDCP2X_RRX_SIZE);

	/* Compute Dkey0. */
	/* Add m = Rtx || Rrx. */
	sunxi_hdcp2x_tx_aes128_encrypt(aes_iv, aes_key, kd);

	/* Compute Dkey , counter is 1: Rrx | 0x01. */
	aes_iv[HDCP2X_TX_DKEY] ^= HDCP2X_TX_DKEY_CTR1;
	sunxi_hdcp2x_tx_aes128_encrypt(aes_iv, aes_key, &kd[HDCP2X_TX_KM_SIZE]);

	/* Create hash with HMAC-SHA256. */
	/* Key:	Kd XOR Rrx (least sign. 64 bits). */
	memcpy(&hash_key[HDCP2X_TX_SHA256_HASH_SIZE - HDCP2X_RRX_SIZE], r_rx,
	       HDCP2X_RRX_SIZE);

	sunxi_hdcp2x_tx_memxor(hash_key, hash_key, kd, HDCP2X_TX_SHA256_HASH_SIZE);
	sunxi_hdcp2x_cmn_hmac_sha256_hash(rn, HDCP2X_RN_SIZE, hash_key,
					 HDCP2X_TX_SHA256_HASH_SIZE, lprime);
}

void sunxi_hdcp2x_tx_compute_v(const u8 *rn, const u8 *r_rx, const u8 *rx_info,
			      const u8 *r_tx, const u8 *rcvid_list, const u8 rcvid_count,
			      const u8 *seq_num_v, const u8 *km, u8 *hash_v)
{
	u8 kd[HDCP2X_TX_DKEY_SIZE * HDCP2X_TX_AES128_SIZE] = {0};
	u8 aes_iv[HDCP2X_TX_AES128_SIZE] = {0};
	u8 aes_key[HDCP2X_TX_AES128_SIZE] = {0};
	u8 hash_input[(HDCP2X_TX_MAX_DEV_COUNT * HDCP2X_RECEIVER_ID_SIZE) +
				  HDCP2X_RXINFO_SIZE + HDCP2X_SEQ_NUM_V_SIZE];
	int idx = 0;

	memcpy(aes_key, km, HDCP2X_TX_KM_SIZE);

	/* Determine Dkey. */
	/* Add m = Rtx || Rrx. */
	memcpy(aes_iv, r_tx, HDCP2X_RTX_SIZE);
	memcpy(&aes_iv[HDCP2X_RTX_SIZE], r_rx, HDCP2X_RRX_SIZE);

	sunxi_hdcp2x_tx_aes128_encrypt(aes_iv, aes_key, kd);

	aes_iv[HDCP2X_TX_DKEY] ^= HDCP2X_TX_DKEY_CTR1;
	sunxi_hdcp2x_tx_aes128_encrypt(aes_iv, aes_key, &kd[HDCP2X_TX_KM_SIZE]);

	/* Create hash with HMAC-SHA256. */
	/* Input: ReceiverID list || RxInfo || seq_num_V. */
	memcpy(hash_input, rcvid_list, (rcvid_count * HDCP2X_RECEIVER_ID_SIZE));
	idx += (rcvid_count * HDCP2X_RECEIVER_ID_SIZE);
	memcpy(&hash_input[idx], rx_info, HDCP2X_RXINFO_SIZE);
	idx += HDCP2X_RXINFO_SIZE;
	memcpy(&hash_input[idx], seq_num_v, HDCP2X_SEQ_NUM_V_SIZE);
	idx += HDCP2X_SEQ_NUM_V_SIZE;

	sunxi_hdcp2x_cmn_hmac_sha256_hash(hash_input, idx, kd, sizeof(kd), hash_v);
}

void sunxi_hdcp2x_tx_compute_m(const u8 *rn, const u8 *r_rx, const u8 *r_tx,
			      const u8 *stream_id_type, const u8 *k,
			      const u8 *seq_num_m, const u8 *km, u8 *m_hash)
{
	u8 aes_iv[HDCP2X_TX_AES128_SIZE];
	u8 aes_key[HDCP2X_TX_AES128_SIZE];
	u8 kd[HDCP2X_TX_DKEY_SIZE * HDCP2X_TX_AES128_SIZE];
	u8 sha256_kd[HDCP2X_TX_SHA256_HASH_SIZE];
	u8 hash_input[(HDCP2X_TX_MAX_DEV_COUNT * HDCP2X_RECEIVER_ID_SIZE) +
				  HDCP2X_RXINFO_SIZE + HDCP2X_SEQ_NUM_V_SIZE];
	u16 stream_id_count;
	int idx = 0;

	stream_id_count  = k[0] << BITS_PER_BYTE;
	stream_id_count |= k[1];

	memcpy(aes_key, km, HDCP2X_TX_KM_SIZE);
	memcpy(aes_iv, r_tx, HDCP2X_RTX_SIZE);
	memcpy(&aes_iv[HDCP2X_RTX_SIZE], r_rx, HDCP2X_RRX_SIZE);

	sunxi_hdcp2x_tx_aes128_encrypt(aes_iv, aes_key, kd);

	/* Determine Dkey0. */
	/* Add m = Rtx || Rrx. */
	aes_iv[HDCP2X_TX_DKEY] ^= HDCP2X_TX_DKEY_CTR1;

	sunxi_hdcp2x_tx_aes128_encrypt(aes_iv, aes_key, &kd[HDCP2X_TX_KM_SIZE]);

	sha256_ext(kd, sizeof(kd), sha256_kd);

	/* Create hash with HMAC-SHA256. */
	/* Input: StreamID_Type list || seq_num_M. */
	memcpy(hash_input, stream_id_type, (stream_id_count * HDCP2X_TX_STREAMID_TYPE_SIZE));
	idx += (stream_id_count * HDCP2X_TX_STREAMID_TYPE_SIZE);
	memcpy(&hash_input[idx], seq_num_m, HDCP2X_SEQ_NUM_V_SIZE);
	idx += HDCP2X_SEQ_NUM_V_SIZE;

	/* HashKey:	SHA256(Kd) */
	sunxi_hdcp2x_cmn_hmac_sha256_hash(hash_input, idx, sha256_kd, sizeof(sha256_kd), m_hash);
}

int sunxi_hdcp2x_tx_encryptedkm(const struct sunxi_dptx_hdcp2_certrx *rx_certificate,
			       const u8 *km_ptr, u8 *masking_seed, u8 *encrypted_km)
{
	return sunxi_hdcp2x_tx_rsa_oae_encrypt(rx_certificate->public_key_n, HDCP2X_TX_CERT_PUB_KEY_N_SIZE,
					      rx_certificate->public_key_e, HDCP2X_TX_CERT_PUB_KEY_E_SIZE,
					      km_ptr, HDCP2X_TX_KM_SIZE,
					      masking_seed, encrypted_km);
}
