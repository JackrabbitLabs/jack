/* SPDX-License-Identifier: Apache-2.0 */
/**
 * @file 		cmd_encoder.h
 *
 * @brief 		Header file for methods to convert the CLI parameters into an 
 *              FM API Request
 *
 * @copyright 	Copyright (C) 2024 Jackrabbit Founders LLC. All rights reserved.
 *
 * @date 		Mar 2024
 * @author 		Barrett Edwards <code@jrlabs.io>
 * 
 */
/* INCLUDES ==================================================================*/

#ifndef _CMD_ENCODER_H
#define _CMD_ENCODER_H

/* mctp_state
 * mctp_msg
 */
#include <mctp.h>

/* MACROS ====================================================================*/

/* ENUMERATIONS ==============================================================*/

/* STRUCTS ===================================================================*/

/* PROTOTYPES ================================================================*/

struct mctp_action *submit_ctrl(
	struct mctp *m,
	struct mctp_ctrl_msg *msg,
	int retry,
	void *user_data,
	void (*fn_submitted)(struct mctp *m, struct mctp_action *a),
	void (*fn_completed)(struct mctp *m, struct mctp_action *a),
	void (*fn_failed)(struct mctp *m, struct mctp_action *a)
	);

struct mctp_action *submit_emapi(
	struct mctp *m,
	struct emapi_msg *msg,
	int retry,
	void *user_data,
	void (*fn_submitted)(struct mctp *m, struct mctp_action *a),
	void (*fn_completed)(struct mctp *m, struct mctp_action *a),
	void (*fn_failed)(struct mctp *m, struct mctp_action *a)
	);

struct mctp_action *submit_fmapi(
	struct mctp *m,
	struct fmapi_msg *fm,
	int retry,
	void *user_data,
	void (*fn_submitted)(struct mctp *m, struct mctp_action *a),
	void (*fn_completed)(struct mctp *m, struct mctp_action *a),
	void (*fn_failed)(struct mctp *m, struct mctp_action *a)
	);

struct mctp_action *submit_cli_request(struct mctp *m, void *user_data);

/* GLOBAL VARIABLES ==========================================================*/

#endif //_CMD_ENCODER_H
