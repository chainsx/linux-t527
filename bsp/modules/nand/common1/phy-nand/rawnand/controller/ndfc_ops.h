/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __NDFC_OPS_H__
#define __NDFC_OPS_H__

#include "ndfc_base.h"

extern const u8 rand_factor_1st_spare_data[12][128];

int gen_rand_num(unsigned short seed, int len, unsigned char *out);
int wait_reg_status_half_us(volatile u32 *reg, u32 mark, u32 value);
s32 wait_reg_status(volatile u32 *reg, u32 mark, u32 value, u32 us);
void ndfc_print_dma(struct nand_controller_info *nctri);
void ndfc_print_random(struct nand_controller_info *nctri);
u32 _cal_random_seed(u32 page);
void _set_addr(struct nand_controller_info *nctri, u8 acnt, u8 *abuf);
void ndfc_disable_ecc(struct nand_controller_info *nctri);
void ndfc_enable_ecc(struct nand_controller_info *nctri, u32 pipline, u32 randomize);
void ndfc_enable_ldpc_ecc(struct nand_controller_info *nctri, u32 pipline);
void ndfc_repeat_mode_enable(struct nand_controller_info *nctri);
void ndfc_repeat_mode_disable(struct nand_controller_info *nctri);
s32 ndfc_wait_cmdfifo_free(struct nand_controller_info *nctri);
s32 ndfc_wait_fsm_ready(struct nand_controller_info *nctri);
s32 ndfc_wait_cmd_finish(struct nand_controller_info *nctri);
void ndfc_select_chip(struct nand_controller_info *nctri, u32 chip);
void ndfc_select_rb(struct nand_controller_info *nctri, u32 rb);
void ndfc_deselect_chip(struct nand_controller_info *nctri, u32 chip);
void ndfc_deselect_rb(struct nand_controller_info *nctri, u32 rb);
u32 ndfc_get_selected_rb_no(struct nand_controller_info *nctri);
s32 ndfc_get_rb_sta(struct nand_controller_info *nctri, u32 rb);
u32 ndfc_get_page_size(struct nand_controller_info *nctri);
int ndfc_set_page_size(struct nand_controller_info *nctri, u32 page_size);
void ndfc_set_ecc_mode(struct nand_controller_info *nctri, u8 ecc_mode);
void ndfc_boot0_ldpc_mode(struct nand_controller_info *nctri);
void ndfc_enable_encode(struct nand_controller_info *nctri);
void ndfc_enable_decode(struct nand_controller_info *nctri);
s32 ndfc_get_ecc_mode(struct nand_controller_info *nctri);
void ndfc_set_rand_seed(struct nand_controller_info *nctri, u32 page_no);
void ndfc_set_new_rand_seed(struct nand_controller_info *nctri, u32 page_no);
void ndfc_set_default_rand_seed(struct nand_controller_info *nctri);
void ndfc_enable_randomize(struct nand_controller_info *nctri);
void ndfc_disable_randomize(struct nand_controller_info *nctri);
void ndfc_enable_rb_b2r_int(struct nand_controller_info *nctri);
void ndfc_disable_rb_b2r_int(struct nand_controller_info *nctri);
void ndfc_clear_rb_b2r_int(struct nand_controller_info *nctri);
u32 ndfc_get_rb_b2r_int_sta(struct nand_controller_info *nctri);
u32 ndfc_check_rb_b2r_int_occur(struct nand_controller_info *nctri);
void ndfc_enable_cmd_int(struct nand_controller_info *nctri);
void ndfc_disable_cmd_int(struct nand_controller_info *nctri);
void ndfc_clear_cmd_int(struct nand_controller_info *nctri);
u32 ndfc_get_cmd_int_sta(struct nand_controller_info *nctri);
u32 ndfc_check_cmd_int_occur(struct nand_controller_info *nctri);
void ndfc_enable_dma_int(struct nand_controller_info *nctri);
void ndfc_disable_dma_int(struct nand_controller_info *nctri);
void ndfc_clear_dma_int(struct nand_controller_info *nctri);
u32 ndfc_get_dma_int_sta(struct nand_controller_info *nctri);
u32 ndfc_check_dma_int_occur(struct nand_controller_info *nctri);
s32 ndfc_wait_rb_ready_int(struct nand_controller_info *nctri, u32 rb);
s32 ndfc_wait_rb_ready(struct nand_controller_info *nctri, u32 rb);
s32 ndfc_wait_all_rb_ready(struct nand_controller_info *nctri);
s32 ndfc_write_wait_rb_ready(struct nand_controller_info *nctri, u32 rb);
s32 ndfc_change_page_size(struct nand_controller_info *nctri);
s32 _normal_cmd_io_send(struct nand_controller_info *nctri, struct _nctri_cmd *icmd);
s32 _normal_cmd_io_wait(struct nand_controller_info *nctri, struct _nctri_cmd *icmd);
s32 _batch_cmd_io_wait(struct nand_controller_info *nctri, struct _nctri_cmd_seq *cmd_list);
void ndfc_clean_cmd_seq(struct _nctri_cmd_seq *cmd_list);
void print_cmd_seq(struct _nctri_cmd_seq *cmd_list);
s32 ndfc_execute_cmd(struct nand_controller_info *nctri, struct _nctri_cmd_seq *cmd_list);
s32 ndfc_get_spare_data(struct nand_controller_info *nctri, u8 *sbuf, u32 udata_cnt);
s32 ndfc_set_spare_data(struct nand_controller_info *nctri, u8 *sbuf, u32 udata_cnt);
s32 ndfc_set_user_data_len(struct nand_controller_info *nctri);
s32 ndfc_set_user_data_len_cfg(struct nand_controller_info *nctri, u32 udata_cnt);
s32 ndfc_set_user_data_len_cfg_4bytesper1k(struct nand_controller_info *nctri, u32 udata_cnt);
int ndfc_is_toogle_interface(struct nand_controller_info *nctri);
int ndfc_set_legacy_interface(struct nand_controller_info *nctri);
int ndfc_set_toggle_interface(struct nand_controller_info *nctri);
int ndfc_set_dummy_byte(struct nand_controller_info *nctri, int dummy_byte);
void ndfc_enable_dummy_byte(struct nand_controller_info *nctri);
void ndfc_disable_dummy_byte(struct nand_controller_info *nctri);
void ndfc_get_nand_interface(struct nand_controller_info *nctri, u32 *pddr_type, u32 *psdr_edo, u32 *pddr_edo, u32 *pddr_delay);
s32 ndfc_change_nand_interface(struct nand_controller_info *nctri, u32 ddr_type, u32 sdr_edo, u32 ddr_edo, u32 ddr_delay);
void ndfc_emce_ctrl_setting(struct nand_controller_info *nctri, u32 enable_flag);
void ndfc_emce_fac_compare_value_setting(struct nand_controller_info *nctri, u32 value);
void ndfc_emce_calculate_factor_setting(struct nand_controller_info *nctri, u32 *values, u32 offset_start, u32 lenth);
void _setup_ndfc_ddr2_para(struct nand_controller_info *nctri);
void _set_ndfc_def_timing_param(struct nand_chip_info *nci);
extern __s32 nand_rb_wait_time_out(__u32 no, __u32 *flag);
s32 init_nctri(struct nand_controller_info *nctri);
int save_nctri(struct nand_controller_info *nctri);
s32 fill_nctri(struct nand_controller_info *nctri);
int recover_nctri(struct nand_controller_info *nctri);
void ndfc_print_reg(struct nand_controller_info *nctri);
void ndfc_print_save_reg(struct nand_controller_info *nctri);
int ndfc_soft_reset(struct nand_controller_info *nctri);
int batch_cmd_io_send(struct nand_controller_info *nctri, struct _nctri_cmd_seq *cmd_list);
int ndfc_dma_config_start(struct nand_controller_info *nctri, u8 rw, void *buff_addr, u32 len);
void ndfc_encode_select(struct nand_controller_info *nctri);
void ndfc_encode_default(struct nand_controller_info *nctri);
void ndfc_channel_select(struct nand_controller_info *nctri, u32 channel_sel);
void delete_from_nctri_by_channel(int channel);
void delete_nctri(void);

int ndfc_get_version(void);

struct ndfc_df_func_ops {
	s32 (*init_nctri)(struct nand_controller_info *nctri);
	int (*save_nctri)(struct nand_controller_info *nctri);
	s32 (*fill_nctri)(struct nand_controller_info *nctri);
	int (*recover_nctri)(struct nand_controller_info *nctri);
	void (*ndfc_print_reg)(struct nand_controller_info *nctri);
	void (*ndfc_print_save_reg)(struct nand_controller_info *nctri);
	int (*ndfc_soft_reset)(struct nand_controller_info *nctri);
	void (*ndfc_encode_select)(struct nand_controller_info *nctri);
	void (*ndfc_encode_default)(struct nand_controller_info *nctri);
	void (*ndfc_channel_select)(struct nand_controller_info *nctri, u32 channel_sel);
	int (*batch_cmd_io_send)(struct nand_controller_info *nctri, struct _nctri_cmd_seq *cmd_list);
	int (*ndfc_dma_config_start)(struct nand_controller_info *nctri, u8 rw, void *buff_addr, u32 len);
};

extern struct ndfc_df_func_ops ndfc_df_ops;
#endif /*NDFC_OPS_H*/
