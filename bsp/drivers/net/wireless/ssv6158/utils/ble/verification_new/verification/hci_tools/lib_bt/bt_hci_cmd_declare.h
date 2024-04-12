#ifndef _BT_HCI_CMDS_H_
#define _BT_HCI_CMDS_H_

#include "bt_hci_lib.h"

extern bt_hci_cmd bt_hci_cmd_reset;
extern bt_hci_cmd bt_hci_cmd_read_local_version_information;
extern bt_hci_cmd bt_hci_cmd_read_local_supported_features;
extern bt_hci_cmd bt_hci_cmd_read_local_supported_commands;
extern bt_hci_cmd bt_hci_cmd_read_buffer_size;
extern bt_hci_cmd bt_hci_cmd_read_bd_addr;
extern bt_hci_cmd bt_hci_cmd_set_event_mask;
extern bt_hci_cmd bt_hci_cmd_disconnect;
extern bt_hci_cmd bt_hci_cmd_read_remote_version_information;

extern bt_hci_cmd bt_hci_cmd_le_transmitter_test;
extern bt_hci_cmd bt_hci_cmd_le_receiver_test;
extern bt_hci_cmd bt_hci_cmd_le_test_end;

extern bt_hci_cmd bt_hci_cmd_le_read_local_supported_features;
extern bt_hci_cmd bt_hci_cmd_le_read_local_supported_states;
extern bt_hci_cmd bt_hci_cmd_le_read_remote_used_features;
extern bt_hci_cmd bt_hci_cmd_le_read_buffer_size;
extern bt_hci_cmd bt_hci_cmd_le_set_random_address;
extern bt_hci_cmd bt_hci_cmd_le_rand;
extern bt_hci_cmd bt_hci_cmd_le_encrypt;
extern bt_hci_cmd bt_hci_cmd_le_start_encryption;
extern bt_hci_cmd bt_hci_cmd_le_long_term_key_request_reply;
extern bt_hci_cmd bt_hci_cmd_le_long_term_key_request_negative_reply;
extern bt_hci_cmd bt_hci_cmd_le_set_advertising_parameters;
extern bt_hci_cmd bt_hci_cmd_le_set_advertising_data;
extern bt_hci_cmd bt_hci_cmd_le_set_advertise_enable;
extern bt_hci_cmd bt_hci_cmd_le_set_scan_response_data;
extern bt_hci_cmd bt_hci_cmd_le_set_scan_parameters;
extern bt_hci_cmd bt_hci_cmd_le_set_scan_enable;
extern bt_hci_cmd bt_hci_cmd_le_set_event_mask;
extern bt_hci_cmd bt_hci_cmd_le_create_connection;
extern bt_hci_cmd bt_hci_cmd_le_create_connection_cancel;

extern bt_hci_cmd bt_hci_cmd_le_read_white_list_size;
extern bt_hci_cmd bt_hci_cmd_le_clear_white_list;
extern bt_hci_cmd bt_hci_cmd_le_add_device_to_white_list;
extern bt_hci_cmd bt_hci_cmd_le_remove_device_from_white_list;

extern bt_hci_cmd bt_hci_cmd_le_connection_update;
extern bt_hci_cmd bt_hci_cmd_le_set_host_channel_classification;
extern bt_hci_cmd bt_hci_cmd_le_read_channel_map;

extern bt_hci_cmd bt_hci_cmd_ti_get_system_status;
extern bt_hci_cmd bt_hci_cmd_ti_sleep_mode_configurations;

extern bt_hci_cmd bt_hci_cmd_ti_update_uart_hci_baudrate;
extern bt_hci_cmd bt_hci_cmd_ti_set_le_test_mode_parameters;
extern bt_hci_cmd bt_hci_cmd_ti_read_rssi;
extern bt_hci_cmd bt_hci_cmd_ti_le_drpb_reset;
extern bt_hci_cmd bt_hci_cmd_ti_le_output_power;
extern bt_hci_cmd bt_hci_cmd_ti_cont_tx_mode;
extern bt_hci_cmd bt_hci_cmd_ti_write_bd_addr;
extern bt_hci_cmd bt_hci_cmd_ti_pkt_tx_rx;
extern bt_hci_cmd bt_hci_cmd_ti_ber_meter_start;
extern bt_hci_cmd bt_hci_cmd_ti_read_ber_meter_result;

extern bt_hci_cmd bt_hci_cmd_ssv_le_transmitter_test;
extern bt_hci_cmd bt_hci_cmd_ssv_reg_read;
extern bt_hci_cmd bt_hci_cmd_ssv_reg_write;
extern bt_hci_cmd bt_hci_cmd_ssv_ble_init;
extern bt_hci_cmd bt_hci_cmd_ssv_slave_subrate;
extern bt_hci_cmd bt_hci_cmd_ssv_set_advertising_channel_priority;
extern bt_hci_cmd bt_hci_cmd_ssv_acl_evt_to_external_host;


extern bt_hci_cmd bt_hci_cmd_dut_clean_buff;
extern bt_hci_cmd bt_hci_cmd_dut_check_payload;
extern bt_hci_cmd bt_hci_cmd_dut_query_adv_cnt;
extern bt_hci_cmd bt_hci_cmd_dut_query_num_of_packets_cnt;
extern bt_hci_cmd bt_hci_cmd_dut_query_data_buffer_overflow_cnt;
extern bt_hci_cmd bt_hci_cmd_dut_query_rx_acl_packets_cnt;
#endif
