/* SPDX-License-Identifier: Apache-2.0 */
/**
 * @file 		ctrl_handler.h
 *
 * @brief 		Header file for methods to respond to MCTP Control Messages
 *
 * @copyright 	Copyright (C) 2024 Jackrabbit Founders LLC. All rights reserved.
 *
 * @date 		Mar 2024
 * @author 		Barrett Edwards <code@jrlabs.io>
 * 
 */
/* INCLUDES ==================================================================*/

#ifndef _CTRL_HANDLER_H
#define _CTRL_HANDLER_H

/* mctp_state
 * mctp_msg
 */
#include <mctp.h>

/* MACROS ====================================================================*/

/* ENUMERATIONS ==============================================================*/

/* STRUCTS ===================================================================*/

/* PROTOTYPES ================================================================*/

int ctrl_handler(struct mctp *m, struct mctp_msg *mm);

/* GLOBAL VARIABLES ==========================================================*/

#endif //_CTRL_HANDLER_H
