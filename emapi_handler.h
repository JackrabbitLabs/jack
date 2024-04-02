/* SPDX-License-Identifier: Apache-2.0 */
/**
 * @file 		emapi_handler.h
 *
 * @brief 		Header file for methods to respond to CXL Emulator API commands
 *
 * @copyright 	Copyright (C) 2024 Jackrabbit Founders LLC. All rights reserved.
 *
 * @date 		Mar 2024
 * @author 		Barrett Edwards <code@jrlabs.io>
 * 
 */
/* INCLUDES ==================================================================*/

#ifndef _EMAPI_HANDLER_H
#define _EMAPI_HANDLER_H

/* mctp_state
 * mctp_msg
 */
#include <mctp.h>

/* MACROS ====================================================================*/

/* ENUMERATIONS ==============================================================*/

/* STRUCTS ===================================================================*/

/* PROTOTYPES ================================================================*/

int emapi_handler(struct mctp *m, struct mctp_msg *mm);

/* GLOBAL VARIABLES ==========================================================*/

#endif //_EMAPI_HANDLER_H
