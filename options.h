/* SPDX-License-Identifier: Apache-2.0 */
/**
 * @file 		options.h 
 *
 * @brief 		Header file for CXL Fabric Management CLI options
 *
 * @copyright 	Copyright (C) 2024 Jackrabbit Founders LLC. All rights reserved.
 *
 * @date 		Jan 2024
 * @author 		Barrett Edwards <code@jrlabs.io>
 *
 * Macro / Enumeration Prefixes (CL)
 * CLAP - CLI Options Parsers Enumeration (AP)
 * CLCM - CLI Command Opcod (CM)
 * CLMR - CLI Macros (MR)
 * CLOP	- CLI Option (CL)
 * CLPC - Physical Port Control Opcodes (PC)
 * CLPU - Port Unbind Mode Options (PU)
 * 
 * Standard key mapping 
 * -h --help 			Display Help
 * -T --tcp-port 		Server TCP Port
 * -V --verbosity 		Set Verbosity Flag
 * -X --verbosity-hex	Set all Verbosity Flags with hex value
 * -p --ppid		 	Physical Port ID
 * -c --vcsid 			Virtual CXL Switch ID
 * -b --vppbid			Virtual PCIe-to-PCIe Bridge ID
 * -a --all 			All of collection 
 * -l --ldid			LD-ID (for MLD devices)
 * -w --write 			Perform a Write transaction
 * -n --length 			Length 
 * -o --offset 			Memory Offset
 *    --data 			Write Data (up to 4 bytes)
 *    --infile 			Filename for input data
 *    --outfile 		Filename for output data
 * 
 * Non char key mapping
 * 701 - usage
 * 702 - version
 * 703 - data
 * 704 - infile
 * 705 - outfile
 * 706 - print-options
 */
#ifndef _OPTIONS_H
#define _OPTIONS_H

/* INCLUDES ==================================================================*/

/* __u8
 * __u15
 * __u32
 * __u64
 * __s32
 */
#include <linux/types.h> 

/* MACROS ====================================================================*/

/**
 * CLI Macros (MR)
 *
 * These are values that don't belong in an enumeration
 */
#define CLMR_MAX_LD_MEM_LEN 	4096
#define CLMR_MAX_LD 			16
#define CLMR_HELP_COLUMN 		30
#define CLMR_MAX_HELP_WIDTH 	100
#define CLMR_MAX_NAME_LEN 		64
#define CLMR_AER_HEADER_LEN 	32

#define DEFAULT_SERVER_PORT 	2508

/* ENUMERATIONS ==============================================================*/

/** 
 * Verbosity Options (VO)
 */
enum _JKVO 
{
	JKVO_GENERAL	= 0,
	JKVO_CALLSTACK 	= 1,
	JKVO_STEPS		= 2,
	JKVO_MAX
};

/** 
 * Verbosity Bitfield Index (VB)
 */
enum _JKVB
{
	JKVB_GENERAL	= (0x01 << 0),
	JKVB_CALLSTACK 	= (0x01 << 1),
	JKVB_STEPS		= (0x01 << 2),
};

/**
 * CLI Options Parsers Enumeration (AP)
 *
 * This enumeration identifies each argp parser. It is used to print options 
 */
enum _CLAP 
{
	CLAP_MAIN 					= 0,
	CLAP_MCTP 					= 1, 
	CLAP_SHOW 					= 2,
	CLAP_PORT 					= 3,
	CLAP_SET  					= 4,
	CLAP_LD   					= 5, 
	CLAP_AER  					= 6, 
	CLAP_SHOW_SWITCH 			= 7,
	CLAP_SHOW_PORT 				= 8,
	CLAP_SHOW_VCS 				= 9,
	CLAP_SHOW_QOS 				= 10,
	CLAP_SHOW_LD 				= 11,
	CLAP_PORT_BIND 				= 12,
	CLAP_PORT_UNBIND 			= 13,
	CLAP_PORT_CONFIG			= 14,
	CLAP_PORT_CTRL 				= 15,
	CLAP_SET_LD 				= 16,
	CLAP_SET_QOS				= 17,
	CLAP_LD_CONFIG 				= 18,
	CLAP_LD_MEM    				= 19,
	CLAP_SHOW_QOS_ALLOCATED 	= 20,
	CLAP_SHOW_QOS_CONTROL   	= 21,
	CLAP_SHOW_QOS_LIMIT     	= 22,
	CLAP_SHOW_QOS_STATUS    	= 23,
	CLAP_SHOW_LD_ALLOCATIONS	= 24,
	CLAP_SHOW_LD_INFO           = 25,
	CLAP_SET_LD_ALLOCATIONS     = 26,
	CLAP_SET_QOS_ALLOCATED      = 27,
	CLAP_SET_QOS_CONTROL        = 28,
	CLAP_SET_QOS_LIMIT          = 29,
	CLAP_SHOW_DEV  	            = 30,
	CLAP_PORT_CONN 	            = 31,
	CLAP_PORT_DISCONN           = 32,
	CLAP_SHOW_IDENTITY          = 33,
	CLAP_SHOW_MSG_LIMIT         = 34,
	CLAP_SET_MSG_LIMIT          = 35,
	CLAP_SHOW_BOS          		= 36,

