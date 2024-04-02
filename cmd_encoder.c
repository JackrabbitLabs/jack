/* SPDX-License-Identifier: Apache-2.0 */
/**
 * @file 		jack.c
 *
 * @brief 		Code file for FM API over TCP CLI
 *
 * @copyright 	Copyright (C) 2024 Jackrabbit Founders LLC. All rights reserved.
 *
 * @date 		Jan 2024
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

#include "cmd_encoder.h"
#include "options.h"

/* MACROS ====================================================================*/

#ifdef JACK_VERBOSE
 #define INIT 			unsigned step = 0;
 #define ENTER 					if (opts[CLOP_VERBOSITY].u64 & MCTP_VERBOSE_THREADS) 	printf("%d:%s Enter\n", 				gettid(), __FUNCTION__);
 #define STEP 			step++; if (opts[CLOP_VERBOSITY].u64 & MCTP_VERBOSE_STEPS) 		printf("%d:%s STEP: %u\n", 				gettid(), __FUNCTION__, step);
 #define HEX32(k, i)			if (opts[CLOP_VERBOSITY].u64 & MCTP_VERBOSE_STEPS) 		printf("%d:%s STEP: %u %s: 0x%x\n",		gettid(), __FUNCTION__, step, k, i);
 #define INT32(k, i)			if (opts[CLOP_VERBOSITY].u64 & MCTP_VERBOSE_STEPS) 		printf("%d:%s STEP: %u %s: %d\n",		gettid(), __FUNCTION__, step, k, i);
 #define ERR32(k, i)			if (opts[CLOP_VERBOSITY].u64 & MCTP_VERBOSE_ERROR) 		printf("%d:%s STEP: %u ERR: %s: %d\n",	gettid(), __FUNCTION__, step, k, i);
 #define EXIT(rc) 				if (opts[CLOP_VERBOSITY].u64 & MCTP_VERBOSE_THREADS)	printf("%d:%s Exit: %d\n", 				gettid(), __FUNCTION__,rc);
#else
 #define INIT
 #define ENTER
 #define STEP
 #define HEX32(k, i)
 #define INT32(k, i)
 #define ERR32(k, i)
 #define EXIT(rc)
#endif

#define JKLN_CMD_TIMEOUT_SEC	10
#define JKLN_CMD_TIMEOUT_NSEC	0


/* ENUMERATIONS ==============================================================*/

/* STRUCTS ===================================================================*/

/* PROTOTYPES ================================================================*/

/* GLOBAL VARIABLES ==========================================================*/

/* FUNCTIONS =================================================================*/

struct mctp_action *submit_ctrl(
	struct mctp *m,
	struct mctp_ctrl_msg *msg,
	int retry,
	void *user_data,
	void (*fn_submitted)(struct mctp *m, struct mctp_action *a),
	void (*fn_completed)(struct mctp *m, struct mctp_action *a),
	void (*fn_failed)(struct mctp *m, struct mctp_action *a)
	)
{
	struct timespec delta;

	// Initialize variables
	delta.tv_sec = JKLN_CMD_TIMEOUT_SEC;
	delta.tv_nsec = JKLN_CMD_TIMEOUT_NSEC;

	// Set MCTP Control Header fields 
	msg->hdr.req = 1;
	msg->hdr.datagram = 0;
	msg->hdr.inst = 0;
	msg->len = mctp_len_ctrl((__u8*)&msg->hdr);

	// Submit to MCTP library 
	return mctp_submit(
		m,						// struct mctp*
		MCMT_CONTROL,			// [MCMT] 
		msg, 					// void* to mctp payload 
		msg->len+MCLN_CTRL,		// Length of mctp payload 
		retry, 					// Retry attempts 
		&delta,
		user_data, 				// To keep with mctp_action 
		fn_submitted, 			// fn_submitted 	
 		fn_completed, 			// fn_completed 
		fn_failed 				// fn_failed 	
		);
}

