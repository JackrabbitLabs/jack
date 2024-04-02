/* SPDX-License-Identifier: Apache-2.0 */
/**
 * @file 		emapi_handler.c
 *
 * @brief 		Code file to handle EM API messages 
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

#include <fmapi.h>
#include <emapi.h>

/* mctp_init()
 * mctp_set_mh()
 * mctp_run()
 */
#include <mctp.h>

#include "options.h"

/* MACROS ====================================================================*/

#ifdef JACK_VERBOSE
 #define INIT 			unsigned step = 0;
 #define ENTER 					if (opts[CLOP_VERBOSITY].u64 & JKVB_CALLSTACK) 	printf("%d:%s Enter\n", 			gettid(), __FUNCTION__);
 #define STEP 			step++; if (opts[CLOP_VERBOSITY].u64 & JKVB_STEPS) 		printf("%d:%s STEP: %u\n", 			gettid(), __FUNCTION__, step);
 #define HEX32(m, i)			if (opts[CLOP_VERBOSITY].u64 & JKVB_STEPS) 		printf("%d:%s STEP: %u %s: 0x%x\n",	gettid(), __FUNCTION__, step, m, i);
 #define INT32(m, i)			if (opts[CLOP_VERBOSITY].u64 & JKVB_STEPS) 		printf("%d:%s STEP: %u %s: %d\n",	gettid(), __FUNCTION__, step, m, i);
 #define EXIT(rc) 				if (opts[CLOP_VERBOSITY].u64 & JKVB_CALLSTACK) 	printf("%d:%s Exit: %d\n", 			gettid(), __FUNCTION__,rc);
#else
 #define ENTER
 #define EXIT(rc)
 #define STEP
 #define HEX32(m, i)
 #define INT32(m, i)
 #define INIT 
#endif // JACK_VERBOSE

/* ENUMERATIONS ==============================================================*/

/* STRUCTS ===================================================================*/

/* PROTOTYPES ================================================================*/

/* GLOBAL VARIABLES ==========================================================*/

/* FUNCTIONS =================================================================*/

/**
 * Handler for all CXL Emulator API Opcodes
 * 
 * @return 	0 upon success, 1 otherwise 
 *
 * STEPS 
 * 1: Set buffer pointers 
 * 2: Deserialize Header
 * 3: Verify Response 
 * 4: Handle Opcode
 */
int emapi_handler(struct mctp *m, struct mctp_msg *mm)
{
	INIT
	struct emapi_msg msg;
	struct emapi_buf *buf;
	int rv;

	ENTER

	// Initialize variables 
	rv = 1;

	STEP // 1: Set buffer pointers 
	buf = (struct emapi_buf*) mm->payload;

	STEP // 2: Deserialize Header
	if ( emapi_deserialize(&msg.hdr, buf->hdr, EMOB_HDR, NULL) == 0 )
		goto end;

	STEP // 3: Verify response 

	// Verify EM API Message Type
	if (msg.hdr.type != EMMT_RSP) 
		goto end;

	// Check the return code
	if (msg.hdr.rc != EMRC_SUCCESS && msg.hdr.rc != EMRC_BACKGROUND_OP_STARTED) 
	{
		printf("Error: %s\n", emrc(msg.hdr.rc));
		rv = msg.hdr.rc;
		goto end;
	}

	STEP // 4: Handle Opcode
	HEX32("Opcode",  msg.hdr.opcode);
	switch(msg.hdr.opcode)
	{
		case EMOP_EVENT: 						// 0x00
			break;

		case EMOP_LIST_DEV:						// 0x01
		{
			unsigned i, num;

			num = msg.hdr.a;
			emapi_deserialize(&msg.obj, buf->payload, EMOB_LIST_DEV, &num);

			for ( i = 0 ; i < num ; i++ )
				printf("%3d: %s\n", msg.obj.dev[i].id, msg.obj.dev[i].name);
		}
			break;

		case EMOP_CONN_DEV:						// 0x02
			break;

		case EMOP_DISCON_DEV: 					// 0x03
			break;

		default: 
			goto end;
	}

	rv = 0;

end:

	// Return mctp_msg to free pool
	pq_push(m->msgs, mm);

	EXIT(rv)

	return rv;
}