	CLAP_MAX
};

/**
 * CLI Command Opcode (CM)
 */
enum _CLCM 
{
	CLCM_NULL 				= 0,

	CLCM_AER				= 1,

	CLCM_PORT_BIND 			= 2,
	CLCM_PORT_CONFIG 		= 3,
	CLCM_PORT_CTRL  		= 4,
	CLCM_PORT_UNBIND 		= 5,

	CLCM_LD_CONFIG 			= 6, 
	CLCM_LD_MEM 			= 7, 

	CLCM_MCTP				= 8,
	CLCM_MCTP_GET_EID		= 9,
	CLCM_MCTP_GET_TYPE		= 10,
	CLCM_MCTP_GET_UUID		= 11,
	CLCM_MCTP_GET_VER		= 12,
	CLCM_MCTP_SET_EID		= 13,

	CLCM_SET_LD_ALLOCATIONS	= 14,
	CLCM_SET_QOS_ALLOCATED	= 15,
	CLCM_SET_QOS_CONTROL	= 16,
	CLCM_SET_QOS_LIMIT		= 17,

	CLCM_SHOW_PORT			= 18,
	CLCM_SHOW_LD_ALLOCATIONS= 19,
	CLCM_SHOW_LD_INFO		= 20,
	CLCM_SHOW_QOS_ALLOCATED	= 21,
	CLCM_SHOW_QOS_CONTROL	= 22,
	CLCM_SHOW_QOS_LIMIT		= 23,
	CLCM_SHOW_QOS_STATUS	= 24,
	CLCM_SHOW_SWITCH		= 25,
	CLCM_SHOW_VCS			= 26,
	CLCM_SHOW_DEV			= 27,
	CLCM_PORT_CONN 			= 28,
	CLCM_PORT_DISCONN 		= 29,
	CLCM_SHOW_IDENTITY      = 30,
	CLCM_SHOW_MSG_LIMIT     = 31,
	CLCM_SET_MSG_LIMIT      = 32,
	CLCM_SHOW_BOS           = 33,
	CLCM_LIST 				= 34,

	CLCM_MAX
};

/**
 * CLI Option (OP)
 */
enum _CLOP
{	
	/* General CLI Options */
	CLOP_VERBOSITY 			= 0,	//!< Bit Field <u64>
	CLOP_TCP_PORT 			= 1,	//!< TCP Port to connect to <u16>
	CLOP_CMD 				= 2,	//!< Command to RUN <val>
	CLOP_INFILE 			= 3, 	//!< Filename for input <str,buf,len>

	/* Hiden Options */
	CLOP_PRNT_OPTS 			= 4, 	//!< Print Options Array when completed parsing <set>

	/* MCTP Options */
	CLOP_MCTP_EID 			= 5,	//!< EID value for MCTP Set EID Command <u8>
	CLOP_MCTP_TYPE 			= 6,	//!< Type value for MCTP Command Get Ver Command <u8>

	/* ID Options */
	CLOP_VCSID 				= 7,	//!< Virtual CXL Switch ID <u8>
	CLOP_PPID 				= 8,	//!< Physical Port ID <u8>
	CLOP_VPPBID 			= 9, 	//!< Virtual Pcie-to-PCIe Bridge ID <u8>
	CLOP_LDID 				= 10,	//!< Logical Device ID <u16>
	CLOP_ALL 				= 11,	//!< Perform aciton on all of collection <set>

	/* Port Options */
	CLOP_UNBIND_MODE 		= 12,	//!< Port Unbind Option [CLPU] <val>
	CLOP_PORT_CONTROL 		= 13,	//!< Action to perform [CLPC] <val>