struct mctp_action *submit_emapi(
	struct mctp *m,
	struct emapi_msg *msg,
	int retry,
	void *user_data,
	void (*fn_submitted)(struct mctp *m, struct mctp_action *a),
	void (*fn_completed)(struct mctp *m, struct mctp_action *a),
	void (*fn_failed)(struct mctp *m, struct mctp_action *a)
	)
{
	int len;
	struct emapi_buf buf;
	struct timespec delta;

	// Initialize variables
	delta.tv_sec = JKLN_CMD_TIMEOUT_SEC;
	delta.tv_nsec = JKLN_CMD_TIMEOUT_NSEC;

	// Serialize payload 
	len = emapi_serialize((__u8*)&buf.payload, &msg->obj, emapi_emob_req(msg->hdr.opcode), NULL);

	// Set EM API message category as a request
	emapi_fill_hdr(&msg->hdr, EMMT_REQ, 0, 0, msg->hdr.opcode, len, msg->hdr.a, msg->hdr.b);

	// Serialize EM API Header into buffer
	emapi_serialize((__u8*)&buf.hdr, &msg->hdr, EMOB_HDR, NULL);

	// Submit to MCTP library 
	return mctp_submit(
		m,						// struct mctp*
		MCMT_CSE,				// [MCMT] 
		&buf, 					// void* to mctp payload 
		msg->hdr.len + EMLN_HDR,// Length of mctp payload 
		retry, 					// Retry attempts 
		&delta,
		user_data,            	// To keep with mctp_action 
		fn_submitted, 			// fn_submitted 	
 		fn_completed, 			// fn_completed 
		fn_failed 				// fn_failed 	
		);
}

struct mctp_action *submit_fmapi(
	struct mctp *m,
	struct fmapi_msg *msg,
	int retry,
	void *user_data,
	void (*fn_submitted)(struct mctp *m, struct mctp_action *a),
	void (*fn_completed)(struct mctp *m, struct mctp_action *a),
	void (*fn_failed)(struct mctp *m, struct mctp_action *a)
	)
{
	int len;
	struct fmapi_buf buf;
	struct timespec delta;

	// Initialize variables
	delta.tv_sec = JKLN_CMD_TIMEOUT_SEC;
	delta.tv_nsec = JKLN_CMD_TIMEOUT_NSEC;

	// Serialize Object
	len = fmapi_serialize((__u8*)&buf.payload, &msg->obj, fmapi_fmob_req(msg->hdr.opcode));

	// Fill Header 
	fmapi_fill_hdr(&msg->hdr, FMMT_REQ, 0, msg->hdr.opcode, 0, len, 0, 0);

	// Serialize Header 
	fmapi_serialize((__u8*)&buf.hdr, &msg->hdr, FMOB_HDR);

	// Submit to MCTP library 
	return mctp_submit(
		m,						// struct mctp*
		MCMT_CXLFMAPI,			// [MCMT] 
		&buf, 					// void* to mctp payload 
		msg->hdr.len + FMLN_HDR,// Length of mctp payload 
		retry, 					// Retry attempts 
		&delta,
		user_data, 				// To keep with mctp_action
		fn_submitted, 			// fn_submitted 	
 		fn_completed, 			// fn_completed 
		fn_failed 				// fn_failed 	
		);
}

/**
 * Prepare an MCTP Message Request from CLI Options
 *
 * @param mm 	struct mctp_msg* this is the message to store the request in
 * @return 		0 upon success, non-zero otherwise
 *
 * STEPS
 * 1: Set buffer pointers 
 * 3: Command switch
 */
struct mctp_action *submit_cli_request(struct mctp *m, void *user_data)
{
	INIT
	struct mctp_action *ma;
	struct fmapi_msg msg, sub;
	struct emapi_msg em;
	struct mctp_ctrl_msg mc;
			
	ENTER

	// Initialize Variables 
	ma = NULL;

	STEP // 1: Set buffer pointers 

