/* SPDX-License-Identifier: Apache-2.0 */
/**
 * @file 		fmapi_handler.h
 *
 * @brief 		Header file for methods to respond to FM API commands
 *
 * @copyright 	Copyright (C) 2024 Jackrabbit Founders LLC. All rights reserved.
 *
 * @date 		Jan 2024
 * @author 		Barrett Edwards <code@jrlabs.io>
 * 
 */
/* INCLUDES ==================================================================*/

#ifndef _FMAPI_HANDLER_H
#define _FMAPI_HANDLER_H

/* mctp_state
 * mctp_msg
 */
#include <mctp.h>

/* MACROS ====================================================================*/

/* ENUMERATIONS ==============================================================*/

/* STRUCTS ===================================================================*/

/* PROTOTYPES ================================================================*/

int fmapi_handler(struct mctp *m, struct mctp_msg *mm, struct mctp_msg *req);
int fmapi_update(struct mctp *m, struct mctp_action *ma);

/* GLOBAL VARIABLES ==========================================================*/

extern struct cxl_switch *cxls;

#endif //_FMAPI_HANDLER_H