	/* Config & Memory Options */
	CLOP_REGISTER 			= 14,	//!< Register number <u8>
	CLOP_EXT_REGISTER 		= 15,	//!< Extended Register number <u8>
	CLOP_FDBE 				= 16,	//!< First Dword Byte Enable <u8>
	CLOP_LDBE				= 17,	//!< First Dword Byte Enable <u8>
	CLOP_WRITE 				= 18,	//!< Perform a write transacion <set>
	CLOP_OFFSET 			= 19, 	//!< Offset into a buffer address space <u64>
	CLOP_LEN				= 20,	//!< Length of data parameter <len>

	/* LD Options */
	CLOP_LD_RNG1 			= 21,	//!< LD Allocations Range1 Array <num,len,buf>
	CLOP_LD_RNG2 			= 22,	//!< LD Allocations Range2 Array <num,len,buf>

	/* QoS Options */
	CLOP_CONGEST_ENABLE 	= 23,	//!< Egress Port Congestion Enable <set>
	CLOP_TEMP_THROTTLE 		= 24,	//!< Temporary Throughput Reduction Enable <set>
  	CLOP_EGRESS_MOD_PCNT  	= 25, 	//!< Egress Moderate Percentage <u8>
  	CLOP_EGRESS_SEV_PCNT 	= 26, 	//!< Egress Severe Percentage <u8>
  	CLOP_BP_SAMPLE_INTVL 	= 27, 	//!< Backpressure Sample Interval <u8>
  	CLOP_REQCMPBASIS 		= 28, 	//!< ReqCmpBasis <u16>
  	CLOP_CCINTERVAL 		= 29, 	//!< Completion Collection Interval <u8>
	CLOP_QOS_ALLOCATED 		= 30,	//!< QoS BW Allocation Fraction list <num,len,buf>
	CLOP_QOS_LIMIT 			= 31,	//!< QoS BW Limit Fraction list <num,len,buf>

	/* AER Options */
	CLOP_AER_ERROR 			= 32,	//!< AER Error (4 Byte HEX) <u32>
	CLOP_AER_HEADER 		= 33,	//!< AER TLP Header (32 Byte HEX String) <num,len,buf>

	CLOP_DATA 				= 34,	//!< Immediate Data for Write transaction <u32>

	CLOP_OUTFILE			= 35,	//!< Filename for output <str>

	CLOP_MCTP_VERBOSITY 	= 36,	//!< Verbosity setting for MCTP library
	CLOP_DEVICE         	= 37,	//!< Device ID <u8>
	CLOP_NUM 				= 38, 	//!< Number of items <u8>
	CLOP_LIMIT				= 39, 	//!< Message Response Limit <u8>
	CLOP_TCP_ADDRESS		= 40,	//!< TCP Address to connect to <u32>
	CLOP_NO_INIT			= 41,	//!< Do not initialize local state at start up
	CLOP_MAX
};

/**
 * Physical Port Control Opcodes (PC)
 *
 * CXL 2.0 v1.0 Table 93
 */
enum _CLPC 
{
	CLPC_ASSERT		= 0,
	CLPC_DEASSERT 	= 1, 
	CLPC_RESET	 	= 2, 
	CLPC_MAX
};

/** 
 * Port Unbind Mode Options (PU)
 *
 * CXL 2.0 v1.0 Table 101
 */
enum _CLPU 
{
	CLPU_WAIT 		= 0,
	CLPU_MANAGED 	= 1,
	CLPU_SURPRISE  	= 2,
	CLPU_MAX	
};

/* STRUCTS ===================================================================*/

/**
 * CLI Option Struct
 *
 * Each command line parameter is stored in one of these objects
 */
struct opt
{
	int 			set;	//!< Not set (0), set (1) 
	__u8 			u8; 	//!< Unsigned char value
	__u16 			u16; 	//!< Unsigned long value
	__u32 			u32;	//!< Unsigned long value 
	__u64 			u64;	//!< Unsigned long long value 
	__s32 	 		val;	//!< Generic signed value
	__u64 			num;	//!< Number of items 
	__u64 			len;	//!< Data Buffer Length 
	char 			*str;	//!< String value 
	__u8 			*buf;	//!< Data buffer 
};

/* GLOBAL VARIABLES ==========================================================*/

/**
 * Global varible to store parsed CLI options
 */
extern struct opt *opts;

/* PROTOTYPES ================================================================*/

/**
 * Free allocated memory by option parsing proceedure
 *
 * @return 0 upon success. Non zero otherwise
 */
int options_free(struct opt *opts);

/**
 * Parse command line options 
 */
int options_parse(int argc, char *argv[]);

#endif //ifndef _OPTIONS_H