	STEP // 2: Handle Command
	switch (opts[CLOP_CMD].val)
	{
		case CLCM_NULL:
			goto end;

		case CLCM_AER:
		{
			int vcsid, vppbid;

			vcsid = 0;
			vppbid = 0;

			if (opts[CLOP_VCSID].set)
				vcsid = opts[CLOP_VCSID].u8;			//!< Virtual CXL Switch ID 

			if (opts[CLOP_VPPBID].set)
				vppbid = opts[CLOP_VPPBID].u8; 			//!< Virtual Pcie-to-PCIe Bridge ID <u8>

			if (!opts[CLOP_AER_ERROR].set)
				goto end;

			if (!opts[CLOP_AER_HEADER].set)					//!< TLP Header to place in AER registers, as defined in the PCIe specification
				goto end;

			fmapi_fill_vsc_aer(&msg, vcsid, vppbid, opts[CLOP_AER_ERROR].u32, opts[CLOP_AER_HEADER].buf);
			ma = submit_fmapi(m, &msg, 0, user_data, NULL, NULL, NULL);
		}
			break;

		case CLCM_PORT_BIND:
		{
			int vcsid, vppbid, ppid, ldid;

			vcsid = 0;
			vppbid = 0;
			ppid = 0;
			ldid = 0xFFFF;

			if (opts[CLOP_VCSID].set)
				vcsid = opts[CLOP_VCSID].u8;			//!< Virtual CXL Switch ID 

			if (opts[CLOP_PPID].set)
				ppid = opts[CLOP_PPID].u8;				//!< Physical Port ID <u8>

			if (opts[CLOP_VPPBID].set)
				vppbid = opts[CLOP_VPPBID].u8; 		//!< Virtual Pcie-to-PCIe Bridge ID <u8>

			if (opts[CLOP_LDID].set)
				ldid = opts[CLOP_LDID].u16;			//!< Logical Device ID <u16>

			fmapi_fill_vsc_bind(&msg, vcsid, vppbid, ppid, ldid); 
			ma = submit_fmapi(m, &msg, 0, user_data, NULL, NULL, NULL);
		}
			break;

		case CLCM_PORT_CONFIG:
		{
			int ppid, reg, ext, fdbe, type;

			ppid = 0;
			reg = 0;
			ext = 0;
			fdbe = 0;
			type = 0;

			if (opts[CLOP_PPID].set)
				ppid = opts[CLOP_PPID].u8;				//!< PPB ID: Target PPB physical port

			if (opts[CLOP_REGISTER].set)
				reg = opts[CLOP_REGISTER].u8;			//!< Register Number as defined in PCIe spec 

			if (opts[CLOP_EXT_REGISTER].set)
				ext = opts[CLOP_EXT_REGISTER].u8;		//!< Extended Register Number as defined in PCIe spec 

			if (opts[CLOP_FDBE].set)
				fdbe = opts[CLOP_FDBE].u8; 			//!< First DWord Byte Enable as defined in PCIe spec 
			else
				fdbe = 0x1;			 				//!< First DWord Byte Enable as defined in PCIe spec 

			if (opts[CLOP_WRITE].set) 
				type = FMCT_WRITE;					//!< Transation type [FMCT]

			fmapi_fill_psc_cfg(&msg, ppid, reg, ext, fdbe, type, (__u8*)&opts[CLOP_DATA].u32);
			ma = submit_fmapi(m, &msg, 0, user_data, NULL, NULL, NULL);
		}
			break;

		case CLCM_PORT_CONN:
		{
			emapi_fill_conn(&em, opts[CLOP_PPID].u8, opts[CLOP_DEVICE].u8); 
			ma = submit_emapi(m, &em, 0, user_data, NULL, NULL, NULL);
		}
			break;

		case CLCM_PORT_DISCONN:
		{
			emapi_fill_disconn(&em, opts[CLOP_PPID].u8, opts[CLOP_ALL].set);
			ma = submit_emapi(m, &em, 0, user_data, NULL, NULL, NULL);
		}
			break;

		case CLCM_PORT_CTRL:
		{
			int ppid, opcode;

			ppid = 0;
			opcode = 0;

			if (opts[CLOP_PPID].set)
				ppid = opts[CLOP_PPID].u8;

			switch (opts[CLOP_PORT_CONTROL].val)
			{
				case CLPC_ASSERT:	opcode = FMPO_ASSERT_PERST; 	break;
				case CLPC_DEASSERT: opcode = FMPO_DEASSERT_PERST; 	break;
				case CLPC_RESET: 	opcode = FMPO_RESET_PPB; 		break;
				default:			goto end;
			}

			fmapi_fill_psc_port_ctrl(&msg, ppid, opcode);
			ma = submit_fmapi(m, &msg, 0, user_data, NULL, NULL, NULL);
		}
			break;

		case CLCM_PORT_UNBIND:
		{
			int vcsid, vppbid, option;

			vcsid = 0; 
			option = 0;
			vppbid = 0xFFFF;

			// Prepare Object
			if (opts[CLOP_VCSID].set)
				vcsid = opts[CLOP_VCSID].u8;		//!< Virtual CXL Switch ID 

			if (opts[CLOP_VPPBID].set)
				vppbid = opts[CLOP_VPPBID].u8; 		//!< Virtual Pcie-to-PCIe Bridge ID <u8>

			if (opts[CLOP_UNBIND_MODE].set) 
			{			//!< Unbind Option [FMUB]
				switch(opts[CLOP_UNBIND_MODE].val)
				{
					case CLPU_WAIT: 	option = FMUB_WAIT;					break;
					case CLPU_MANAGED:	option = FMUB_MANAGED_HOT_REMOVE;	break;
					case CLPU_SURPRISE:	option = FMUB_SURPRISE_HOT_REMOVE;	break;
					default: 			goto end;
				}
			}

			fmapi_fill_vsc_unbind(&msg, vcsid, vppbid, option);

			ma = submit_fmapi(m, &msg, 0, user_data, NULL, NULL, NULL);
		}
			break;

		case CLCM_LD_CONFIG:
		{
 			int ppid, ldid, reg, ext, fdbe, type;

 			ppid = 0;
			ldid = 0;
			reg = 0;
			ext = 0;
			fdbe = 1;
			type = FMCT_READ;

			// Prepare Object
			if (opts[CLOP_PPID].set)
				ppid = opts[CLOP_PPID].u8;				//!< PPB ID: Target PPB physical port

			if (opts[CLOP_REGISTER].set)
				reg = opts[CLOP_REGISTER].u8;				//!< Register Number as defined in PCIe spec 

			if (opts[CLOP_EXT_REGISTER].set)
				ext = opts[CLOP_EXT_REGISTER].u8;		//!< Extended Register Number as defined in PCIe spec 

			if (opts[CLOP_FDBE].set)
				fdbe = opts[CLOP_FDBE].u8; 				//!< First DWord Byte Enable as defined in PCIe spec 

			if (opts[CLOP_WRITE].set) 
				type = FMCT_WRITE;						//!< Transation type [FMCT]
			
			if (opts[CLOP_LDID].set)
				ldid = opts[CLOP_LDID].u16;				//!< Logical Device ID <u16>

			fmapi_fill_mpc_cfg(&msg, ppid, ldid, reg, ext, fdbe, type, (__u8*)&opts[CLOP_DATA].u32);
			ma = submit_fmapi(m, &msg, 0, user_data, NULL, NULL, NULL);
		}
			break;

		case CLCM_LD_MEM:
		{
 			int ppid, ldid, len, fdbe, ldbe, type;
 			__u64 offset;
			__u8 *data;

 			ppid = 0;
			ldid = 0;
			offset = 0;
			len = 0;
			fdbe = 0xF;			 					//!< First DWord Byte Enable as defined in PCIe spec 
			ldbe = 0xF;			 					//!< First DWord Byte Enable as defined in PCIe spec 
			type = FMCT_READ;

			// Prepare Object
			if (opts[CLOP_PPID].set)
				ppid = opts[CLOP_PPID].u8;					//!< PPB ID: Target PPB physical port

			if (opts[CLOP_FDBE].set)
				fdbe = opts[CLOP_FDBE].u8; 				//!< First DWord Byte Enable as defined in PCIe spec 

			if (opts[CLOP_LDBE].set)
				ldbe = opts[CLOP_LDBE].u8; 				//!< Last DWord Byte Enable as defined in PCIe spec 

			if (opts[CLOP_WRITE].set) 
				type = FMCT_WRITE;						//!< Transation type [FMCT]
			
			if (opts[CLOP_LDID].set)
				ldid = opts[CLOP_LDID].u16;				//!< Logical Device ID <u16>

			if (opts[CLOP_LEN].set)
				len = opts[CLOP_LEN].len;					//!< Transaction Length in bytes, max of 4 kB

			if (opts[CLOP_OFFSET].set)
				offset = opts[CLOP_OFFSET].u64;			//!< Transaction Offset into target device mem space

			data = (__u8*)&opts[CLOP_DATA].u32;	//!< Transaction Data: Write data. Only valid for write transactions

			if (opts[CLOP_INFILE].set)
				data = opts[CLOP_INFILE].buf;	//!< Transaction Data: Write data. Only valid for write transactions

			fmapi_fill_mpc_mem(&msg, ppid, ldid, offset, len, fdbe, ldbe, type, data);
			ma = submit_fmapi(m, &msg, 0, user_data, NULL, NULL, NULL);
		}
			break;

		case CLCM_MCTP:
			break;

		case CLCM_MCTP_GET_EID:
		{
			mctp_ctrl_fill_get_eid(&mc);
			ma = submit_ctrl(m, &mc, 0, user_data, NULL, NULL, NULL);
		}
			break;

		case CLCM_MCTP_GET_TYPE:
		{
			mctp_ctrl_fill_get_type(&mc);
			ma = submit_ctrl(m, &mc, 0, user_data, NULL, NULL, NULL);
		}
			break;

		case CLCM_MCTP_GET_UUID:
		{
			mctp_ctrl_fill_get_uuid(&mc);
			ma = submit_ctrl(m, &mc, 0, user_data, NULL, NULL, NULL);
		}
			break;

		case CLCM_MCTP_GET_VER:
		{
			mctp_ctrl_fill_get_ver(&mc, opts[CLOP_MCTP_TYPE].u8);
			ma = submit_ctrl(m, &mc, 0, user_data, NULL, NULL, NULL);
		}
			break;

		case CLCM_MCTP_SET_EID:
		{
			mctp_ctrl_fill_set_eid(&mc, opts[CLOP_MCTP_EID].u8);
			ma = submit_ctrl(m, &mc, 0, user_data, NULL, NULL, NULL);
		}
			break;

		case CLCM_SET_MSG_LIMIT:
		{
			fmapi_fill_isc_set_msg_limit(&msg, opts[CLOP_LIMIT].u8);
			ma = submit_fmapi(m, &msg, 0, user_data, NULL, NULL, NULL);
		}
			break;

		case CLCM_SET_LD_ALLOCATIONS:
		{
			int start, num;

			start = 0;
			num  = opts[CLOP_LD_RNG1].num;
			if (opts[CLOP_LDID].set)
				start = opts[CLOP_LDID].u16;

			fmapi_fill_mcc_set_alloc(&sub, start, num, (__u64*)opts[CLOP_LD_RNG1].buf, (__u64*)opts[CLOP_LD_RNG2].buf);
			fmapi_fill_mpc_tmc(&msg, opts[CLOP_PPID].u8, MCMT_CXLCCI, &sub);
			ma = submit_fmapi(m, &msg, 0, user_data, NULL, NULL, NULL);
		}
			break;

		case CLCM_SET_QOS_ALLOCATED:
		{
			int start, num;

			num = opts[CLOP_QOS_ALLOCATED].num;
			start = 0;

			if (opts[CLOP_LDID].set)
				start = opts[CLOP_LDID].u16;

			if (!opts[CLOP_QOS_ALLOCATED].set)
				goto end;

			fmapi_fill_mcc_set_qos_alloc(&sub, start, num, opts[CLOP_QOS_ALLOCATED].buf);
			fmapi_fill_mpc_tmc(&msg, opts[CLOP_PPID].u8, MCMT_CXLCCI, &sub);
			ma = submit_fmapi(m, &msg, 0, user_data, NULL, NULL, NULL);
		}
			break;

		case CLCM_SET_QOS_CONTROL:
		{
			int epc, ttr, mod, sev, si, rcb, ci;

			epc = 0;
			ttr = 0;
			mod = 0;
			sev = 0;
			si = 0;
			rcb = 0;
			ci = 0;

			if (opts[CLOP_CONGEST_ENABLE].set)
				epc = 1;
				
			if (opts[CLOP_TEMP_THROTTLE].set)
				ttr = 1;
				
			if (opts[CLOP_EGRESS_MOD_PCNT].set)
				mod = opts[CLOP_EGRESS_MOD_PCNT].u8;
				
			if (opts[CLOP_EGRESS_SEV_PCNT].set)
				sev = opts[CLOP_EGRESS_SEV_PCNT].u8;
				
			if (opts[CLOP_BP_SAMPLE_INTVL].set)
				si = opts[CLOP_BP_SAMPLE_INTVL].u8;
				
			if (opts[CLOP_REQCMPBASIS].set)
				rcb = opts[CLOP_REQCMPBASIS].u16;
				
			if (opts[CLOP_CCINTERVAL].set)
				ci = opts[CLOP_CCINTERVAL].u8;

			fmapi_fill_mcc_set_qos_ctrl(&sub,  epc,	ttr, mod, sev, si, rcb, ci);
			fmapi_fill_mpc_tmc(&msg, opts[CLOP_PPID].u8, MCMT_CXLCCI, &sub);
			ma = submit_fmapi(m, &msg, 0, user_data, NULL, NULL, NULL);
		}
		break;

		case CLCM_SET_QOS_LIMIT:
		{
			int start, num;

			num = opts[CLOP_QOS_LIMIT].num;
			start = 0;

			if (opts[CLOP_LDID].set)
				start = opts[CLOP_LDID].u16;

			if (!opts[CLOP_QOS_LIMIT].set)
				goto end;

			fmapi_fill_mcc_set_qos_limit(&sub, start, num, opts[CLOP_QOS_LIMIT].buf);
			fmapi_fill_mpc_tmc(&msg, opts[CLOP_PPID].u8, MCMT_CXLCCI, &sub);
			ma = submit_fmapi(m, &msg, 0, user_data, NULL, NULL, NULL);
		}
			break;

		case CLCM_SHOW_BOS:
		{
			fmapi_fill_isc_bos(&msg);
			ma = submit_fmapi(m, &msg, 0, user_data, NULL, NULL, NULL);
		}
			break;

		case CLCM_SHOW_IDENTITY:
		{
			fmapi_fill_isc_id(&msg);
			ma = submit_fmapi(m, &msg, 0, user_data, NULL, NULL, NULL);
		}
			break;

		case CLCM_SHOW_MSG_LIMIT:
		{
			fmapi_fill_isc_get_msg_limit(&msg);
			ma = submit_fmapi(m, &msg, 0, user_data, NULL, NULL, NULL);
		}
			break;

		case CLCM_SHOW_PORT:
		{
			if (opts[CLOP_PPID].set) 
			{
				if (opts[CLOP_PPID].num > 0) 
					fmapi_fill_psc_get_ports(&msg, opts[CLOP_PPID].num, opts[CLOP_PPID].buf);
				else 
					fmapi_fill_psc_get_port(&msg, opts[CLOP_PPID].u8);
			} 
			else if (opts[CLOP_ALL].set) 
				fmapi_fill_psc_get_all_ports(&msg);
			else 
				goto end;

			ma = submit_fmapi(m, &msg, 0, user_data, NULL, NULL, NULL);
		}
			break;

		case CLCM_SHOW_LD_ALLOCATIONS:
		{
			fmapi_fill_mcc_get_alloc(&sub, 0, 0);
			fmapi_fill_mpc_tmc(&msg, opts[CLOP_PPID].u8, MCMT_CXLCCI, &sub);
			ma = submit_fmapi(m, &msg, 0, user_data, NULL, NULL, NULL);
		}
			break;

		case CLCM_SHOW_LD_INFO:
		{
			fmapi_fill_mcc_get_info(&sub);
			fmapi_fill_mpc_tmc(&msg, opts[CLOP_PPID].u8, MCMT_CXLCCI, &sub);
			ma = submit_fmapi(m, &msg, 0, user_data, NULL, NULL, NULL);
		}
			break;

		case CLCM_SHOW_QOS_ALLOCATED:
		{
			__u8 num, start; 

			num = 255;
			start = 0;

			if (opts[CLOP_NUM].set)
				num = opts[CLOP_NUM].u8;

			if (opts[CLOP_LDID].set)
				start = opts[CLOP_LDID].u16;

			fmapi_fill_mcc_get_qos_alloc(&sub, start, num);
			fmapi_fill_mpc_tmc(&msg, opts[CLOP_PPID].u8, MCMT_CXLCCI, &sub);
			ma = submit_fmapi(m, &msg, 0, user_data, NULL, NULL, NULL);
		}
			break;

		case CLCM_SHOW_QOS_CONTROL:
		{
			fmapi_fill_mcc_get_qos_ctrl(&sub);
			fmapi_fill_mpc_tmc(&msg, opts[CLOP_PPID].u8, MCMT_CXLCCI, &sub);
			ma = submit_fmapi(m, &msg, 0, user_data, NULL, NULL, NULL);
		}
			break;

		case CLCM_SHOW_QOS_LIMIT:
		{
			__u8 num, start; 

			num = 255;
			start = 0;

			if (opts[CLOP_NUM].set)
				num = opts[CLOP_NUM].u8;

			if (opts[CLOP_LDID].set)
				start = opts[CLOP_LDID].u16;

			fmapi_fill_mcc_get_qos_limit(&sub, start, num);
			fmapi_fill_mpc_tmc(&msg, opts[CLOP_PPID].u8, MCMT_CXLCCI, &sub);
			ma = submit_fmapi(m, &msg, 0, user_data, NULL, NULL, NULL);
		}
			break;

		case CLCM_SHOW_QOS_STATUS:
		{
			fmapi_fill_mcc_get_qos_status(&sub);
			fmapi_fill_mpc_tmc(&msg, opts[CLOP_PPID].u8, MCMT_CXLCCI, &sub);
			ma = submit_fmapi(m, &msg, 0, user_data, NULL, NULL, NULL);
		}
			break;

		case CLCM_SHOW_SWITCH:
		{
			fmapi_fill_psc_id(&msg);
			ma = submit_fmapi(m, &msg, 0, user_data, NULL, NULL, NULL);
		}
			break;

		case CLCM_SHOW_DEV:
		{
			int a = 0;
			int b = 0;

			if (opts[CLOP_DEVICE].set) {
				a = 1;
				b = opts[CLOP_DEVICE].u8;
			} 

			if (opts[CLOP_ALL].set) {
				a = 0;
			}

			emapi_fill_listdev(&em, a, b);
			ma = submit_emapi(m, &em, 0, user_data, NULL, NULL, NULL);
		}
			break;

		case CLCM_SHOW_VCS:
		{
			int vcsid = 0;

			if (opts[CLOP_VCSID].set) 
				vcsid = opts[CLOP_VCSID].u8;

			fmapi_fill_vsc_get_vcs(&msg, vcsid, 0, 255);
			ma = submit_fmapi(m, &msg, 0, user_data, NULL, NULL, NULL);
		}
			break;

		default:
			goto end;
	}

end:

	EXIT(0);

	return ma;
}

