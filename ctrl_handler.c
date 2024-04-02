/* SPDX-License-Identifier: Apache-2.0 */
/**
 * @file 		ctrl_handler.c
 *
 * @brief 		Code file to repond to MCTP Control Messages
 *
 * @copyright 	Copyright (C) 2024 Jackrabbit Founders LLC. All rights reserved.
 *
 * @date 		Mar 2024
 * @author 		Barrett Edwards <code@jrlabs.io>
 * 
 */
/* INCLUDES ==================================================================*/

/* gettid()
 */
#define _GNU_SOURCE

#include <unistd.h>

/* printf()
 */
#include <stdio.h>

/* memset()
 */
#include <string.h>

#include <stdlib.h>

/* autl_prnt_buf()
 */
#include <arrayutils.h>

#include <ptrqueue.h>

/* struct mctp
 * struct mctp_msg
 * struct mctp_ctrl_msg
 */
#include <mctp.h>

#include "ctrl_handler.h"
#include "options.h"

/* MACROS ====================================================================*/

#ifdef JACK_VERBOSE
 #define INIT 			unsigned step = 0;
 #define ENTER 					if (m->verbose & MCTP_VERBOSE_THREADS) 	printf("%d:%s Enter\n", 				gettid(), __FUNCTION__);
 #define STEP 			step++; if (m->verbose & MCTP_VERBOSE_STEPS) 	printf("%d:%s STEP: %u\n", 				gettid(), __FUNCTION__, step);
 #define HEX32(k, i)			if (m->verbose & MCTP_VERBOSE_STEPS) 	printf("%d:%s STEP: %u %s: 0x%x\n",		gettid(), __FUNCTION__, step, k, i);
 #define INT32(k, i)			if (m->verbose & MCTP_VERBOSE_STEPS) 	printf("%d:%s STEP: %u %s: %d\n",		gettid(), __FUNCTION__, step, k, i);
 #define ERR32(k, i)			if (m->verbose & MCTP_VERBOSE_ERROR) 	printf("%d:%s STEP: %u ERR: %s: %d\n",	gettid(), __FUNCTION__, step, k, i);
 #define EXIT(rc) 				if (m->verbose & MCTP_VERBOSE_THREADS)	printf("%d:%s Exit: %d\n", 				gettid(), __FUNCTION__,rc);
#else
 #define INIT
 #define ENTER
 #define STEP
 #define HEX32(k, i)
 #define INT32(k, i)
 #define ERR32(k, i)
 #define EXIT(rc)
#endif

/* ENUMERATIONS ==============================================================*/

/* STRUCTS ===================================================================*/

/* PROTOTYPES ================================================================*/

/* GLOBAL VARIABLES ==========================================================*/

/* FUNCTIONS =================================================================*/

/** 
 * Handle Responses that are MCTP Control Messages 
 *
 * @return 0 upon success. Non zero otherwise.
 *
 * STEPS
 * 1: Set payload pointers 
 * 2: Validate Response
 * 3: Handle opcode
 */
int ctrl_handler(struct mctp *m, struct mctp_msg *mm)
{
	INIT
	struct mctp_ctrl_msg *msg;
	int rv; 

	ENTER

	// Initialize Variables
	rv = 1;

	STEP // 1: Set payload pointers 
	msg = (struct mctp_ctrl_msg*) mm->payload;

	STEP // 2: Validate Response

	// Check MCTP reequest bit

	// Check MCTP Instance ID 

	// Check MCTP Completion Code
	if (msg->obj.get_eid_rsp.comp_code != MCCC_SUCCESS) 
	{
		printf("Error: MCTP Control Command %s Failed: %s\n", mccm(msg->hdr.cmd), mccc(msg->obj.get_eid_rsp.comp_code));
		goto end;
	}

	STEP // 3: Handle opcode
	switch(msg->hdr.cmd)
	{
		case MCCM_RESERVED:
			break;

		case MCCM_SET_ENDPOINT_ID:
		{
	  	 	printf("EID: 0x%02x\n", msg->obj.set_eid_rsp.eid); 
		}
			break;

		case MCCM_GET_ENDPOINT_ID:
	   	{
	  	 	printf("EID: 0x%02x\n", msg->obj.get_eid_rsp.eid); 
		}
			break;

		case MCCM_GET_ENDPOINT_UUID:
	   	{
			char buf[37];

			memset(buf, 0, 37);

			// Convert UUID into String for printing
			uuid_unparse(msg->obj.get_uuid_rsp.uuid, buf);

			printf("MCTP UUID: %s\n", buf); 
		}
			break;

		case MCCM_GET_VERSION_SUPPORT:
		{
			struct mctp_ver *mv;
			char buf[11];

			for ( int i = 0 ; i < msg->obj.get_ver_rsp.count ; i++) 
			{
				mv = &msg->obj.get_ver_rsp.versions[i];
			
				rv = mctp_sprnt_ver(buf, (struct mctp_version*) mv);	

				printf("[%02d] %s\n", i, buf); 
			}
		}
			break;

		case MCCM_GET_MESSAGE_TYPE_SUPPORT:
		{
			for ( int i = 0 ; i < msg->obj.get_msg_type_rsp.count ; i++)
				printf("%02d: %d - %s\n", i, msg->obj.get_msg_type_rsp.list[i], mcmt(msg->obj.get_msg_type_rsp.list[i])); 
		}
			break;

		case MCCM_GET_VENDOR_MESSAGE_SUPPORT:
		case MCCM_RESOLVE_ENDPOINT_ID:
		case MCCM_ALLOCATE_ENDPOINT_IDS:
		case MCCM_ROUTING_INFO_UPDATE:
		case MCCM_GET_ROUTING_TABLE_ENTRIES:
		case MCCM_PREPARE_ENDPOINT_DISCOVERY:
		case MCCM_ENDPOINT_DISCOVERY:
		case MCCM_DISCOVERY_NOTIFY:
		case MCCM_GET_NETWORK_ID:
		case MCCM_QUERY_HOP:
		case MCCM_RESOLVE_UUID:
		case MCCM_QUERY_RATE_LIMIT:
		case MCCM_REQUEST_TX_RATE_LIMIT:
		case MCCM_UPDATE_RATE_LIMIT:
		case MCCM_QUERY_SUPPORTED_INTERFACES:
		default: goto end;
	}

	rv = 0;

end:

	// Return mctp_msg to free pool
	pq_push(m->msgs, mm);
	
	EXIT(rv)

	return rv;
}

