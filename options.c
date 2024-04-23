/* SPDX-License-Identifier: Apache-2.0 */
/**
 * @file 		options.c
 *
 * @brief 		Code file for CXL Fabric Management CLI options
 *
 * @copyright 	Copyright (C) 2024 Jackrabbit Founders LLC. All rights reserved.
 *
 * @date 		Jan 2024
 * @author 		Barrett Edwards <code@jrlabs.io>
 *
 */

/* INCLUDES ==================================================================*/

/* memset()
 */
#include <string.h> 

#include <stdlib.h>

/* inet_pton()
 */
#include <arpa/inet.h>

/* __u8
 * __u16
 * __u32
 *__u64
 */
#include <linux/types.h> 

/* struct argp
 * struct argp_state
 * argp_parse()
 */
#include <argp.h>

/* autl_prnt_buf();
 */
#include <arrayutils.h>

#include "options.h"

/* MACROS ====================================================================*/

/* ENUMERATIONS ==============================================================*/

/* STRUCTS ===================================================================*/

/**
 * Local struct used to define shell environment variables and the key to parse them
 */
struct envopt
{
	char key;
	char *name;
};

/* PROTOTYPES ================================================================*/

static void print_help(int option);
static void print_options(struct argp_option *o);
static void print_usage(int option, struct argp_option *o);

static int pr_main(int key, char *arg, struct argp_state *state);
static int pr_mctp(int key, char *arg, struct argp_state *state);
static int pr_show(int key, char *arg, struct argp_state *state);
static int pr_port(int key, char *arg, struct argp_state *state);
static int pr_set(int key, char *arg, struct argp_state *state);
static int pr_ld(int key, char *arg, struct argp_state *state);
static int pr_aer(int key, char *arg, struct argp_state *state);
static int pr_show_bos(int key, char *arg, struct argp_state *state);
static int pr_show_identity(int key, char *arg, struct argp_state *state);
static int pr_show_limit(int key, char *arg, struct argp_state *state);
static int pr_show_switch(int key, char *arg, struct argp_state *state);
static int pr_show_dev(int key, char *arg, struct argp_state *state);
static int pr_show_port(int key, char *arg, struct argp_state *state);
static int pr_show_vcs(int key, char *arg, struct argp_state *state);
static int pr_show_qos(int key, char *arg, struct argp_state *state);
static int pr_show_ld(int key, char *arg, struct argp_state *state);
static int pr_port_bind(int key, char *arg, struct argp_state *state);
static int pr_port_unbind(int key, char *arg, struct argp_state *state);
static int pr_port_config(int key, char *arg, struct argp_state *state);
static int pr_port_connect(int key, char *arg, struct argp_state *state);
static int pr_port_disconnect(int key, char *arg, struct argp_state *state);
static int pr_port_ctrl(int key, char *arg, struct argp_state *state);
static int pr_set_ld(int key, char *arg, struct argp_state *state);
static int pr_set_limit(int key, char *arg, struct argp_state *state);
static int pr_set_qos(int key, char *arg, struct argp_state *state);
static int pr_ld_config(int key, char *arg, struct argp_state *state);
static int pr_ld_mem(int key, char *arg, struct argp_state *state);
static int pr_show_qos_allocated(int key, char *arg, struct argp_state *state);
static int pr_show_qos_control(int key, char *arg, struct argp_state *state);
static int pr_show_qos_limit(int key, char *arg, struct argp_state *state);
static int pr_show_qos_status(int key, char *arg, struct argp_state *state);
static int pr_show_ld_allocations(int key, char *arg, struct argp_state *state);
static int pr_show_ld_info(int key, char *arg, struct argp_state *state);
static int pr_set_ld_allocations(int key, char *arg, struct argp_state *state);
static int pr_set_qos_allocated(int key, char *arg, struct argp_state *state);
static int pr_set_qos_control(int key, char *arg, struct argp_state *state);
static int pr_set_qos_limit(int key, char *arg, struct argp_state *state);

/* GLOBAL VARIABLES ==========================================================*/

/**
 * Global varible to store parsed CLI options
 */
struct opt *opts;

const char *argp_program_version = "version 0.2";
const char *argp_program_bug_address = "code@jrlabs.io";

/**
 * String representation of CLOP Enumeration 
 */ 
char *STR_CLOP[] = {
	"VERBOSITY",			
	"TCP_PORT",			
	"CMD",				
	"INFILE",			
	"PRNT_OPTS",			
	"MCTP_EID",			
	"MCTP_TYPE",	
	"VCSID",				
	"PPID",				
	"VPPBID",			
	"LDID",				
	"ALL",				
	"UNBIND_MODE",		
	"PORT_CONTROL",		
	"REGISTER",			
	"EXT_REGISTER",		
	"FDBE",				
	"LDBE",				
	"WRITE",		
	"OFFSET",			
	"LEN",				
	"LD_RNG1",			
	"LD_RNG2",			
	"CONGEST_ENABLE",	
	"TEMP_THROTTLE",	
  	"EGRESS_MOD_PCNT", 	
  	"EGRESS_SEV_PCNT", 	
  	"BP_SAMPLE_INTVL", 	
  	"REQCMPBASIS", 		
  	"CCINTERVAL", 		
	"QOS_ALLOCATED",		
	"QOS_LIMIT",			
	"AER_ERROR",			
	"AER_HEADER",
	"DATA",
	"OUTFILE",
	"MCTP_VERBOSITY",
	"CLOP_DEVICE",
	"NUM"
};

/**
 * Global array of CLI options to pull from the shell environment if present
 */
struct envopt envopts[] = 
{
	{'T', "JACK_TCP_ADDRESS"},
	{'P', "JACK_TCP_PORT"},
	{'X', "JACK_VERBOSITY"},
	{'Z', "JACK_MCTP_VERBOSITY"},
	{0,0}
};

/**
 * Global char pointer to dynamically store the name of the application 
 *
 * This is allocated and stored when parse_options() is called
 */
static char *app_name;

/**
 *  CLAP_MAIN - Options for main level parser
 */
struct argp_option ao_main[] =						
{
	{0,0,0,0,"Command Options",1}, // Group

  	{"all",       'A',  NULL, OPTION_HIDDEN, "All Physical Ports", 0},	
  	{"ppid", 	  'p', "INT", OPTION_HIDDEN, "Physical Port ID", 0},
  	{"ldid",      'l', "INT", OPTION_HIDDEN, "LD-ID (for MLD devices)", 0},
  	{"vcsid",     'c', "INT", OPTION_HIDDEN, "Virtual CXL Switch ID", 0},
  	{"vppbid",    'b', "INT", OPTION_HIDDEN, "Virtual PCIe-to-PCIe Bridge ID", 0},

	{0,0,0,0, "Networking Options",7},	
  	{"tcp-port",  	'P', "INT", 0, "Server TCP Port", 0},
  	{"tcp-address", 'T', "INT", 0, "Server TCP Address", 0},
	{0,0,0,0, "Verbose Options",8},	// Group
  	{"verbosity",     'V', "INT", 0, "Set Verbosity Flag", 0},	
  	{"verbosity-hex", 'X', "HEX", 0, "Set all Verbosity Flags with hex value", 0},	
  	{"mctp-verbosity",'Z', "HEX", OPTION_HIDDEN, "Set all MCTP Verbosity Flags with hex value", 0},	
  	{"no-init",		  'N', NULL,  OPTION_HIDDEN, "Do not initialize local state at start up", 0},	
  	{"print-options", 706,  NULL, OPTION_HIDDEN, "Print CLI Options", 0},	
	{0,0,0,0, "Help Options", 9}, // Group
  	{"help",    'h',  NULL, 0, "Display Help", 0},
  	{"usage",   701,  NULL, 0, "Display Usage", 0},	
  	{"version", 702,  NULL, 0, "Display Version", 0},
	{0,0,0,0,0,0} // Final option should be all null
};

/**
 * CLAP_MCTP - Options for: <app> mctp 
 */
struct argp_option ao_mctp[] = 	
{
	{0,0,0,0,"Command Options",1}, // Group
  	{"set-eid", 's', "INT", 0, "Set Remote Endpoint ID", 0},	
  	{"get-eid", 'g',  NULL, 0, "Get Remote Endpoint ID", 0},	
  	{"get-uuid",'u',  NULL, 0, "Get Remote Endpoint UUID",0},	
  	{"get-type",'t',  NULL, 0, "Get MCTP Message Type Support",0},	
  	{"get-ver",	'r', "INT", 0, "Get MCTP Version Support",0},

	{0,0,0,0, "Networking Options",7},	
  	{"tcp-port",  'P', "INT", 0, "Server TCP Port", 0},
  	{"tcp-address", 'T', "INT", 0, "Server TCP Address", 0},
	{0,0,0,0, "Verbose Options",8},	// Group
  	{"verbosity",     'V', "INT", 0, "Set Verbosity Flag", 0},	
  	{"verbosity-hex", 'X', "HEX", 0, "Set all Verbosity Flags with hex value", 0},	
  	{"mctp-verbosity",'Z', "HEX", OPTION_HIDDEN, "Set all MCTP Verbosity Flags with hex value", 0},	
  	{"no-init",		  'N', NULL,  OPTION_HIDDEN, "Do not initialize local state at start up", 0},	
  	{"print-options", 706,  NULL, OPTION_HIDDEN, "Print CLI Options", 0},	
	{0,0,0,0, "Help Options", 9}, // Group
  	{"help",    'h', NULL, 0, "Display Help", 0},
  	{"usage",   701, NULL, 0, "Display Usage", 0},	
  	{"version", 702, NULL, 0, "Display Version", 0},
	{0,0,0,0,0,0} // Final option should be all null
};

/**
 * CLAP_SHOW - Options for: <app> show
 */
struct argp_option ao_show[] = 	
{
	{0,0,0,0,"Command Options",1}, // Group

  	{"all",       'a',  NULL, OPTION_HIDDEN, "Perform on all items", 0},	
  	{"ppid", 	  'p', "INT", OPTION_HIDDEN, "Physical Port ID", 0},
  	{"vcsid",     'c', "INT", OPTION_HIDDEN, "Virtual CXL Switch ID", 0},
	
	{0,0,0,0, "Networking Options",7},	
  	{"tcp-port",  'P', "INT", 0, "Server TCP Port", 0},
  	{"tcp-address", 'T', "INT", 0, "Server TCP Address", 0},
	{0,0,0,0, "Verbose Options",8},	// Group
  	{"verbosity",     'V', "INT", 0, "Set Verbosity Flag", 0},	
  	{"verbosity-hex", 'X', "HEX", 0, "Set all Verbosity Flags with hex value", 0},	
  	{"mctp-verbosity",'Z', "HEX", OPTION_HIDDEN, "Set all MCTP Verbosity Flags with hex value", 0},	
  	{"no-init",		  'N', NULL,  OPTION_HIDDEN, "Do not initialize local state at start up", 0},	
  	{"print-options", 706,  NULL, OPTION_HIDDEN, "Print CLI Options", 0},	
	{0,0,0,0, "Help Options", 9}, // Group
  	{"help",    'h', NULL, 0, "Display Help", 0},
  	{"usage",   701, NULL, 0, "Display Usage", 0},	
  	{"version", 702, NULL, 0, "Display Version", 0},
	{0,0,0,0,0,0} // Final option should be all null
};

/**
 * CLAP_PORT - Options for: <app> port
 */
struct argp_option ao_port[] = 	
{
	{0,0,0,0,"Command Options",1}, 

  	{"ppid", 	  'p', "INT", OPTION_HIDDEN, "Physical Port ID", 0},
  	{"ldid",      'l', "INT", OPTION_HIDDEN, "LD-ID (for MLD devices)", 0},
  	{"vcsid",     'c', "INT", OPTION_HIDDEN, "Virtual CXL Switch ID", 0},
  	{"vppbid",    'b', "INT", OPTION_HIDDEN, "Virtual PCIe-to-PCIe Bridge ID", 0},
	
	{0,0,0,0, "Networking Options",7},	
  	{"tcp-port",  'P', "INT", 0, "Server TCP Port", 0},
  	{"tcp-address", 'T', "INT", 0, "Server TCP Address", 0},
	{0,0,0,0, "Verbose Options",8},	// Group
  	{"verbosity",     'V', "INT", 0, "Set Verbosity Flag", 0},	
  	{"verbosity-hex", 'X', "HEX", 0, "Set all Verbosity Flags with hex value", 0},	
  	{"mctp-verbosity",'Z', "HEX", OPTION_HIDDEN, "Set all MCTP Verbosity Flags with hex value", 0},	
  	{"no-init",		  'N', NULL,  OPTION_HIDDEN, "Do not initialize local state at start up", 0},	
  	{"print-options", 706,  NULL, OPTION_HIDDEN, "Print CLI Options", 0},	
	{0,0,0,0, "Help Options", 9}, // Group
  	{"help",    'h', NULL, 0, "Display Help", 0},
  	{"usage",   701, NULL, 0, "Display Usage", 0},	
  	{"version", 702, NULL, 0, "Display Version", 0},
	{0,0,0,0,0,0} // Final option should be all null
};

/**
 * CLAP_SET - Options for: <app> set
 */
struct argp_option ao_set[] = 	
{
	{0,0,0,0,"Command Options",1}, 

  	{"ppid", 	  'p', "INT", OPTION_HIDDEN, "Physical Port ID", 0},
	
	{0,0,0,0, "Networking Options",7},	
  	{"tcp-port",  'P', "INT", 0, "Server TCP Port", 0},
  	{"tcp-address", 'T', "INT", 0, "Server TCP Address", 0},
	{0,0,0,0, "Verbose Options",8},	// Group
  	{"verbosity",     'V', "INT", 0, "Set Verbosity Flag", 0},	
  	{"verbosity-hex", 'X', "HEX", 0, "Set all Verbosity Flags with hex value", 0},	
  	{"mctp-verbosity",'Z', "HEX", OPTION_HIDDEN, "Set all MCTP Verbosity Flags with hex value", 0},	
  	{"no-init",		  'N', NULL,  OPTION_HIDDEN, "Do not initialize local state at start up", 0},	
  	{"print-options", 706,  NULL, OPTION_HIDDEN, "Print CLI Options", 0},	
	{0,0,0,0, "Help Options", 9}, // Group
  	{"help",    'h', NULL, 0, "Display Help", 0},
  	{"usage",   701, NULL, 0, "Display Usage", 0},	
  	{"version", 702, NULL, 0, "Display Version", 0},
	{0,0,0,0,0,0} // Final option should be all null
};

/**
 * CLAP_LD - Options for: <app> ld
 */
struct argp_option ao_ld[] = 	
{
	{0,0,0,0,"Command Options",1}, 

  	{"ppid", 	  'p', "INT", OPTION_HIDDEN, "Physical Port ID", 0},
	
	{0,0,0,0, "Networking Options",7},	
  	{"tcp-port",  'P', "INT", 0, "Server TCP Port", 0},
  	{"tcp-address", 'T', "INT", 0, "Server TCP Address", 0},
	{0,0,0,0, "Verbose Options",8},	// Group
  	{"verbosity",     'V', "INT", 0, "Set Verbosity Flag", 0},	
  	{"verbosity-hex", 'X', "HEX", 0, "Set all Verbosity Flags with hex value", 0},	
  	{"mctp-verbosity",'Z', "HEX", OPTION_HIDDEN, "Set all MCTP Verbosity Flags with hex value", 0},	
  	{"no-init",		  'N', NULL,  OPTION_HIDDEN, "Do not initialize local state at start up", 0},	
  	{"print-options", 706,  NULL, OPTION_HIDDEN, "Print CLI Options", 0},	
	{0,0,0,0, "Help Options", 9}, // Group
  	{"help",    'h', NULL, 0, "Display Help", 0},
  	{"usage",   701, NULL, 0, "Display Usage", 0},	
  	{"version", 702, NULL, 0, "Display Version", 0},
	{0,0,0,0,0,0} // Final option should be all null
};

/**
 * CLAP_AER - Options for: <app> aer
 */
struct argp_option ao_aer[] = 	
{

	{0,0,0,0,"Command Options",1}, 
  	{"error",     'e', "HEX", 0, "AER Error (4 Byte HEX)", 0},
  	{"tlp-header",'t', "STR", 0, "AER TLP Header (32 Byte HEX String)", 0},

	{0,0,0,0,"Target Options",3}, 
  	{"vcsid",     'c', "INT", 0, "Virtual CXL Switch ID", 0},
  	{"vppbid",    'b', "INT", 0, "Virtual PCIe-to-PCIe Bridge ID", 0},

	{0,0,0,0, "Networking Options",7},	
  	{"tcp-port",  'P', "INT", 0, "Server TCP Port", 0},
  	{"tcp-address", 'T', "INT", 0, "Server TCP Address", 0},
	{0,0,0,0, "Verbose Options",8},	// Group
  	{"verbosity",     'V', "INT", 0, "Set Verbosity Flag", 0},	
  	{"verbosity-hex", 'X', "HEX", 0, "Set all Verbosity Flags with hex value", 0},	
  	{"mctp-verbosity",'Z', "HEX", OPTION_HIDDEN, "Set all MCTP Verbosity Flags with hex value", 0},	
  	{"no-init",		  'N', NULL,  OPTION_HIDDEN, "Do not initialize local state at start up", 0},	
  	{"print-options", 706,  NULL, OPTION_HIDDEN, "Print CLI Options", 0},	
	{0,0,0,0, "Help Options", 9}, // Group
  	{"help",    'h', NULL, 0, "Display Help", 0},
  	{"usage",   701, NULL, 0, "Display Usage", 0},	
  	{"version", 702, NULL, 0, "Display Version", 0},
	{0,0,0,0,0,0} // Final option should be all null
};

/**
 * CLAP_SHOW_BOS - Options for: <app> show bos
 */
struct argp_option ao_show_bos[] = 	
{
	{0,0,0,0,"Command Options",1}, // Group

	{0,0,0,0, "Networking Options",7},	
  	{"tcp-port",  'P', "INT", 0, "Server TCP Port", 0},
  	{"tcp-address", 'T', "INT", 0, "Server TCP Address", 0},
	{0,0,0,0, "Verbose Options",8},	// Group
  	{"verbosity",     'V', "INT", 0, "Set Verbosity Flag", 0},	
  	{"verbosity-hex", 'X', "HEX", 0, "Set all Verbosity Flags with hex value", 0},	
  	{"mctp-verbosity",'Z', "HEX", OPTION_HIDDEN, "Set all MCTP Verbosity Flags with hex value", 0},	
  	{"no-init",		  'N', NULL,  OPTION_HIDDEN, "Do not initialize local state at start up", 0},	
  	{"print-options", 706,  NULL, OPTION_HIDDEN, "Print CLI Options", 0},	
	{0,0,0,0, "Help Options", 9}, // Group
  	{"help",    'h', NULL, 0, "Display Help", 0},
  	{"usage",   701, NULL, 0, "Display Usage", 0},	
  	{"version", 702, NULL, 0, "Display Version", 0},
	{0,0,0,0,0,0} // Final option should be all null
};

/**
 * CLAP_SHOW_ID - Options for: <app> show identiy
 */
struct argp_option ao_show_identity[] = 	
{
	{0,0,0,0,"Command Options",1}, // Group

	{0,0,0,0, "Networking Options",7},	
  	{"tcp-port",  'P', "INT", 0, "Server TCP Port", 0},
  	{"tcp-address", 'T', "INT", 0, "Server TCP Address", 0},
	{0,0,0,0, "Verbose Options",8},	// Group
  	{"verbosity",     'V', "INT", 0, "Set Verbosity Flag", 0},	
  	{"verbosity-hex", 'X', "HEX", 0, "Set all Verbosity Flags with hex value", 0},	
  	{"mctp-verbosity",'Z', "HEX", OPTION_HIDDEN, "Set all MCTP Verbosity Flags with hex value", 0},	
  	{"no-init",		  'N', NULL,  OPTION_HIDDEN, "Do not initialize local state at start up", 0},	
  	{"print-options", 706,  NULL, OPTION_HIDDEN, "Print CLI Options", 0},	
	{0,0,0,0, "Help Options", 9}, // Group
  	{"help",    'h', NULL, 0, "Display Help", 0},
  	{"usage",   701, NULL, 0, "Display Usage", 0},	
  	{"version", 702, NULL, 0, "Display Version", 0},
	{0,0,0,0,0,0} // Final option should be all null
};

/**
 * CLAP_SHOW_MSG_LIMIT - Options for: <app> show limit
 */
struct argp_option ao_show_limit[] = 	
{
	{0,0,0,0,"Command Options",1}, // Group

	{0,0,0,0, "Networking Options",7},	
  	{"tcp-port",  'P', "INT", 0, "Server TCP Port", 0},
  	{"tcp-address", 'T', "INT", 0, "Server TCP Address", 0},
	{0,0,0,0, "Verbose Options",8},	// Group
  	{"verbosity",     'V', "INT", 0, "Set Verbosity Flag", 0},	
  	{"verbosity-hex", 'X', "HEX", 0, "Set all Verbosity Flags with hex value", 0},	
  	{"mctp-verbosity",'Z', "HEX", OPTION_HIDDEN, "Set all MCTP Verbosity Flags with hex value", 0},	
  	{"no-init",		  'N', NULL,  OPTION_HIDDEN, "Do not initialize local state at start up", 0},	
  	{"print-options", 706,  NULL, OPTION_HIDDEN, "Print CLI Options", 0},	
	{0,0,0,0, "Help Options", 9}, // Group
  	{"help",    'h', NULL, 0, "Display Help", 0},
  	{"usage",   701, NULL, 0, "Display Usage", 0},	
  	{"version", 702, NULL, 0, "Display Version", 0},
	{0,0,0,0,0,0} // Final option should be all null
};

/**
 * CLAP_SHOW_SWITCH - Options for: <app> show switch
 */
struct argp_option ao_show_switch[] = 	
{
	{0,0,0,0,"Command Options",1}, // Group

	{0,0,0,0, "Networking Options",7},	
  	{"tcp-port",  'P', "INT", 0, "Server TCP Port", 0},
  	{"tcp-address", 'T', "INT", 0, "Server TCP Address", 0},
	{0,0,0,0, "Verbose Options",8},	// Group
  	{"verbosity",     'V', "INT", 0, "Set Verbosity Flag", 0},	
  	{"verbosity-hex", 'X', "HEX", 0, "Set all Verbosity Flags with hex value", 0},	
  	{"mctp-verbosity",'Z', "HEX", OPTION_HIDDEN, "Set all MCTP Verbosity Flags with hex value", 0},	
  	{"no-init",		  'N', NULL,  OPTION_HIDDEN, "Do not initialize local state at start up", 0},	
  	{"print-options", 706,  NULL, OPTION_HIDDEN, "Print CLI Options", 0},	
	{0,0,0,0, "Help Options", 9}, // Group
  	{"help",    'h', NULL, 0, "Display Help", 0},
  	{"usage",   701, NULL, 0, "Display Usage", 0},	
  	{"version", 702, NULL, 0, "Display Version", 0},
	{0,0,0,0,0,0} // Final option should be all null
};

/**
 * CLAP_SHOW_DEV - Options for: <app> show device
 */
struct argp_option ao_show_dev[] = 	
{
	{0,0,0,0,"Command Options",1}, // Group

	{0,0,0,0,"Target Options",3}, 
  	{"all",    'a',  NULL, 0, "All Devices", 0},	
  	{"dev",    'd', "INT", 0, "Device Profile ID", 0},

	{0,0,0,0, "Networking Options",7},	
  	{"tcp-port",  'P', "INT", 0, "Server TCP Port", 0},
  	{"tcp-address", 'T', "INT", 0, "Server TCP Address", 0},
	{0,0,0,0, "Verbose Options",8},	// Group
  	{"verbosity",     'V', "INT", 0, "Set Verbosity Flag", 0},	
  	{"verbosity-hex", 'X', "HEX", 0, "Set all Verbosity Flags with hex value", 0},	
  	{"mctp-verbosity",'Z', "HEX", OPTION_HIDDEN, "Set all MCTP Verbosity Flags with hex value", 0},	
  	{"no-init",		  'N', NULL,  OPTION_HIDDEN, "Do not initialize local state at start up", 0},	
  	{"print-options", 706,  NULL, OPTION_HIDDEN, "Print CLI Options", 0},	
	{0,0,0,0, "Help Options", 9}, // Group
  	{"help",    'h', NULL, 0, "Display Help", 0},
  	{"usage",   701, NULL, 0, "Display Usage", 0},	
  	{"version", 702, NULL, 0, "Display Version", 0},
	{0,0,0,0,0,0} // Final option should be all null
};

/**
 * CLAP_SHOW_PORT - Options for: <app> show port
 */
struct argp_option ao_show_port[] = 	
{
	{0,0,0,0,"Command Options",1}, // Group

	{0,0,0,0,"Target Options",3}, 
  	{"all",   'a',  NULL, 0, "All Physical Ports", 0},	
  	{"ppid",  'p', "INT", 0, "Physical Port ID", 0},

	{0,0,0,0, "Networking Options",7},	
  	{"tcp-port",  'P', "INT", 0, "Server TCP Port", 0},
  	{"tcp-address", 'T', "INT", 0, "Server TCP Address", 0},
	{0,0,0,0, "Verbose Options",8},	// Group
  	{"verbosity",     'V', "INT", 0, "Set Verbosity Flag", 0},	
  	{"verbosity-hex", 'X', "HEX", 0, "Set all Verbosity Flags with hex value", 0},	
  	{"mctp-verbosity",'Z', "HEX", OPTION_HIDDEN, "Set all MCTP Verbosity Flags with hex value", 0},	
  	{"no-init",		  'N', NULL,  OPTION_HIDDEN, "Do not initialize local state at start up", 0},	
  	{"print-options", 706,  NULL, OPTION_HIDDEN, "Print CLI Options", 0},	
	{0,0,0,0, "Help Options", 9}, // Group
  	{"help",    'h', NULL, 0, "Display Help", 0},
  	{"usage",   701, NULL, 0, "Display Usage", 0},	
  	{"version", 702, NULL, 0, "Display Version", 0},
	{0,0,0,0,0,0} // Final option should be all null
};

/**
 * CLAP_SHOW_VCS - Options for: <app> show vcs
 */
struct argp_option ao_show_vcs[] = 	
{
	{0,0,0,0,"Command Options",1}, // Group

	{0,0,0,0,"Target Options",3}, 
  	{"all",     'a',  NULL, 0, "All Virtual CXL Switches", 0},	
  	{"vcsid",   'c', "INT", 0, "Virtual CXL Switch ID", 0},

	{0,0,0,0, "Networking Options",7},	
  	{"tcp-port",  'P', "INT", 0, "Server TCP Port", 0},
  	{"tcp-address", 'T', "INT", 0, "Server TCP Address", 0},
	{0,0,0,0, "Verbose Options",8},	// Group
  	{"verbosity",     'V', "INT", 0, "Set Verbosity Flag", 0},	
  	{"verbosity-hex", 'X', "HEX", 0, "Set all Verbosity Flags with hex value", 0},	
  	{"mctp-verbosity",'Z', "HEX", OPTION_HIDDEN, "Set all MCTP Verbosity Flags with hex value", 0},	
  	{"no-init",		  'N', NULL,  OPTION_HIDDEN, "Do not initialize local state at start up", 0},	
  	{"print-options", 706,  NULL, OPTION_HIDDEN, "Print CLI Options", 0},	
	{0,0,0,0, "Help Options", 9}, // Group
  	{"help",    'h', NULL, 0, "Display Help", 0},
  	{"usage",   701, NULL, 0, "Display Usage", 0},	
  	{"version", 702, NULL, 0, "Display Version", 0},
	{0,0,0,0,0,0} // Final option should be all null
};

/**
 * CLAP_SHOW_QOS - Options for: <app> show qos
 */
struct argp_option ao_show_qos[] = 	
{
	{0,0,0,0,"Command Options",1}, // Group

  	{"ppid", 	  'p', "INT", OPTION_HIDDEN, "Physical Port ID", 0},
	
	{0,0,0,0, "Networking Options",7},	
  	{"tcp-port",  'P', "INT", 0, "Server TCP Port", 0},
  	{"tcp-address", 'T', "INT", 0, "Server TCP Address", 0},
	{0,0,0,0, "Verbose Options",8},	// Group
  	{"verbosity",     'V', "INT", 0, "Set Verbosity Flag", 0},	
  	{"verbosity-hex", 'X', "HEX", 0, "Set all Verbosity Flags with hex value", 0},	
  	{"mctp-verbosity",'Z', "HEX", OPTION_HIDDEN, "Set all MCTP Verbosity Flags with hex value", 0},	
  	{"no-init",		  'N', NULL,  OPTION_HIDDEN, "Do not initialize local state at start up", 0},	
  	{"print-options", 706,  NULL, OPTION_HIDDEN, "Print CLI Options", 0},	
	{0,0,0,0, "Help Options", 9}, // Group
  	{"help",    'h', NULL, 0, "Display Help", 0},
  	{"usage",   701, NULL, 0, "Display Usage", 0},	
  	{"version", 702, NULL, 0, "Display Version", 0},
	{0,0,0,0,0,0} // Final option should be all null
};

/**
 * CLAP_SHOW_LD - Options for: <app> show ld
 */
struct argp_option ao_show_ld[] = 	
{
	{0,0,0,0,"Command Options",1}, // Group

  	{"ppid", 	  'p', "INT", OPTION_HIDDEN, "Physical Port ID", 0},
	
	{0,0,0,0, "Networking Options",7},	
  	{"tcp-port",  'P', "INT", 0, "Server TCP Port", 0},
  	{"tcp-address", 'T', "INT", 0, "Server TCP Address", 0},
	{0,0,0,0, "Verbose Options",8},	// Group
  	{"verbosity",     'V', "INT", 0, "Set Verbosity Flag", 0},	
  	{"verbosity-hex", 'X', "HEX", 0, "Set all Verbosity Flags with hex value", 0},	
  	{"mctp-verbosity",'Z', "HEX", OPTION_HIDDEN, "Set all MCTP Verbosity Flags with hex value", 0},	
  	{"no-init",		  'N', NULL,  OPTION_HIDDEN, "Do not initialize local state at start up", 0},	
  	{"print-options", 706,  NULL, OPTION_HIDDEN, "Print CLI Options", 0},	
	{0,0,0,0, "Help Options", 9}, // Group
  	{"help",    'h', NULL, 0, "Display Help", 0},
  	{"usage",   701, NULL, 0, "Display Usage", 0},	
  	{"version", 702, NULL, 0, "Display Version", 0},
	{0,0,0,0,0,0} // Final option should be all null
};

/**
 * CLAP_PORT_BIND - Options for: <app> port bind
 */
struct argp_option ao_port_bind[] = 	
{
	{0,0,0,0,"Command Options",1}, 

	{0,0,0,0,"Target Options",3}, 
  	{"vcsid", 'c', "INT", 0, "Virtual CXL Switch ID", 0},
  	{"vppbid",'b', "INT", 0, "Virtual PCIe-to-PCIe Bridge ID", 0},
  	{"ppid",  'p', "INT", 0, "Physical Port ID", 0},
  	{"ldid",  'l', "INT", 0, "LD-ID (for MLD devices)", 0},

	{0,0,0,0, "Networking Options",7},	
  	{"tcp-port",  'P', "INT", 0, "Server TCP Port", 0},
  	{"tcp-address", 'T', "INT", 0, "Server TCP Address", 0},
	{0,0,0,0, "Verbose Options",8},	// Group
  	{"verbosity",     'V', "INT", 0, "Set Verbosity Flag", 0},	
  	{"verbosity-hex", 'X', "HEX", 0, "Set all Verbosity Flags with hex value", 0},	
  	{"mctp-verbosity",'Z', "HEX", OPTION_HIDDEN, "Set all MCTP Verbosity Flags with hex value", 0},	
  	{"no-init",		  'N', NULL,  OPTION_HIDDEN, "Do not initialize local state at start up", 0},	
  	{"print-options", 706,  NULL, OPTION_HIDDEN, "Print CLI Options", 0},	
	{0,0,0,0, "Help Options", 9}, // Group
  	{"help",    'h', NULL, 0, "Display Help", 0},
  	{"usage",   701, NULL, 0, "Display Usage", 0},	
  	{"version", 702, NULL, 0, "Display Version", 0},
	{0,0,0,0,0,0} // Final option should be all null
};

/**
 * CLAP_PORT_UNBIND - Options for: <app> port unbind
 */
struct argp_option ao_port_unbind[] = 	
{
	{0,0,0,0,"Command Options",1}, 
  	{"wait",    'w',     0, 0, "Wait for port link down before unbinding", 0},
  	{"managed", 'm',     0, 0, "Simulate Managed Hot-Remove", 0},
  	{"surprise",'s',     0, 0, "Simulate Surpise Hot-Remove", 0},

	{0,0,0,0,"Target Options",3}, 
  	{"vcsid",   'c', "INT", 0, "Virtual CXL Switch ID", 0},
  	{"vppbid",  'b', "INT", 0, "Virtual PCIe-to-PCIe Bridge ID", 0},

	{0,0,0,0, "Networking Options",7},	
  	{"tcp-port",  'P', "INT", 0, "Server TCP Port", 0},
  	{"tcp-address", 'T', "INT", 0, "Server TCP Address", 0},
	{0,0,0,0, "Verbose Options",8},	// Group
  	{"verbosity",     'V', "INT", 0, "Set Verbosity Flag", 0},	
  	{"verbosity-hex", 'X', "HEX", 0, "Set all Verbosity Flags with hex value", 0},	
  	{"mctp-verbosity",'Z', "HEX", OPTION_HIDDEN, "Set all MCTP Verbosity Flags with hex value", 0},	
  	{"no-init",		  'N', NULL,  OPTION_HIDDEN, "Do not initialize local state at start up", 0},	
  	{"print-options", 706,  NULL, OPTION_HIDDEN, "Print CLI Options", 0},	
	{0,0,0,0, "Help Options", 9}, // Group
  	{"help",    'h', NULL, 0, "Display Help", 0},
  	{"usage",   701, NULL, 0, "Display Usage", 0},	
  	{"version", 702, NULL, 0, "Display Version", 0},
	{0,0,0,0,0,0} // Final option should be all null
};

/**
 * CLAP_PORT_CONFIG - Options for: <app> port config
 */
struct argp_option ao_port_config[] = 	
{
	{0,0,0,0,"Command Options",1}, 
  	{"register",     'r', "INT", 0, "Register Number", 0},
  	{"ext-register", 'e', "INT", 0, "Extended Register Number", 0},
  	{"fdbe",         'f', "INT", 0, "First Dword Byte Enable", 0},

	{0,0,0,0,"Write Options",2}, 
  	{"write",        'w',   NULL, 0, "Perform a Write transaction", 0},
  	{"data",         703,  "HEX", 0, "Write Data (up to 4 bytes)", 0},

	{0,0,0,0,"Target Options",3}, 
  	{"ppid",         'p', "INT", 0, "Physical Port ID", 0},

	{0,0,0,0, "Networking Options",7},	
  	{"tcp-port",  'P', "INT", 0, "Server TCP Port", 0},
  	{"tcp-address", 'T', "INT", 0, "Server TCP Address", 0},
	{0,0,0,0, "Verbose Options",8},	// Group
  	{"verbosity",     'V', "INT", 0, "Set Verbosity Flag", 0},	
  	{"verbosity-hex", 'X', "HEX", 0, "Set all Verbosity Flags with hex value", 0},	
  	{"mctp-verbosity",'Z', "HEX", OPTION_HIDDEN, "Set all MCTP Verbosity Flags with hex value", 0},	
  	{"no-init",		  'N', NULL,  OPTION_HIDDEN, "Do not initialize local state at start up", 0},	
  	{"print-options", 706,  NULL, OPTION_HIDDEN, "Print CLI Options", 0},	
	{0,0,0,0, "Help Options", 9}, // Group
  	{"help",    'h', NULL, 0, "Display Help", 0},
  	{"usage",   701, NULL, 0, "Display Usage", 0},	
  	{"version", 702, NULL, 0, "Display Version", 0},
	{0,0,0,0,0,0} // Final option should be all null
};

/**
 * CLAP_PORT_CONN - Options for: <app> port connect
 */
struct argp_option ao_port_connect[] = 	
{
	{0,0,0,0,"Command Options",1}, 
  	{"dev",          'd', "INT", 0, "Device Profile ID", 0},

	{0,0,0,0,"Target Options",3}, 
  	{"ppid",         'p', "INT", 0, "Physical Port ID", 0},

	{0,0,0,0, "Networking Options",7},	
  	{"tcp-port",  'P', "INT", 0, "Server TCP Port", 0},
  	{"tcp-address", 'T', "INT", 0, "Server TCP Address", 0},
	{0,0,0,0, "Verbose Options",8},	// Group
  	{"verbosity",     'V', "INT", 0, "Set Verbosity Flag", 0},	
  	{"verbosity-hex", 'X', "HEX", 0, "Set all Verbosity Flags with hex value", 0},	
  	{"mctp-verbosity",'Z', "HEX", OPTION_HIDDEN, "Set all MCTP Verbosity Flags with hex value", 0},	
  	{"no-init",		  'N', NULL,  OPTION_HIDDEN, "Do not initialize local state at start up", 0},	
  	{"print-options", 706,  NULL, OPTION_HIDDEN, "Print CLI Options", 0},	
	{0,0,0,0, "Help Options", 9}, // Group
  	{"help",    'h', NULL, 0, "Display Help", 0},
  	{"usage",   701, NULL, 0, "Display Usage", 0},	
  	{"version", 702, NULL, 0, "Display Version", 0},
	{0,0,0,0,0,0} // Final option should be all null
};

/**
 * CLAP_PORT_DISCONN - Options for: <app> port disconnect
 */
struct argp_option ao_port_disconnect[] = 	
{
	{0,0,0,0,"Command Options",1}, 

	{0,0,0,0,"Target Options",3}, 
  	{"all",   		 'a',  NULL, 0, "All Devices", 0},	
  	{"ppid",         'p', "INT", 0, "Physical Port ID", 0},

	{0,0,0,0, "Networking Options",7},	
  	{"tcp-port",  'P', "INT", 0, "Server TCP Port", 0},
  	{"tcp-address", 'T', "INT", 0, "Server TCP Address", 0},
	{0,0,0,0, "Verbose Options",8},	// Group
  	{"verbosity",     'V', "INT", 0, "Set Verbosity Flag", 0},	
  	{"verbosity-hex", 'X', "HEX", 0, "Set all Verbosity Flags with hex value", 0},	
  	{"mctp-verbosity",'Z', "HEX", OPTION_HIDDEN, "Set all MCTP Verbosity Flags with hex value", 0},	
  	{"no-init",		  'N', NULL,  OPTION_HIDDEN, "Do not initialize local state at start up", 0},	
  	{"print-options", 706,  NULL, OPTION_HIDDEN, "Print CLI Options", 0},	
	{0,0,0,0, "Help Options", 9}, // Group
  	{"help",    'h', NULL, 0, "Display Help", 0},
  	{"usage",   701, NULL, 0, "Display Usage", 0},	
  	{"version", 702, NULL, 0, "Display Version", 0},
	{0,0,0,0,0,0} // Final option should be all null
};

/**
 * CLAP_PORT_CTRL - Options for: <app> port control
 */
struct argp_option ao_port_ctrl[] = 	
{
	{0,0,0,0,"Command Options",1}, 
  	{"assert-perst",   'a',  NULL, 0, "Assert PERST", 0},
  	{"deassert-perst", 'd',  NULL, 0, "Deassert PERST", 0},
  	{"reset",          'r',  NULL, 0, "Reset PCIe-to-PCIe Bridge", 0},

	{0,0,0,0,"Target Options",3}, 
  	{"ppid",           'p', "INT", 0, "Physical Port ID", 0},

	{0,0,0,0, "Networking Options",7},	
  	{"tcp-port",  'P', "INT", 0, "Server TCP Port", 0},
  	{"tcp-address", 'T', "INT", 0, "Server TCP Address", 0},
	{0,0,0,0, "Verbose Options",8},	// Group
  	{"verbosity",     'V', "INT", 0, "Set Verbosity Flag", 0},	
  	{"verbosity-hex", 'X', "HEX", 0, "Set all Verbosity Flags with hex value", 0},	
  	{"mctp-verbosity",'Z', "HEX", OPTION_HIDDEN, "Set all MCTP Verbosity Flags with hex value", 0},	
  	{"no-init",		  'N', NULL,  OPTION_HIDDEN, "Do not initialize local state at start up", 0},	
  	{"print-options", 706,  NULL, OPTION_HIDDEN, "Print CLI Options", 0},	
	{0,0,0,0, "Help Options", 9}, // Group
  	{"help",    'h', NULL, 0, "Display Help", 0},
  	{"usage",   701, NULL, 0, "Display Usage", 0},	
  	{"version", 702, NULL, 0, "Display Version", 0},
	{0,0,0,0,0,0} // Final option should be all null
};

/**
 * CLAP_SET_LD - Options for: <app> set ld
 */
struct argp_option ao_set_ld[] = 	
{
	{0,0,0,0,"Command Options",1}, 

  	{"ppid", 	  'p', "INT", OPTION_HIDDEN, "Physical Port ID", 0},
	
	{0,0,0,0, "Networking Options",7},	
  	{"tcp-port",  'P', "INT", 0, "Server TCP Port", 0},
  	{"tcp-address", 'T', "INT", 0, "Server TCP Address", 0},
	{0,0,0,0, "Verbose Options",8},	// Group
  	{"verbosity",     'V', "INT", 0, "Set Verbosity Flag", 0},	
  	{"verbosity-hex", 'X', "HEX", 0, "Set all Verbosity Flags with hex value", 0},	
  	{"mctp-verbosity",'Z', "HEX", OPTION_HIDDEN, "Set all MCTP Verbosity Flags with hex value", 0},	
  	{"no-init",		  'N', NULL,  OPTION_HIDDEN, "Do not initialize local state at start up", 0},	
  	{"print-options", 706,  NULL, OPTION_HIDDEN, "Print CLI Options", 0},	
	{0,0,0,0, "Help Options", 9}, // Group
  	{"help",    'h', NULL, 0, "Display Help", 0},
  	{"usage",   701, NULL, 0, "Display Usage", 0},	
  	{"version", 702, NULL, 0, "Display Version", 0},
	{0,0,0,0,0,0} // Final option should be all null
};

/**
 * CLAP_SET_MSG_LIMIT - Options for: <app> set limit
 */
struct argp_option ao_set_limit[] = 	
{
	{0,0,0,0,"Command Options",1}, 
  	{"limit", 	  'n', "INT", 0, "Response Message Limit (n of 2^n) [8-20]", 0},
	
	{0,0,0,0, "Networking Options",7},	
  	{"tcp-port",  'P', "INT", 0, "Server TCP Port", 0},
  	{"tcp-address", 'T', "INT", 0, "Server TCP Address", 0},
	{0,0,0,0, "Verbose Options",8},	// Group
  	{"verbosity",     'V', "INT", 0, "Set Verbosity Flag", 0},	
  	{"verbosity-hex", 'X', "HEX", 0, "Set all Verbosity Flags with hex value", 0},	
  	{"mctp-verbosity",'Z', "HEX", OPTION_HIDDEN, "Set all MCTP Verbosity Flags with hex value", 0},	
  	{"no-init",		  'N', NULL,  OPTION_HIDDEN, "Do not initialize local state at start up", 0},	
  	{"print-options", 706,  NULL, OPTION_HIDDEN, "Print CLI Options", 0},	
	{0,0,0,0, "Help Options", 9}, // Group
  	{"help",    'h', NULL, 0, "Display Help", 0},
  	{"usage",   701, NULL, 0, "Display Usage", 0},	
  	{"version", 702, NULL, 0, "Display Version", 0},
	{0,0,0,0,0,0} // Final option should be all null
};

/**
 * CLAP_SET_QOS - Options for: <app> set qos
 */
struct argp_option ao_set_qos[] = 	
{
	{0,0,0,0,"Command Options",1}, 

  	{"ppid", 	  'p', "INT", OPTION_HIDDEN, "Physical Port ID", 0},
	
	{0,0,0,0, "Networking Options",7},	
  	{"tcp-port",  'P', "INT", 0, "Server TCP Port", 0},
  	{"tcp-address", 'T', "INT", 0, "Server TCP Address", 0},
	{0,0,0,0, "Verbose Options",8},	// Group
  	{"verbosity",     'V', "INT", 0, "Set Verbosity Flag", 0},	
  	{"verbosity-hex", 'X', "HEX", 0, "Set all Verbosity Flags with hex value", 0},	
  	{"mctp-verbosity",'Z', "HEX", OPTION_HIDDEN, "Set all MCTP Verbosity Flags with hex value", 0},	
  	{"no-init",		  'N', NULL,  OPTION_HIDDEN, "Do not initialize local state at start up", 0},	
  	{"print-options", 706,  NULL, OPTION_HIDDEN, "Print CLI Options", 0},	
	{0,0,0,0, "Help Options", 9}, // Group
  	{"help",    'h', NULL, 0, "Display Help", 0},
  	{"usage",   701, NULL, 0, "Display Usage", 0},	
  	{"version", 702, NULL, 0, "Display Version", 0},
	{0,0,0,0,0,0} // Final option should be all null
};

/**
 * CLAP_LD_CONFIG - Options for: <app> ld config
 */
struct argp_option ao_ld_config[] = 	
{
	{0,0,0,0,"Command Options",1}, 
  	{"register",     'r',  "INT", 0, "Register Number", 0},
  	{"ext-register", 'e',  "INT", 0, "Extended Register Number", 0},
  	{"fdbe",         'f',  "INT", 0, "First Dword Byte Enable", 0},

	{0,0,0,0,"Write Options",2}, 
  	{"write",        'w',   NULL, 0, "Perform a Write transaction", 0},
  	{"data",         703,  "HEX", 0, "Write Data (up to 4 bytes)", 0},

	{0,0,0,0,"Target Options",3}, 
  	{"ppid",         'p',  "INT", 0, "Physical Port ID", 0},
  	{"ldid",         'l',  "INT", 0, "LD-ID (for MLD devices)", 0},

	{0,0,0,0, "Networking Options",7},	
  	{"tcp-port",  'P', "INT", 0, "Server TCP Port", 0},
  	{"tcp-address", 'T', "INT", 0, "Server TCP Address", 0},
	{0,0,0,0, "Verbose Options",8},	// Group
  	{"verbosity",     'V', "INT", 0, "Set Verbosity Flag", 0},	
  	{"verbosity-hex", 'X', "HEX", 0, "Set all Verbosity Flags with hex value", 0},	
  	{"mctp-verbosity",'Z', "HEX", OPTION_HIDDEN, "Set all MCTP Verbosity Flags with hex value", 0},	
  	{"no-init",		  'N', NULL,  OPTION_HIDDEN, "Do not initialize local state at start up", 0},	
  	{"print-options", 706,  NULL, OPTION_HIDDEN, "Print CLI Options", 0},	
	{0,0,0,0, "Help Options", 9}, // Group
  	{"help",    'h', NULL, 0, "Display Help", 0},
  	{"usage",   701, NULL, 0, "Display Usage", 0},	
  	{"version", 702, NULL, 0, "Display Version", 0},
	{0,0,0,0,0,0} // Final option should be all null
};

/**
 * CLAP_LD_MEM - Options for: <app> ld mem
 */
struct argp_option ao_ld_mem[] = 	
{
	{0,0,0,0,"Command Options",1}, 
  	{"fdbe",    'f',   "INT", 0, "First Dword Byte Enable", 0},
  	{"ldbe",    'd',   "INT", 0, "Last Dword Byte Enable", 0},
  	{"length",  'n',   "INT", 0, "Transaction Data Length (up to 4KB)", 0},
  	{"offset",  'o',   "INT", 0, "Transaction Offset in tareget's memory space", 0},
  	{"write",   'w',   NULL,  0, "Perform a Write transaction", 0},
  	{"data",    703,   "HEX", 0, "Write Data (up to 4 bytes)", 0},
  	{"infile",  704,  "FILE", 0, "Filename for input data", 0},

	{0,0,0,0,"Target Options",3}, 
  	{"ppid",    'p',   "INT", 0, "Physical Port ID", 0},
  	{"ldid",    'l',   "INT", 0, "LD-ID (for MLD devices)", 0},

	{0,0,0,0, "Networking Options",7},	
  	{"tcp-port",  'P', "INT", 0, "Server TCP Port", 0},
  	{"tcp-address", 'T', "INT", 0, "Server TCP Address", 0},
	{0,0,0,0, "Verbose Options",8},	// Group
  	{"verbosity",     'V', "INT", 0, "Set Verbosity Flag", 0},	
  	{"verbosity-hex", 'X', "HEX", 0, "Set all Verbosity Flags with hex value", 0},	
  	{"mctp-verbosity",'Z', "HEX", OPTION_HIDDEN, "Set all MCTP Verbosity Flags with hex value", 0},	
  	{"no-init",		  'N', NULL,  OPTION_HIDDEN, "Do not initialize local state at start up", 0},	
  	{"print-options", 706,  NULL, OPTION_HIDDEN, "Print CLI Options", 0},	
	{0,0,0,0, "Help Options", 9}, // Group
  	{"help",    'h', NULL, 0, "Display Help", 0},
  	{"usage",   701, NULL, 0, "Display Usage", 0},	
  	{"version", 702, NULL, 0, "Display Version", 0},
	{0,0,0,0,0,0} // Final option should be all null
};

/**
 * CLAP_SHOW_QOS_ALLOCATED - Options for: <app> show qos allocated
 */
struct argp_option ao_show_qos_allocated[] = 	
{
	{0,0,0,0,"Command Options",1}, // Group

	{0,0,0,0,"Target Options",3}, 
  	{"ppid",  'p', "INT", 0, "Physical Port ID", 0},
  	{"ldid",  'l', "INT", 0, "Starting LD-ID (for MLD devices)", 0},
  	{"num",   'n', "INT", 0, "Num LD IDs Requested", 0},

	{0,0,0,0, "Networking Options",7},	
  	{"tcp-port",  'P', "INT", 0, "Server TCP Port", 0},
  	{"tcp-address", 'T', "INT", 0, "Server TCP Address", 0},
	{0,0,0,0, "Verbose Options",8},	// Group
  	{"verbosity",     'V', "INT", 0, "Set Verbosity Flag", 0},	
  	{"verbosity-hex", 'X', "HEX", 0, "Set all Verbosity Flags with hex value", 0},	
  	{"mctp-verbosity",'Z', "HEX", OPTION_HIDDEN, "Set all MCTP Verbosity Flags with hex value", 0},	
  	{"no-init",		  'N', NULL,  OPTION_HIDDEN, "Do not initialize local state at start up", 0},	
  	{"print-options", 706,  NULL, OPTION_HIDDEN, "Print CLI Options", 0},	
	{0,0,0,0, "Help Options", 9}, // Group
  	{"help",    'h', NULL, 0, "Display Help", 0},
  	{"usage",   701, NULL, 0, "Display Usage", 0},	
  	{"version", 702, NULL, 0, "Display Version", 0},
	{0,0,0,0,0,0} // Final option should be all null
};

/**
 * CLAP_SHOW_QOS_CONTROL - Options for: <app> show qos control
 */
struct argp_option ao_show_qos_control[] = 	
{
	{0,0,0,0,"Command Options",1}, // Group

	{0,0,0,0,"Target Options",3}, 
  	{"ppid",  'p', "INT", 0, "Physical Port ID", 0},

	{0,0,0,0, "Networking Options",7},	
  	{"tcp-port",  'P', "INT", 0, "Server TCP Port", 0},
  	{"tcp-address", 'T', "INT", 0, "Server TCP Address", 0},
	{0,0,0,0, "Verbose Options",8},	// Group
  	{"verbosity",     'V', "INT", 0, "Set Verbosity Flag", 0},	
  	{"verbosity-hex", 'X', "HEX", 0, "Set all Verbosity Flags with hex value", 0},	
  	{"mctp-verbosity",'Z', "HEX", OPTION_HIDDEN, "Set all MCTP Verbosity Flags with hex value", 0},	
  	{"no-init",		  'N', NULL,  OPTION_HIDDEN, "Do not initialize local state at start up", 0},	
  	{"print-options", 706,  NULL, OPTION_HIDDEN, "Print CLI Options", 0},	
	{0,0,0,0, "Help Options", 9}, // Group
  	{"help",    'h', NULL, 0, "Display Help", 0},
  	{"usage",   701, NULL, 0, "Display Usage", 0},	
  	{"version", 702, NULL, 0, "Display Version", 0},
	{0,0,0,0,0,0} // Final option should be all null
};

/**
 * CLAP_SHOW_QOS_LIMIT - Options for: <app> show qos limit
 */
struct argp_option ao_show_qos_limit[] = 	
{
	{0,0,0,0,"Command Options",1}, // Group

	{0,0,0,0,"Target Options",3}, 
  	{"ppid",  'p', "INT", 0, "Physical Port ID", 0},
  	{"ldid",  'l', "INT", 0, "Starting LD-ID (for MLD devices)", 0},
  	{"num",   'n', "INT", 0, "Num LD IDs Requested", 0},

	{0,0,0,0, "Networking Options",7},	
  	{"tcp-port",  'P', "INT", 0, "Server TCP Port", 0},
  	{"tcp-address", 'T', "INT", 0, "Server TCP Address", 0},
	{0,0,0,0, "Verbose Options",8},	// Group
  	{"verbosity",     'V', "INT", 0, "Set Verbosity Flag", 0},	
  	{"verbosity-hex", 'X', "HEX", 0, "Set all Verbosity Flags with hex value", 0},	
  	{"mctp-verbosity",'Z', "HEX", OPTION_HIDDEN, "Set all MCTP Verbosity Flags with hex value", 0},	
  	{"no-init",		  'N', NULL,  OPTION_HIDDEN, "Do not initialize local state at start up", 0},	
  	{"print-options", 706,  NULL, OPTION_HIDDEN, "Print CLI Options", 0},	
	{0,0,0,0, "Help Options", 9}, // Group
  	{"help",    'h', NULL, 0, "Display Help", 0},
  	{"usage",   701, NULL, 0, "Display Usage", 0},	
  	{"version", 702, NULL, 0, "Display Version", 0},
	{0,0,0,0,0,0} // Final option should be all null
};

/**
 * CLAP_SHOW_QOS_STATUS - Options for: <app> show qos status
 */
struct argp_option ao_show_qos_status[] = 	
{
	{0,0,0,0,"Command Options",1}, // Group

	{0,0,0,0,"Target Options",3}, 
  	{"ppid",  'p', "INT", 0, "Physical Port ID", 0},

	{0,0,0,0, "Networking Options",7},	
  	{"tcp-port",  'P', "INT", 0, "Server TCP Port", 0},
  	{"tcp-address", 'T', "INT", 0, "Server TCP Address", 0},
	{0,0,0,0, "Verbose Options",8},	// Group
  	{"verbosity",     'V', "INT", 0, "Set Verbosity Flag", 0},	
  	{"verbosity-hex", 'X', "HEX", 0, "Set all Verbosity Flags with hex value", 0},	
  	{"mctp-verbosity",'Z', "HEX", OPTION_HIDDEN, "Set all MCTP Verbosity Flags with hex value", 0},	
  	{"no-init",		  'N', NULL,  OPTION_HIDDEN, "Do not initialize local state at start up", 0},	
  	{"print-options", 706,  NULL, OPTION_HIDDEN, "Print CLI Options", 0},	
	{0,0,0,0, "Help Options", 9}, // Group
  	{"help",    'h', NULL, 0, "Display Help", 0},
  	{"usage",   701, NULL, 0, "Display Usage", 0},	
  	{"version", 702, NULL, 0, "Display Version", 0},
	{0,0,0,0,0,0} // Final option should be all null
};

/**
 * CLAP_SHOW_LD_ALLOCATIONS - Options for: <app> show ld allocations
 */
struct argp_option ao_show_ld_allocations[] = 	
{
	{0,0,0,0,"Command Options",1}, // Group
	
	{0,0,0,0,"Target Options",3}, 
  	{"ppid",  'p', "INT", 0, "Physical Port ID", 0},

	{0,0,0,0, "Networking Options",7},	
  	{"tcp-port",  'P', "INT", 0, "Server TCP Port", 0},
  	{"tcp-address", 'T', "INT", 0, "Server TCP Address", 0},
	{0,0,0,0, "Verbose Options",8},	// Group
  	{"verbosity",     'V', "INT", 0, "Set Verbosity Flag", 0},	
  	{"verbosity-hex", 'X', "HEX", 0, "Set all Verbosity Flags with hex value", 0},	
  	{"mctp-verbosity",'Z', "HEX", OPTION_HIDDEN, "Set all MCTP Verbosity Flags with hex value", 0},	
  	{"no-init",		  'N', NULL,  OPTION_HIDDEN, "Do not initialize local state at start up", 0},	
  	{"print-options", 706,  NULL, OPTION_HIDDEN, "Print CLI Options", 0},	
	{0,0,0,0, "Help Options", 9}, // Group
  	{"help",    'h', NULL, 0, "Display Help", 0},
  	{"usage",   701, NULL, 0, "Display Usage", 0},	
  	{"version", 702, NULL, 0, "Display Version", 0},
	{0,0,0,0,0,0} // Final option should be all null
};

/**
 * CLAP_SHOW_LD_INFO - Options for: <app> show ld info
 */
struct argp_option ao_show_ld_info[] = 	
{
	{0,0,0,0,"Command Options",1}, // Group

	{0,0,0,0,"Target Options",3}, 
  	{"ppid",  'p', "INT", 0, "Physical Port ID", 0},

	{0,0,0,0, "Networking Options",7},	
  	{"tcp-port",  'P', "INT", 0, "Server TCP Port", 0},
  	{"tcp-address", 'T', "INT", 0, "Server TCP Address", 0},
	{0,0,0,0, "Verbose Options",8},	// Group
  	{"verbosity",     'V', "INT", 0, "Set Verbosity Flag", 0},	
  	{"verbosity-hex", 'X', "HEX", 0, "Set all Verbosity Flags with hex value", 0},	
  	{"mctp-verbosity",'Z', "HEX", OPTION_HIDDEN, "Set all MCTP Verbosity Flags with hex value", 0},	
  	{"no-init",		  'N', NULL,  OPTION_HIDDEN, "Do not initialize local state at start up", 0},	
  	{"print-options", 706,  NULL, OPTION_HIDDEN, "Print CLI Options", 0},	
	{0,0,0,0, "Help Options", 9}, // Group
  	{"help",    'h', NULL, 0, "Display Help", 0},
  	{"usage",   701, NULL, 0, "Display Usage", 0},	
  	{"version", 702, NULL, 0, "Display Version", 0},
	{0,0,0,0,0,0} // Final option should be all null
};

/**
 * CLAP_SET_LD_ALLOCATIONS - Options for: <app> set ld allocations
 */
struct argp_option ao_set_ld_allocations[] = 	
{
	{0,0,0,0,"Command Options",1}, 
  	{"range1", '1', "HEX", 0, "Range 1 Allocation Multipler list. e.g. 1,2,3,.,n", 0},
  	{"range2", '2', "HEX", 0, "Range 2 Allocation Multipler list. e.g. 1,2,3,.,n", 0},

	{0,0,0,0,"Target Options",3}, 
  	{"ppid",         'p', "INT", 0, "Physical Port ID", 0},
  	{"ldid",         'l', "INT", 0, "Starting LD-ID (for MLD devices)", 0},

	{0,0,0,0, "Networking Options",7},	
  	{"tcp-port",  'P', "INT", 0, "Server TCP Port", 0},
  	{"tcp-address", 'T', "INT", 0, "Server TCP Address", 0},
	{0,0,0,0, "Verbose Options",8},	// Group
  	{"verbosity",     'V', "INT", 0, "Set Verbosity Flag", 0},	
  	{"verbosity-hex", 'X', "HEX", 0, "Set all Verbosity Flags with hex value", 0},	
  	{"mctp-verbosity",'Z', "HEX", OPTION_HIDDEN, "Set all MCTP Verbosity Flags with hex value", 0},	
  	{"no-init",		  'N', NULL,  OPTION_HIDDEN, "Do not initialize local state at start up", 0},	
  	{"print-options", 706,  NULL, OPTION_HIDDEN, "Print CLI Options", 0},	
	{0,0,0,0, "Help Options", 9}, // Group
  	{"help",    'h', NULL, 0, "Display Help", 0},
  	{"usage",   701, NULL, 0, "Display Usage", 0},	
  	{"version", 702, NULL, 0, "Display Version", 0},
	{0,0,0,0,0,0} // Final option should be all null
};

/**
 * CLAP_SET_QOS_ALLOCATED - Options for: <app> set qos allocated
 */
struct argp_option ao_set_qos_allocated[] = 	
{
	{0,0,0,0,"Command Options",1}, 
  	{"fraction", 'f', "INT", 0, "QoS BW Allocation Fraction list. Default: 0 [0-255] e.g. 1,2,3,.,n", 0},

	{0,0,0,0,"Target Options",3}, 
  	{"ppid",     'p', "INT", 0, "Physical Port ID", 0},
  	{"ldid",     'l', "INT", 0, "Starting LD-ID (for MLD devices)", 0},

	{0,0,0,0, "Networking Options",7},	
  	{"tcp-port",  'P', "INT", 0, "Server TCP Port", 0},
  	{"tcp-address", 'T', "INT", 0, "Server TCP Address", 0},
	{0,0,0,0, "Verbose Options",8},	// Group
  	{"verbosity",     'V', "INT", 0, "Set Verbosity Flag", 0},	
  	{"verbosity-hex", 'X', "HEX", 0, "Set all Verbosity Flags with hex value", 0},	
  	{"mctp-verbosity",'Z', "HEX", OPTION_HIDDEN, "Set all MCTP Verbosity Flags with hex value", 0},	
  	{"no-init",		  'N', NULL,  OPTION_HIDDEN, "Do not initialize local state at start up", 0},	
  	{"print-options", 706,  NULL, OPTION_HIDDEN, "Print CLI Options", 0},	
	{0,0,0,0, "Help Options", 9}, // Group
  	{"help",    'h', NULL, 0, "Display Help", 0},
  	{"usage",   701, NULL, 0, "Display Usage", 0},	
  	{"version", 702, NULL, 0, "Display Version", 0},
	{0,0,0,0,0,0} // Final option should be all null
};

/**
 * CLAP_SET_QOS_CONTROL - Options for: <app> set qos control

 */
struct argp_option ao_set_qos_control[] = 	
{
	{0,0,0,0,"Command Options",1}, 
  	{"congestion",   'e',  NULL, 0, "Egress Port Congestion Enable", 0},
  	{"temporary",    't',  NULL, 0, "Temporary Throughput Reduction Enable", 0},
  	{"moderate",     'm', "INT", 0, "Egress Moderate Percentage. Default: 10 [1-100]", 0},
  	{"severe",       's', "INT", 0, "Egress Severe Percentage. Default: 25 [1-100]", 0},
  	{"backpressure", 'k', "INT", 0, "Backpressure Sample Interval x 100 ns. Default: 8 [0-15]", 0},
  	{"reqcmpbasis",  'q', "INT", 0, "ReqCmpBasisB. Default: 0 [0-65,535]", 0},
  	{"ccinterval",   'i', "INT", 0, "Completion Collection Interval. Default: 64 [0-255] ", 0},

	{0,0,0,0,"Target Options",3}, 
  	{"ppid",         'p', "INT", 0, "Physical Port ID", 0},

	{0,0,0,0, "Networking Options",7},	
  	{"tcp-port",  'P', "INT", 0, "Server TCP Port", 0},
  	{"tcp-address", 'T', "INT", 0, "Server TCP Address", 0},
	{0,0,0,0, "Verbose Options",8},	// Group
  	{"verbosity",     'V', "INT", 0, "Set Verbosity Flag", 0},	
  	{"verbosity-hex", 'X', "HEX", 0, "Set all Verbosity Flags with hex value", 0},	
  	{"mctp-verbosity",'Z', "HEX", OPTION_HIDDEN, "Set all MCTP Verbosity Flags with hex value", 0},	
  	{"no-init",		  'N', NULL,  OPTION_HIDDEN, "Do not initialize local state at start up", 0},	
  	{"print-options", 706,  NULL, OPTION_HIDDEN, "Print CLI Options", 0},	
	{0,0,0,0, "Help Options", 9}, // Group
  	{"help",    'h', NULL, 0, "Display Help", 0},
  	{"usage",   701, NULL, 0, "Display Usage", 0},	
  	{"version", 702, NULL, 0, "Display Version", 0},
	{0,0,0,0,0,0} // Final option should be all null
};

/**
 * CLAP_SET_QOS_LIMIT - Options for: <app> set qos limit
 */
struct argp_option ao_set_qos_limit[] = 	
{
	{0,0,0,0,"Command Options",1}, 
  	{"fraction", 'f', "INT", 0, "QoS BW Limit Fraction list. Default: 0 [0-255] e.g. 1,2,3,.,n", 0},

	{0,0,0,0,"Target Options",3}, 
  	{"ppid",     'p', "INT", 0, "Physical Port ID", 0},
  	{"ldid",     'l', "INT", 0, "Starting LD-ID (for MLD devices)", 0},

	{0,0,0,0, "Networking Options",7},	
  	{"tcp-port",  'P', "INT", 0, "Server TCP Port", 0},
  	{"tcp-address", 'T', "INT", 0, "Server TCP Address", 0},
	{0,0,0,0, "Verbose Options",8},	// Group
  	{"verbosity",     'V', "INT", 0, "Set Verbosity Flag", 0},	
  	{"verbosity-hex", 'X', "HEX", 0, "Set all Verbosity Flags with hex value", 0},	
  	{"mctp-verbosity",'Z', "HEX", OPTION_HIDDEN, "Set all MCTP Verbosity Flags with hex value", 0},	
  	{"no-init",		  'N', NULL,  OPTION_HIDDEN, "Do not initialize local state at start up", 0},	
  	{"print-options", 706,  NULL, OPTION_HIDDEN, "Print CLI Options", 0},	
	{0,0,0,0, "Help Options", 9}, // Group
  	{"help",    'h', NULL, 0, "Display Help", 0},
  	{"usage",   701, NULL, 0, "Display Usage", 0},	
  	{"version", 702, NULL, 0, "Display Version", 0},
	{0,0,0,0,0,0} // Final option should be all null
};

/**
 * struct argp objects
 *
 * Use [CLAP] enum to index into this array
 */
struct argp ap_main 				= {ao_main 					, pr_main				, 0, 0, 0, 0, 0};
struct argp ap_mctp 				= {ao_mctp 					, pr_mctp				, 0, 0, 0, 0, 0};
struct argp ap_show 				= {ao_show 					, pr_show				, 0, 0, 0, 0, 0};
struct argp ap_port 				= {ao_port 					, pr_port				, 0, 0, 0, 0, 0};
struct argp ap_set  				= {ao_set  					, pr_set 				, 0, 0, 0, 0, 0};
struct argp ap_ld   				= {ao_ld   					, pr_ld  				, 0, 0, 0, 0, 0};
struct argp ap_aer  				= {ao_aer  					, pr_aer 				, 0, 0, 0, 0, 0};
struct argp ap_show_bos      		= {ao_show_bos      	    , pr_show_bos     		, 0, 0, 0, 0, 0};
struct argp ap_show_identity		= {ao_show_identity		    , pr_show_identity		, 0, 0, 0, 0, 0};
struct argp ap_show_limit   		= {ao_show_limit      	    , pr_show_limit   		, 0, 0, 0, 0, 0};
struct argp ap_show_switch 			= {ao_show_switch 			, pr_show_switch 		, 0, 0, 0, 0, 0};
struct argp ap_show_dev 			= {ao_show_dev 				, pr_show_dev  			, 0, 0, 0, 0, 0};
struct argp ap_show_port			= {ao_show_port				, pr_show_port 			, 0, 0, 0, 0, 0};
struct argp ap_show_vcs             = {ao_show_vcs           	, pr_show_vcs    		, 0, 0, 0, 0, 0};
struct argp ap_show_qos             = {ao_show_qos           	, pr_show_qos    		, 0, 0, 0, 0, 0};
struct argp ap_show_ld              = {ao_show_ld            	, pr_show_ld     		, 0, 0, 0, 0, 0};
struct argp ap_port_bind            = {ao_port_bind          	, pr_port_bind   		, 0, 0, 0, 0, 0};
struct argp ap_port_connect         = {ao_port_connect       	, pr_port_connect		, 0, 0, 0, 0, 0};
struct argp ap_port_disconnect      = {ao_port_disconnect      	, pr_port_disconnect	, 0, 0, 0, 0, 0};
struct argp ap_port_unbind          = {ao_port_unbind        	, pr_port_unbind 		, 0, 0, 0, 0, 0};
struct argp ap_port_config          = {ao_port_config        	, pr_port_config 		, 0, 0, 0, 0, 0};
struct argp ap_port_ctrl            = {ao_port_ctrl         	, pr_port_ctrl  		, 0, 0, 0, 0, 0};
struct argp ap_set_ld               = {ao_set_ld            	, pr_set_ld 			, 0, 0, 0, 0, 0};
struct argp ap_set_limit            = {ao_set_limit         	, pr_set_limit			, 0, 0, 0, 0, 0};
struct argp ap_set_qos              = {ao_set_qos           	, pr_set_qos     		, 0, 0, 0, 0, 0};
struct argp ap_ld_config            = {ao_ld_config         	, pr_ld_config   		, 0, 0, 0, 0, 0};
struct argp ap_ld_mem               = {ao_ld_mem            	, pr_ld_mem      		, 0, 0, 0, 0, 0};
struct argp ap_show_qos_allocated   = {ao_show_qos_allocated	, pr_show_qos_allocated , 0, 0, 0, 0, 0};
struct argp ap_show_qos_control     = {ao_show_qos_control  	, pr_show_qos_control   , 0, 0, 0, 0, 0};
struct argp ap_show_qos_limit       = {ao_show_qos_limit    	, pr_show_qos_limit     , 0, 0, 0, 0, 0};
struct argp ap_show_qos_status      = {ao_show_qos_status   	, pr_show_qos_status    , 0, 0, 0, 0, 0};
struct argp ap_show_ld_allocations  = {ao_show_ld_allocations   , pr_show_ld_allocations, 0, 0, 0, 0, 0};
struct argp ap_show_ld_info         = {ao_show_ld_info          , pr_show_ld_info       , 0, 0, 0, 0, 0};
struct argp ap_set_ld_allocations   = {ao_set_ld_allocations    , pr_set_ld_allocations , 0, 0, 0, 0, 0};
struct argp ap_set_qos_allocated    = {ao_set_qos_allocated     , pr_set_qos_allocated  , 0, 0, 0, 0, 0};
struct argp ap_set_qos_control      = {ao_set_qos_control       , pr_set_qos_control    , 0, 0, 0, 0, 0};
struct argp ap_set_qos_limit 		= {ao_set_qos_limit 		, pr_set_qos_limit 		, 0, 0, 0, 0, 0};

/* FUNCTIONS =================================================================*/

/**
 * Return a string representation of CLI Option Names [CLOP]
 */
char *clop(int u)
{
	if (u >= CLOP_MAX) 
		return NULL;
	else 
		return STR_CLOP[u];
}

/**
 * Convert either a hex or decimal string to an unsigned long value
 */
static inline unsigned long hexordec_to_ul(char *arg)
{
	if (!strncmp("0x", arg, 2)) 
		return strtoul(arg, NULL, 16);
	else 
		return strtoul(arg, NULL, 10);
}

/**
 * Convert either a hex or decimal string to an unsigned long long value
 */
static inline unsigned long long hexordec_to_ull(char *arg)
{
	if (!strncmp("0x", arg, 2)) 
		return strtoull(arg, NULL, 16);
	else 
		return strtoull(arg, NULL, 10);
}


static int range_to_u8_array(__u8 *dst, char *src)
{
	int i, start, end;
	int s; // src index 
	int d; // dst index 
	int t; // tmp index 
	char tmp[256];

	start = -1;
	end = 0;
	s = 0;
	d = 0;
	t = 0;
	memset(tmp, 0, 256);

	while (src[s] != 0)
	{
		// Convert prior text to a single __u8
		if (src[s] == ',') 
		{
			// If start is >=0 then there was a previous '-'
			if (start >= 0) {
				end = hexordec_to_ul(tmp);	
				memset(tmp, 0, t);
				t = 0;
				for (i = start ; i <= end ; i++)
					dst[d++] = i;	
				start = -1;
			}
			else if (t > 0){
				dst[d++] = hexordec_to_ul(tmp);	
				memset(tmp, 0, t);
				t = 0;
			}
		}

		// Store prior text until next value is completed 
		else if (src[s] == '-')
		{
			if (start < 0 && t > 0) {
				start = hexordec_to_ul(tmp);
				memset(tmp, 0, t);
				t = 0;
			}
		}
		else 
		{
			tmp[t++] = src[s];
		}
		s++;
	}
	if (start >= 0 && t > 0) {
		end = hexordec_to_ul(tmp);	
		memset(tmp, 0, t);
		t = 0;
		for (i = start ; i <= end ; i++)
			dst[d++] = i;	
	}
	else if (t > 0) {
		dst[d++] = hexordec_to_ul(tmp);	
		memset(tmp, 0, t);
		t = 0;
	}

	return d;
}


/**
 * Free allocated memory by option parsing proceedure
 *
 * @return 0 upon success. Non zero otherwise
 */
int options_free(struct opt *opts)
{
	struct opt *o;
	int i;

	for ( i = 0 ; i < CLOP_MAX ; i++ )
	{
		o = &opts[i];	

		// Free buf field
		if (o->buf)
			free(o->buf);
		o->buf = NULL;

		// Free str field
		if (o->str)
			free(o->str);
		o->str = NULL;
	}

	// Free options array
	free(opts);

	// Free app Name 
	if (app_name)
		free(app_name);

	return 0;
}

/**
 * Split a long string of __u8 string values into an array
 *
 * @param src char* to string of many __u8 values
 * @param dst __u8* array to store parsed values in
 */
static int parse_u8_str(void *dst, char* src)
{
	char *head;
	char buf[3];
	int i; 
	__u8 *array;

	i = 0;
	buf[2] = 0;
	head = src;
	array = (__u8*) dst;
	
	// If there is a leading 0x, skip it
	if (!strncmp("0x", src, 2)) {
		head = &src[2];
	}

	while (*head != 0) 
	{
		buf[0] = head[0];
		buf[1] = head[1];
		array[i] = strtoul(buf, NULL, 16);
		i++;
		head = &head[2];
	}

	return i;
}

/**
 * Split a comma separated string of __u8
 *
 * @param src char* to a comma separated array of __u8 values
 * @param dst __u8* array to store parsed values in
 */
static int parse_u8_csv(void *dst, char* src)
{
	char *head, *tail;
	char buf[8];
	int i; 
	__u8 *array;

	i = 0;
	head = src;
	tail = src; 
	array = (__u8*) dst;
	memset(buf,0,8);

	while (*tail != 0) 
	{
		if (*tail == ',') 
		{
			memcpy(buf, head, tail - head);
			array[i] = strtoull(buf, NULL, 16);
			memset(buf,0,8);
			i++;
			tail++;
			head = tail;
		}
		else 
			tail++;
	}

	// Catch last entry in the CSV list 
	if (head != tail) 
	{
		memcpy(buf, head, tail - head);
		array[i] = strtoull(buf, NULL, 16);
		i++;
	}

	return i;
}

/**
 * Split a comma separated string of __u64
 *
 * @param src char* to a comma separated array of __u64 values
 * @param dst __u64* array to store parsed values in
 */
static int parse_u64_csv(void *dst, char* src)
{
	char *head, *tail;
	char buf[17];
	int i; 
	__u64 *array;

	i = 0;
	head = src;
	tail = src; 
	array = (__u64*) dst;
	memset(buf,0,17);

	while (*tail != 0) 
	{
		if (*tail == ',') 
		{
			memcpy(buf, head, tail - head);
			array[i] = strtoull(buf, NULL, 16);
			memset(buf,0,17);
			i++;
			tail++;
			head = tail;
		}
		else 
			tail++;
	}

	// Catch last entry in the CSV list 
	if (head != tail) 
	{
		memcpy(buf, head, tail - head);
		array[i] = strtoull(buf, NULL, 16);
		i++;
	}
	return i;
}

/**
 * Print the command line flag options to the screen as part of help output
 *
 * @param o the menu level [CLAP] enum
 */
static void print_options(struct argp_option *o)
{
	int len;

	while (o->doc != NULL) 
	{
		// Break if this is the ending  NULL entry
		if ( !o->name && !o->key && !o->arg && !o->flags && !o->doc && !o->group )
			break;

		// Skip Hidden Options
		if (o->flags & OPTION_HIDDEN) {
			o++;
			continue;
		}

		// Determine if this is a section heading
		else if ( !o->name && !o->key && !o->arg && !o->flags && o->doc)
			printf("\n %s:\n", o->doc);

		// Print normal option entry 
		else { 

			// IF this option has a single character key, print the key, else print spaces
			if (isalnum(o->key)) 
				printf("  -%c, ", o->key);
			else 
				printf("      ");
			len = 6;

			// If this option has a long name, print the long name
			if (o->name) {
				printf("--%s", o->name);
				len += strlen(o->name) + 2;
			}

			// If this option has an arg type, print the type 
			if (o->arg) {
				printf("=%s", o->arg);
				len += strlen(o->arg) + 1;
			}

			// Print remaining spaces up to description column
			for ( int i = 0 ; i < CLMR_HELP_COLUMN - len ; i++ )
				printf(" ");
			
			// Print description of this option
			printf("%s\n", o->doc);
		}
		o++;
	}
}

/**
 * Debug function to print out the options array at the end of parsing
 */ 
static void print_options_array(struct opt *o)
{
	int i, len, maxlen;

	maxlen = 0;

	// Find max length of CLOP String
	for (i = 0 ; i < CLOP_MAX ; i++) {
		len = strlen(clop(i));
		if (len > maxlen)
			maxlen = len;
	}

	// Print Header 
	printf("##");						// index 
	printf(" Name");						// OP Name 
	for (int k = 5 ; k <= maxlen ; k++)	// Spaces 
		printf(" ");
	printf(" S"); 						// Set 	
	printf("   u8");  					// u8
	printf("    u16"); 					// u16
	printf("        u32"); 				// u32
	printf("                u64"); 		// u64
	printf("    val"); 					// val
	printf("                num"); 		// num
	printf("                len"); 		// len
	printf(" str");						// str
	printf("\n");

	// Print each entry
	for (i = 0 ; i < CLOP_MAX ; i++) {
		// index 
		printf("%02d", i);

		// OP Name 
		printf(" %s", clop(i));

		// Spaces 
		for (int k = strlen(clop(i)) ; k < maxlen ; k++)
			printf(" ");

		printf(" %d", 			o[i].set); // Set 	
		printf(" 0x%02x", 		o[i].u8);  // u8
		printf(" 0x%04x", 		o[i].u16); // u16
		printf(" 0x%08x", 		o[i].u32); // u32
		printf(" 0x%016llx", 	o[i].u64); // u64
		printf(" 0x%04x", 		o[i].val); // val
		printf(" 0x%016llx", 	o[i].num); // num
		printf(" 0x%016llx", 	o[i].len); // len

		if (o[i].str)
			printf(" %s", o[i].str);

		printf("\n");

		if (o[i].len > 0)
			autl_prnt_buf(o[i].buf, o[i].len, 4, 0);
	}
}

/**
 * Print the usage information for a option level
 *
 * @param option 	Menu item from enum [CLAP]
 * @param o 		struct argp_option* to the string data to pull from	
 * STEPS:
 * 1: Initialize variables
 * 2: Generate header text 
 * 3: Count the number of short options with no argument
 * 4: If there is at least one short option with no arg, append short options with no argument here
 * 5: Append short options with arguments 
 * 6: Append long options
 * 7: Find index of last space before character 80
 * 8: Loop through usage buffer and break it up into smaller chunks 
*/
static void print_usage(int option, struct argp_option *o)
{
	int hdr_len, buf_len, num, i, index;
	char buf[4096];
	char str[4096];
	char *ptr;
	struct argp_option *original;
	
	// STEP 1: Initialize variables
	num = 0;
	index = 0;
	hdr_len = 0; 
	buf_len = 1;
	memset(buf, 0, 4096);
	memset(str, 0, 4096);
	original = o;
	ptr = buf;

	// STEP 2: Generate header text 
	switch(option)
	{
		case CLAP_MAIN: 				sprintf(str, "Usage: %s ",      				app_name); break;
		case CLAP_MCTP:					sprintf(str, "Usage: %s mctp ", 				app_name); break;
		case CLAP_SHOW: 				sprintf(str, "Usage: %s show ", 				app_name); break;
		case CLAP_PORT: 				sprintf(str, "Usage: %s port ", 				app_name); break;
		case CLAP_SET:					sprintf(str, "Usage: %s set ",  				app_name); break;
		case CLAP_LD:                   sprintf(str, "Usage: %s ld ",   				app_name); break;
		case CLAP_AER:                  sprintf(str, "Usage: %s aer ",  				app_name); break;
		case CLAP_SHOW_BOS:             sprintf(str, "Usage: %s show bos ", 			app_name); break;
		case CLAP_SHOW_IDENTITY:        sprintf(str, "Usage: %s show identity ", 		app_name); break;
		case CLAP_SHOW_MSG_LIMIT:       sprintf(str, "Usage: %s show limit ", 			app_name); break;
		case CLAP_SHOW_SWITCH:          sprintf(str, "Usage: %s show switch ", 			app_name); break;
		case CLAP_SHOW_PORT:			sprintf(str, "Usage: %s show port",     		app_name); break;
		case CLAP_SHOW_VCS:             sprintf(str, "Usage: %s show vcs ", 			app_name); break;
		case CLAP_SHOW_QOS:             sprintf(str, "Usage: %s show qos ", 			app_name); break;
		case CLAP_SHOW_LD:              sprintf(str, "Usage: %s show ld ", 				app_name); break;
		case CLAP_PORT_BIND:			sprintf(str, "Usage: %s port bind ",    		app_name); break;
		case CLAP_PORT_UNBIND:          sprintf(str, "Usage: %s port unbind ",  		app_name); break;
		case CLAP_PORT_CONFIG:          sprintf(str, "Usage: %s port config ", 			app_name); break;
		case CLAP_PORT_CTRL:            sprintf(str, "Usage: %s port reset ", 			app_name); break;
		case CLAP_SET_LD:               sprintf(str, "Usage: %s set ld ",      			app_name); break;
		case CLAP_SET_MSG_LIMIT:        sprintf(str, "Usage: %s set limit ",   			app_name); break;
		case CLAP_SET_QOS:              sprintf(str, "Usage: %s set qos ", 				app_name); break;
		case CLAP_LD_CONFIG:            sprintf(str, "Usage: %s ld config ", 			app_name); break;
		case CLAP_LD_MEM:               sprintf(str, "Usage: %s ld mem ", 				app_name); break;
		case CLAP_SHOW_QOS_ALLOCATED:   sprintf(str, "Usage: %s show qos allocated ",   app_name); break;
		case CLAP_SHOW_QOS_CONTROL:     sprintf(str, "Usage: %s show qos control ", 	app_name); break;
		case CLAP_SHOW_QOS_LIMIT:       sprintf(str, "Usage: %s show qos limit ", 		app_name); break;
		case CLAP_SHOW_QOS_STATUS:      sprintf(str, "Usage: %s show qos status ", 		app_name); break;
		case CLAP_SHOW_LD_ALLOCATIONS:	sprintf(str, "Usage: %s show ld allocations ",	app_name); break;
		case CLAP_SHOW_LD_INFO:         sprintf(str, "Usage: %s show ld info ", 		app_name); break;
		case CLAP_SET_LD_ALLOCATIONS:   sprintf(str, "Usage: %s set ld allocations ",   app_name); break;
		case CLAP_SET_QOS_ALLOCATED:    sprintf(str, "Usage: %s set qos allocated ", 	app_name); break;
		case CLAP_SET_QOS_CONTROL:      sprintf(str, "Usage: %s set qos control ", 		app_name); break;
		case CLAP_SET_QOS_LIMIT:        sprintf(str, "Usage: %s set qos limit ", 		app_name); break;
		default: 																				   break;
	}
	hdr_len = strlen(str);

	// STEP 3: Count the number of short options with no argument
	while ( !( !o->name && !o->key && !o->arg && !o->flags && !o->doc && !o->group ) )
	{
		if (isalnum(o->key) && !o->arg) 
			num++;
		o++;
	}
	
	// Reset pointer
	o = original;

	// STEP 4: If there is at least one short option with no arg, append short options with no argument here
	if ( num > 0 ) 
	{
		// Add Leader [-
		sprintf(&buf[buf_len], "[-");
		buf_len += 2;

		// Add each key character
		while ( !( !o->name && !o->key && !o->arg && !o->flags && !o->doc && !o->group ) )
		{
			// If this option has a single character key, print the key, else print spaces
			if (isalnum(o->key) && !o->arg) {
				sprintf(&buf[buf_len], "%c", o->key);
				buf_len += 1;
			}
			o++;
		}

		// Add trailing ]
		sprintf(&buf[buf_len], "] ");
		buf_len += 2;
	}
	
	// Reset pointer
	o = original;

	// STEP 5: Append short options with arguments 
	while ( !( !o->name && !o->key && !o->arg && !o->flags && !o->doc && !o->group ) )
	{
		// If this option has a single character key and an arg 
		if (isalnum(o->key) && o->arg) {
			sprintf(&buf[buf_len], "[-%c=%s] ", o->key, o->arg);
			buf_len += 6;
			buf_len += strlen(o->arg);
		}
		o++;
	}
	
	// Reset pointer
	o = original;

	// STEP 6: Append long options
	while ( !( !o->name && !o->key && !o->arg && !o->flags && !o->doc && !o->group ) )
	{
		// If this option has a long name, print the long name
		if (o->name) 
		{
			sprintf(&buf[buf_len], "[--%s", o->name);
			buf_len += strlen(o->name) + 3;

			// If this option has an arg type, print the type 
			if (o->arg) 
			{
				sprintf(&buf[buf_len], "=%s", o->arg);
				buf_len += strlen(o->arg) + 1;
			}

			// Add trailing ]
			sprintf(&buf[buf_len], "] ");
			buf_len += 2;
		}
		o++;
	}

	// STEP 7: Find index of last space before character 80
	index = 0;
	for ( i = 1 ; i < (CLMR_MAX_HELP_WIDTH-hdr_len) ; i++ ) {
		if (ptr[i] == ' ') 
			index = i;
		if (ptr[i]==0)
			break;
	}

	// STEP 8: Loop through usage buffer and break it up into smaller chunks 
	while (index != 0)
	{
		// Copy the line of the buffer into str 
		memcpy(&str[hdr_len], &ptr[1], index-1); 

		// Set the next char after the string to 0
		str[hdr_len+index-1] = 0;

		// Print the merged string 
		printf("%s\n", str);

		// Clear the header portion of the print str
		memset(str, ' ', hdr_len);

		// Advance buffer
		ptr = &ptr[index];

		// Find index of last space before character 80
		index = 0;
		for ( i = 1 ; i < (CLMR_MAX_HELP_WIDTH-hdr_len) ; i++ ) {
			if (ptr[i] == ' ') 
				index = i;
			if (ptr[i]==0)
				break;
		}
	}
}

/**
 * Print the Help output
 *
 * @param option the level to print [CLAP]
 *
 * STEPS
 * 1: Print the Global Header Statement
 * 2: Print usage	
 * 3: Print level header and flagged options
 */
static void print_help(int option)
{
	// STEP 1: Print the Global Header Statement
	printf("CXL Fabric Management CLI Tool\n");
	
	// STEP 3: Print level header and flagged options
	switch (option)
	{
		case CLAP_MAIN: 	// 0
printf("\n\
Main menu:\n");
printf("\n\
Usage: %s <options> [[subcommand] <subcommand options>. . .] \n", app_name);
printf("\n\
Supported subcommands:\
\n\
  ld           Logical Device Info\n\
  mctp         Interact with the remote MCTP endpoint\n\
  port         Perform port related actions\n\
  set          Configure a component\n\
  show         Obtain & display information from target\n\
  aer          Generate an AER event\n\
");
			print_options(ao_main);
			printf("\n");
			break;

		case CLAP_MCTP:		// 1
printf("\n\
Usage: %s mctp <options>\n", app_name);
printf("\n\
Commands to interact with the remote MCTP Endpoint\n\
");
			print_options(ao_mctp);
			printf("\n");
			break;

		case CLAP_SHOW: 	// 2
printf("\n\
Usage: %s show [subcommand <options>]\n", app_name);
printf("\n\
Supported subcommands:\
\n\
  bos          Background Operation Status\n\
  devices      Emulator Device profiles\n\
  identity     Component information\n\
  limit        Response Message Limit Size\n\
  ld           Logical Device Info\n\
  port         Physical Port State\n\
  qos          Performance Status & Controls\n\
  switch       Physical Switch Identity\n\
  vcs          Virtual CXL Switch\n\
");
			print_options(ao_show);
			printf("\n");
			break;

		case CLAP_PORT: 	// 3
printf("\n\
Usage: %s port [subcommand <options>]\n", app_name);
printf("\n\
Supported subcommands:\
\n\
  bind         Bind Physical Port to vPPB\n\
  config       Send PPB CXL.io Config Request\n\
  connect      Connect Emulator Device Profile\n\
  control      Control unbound physical port\n\
  disconnect   Disconnect Emulator Device Profile\n\
  unbind       Unbind Physical port from vPPB\n\
");
			print_options(ao_port);
			printf("\n");
			break;

		case CLAP_PORT_CONN:
printf("\n\
Usage: %s port connect <options>\n", app_name);
			print_options(ao_port_connect);
			printf("\n");
			break;

		case CLAP_PORT_DISCONN:
printf("\n\
Usage: %s port disconnect <options>\n", app_name);
			print_options(ao_port_disconnect);
			printf("\n");
			break;

		case CLAP_SET:
printf("\n\
Usage: %s set [subcommand <options>]\n", app_name);
printf("\n\
Supported subcommands:\
\n\
  ld           Configure Logical Device\n\
  limit        Message Response Limit size\n\
  qos          Configure Performance QoS settings\n\
");
			print_options(ao_set);
			printf("\n");
			break;

		case CLAP_LD:
printf("\n\
Usage: %s ld [subcommand <options>]\n", app_name);
printf("\n\
Supported subcommands:\
\n\
  config       Write to Logical Device Config Space\n\
  mem          Write to Logical Device Memory Space\n\
");
			print_options(ao_ld);
			printf("\n");
			break;

		case CLAP_AER:
printf("\n\
Usage: %s aer <options>\n", app_name);
			print_options(ao_aer);
			printf("\n");
			break;

		case CLAP_SHOW_BOS:
printf("\n\
Usage: %s show bos <options>\n", app_name);
			print_options(ao_show_bos);
			printf("\n");
			break;

		case CLAP_SHOW_IDENTITY:
printf("\n\
Usage: %s show identity <options>\n", app_name);
			print_options(ao_show_identity);
			printf("\n");
			break;

		case CLAP_SHOW_MSG_LIMIT:
printf("\n\
Usage: %s show limit <options>\n", app_name);
			print_options(ao_show_limit);
			printf("\n");
			break;

		case CLAP_SHOW_SWITCH:
printf("\n\
Usage: %s show switch <options>\n", app_name);
			print_options(ao_show_switch);
			printf("\n");
			break;

		case CLAP_SHOW_DEV:
printf("\n\
Usage: %s show device <options>\n", app_name);
			print_options(ao_show_dev);
			printf("\n");
			break;

		case CLAP_SHOW_PORT:
printf("\n\
Usage: %s show port <options>\n", app_name);
printf("\n\
CXL Versions Field Entries: \n\
 A: CXL 1.1 \n\
 B: CXL 2.0 \n\
 C: CXL 3.0 \n\
 D: CXL 3.1 \n");
printf("\n\
PCIe Speeds Entries: \n\
 1: PCIe 1.0 \n\
 2: PCIe 2.0 \n\
 3: PCIe 3.0 \n\
 4: PCIe 4.0 \n\
 5: PCIe 5.0 \n\
 6: PCIe 6.0 \n");
printf("\n\
Link Flags Entries: \n\
 L: Lane Reversal \n\
 R: PCIe Reset (PERST) \n\
 P: Device Present (PRSNT) \n\
 W: Power Control State (PWR_CTRL) \n");
			print_options(ao_show_port);
			printf("\n");
			break;

		case CLAP_SHOW_VCS:
printf("\n\
Usage: %s show vcs <options>\n", app_name);
			print_options(ao_show_vcs);
			printf("\n");
			break;

		case CLAP_SHOW_QOS:
printf("\n\
Usage: %s show qos [subcommand <options>]\n", app_name);
printf("\n\
Supported subcommands:\
\n\
  allocated    Get QoS Allocated BW\n\
  control      Get QoS Control\n\
  limit        Get QoS BW Limit\n\
  status       Get QoS Status\n\
");
			print_options(ao_show_qos);
			printf("\n");
			break;

		case CLAP_SHOW_LD:
printf("\n\
Usage: %s show ld [subcommand <options>]\n", app_name);
printf("\n\
Supported subcommands:\
\n\
  allocations  Get LD Allocations (alloc)\n\
  info         Get LD Info\n\
");
			print_options(ao_show_ld);
			printf("\n");
			break;

		case CLAP_PORT_BIND:
printf("\n\
Usage: %s port bind <options>\n", app_name);
			print_options(ao_port_bind);
			printf("\n");
			break;

		case CLAP_PORT_UNBIND:
printf("\n\
Usage: %s port unbind <options>\n", app_name);
			print_options(ao_port_unbind);
			printf("\n");
			break;

		case CLAP_PORT_CONFIG:
printf("\n\
Usage: %s port config <options>\n", app_name);
printf("\n\
Defaults to a read operation unless the --write-data option is specified.\n\
");
			print_options(ao_port_config);
			printf("\n");
			break;

		case CLAP_PORT_CTRL:
printf("\n\
Usage: %s port control <options>\n", app_name);
			print_options(ao_port_ctrl);
			printf("\n");
			break;

		case CLAP_SET_LD:
printf("\n\
Usage: %s set ld [subcommand <options>]\n", app_name);
printf("\n\
Supported subcommands:\
\n\
  allocations  Set LD Allocations (alloc)\n\
");
			print_options(ao_set_ld);
			printf("\n");
			break;

		case CLAP_SET_MSG_LIMIT:
printf("\n\
Usage: %s set limit <options>\n", app_name);
			print_options(ao_set_limit);
			printf("\n");
			break;

		case CLAP_SET_QOS:
printf("\n\
Usage: %s set qos [subcommand <options>]\n", app_name);
printf("\n\
Supported subcommands:\
\n\
  allocated    Set QoS Allocated BW (alloc)\n\
  control      Set QoS Control (ctrl)\n\
  limit        Set QoS BW Limit (lim)\n\
");
			print_options(ao_set_qos);
			printf("\n");
			break;

		case CLAP_LD_CONFIG:
printf("\n\
Usage: %s ld config <options>\n", app_name);
			print_options(ao_ld_config);
			printf("\n");
			break;

		case CLAP_LD_MEM:
printf("\n\
Usage: %s ld mem <options>\n", app_name);
			print_options(ao_ld_mem);
			printf("\n");
			break;

		case CLAP_SHOW_QOS_ALLOCATED:
printf("\n\
Usage: %s show qos allocated <options>\n", app_name);
			print_options(ao_show_qos_allocated);
			printf("\n");
			break;

		case CLAP_SHOW_QOS_CONTROL:
printf("\n\
Usage: %s show qos control <options>\n", app_name);
			print_options(ao_show_qos_control);
			printf("\n");
			break;

		case CLAP_SHOW_QOS_LIMIT:
printf("\n\
Usage: %s show qos limit <options>\n", app_name);
			print_options(ao_show_qos_limit);
			printf("\n");
			break;

		case CLAP_SHOW_QOS_STATUS:
printf("\n\
Usage: %s show qos status <options>\n", app_name);
			print_options(ao_show_qos_status);
			printf("\n");
			break;

		case CLAP_SHOW_LD_ALLOCATIONS:
printf("\n\
Usage: %s show ld allocations <options>\n", app_name);
			print_options(ao_show_ld_allocations);
			printf("\n");
			break;

		case CLAP_SHOW_LD_INFO:
printf("\n\
Usage: %s show ld info <options>\n", app_name);
			print_options(ao_show_ld_info);
			printf("\n");
			break;

		case CLAP_SET_LD_ALLOCATIONS:
printf("\n\
Usage: %s set ld allocations <options>\n", app_name);
			print_options(ao_set_ld_allocations);
			printf("\n");
			break;

		case CLAP_SET_QOS_ALLOCATED:
printf("\n\
Usage: %s set qos allocated <options>\n", app_name);
			print_options(ao_set_qos_allocated);
			printf("\n");
			break;

		case CLAP_SET_QOS_CONTROL:
printf("\n\
Usage: %s set qos control <options>\n", app_name);
			print_options(ao_set_qos_control);
			printf("\n");
			break;

		case CLAP_SET_QOS_LIMIT:
printf("\n\
Usage: %s set qos limit <options>\n", app_name);
			print_options(ao_set_qos_limit);
			printf("\n");
			break;

		default: 
			break;
	} // switch (option)
}

/**
 * Common parse function 
 *
 * This function implements the common flags to most parsers 
 *
 * @return 0 success, non-zero to indicate a problem 
 *
 * Global keys
 * -N --no-init  		Do not initialize local state at start up
 * -T --tcp-address 	Server TCP Address
 * -P --tcp-port 		Server TCP Port
 * -T --tcp-address 	Server TCP Address
 * -V --verbosity 		Set Verbosity Flag
 * -X --verbosity-hex	Set all Verbosity Flags with hex value
 * -A --all				All of collection 
 * -p --ppid		 	Physical Port ID
 * -c --vcsid 			Virtual CXL Switch ID
 * -b --vppbid			Virtual PCIe-to-PCIe Bridge ID
 * -h --help 			Display Help
 * -l --ldid			LD-ID (for MLD devices)
 *
 * Standard key mapping 
 * -n --length 			Length 
 * -o --offset 			Memory Offset
 * -w --write 			Perform a Write transaction
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
 *
 * Special Keys:
 * ARGP_KEY_INIT  		Called first. Initialize any data structures here
 * ARGP_KEY_ARG   		Called for non option parameter (no flag)
 * ARGP_KEY_NO_ARGS  	Only called if there are no args at all
 * ARGP_KEY_SUCCESS 	Called after all args completed successfuly
 * ARGP_KEY_END 		Last call. Verify parameters. Fill in missing parameters
 * ARGP_KEY_ERROR 		Called after an error
 * ARGP_KEY_FINI  		Absolutely last call to this parse functon
 */
static int pr_common(int key, char *arg, struct argp_state *state, int type, struct argp_option *ao)
{
	struct opt *opts, *o;
	int rv;

	rv = 0;
	opts = (struct opt*) state->input;

	switch (key)
	{
		// Perform on all in the collection
		case 'A': 
			o = &opts[CLOP_ALL];
			o->set = 1;
			break;

		// Virtual PCIe-to-PCIe Bridge ID 
		case 'b': 
			o = &opts[CLOP_VPPBID];
			o->set = 1;
			o->u8 = hexordec_to_ul(arg);
			break;

		// Virtual CXL Switch ID #
		case 'c': 
		{
			int range, i, len;

			o = &opts[CLOP_VCSID];
			o->set = 1;

			// Detect range 
			range = 0;
			len = strnlen(arg, 256);
			for ( i = 0 ; i < len ; i++ )
				if (arg[i] == ',' || arg[i] == '-')
					range = 1;

			// If there is a command or dash, convert to a range
			if (range) 
			{
				o->buf = malloc(256);
				memset(o->buf, 0, 256);
				o->num = range_to_u8_array(o->buf, arg);
				o->len = o->num * sizeof(__u8);
			}
			else 
				o->u8 = hexordec_to_ul(arg);
		}	
			break;

		// Help
		case 'h': 
			print_help(type);
			exit(0);
			break;

		// Logical Device ID
		case 'l': 
			o = &opts[CLOP_LDID];
			o->set = 1;
			o->u16 = hexordec_to_ul(arg);
			break;

		// Physical Port ID 
		case 'p': 
		{
			int range, i, len;

			o = &opts[CLOP_PPID];
			o->set = 1;

			// Detect range 
			range = 0;
			len = strnlen(arg, 256);
			for ( i = 0 ; i < len ; i++ )
				if (arg[i] == ',' || arg[i] == '-')
					range = 1;

			// If there is a command or dash, convert to a range
			if (range) 
			{
				o->buf = malloc(256);
				memset(o->buf, 0, 256);
				o->num = range_to_u8_array(o->buf, arg);
				o->len = o->num * sizeof(__u8);
			}
			else 
				o->u8 = hexordec_to_ul(arg);
		}
			break;

		// no-init
		case 'N': 
			o = &opts[CLOP_NO_INIT];
			o->set = 1;
			break;

		// TCP Port
		case 'P': 
			o = &opts[CLOP_TCP_PORT];
			o->set = 1;
			o->u16 = hexordec_to_ul(arg);
			break;

		// TCP Address
		case 'T': 
			o = &opts[CLOP_TCP_ADDRESS];
			o->set = 1;
			rv = inet_pton(AF_INET, arg, &o->u32);
			if (rv != 1)
			{
				printf("Invalid TCP IP Address\n");
				exit(rv);
			}
			rv = 0;
			break;

		// Verbosity
		case 'V': 
			o = &opts[CLOP_VERBOSITY];
			o->set = 1;
			o->u64 |= (0x01 << hexordec_to_ul(arg));
			break;

		// Set all Verbosity Flags with hex value
		case 'X': 
			o = &opts[CLOP_VERBOSITY];
			o->set = 1;
			o->u64 = hexordec_to_ull(arg);
			break;

		// Set all MCTP Verbosity Flags with hex value
		case 'Z': 
			o = &opts[CLOP_MCTP_VERBOSITY];
			o->set = 1;
			o->u64 = hexordec_to_ull(arg);
			break;

		// Usage 
		case 701: 
			print_usage(type, ao);
			exit(0);
			break;

		// Version
		case 702: 
			printf("%s\n", argp_program_version);
			exit(0);
			break;

		// print-options
		case 706: 
			o = &opts[CLOP_PRNT_OPTS];
			o->set = 1;
			break;

		// Last call. Verify parameters. Fill in missing values
		case ARGP_KEY_END:				
			break;
	} 
	return rv;	
}

/**
 * Parse function for: main 
 *
 * @return 0 success, non-zero to indicate a problem 
 */
static int pr_main(int key, char *arg, struct argp_state *state)
{
	struct opt *opts;
	int rv = 0;

	opts = (struct opt*) state->input;

	// Call common flag processing first
	rv = pr_common(key, arg, state, CLAP_MAIN, ao_main);

	switch (key)
	{
		// Called for non option parameter
		case ARGP_KEY_ARG: 				
			if (!strcmp(arg, "mctp")) 
				rv = argp_parse(&ap_mctp, state->argc-state->next+1, &state->argv[state->next-1], ARGP_IN_ORDER | ARGP_NO_HELP, 0, opts);

			else if (!strcmp(arg, "show")) 
				rv = argp_parse(&ap_show, state->argc-state->next+1, &state->argv[state->next-1], ARGP_IN_ORDER | ARGP_NO_HELP, 0, opts);

			else if (!strcmp(arg, "port") || !strcmp(arg, "pt")) 
				rv = argp_parse(&ap_port, state->argc-state->next+1, &state->argv[state->next-1], ARGP_IN_ORDER | ARGP_NO_HELP, 0, opts);
			
			else if (!strcmp(arg, "set")) 
				rv = argp_parse(&ap_set, state->argc-state->next+1, &state->argv[state->next-1], ARGP_IN_ORDER | ARGP_NO_HELP, 0, opts);

			else if (!strcmp(arg, "ld")) 
				rv = argp_parse(&ap_ld, state->argc-state->next+1, &state->argv[state->next-1], ARGP_IN_ORDER | ARGP_NO_HELP, 0, opts);

			else if (!strcmp(arg, "aer")) 
				rv = argp_parse(&ap_aer, state->argc-state->next+1, &state->argv[state->next-1], ARGP_IN_ORDER | ARGP_NO_HELP, 0, opts);
			
			else if (!strcmp(arg, "list")) 
			{
				opts[CLOP_CMD].set = 1;
				opts[CLOP_CMD].val = CLCM_LIST;
			}
			else 
				argp_error (state, "Invalid subcommand"); 

			// Stop current parser
			state->next = state->argc; 	
			break;

		// Last call. Verify parameters. Fill in missing values
		case ARGP_KEY_END:				

			if (!opts[CLOP_TCP_PORT].set) 
			{
				opts[CLOP_TCP_PORT].set = 1;
				opts[CLOP_TCP_PORT].u16 = DEFAULT_SERVER_PORT;
			}

			if (opts[CLOP_PRNT_OPTS].set) {
				print_options_array(opts);
			}
			// Help
			if (!opts[CLOP_CMD].set) {
				print_help(CLAP_MAIN);
				exit(0);
			}
			break;
	} 
	return rv;	
}

/**
 * Parse function for: mctp 
 *
 * @return 0 success, non-zero to indicate a problem 
 */
static int pr_mctp(int key, char *arg, struct argp_state *state)
{
	struct opt *opts, *o;
	int rv = 0;

	opts = (struct opt*) state->input;

	// Call common flag processing first
	rv = pr_common(key, arg, state, CLAP_MCTP, ao_mctp);

	switch (key)
	{
		// Get Remote Endpoint ID
		case 'g': 
			// Set Command 
			o = &opts[CLOP_CMD];
			o->set = 1;
			o->val = CLCM_MCTP_GET_EID;
			break;

		// Get MCTP Version Support
		case 'r': 
			// Set Command 
			o = &opts[CLOP_CMD];
			o->set = 1;
			o->val = CLCM_MCTP_GET_VER;

			// Store TYPE
			o = &opts[CLOP_MCTP_TYPE];
			o->set = 1;
			o->u8 = hexordec_to_ul(arg);
			break;

		// Set Remote Endpoint ID
		case 's': 
			// Set Command 
			o = &opts[CLOP_CMD];
			o->set = 1;
			o->val = CLCM_MCTP_SET_EID;

			// Store EID 
			o = &opts[CLOP_MCTP_EID];
			o->set = 1;
			o->u8 = hexordec_to_ul(arg);
			break;

		// Get MCTP Message Type Support 
		case 't': 
			// Set Command 
			o = &opts[CLOP_CMD];
			o->set = 1;
			o->val = CLCM_MCTP_GET_TYPE;
			break;

		// Get Remote Endpoint UUID
		case 'u': 
			// Set Command 
			o = &opts[CLOP_CMD];
			o->set = 1;
			o->val = CLCM_MCTP_GET_UUID;
			break;

		// Called for non option parameter
		case ARGP_KEY_ARG: 				
			argp_error (state, "Invalid subcommand"); 
			break;

		// Last call. Verify parameters. Fill in missing values
		case ARGP_KEY_END:				
			// Fail if no command is set 
			if ( !opts[CLOP_CMD].set) {
				print_help(CLAP_MCTP);
				exit(0);
			}
			break;
	} 
	return rv;	
}

/**
 * Parse function for: show
 *
 * @return 0 success, non-zero to indicate a problem 
 */
static int pr_show(int key, char *arg, struct argp_state *state)
{
	struct opt *opts;
	int rv = 0;
	
	opts = (struct opt*) state->input;

	// Call common flag processing first
	rv = pr_common(key, arg, state, CLAP_SHOW, ao_show);

	switch (key)
	{
		// Called for non option parameter
		case ARGP_KEY_ARG: 				
			if (!strcmp(arg, "bos")) 
				rv = argp_parse(&ap_show_bos, state->argc-state->next+1, &state->argv[state->next-1], ARGP_IN_ORDER | ARGP_NO_HELP, 0, opts);

			else if (!strcmp(arg, "identity") || !strcmp(arg, "id") ) 
				rv = argp_parse(&ap_show_identity, state->argc-state->next+1, &state->argv[state->next-1], ARGP_IN_ORDER | ARGP_NO_HELP, 0, opts);

			else if (!strcmp(arg, "ld")) 
				rv = argp_parse(&ap_show_ld, state->argc-state->next+1, &state->argv[state->next-1], ARGP_IN_ORDER | ARGP_NO_HELP, 0, opts);

			else if (!strcmp(arg, "limit")) 
				rv = argp_parse(&ap_show_limit, state->argc-state->next+1, &state->argv[state->next-1], ARGP_IN_ORDER | ARGP_NO_HELP, 0, opts);

			else if (!strcmp(arg, "port") || !strcmp(arg, "ports")) 
				rv = argp_parse(&ap_show_port, state->argc-state->next+1, &state->argv[state->next-1], ARGP_IN_ORDER | ARGP_NO_HELP, 0, opts);

			else if (!strcmp(arg, "qos")) 
				rv = argp_parse(&ap_show_qos, state->argc-state->next+1, &state->argv[state->next-1], ARGP_IN_ORDER | ARGP_NO_HELP, 0, opts);
			
			else if (!strcmp(arg, "switch") || !strcmp(arg, "sw")) 
				rv = argp_parse(&ap_show_switch, state->argc-state->next+1, &state->argv[state->next-1], ARGP_IN_ORDER | ARGP_NO_HELP, 0, opts);

			else if (!strcmp(arg, "vcs")) 
				rv = argp_parse(&ap_show_vcs, state->argc-state->next+1, &state->argv[state->next-1], ARGP_IN_ORDER | ARGP_NO_HELP, 0, opts);

			else if (!strcmp(arg, "device") ||!strcmp(arg, "devices") || !strcmp(arg, "dev")) 
				rv = argp_parse(&ap_show_dev, state->argc-state->next+1, &state->argv[state->next-1], ARGP_IN_ORDER | ARGP_NO_HELP, 0, opts);

			else 
				argp_error (state, "Invalid subcommand"); 

			// Stop current parser
			state->next = state->argc; 	
			break;

		// Last call. Verify parameters. Fill in missing values
		case ARGP_KEY_END:				
			// Fail if no command is set 
			if (!opts[CLOP_CMD].set) {
				if (opts[CLOP_PRNT_OPTS].set)
					print_options_array(opts);
				print_help(CLAP_SHOW);
				exit(0);
			}
			break;
	} 
	return rv;	
}

/**
 * Parse function for: port
 *
 * @return 0 success, non-zero to indicate a problem 
 */
static int pr_port(int key, char *arg, struct argp_state *state)
{
	struct opt *opts;
	int rv = 0;

	opts = (struct opt*) state->input;

	// Call common flag processing first
	rv = pr_common(key, arg, state, CLAP_PORT, ao_port);

	switch (key)
	{
		// Called for non option parameter
		case ARGP_KEY_ARG: 				
			if (!strcmp(arg, "bind")) 
				rv = argp_parse(&ap_port_bind, state->argc-state->next+1, &state->argv[state->next-1], ARGP_IN_ORDER | ARGP_NO_HELP, 0, opts);

			else if (!strcmp(arg, "config") || !strcmp(arg, "cfg")) 
				rv = argp_parse(&ap_port_config, state->argc-state->next+1, &state->argv[state->next-1], ARGP_IN_ORDER | ARGP_NO_HELP, 0, opts);

			else if (!strcmp(arg, "control") || !strcmp(arg, "ctrl")) 
				rv = argp_parse(&ap_port_ctrl, state->argc-state->next+1, &state->argv[state->next-1], ARGP_IN_ORDER | ARGP_NO_HELP, 0, opts);
			
			else if (!strcmp(arg, "unbind")) 
				rv = argp_parse(&ap_port_unbind, state->argc-state->next+1, &state->argv[state->next-1], ARGP_IN_ORDER | ARGP_NO_HELP, 0, opts);

			else if (!strcmp(arg, "connect") || !strcmp(arg, "conn")) 
				rv = argp_parse(&ap_port_connect, state->argc-state->next+1, &state->argv[state->next-1], ARGP_IN_ORDER | ARGP_NO_HELP, 0, opts);

			else if (!strcmp(arg, "disconnect") || !strcmp(arg, "dis")) 
				rv = argp_parse(&ap_port_disconnect, state->argc-state->next+1, &state->argv[state->next-1], ARGP_IN_ORDER | ARGP_NO_HELP, 0, opts);

			else 
				argp_error (state, "Invalid subcommand"); 

			// Stop current parser
			state->next = state->argc; 	
			break;

		// Last call. Verify parameters. Fill in missing values
		case ARGP_KEY_END:				
			// Fail if no command is set 
			if (!opts[CLOP_CMD].set) {
				if (opts[CLOP_PRNT_OPTS].set)
					print_options_array(opts);
				print_help(CLAP_PORT);
				exit(0);
			}
			break;
	} 
	return rv;	
}

/**
 * Parse function for: set
 *
 * @return 0 success, non-zero to indicate a problem 
 */
static int pr_set(int key, char *arg, struct argp_state *state)
{
	struct opt *opts;
	int rv = 0;

	opts = (struct opt*) state->input;

	// Call common flag processing first
	rv = pr_common(key, arg, state, CLAP_SET, ao_set);

	switch (key)
	{
		// Called for non option parameter
		case ARGP_KEY_ARG: 				
			if (!strcmp(arg, "ld")) 
				rv = argp_parse(&ap_set_ld, state->argc-state->next+1, &state->argv[state->next-1], ARGP_IN_ORDER | ARGP_NO_HELP, 0, opts);

			else if (!strcmp(arg, "limit")) 
				rv = argp_parse(&ap_set_limit, state->argc-state->next+1, &state->argv[state->next-1], ARGP_IN_ORDER | ARGP_NO_HELP, 0, opts);

			else if (!strcmp(arg, "qos")) 
				rv = argp_parse(&ap_set_qos, state->argc-state->next+1, &state->argv[state->next-1], ARGP_IN_ORDER | ARGP_NO_HELP, 0, opts);

			else 
				argp_error (state, "Invalid subcommand"); 

			state->next = state->argc; 	// Stop current parser
			break;

		// Last call. Verify parameters. Fill in missing values
		case ARGP_KEY_END:				
			// Fail if no command is set 
			if (!opts[CLOP_CMD].set) {
				if (opts[CLOP_PRNT_OPTS].set)
					print_options_array(opts);
				print_help(CLAP_SET);
				exit(0);
			}
			break;
	} 
	return rv;	
}

/**
 * Parse function for: ld
 *
 * @return 0 success, non-zero to indicate a problem 
 */
static int pr_ld(int key, char *arg, struct argp_state *state)
{
	struct opt *opts;
	int rv = 0;

	opts = (struct opt*) state->input;

	// Call common flag processing first
	rv = pr_common(key, arg, state, CLAP_LD, ao_ld);

	switch (key)
	{
		// Called for non option parameter
		case ARGP_KEY_ARG: 				
			if (!strcmp(arg, "config") || !strcmp(arg, "cfg")) 
				rv = argp_parse(&ap_ld_config, state->argc-state->next+1, &state->argv[state->next-1], ARGP_IN_ORDER | ARGP_NO_HELP, 0, opts);

			else if (!strcmp(arg, "mem")) 
				rv = argp_parse(&ap_ld_mem, state->argc-state->next+1, &state->argv[state->next-1], ARGP_IN_ORDER | ARGP_NO_HELP, 0, opts);

			else 
				argp_error (state, "Invalid subcommand"); 

			// Stop current parser
			state->next = state->argc; 	
			break;

		// Last call. Verify parameters. Fill in missing values
		case ARGP_KEY_END:				
			// Fail if no command is set 
			if (!opts[CLOP_CMD].set) {
				if (opts[CLOP_PRNT_OPTS].set)
					print_options_array(opts);
				print_help(CLAP_LD);
				exit(0);
			}
			break;
	} 
	return rv;	
}

/**
 * Parse function for: aer 
 *
 * @return 0 success, non-zero to indicate a problem 
 */
static int pr_aer(int key, char *arg, struct argp_state *state)
{
	struct opt *opts, *o;
	int rv = 0;

	opts = (struct opt*) state->input;

	// Call common flag processing first
	rv = pr_common(key, arg, state, CLAP_AER, ao_aer);

	// Set Command 
	o = &opts[CLOP_CMD];
	o->set = 1;
	o->val = CLCM_AER;

	switch (key)
	{
		// AER Error (4 Byte HEX)
		case 'e': 
			o = &opts[CLOP_AER_ERROR];
			o->set = 1;
			o->u32 = hexordec_to_ul(arg);
			break;

		// AER TLP Header (32 Byte HEX String)
		case 't': 
		{
			o = &opts[CLOP_AER_HEADER];
			o->set = 1;

			// Allocate memory for the array of __u8 values 
			o->buf = calloc(sizeof(__u8), CLMR_AER_HEADER_LEN);
			o->num = parse_u8_str(o->buf, arg);
			o->len = o->num *sizeof(__u8);

			if (o->len != CLMR_AER_HEADER_LEN) {
				argp_error (state, "Incorrect length of TLP Header"); 
				exit(1);
			}
		}		
			break;

		// Called for non option parameter
		case ARGP_KEY_ARG: 				
			argp_error (state, "Invalid subcommand"); 

			// Stop current parser
			state->next = state->argc; 	
			break;

		// Last call. Verify parameters. Fill in missing values	
		case ARGP_KEY_END:			
			// Fail if one of the required options isn't set
			if ( !opts[CLOP_VCSID].set || !opts[CLOP_VPPBID].set || 
				!opts[CLOP_AER_ERROR].set || !opts[CLOP_AER_HEADER].set) {
				if (opts[CLOP_PRNT_OPTS].set)
					print_options_array(opts);
				print_help(CLAP_AER);
				exit(0);
			}
			break;
	} 
	return rv;	
}

/**
 * Parse function for: show bos
 *
 * @return 0 success, non-zero to indicate a problem 
 */
static int pr_show_bos(int key, char *arg, struct argp_state *state)
{
	struct opt *opts, *o;
	int rv = 0;

	opts = (struct opt*) state->input;

	// Call common flag processing first
	rv = pr_common(key, arg, state, CLAP_SHOW_BOS, ao_show_bos);

	// Set Command 
	o = &opts[CLOP_CMD];
	o->set = 1;
	o->val = CLCM_SHOW_BOS;

	switch (key)
	{
		// Called for non option parameter
		case ARGP_KEY_ARG: 				
			argp_error (state, "Invalid subcommand"); 

			// Stop current parser
			state->next = state->argc; 	
			break;

		// Last call. Verify parameters. Fill in missing values
		case ARGP_KEY_END:				
			break;
	} 
	return rv;	
}

/**
 * Parse function for: show identity 
 *
 * @return 0 success, non-zero to indicate a problem 
 */
static int pr_show_identity(int key, char *arg, struct argp_state *state)
{
	struct opt *opts, *o;
	int rv = 0;

	opts = (struct opt*) state->input;

	// Call common flag processing first
	rv = pr_common(key, arg, state, CLAP_SHOW_IDENTITY, ao_show_identity);

	// Set Command 
	o = &opts[CLOP_CMD];
	o->set = 1;
	o->val = CLCM_SHOW_IDENTITY;

	switch (key)
	{
		// Called for non option parameter
		case ARGP_KEY_ARG: 				
			argp_error (state, "Invalid subcommand"); 

			// Stop current parser
			state->next = state->argc; 	
			break;

		// Last call. Verify parameters. Fill in missing values
		case ARGP_KEY_END:				
			break;
	} 
	return rv;	
}

/**
 * Parse function for: show limit
 *
 * @return 0 success, non-zero to indicate a problem 
 */
static int pr_show_limit(int key, char *arg, struct argp_state *state)
{
	struct opt *opts, *o;
	int rv = 0;

	opts = (struct opt*) state->input;

	// Call common flag processing first
	rv = pr_common(key, arg, state, CLAP_SHOW_IDENTITY, ao_show_identity);

	// Set Command 
	o = &opts[CLOP_CMD];
	o->set = 1;
	o->val = CLCM_SHOW_MSG_LIMIT;

	switch (key)
	{
		// Called for non option parameter
		case ARGP_KEY_ARG: 				
			argp_error (state, "Invalid subcommand"); 

			// Stop current parser
			state->next = state->argc; 	
			break;

		// Last call. Verify parameters. Fill in missing values
		case ARGP_KEY_END:				
			break;
	} 
	return rv;	
}

/**
 * Parse function for: show switch 
 *
 * @return 0 success, non-zero to indicate a problem 
 */
static int pr_show_switch(int key, char *arg, struct argp_state *state)
{
	struct opt *opts, *o;
	int rv = 0;

	opts = (struct opt*) state->input;

	// Call common flag processing first
	rv = pr_common(key, arg, state, CLAP_SHOW_SWITCH, ao_show_switch);

	// Set Command 
	o = &opts[CLOP_CMD];
	o->set = 1;
	o->val = CLCM_SHOW_SWITCH;

	switch (key)
	{
		// Called for non option parameter
		case ARGP_KEY_ARG: 				
			argp_error (state, "Invalid subcommand"); 

			// Stop current parser
			state->next = state->argc; 	
			break;

		// Last call. Verify parameters. Fill in missing values
		case ARGP_KEY_END:				
			break;
	} 
	return rv;	
}

/**
 * Parse function for: show dev
 *
 * @return 0 success, non-zero to indicate a problem 
 */
static int pr_show_dev(int key, char *arg, struct argp_state *state)
{
	struct opt *opts, *o;
	int rv = 0;

	opts = (struct opt*) state->input;

	// Call common flag processing first
	rv = pr_common(key, arg, state, CLAP_SHOW_DEV, ao_show_dev);

	// Set Command 
	o = &opts[CLOP_CMD];
	o->set = 1;
	o->val = CLCM_SHOW_DEV;

	switch (key)
	{
		// Device ID 
		case 'a': 
			o = &opts[CLOP_ALL];
			o->set = 1;
			break;

		// Device ID 
		case 'd': 
			o = &opts[CLOP_DEVICE];
			o->set = 1;
			o->u8 = hexordec_to_ul(arg);
			break;

		// Called for positional parameter
		case ARGP_KEY_ARG: 				
		{
			int range, i, len;

			o = &opts[CLOP_DEVICE];
			o->set = 1;

			// Detect range 
			range = 0;
			len = strnlen(arg, 256);
			for ( i = 0 ; i < len ; i++ )
				if (arg[i] == ',' || arg[i] == '-')
					range = 1;

			// If there is a command or dash, convert to a range
			if (range) 
			{
				o->buf = malloc(256);
				memset(o->buf, 0, 256);
				o->num = range_to_u8_array(o->buf, arg);
				o->len = o->num * sizeof(__u8);
			}
			else 
				o->u8 = hexordec_to_ul(arg);
		}
			// Stop current parser
			state->next = state->argc; 	
			break;

		// Last call. Verify parameters. Fill in missing values
		case ARGP_KEY_END:				

			// If no port id was set, select all
			if (!opts[CLOP_DEVICE].set) 
				opts[CLOP_ALL].set = 1;
		
			break;
	} 
	return rv;	
}

/**
 * Parse function for: show port 
 *
 * @return 0 success, non-zero to indicate a problem 
 */
static int pr_show_port(int key, char *arg, struct argp_state *state)
{
	struct opt *opts, *o;
	int rv = 0;

	opts = (struct opt*) state->input;

	// Call common flag processing first
	rv = pr_common(key, arg, state, CLAP_SHOW_PORT, ao_show_port);

	// Set Command 
	o = &opts[CLOP_CMD];
	o->set = 1;
	o->val = CLCM_SHOW_PORT;

	switch (key)
	{
		// Called for positional  parameter
		case ARGP_KEY_ARG: 				
		{
			int range, i, len;

			o = &opts[CLOP_PPID];
			o->set = 1;

			// Detect range 
			range = 0;
			len = strnlen(arg, 256);
			for ( i = 0 ; i < len ; i++ )
				if (arg[i] == ',' || arg[i] == '-')
					range = 1;

			// If there is a command or dash, convert to a range
			if (range) 
			{
				o->buf = malloc(256);
				memset(o->buf, 0, 256);
				o->num = range_to_u8_array(o->buf, arg);
				o->len = o->num * sizeof(__u8);
			}
			else 
				o->u8 = hexordec_to_ul(arg);
		}
			// Stop current parser
			state->next = state->argc; 	
			break;

		// Last call. Verify parameters. Fill in missing values
		case ARGP_KEY_END:				

			// If no port id was set, select all
			if (!opts[CLOP_PPID].set) 
				opts[CLOP_ALL].set = 1;
		
			break;
	} 
	return rv;	
}

/**
 * Parse function for: show vcs
 *
 * @return 0 success, non-zero to indicate a problem 
 */
static int pr_show_vcs(int key, char *arg, struct argp_state *state)
{
	struct opt *opts, *o;
	int rv = 0;

	opts = (struct opt*) state->input;

	// Call common flag processing first
	rv = pr_common(key, arg, state, CLAP_SHOW_VCS, ao_show_vcs);

	// Set Command 
	o = &opts[CLOP_CMD];
	o->set = 1;
	o->val = CLCM_SHOW_VCS;

	switch (key)
	{
		// Called for non option parameter
		case ARGP_KEY_ARG: 				
		{
			int range, i, len;

			o = &opts[CLOP_VCSID];
			o->set = 1;

			// Detect range 
			range = 0;
			len = strnlen(arg, 256);
			for ( i = 0 ; i < len ; i++ )
				if (arg[i] == ',' || arg[i] == '-')
					range = 1;

			// If there is a command or dash, convert to a range
			if (range) 
			{
				o->buf = malloc(256);
				memset(o->buf, 0, 256);
				o->num = range_to_u8_array(o->buf, arg);
				o->len = o->num * sizeof(__u8);
			}
			else 
				o->u8 = hexordec_to_ul(arg);
		}	

			// Stop current parser
			state->next = state->argc; 	
			break;

		// Last call. Verify parameters. Fill in missing values	
		case ARGP_KEY_END:				
			
			// If a VCS ID was not specified, set all
			if (!opts[CLOP_VCSID].set)
				opts[CLOP_ALL].set = 1; 

			break;
	} 
	return rv;	
}

/**
 * Parse function for: show qos
 *
 * @return 0 success, non-zero to indicate a problem 
 */
static int pr_show_qos(int key, char *arg, struct argp_state *state)
{
	struct opt *opts;
	int rv = 0;

	opts = (struct opt*) state->input;

	// Call common flag processing first
	rv = pr_common(key, arg, state, CLAP_SHOW_QOS, ao_show_qos);

	switch (key)
	{
		// Called for non option parameter
		case ARGP_KEY_ARG: 				
			if (!strcmp(arg, "allocated") || !strcmp(arg, "alloc")) 
				rv = argp_parse(&ap_show_qos_allocated, state->argc-state->next+1, &state->argv[state->next-1], ARGP_IN_ORDER | ARGP_NO_HELP, 0, opts);

			else if (!strcmp(arg, "control") || !strcmp(arg, "ctrl")) 
				rv = argp_parse(&ap_show_qos_control, state->argc-state->next+1, &state->argv[state->next-1], ARGP_IN_ORDER | ARGP_NO_HELP, 0, opts);

			else if (!strcmp(arg, "limit")) 
				rv = argp_parse(&ap_show_qos_limit, state->argc-state->next+1, &state->argv[state->next-1], ARGP_IN_ORDER | ARGP_NO_HELP, 0, opts);
			
			else if (!strcmp(arg, "status") || !strcmp(arg, "st")) 
				rv = argp_parse(&ap_show_qos_status, state->argc-state->next+1, &state->argv[state->next-1], ARGP_IN_ORDER | ARGP_NO_HELP, 0, opts);

			else 
				argp_error (state, "Invalid subcommand"); 

			// Stop current parser
			state->next = state->argc; 	
			break;

		// Last call. Verify parameters. Fill in missing values
		case ARGP_KEY_END:				
			// Fail if no command is set 
			if ( !opts[CLOP_CMD].set) {
				if (opts[CLOP_PRNT_OPTS].set)
					print_options_array(opts);
				print_help(CLAP_SHOW_QOS);
				exit(0);
			}
			break;
	} 
	return rv;	
}

/**
 * Parse function for: show ld
 *
 * @return 0 success, non-zero to indicate a problem 
 */
static int pr_show_ld(int key, char *arg, struct argp_state *state)
{
	struct opt *opts;
	int rv = 0;

	opts = (struct opt*) state->input;

	// Call common flag processing first
	rv = pr_common(key, arg, state, CLAP_SHOW_LD, ao_show_ld);

	switch (key)
	{
		// Called for non option parameter
		case ARGP_KEY_ARG: 				
			if (!strcmp(arg, "allocations") || !strcmp(arg, "alloc")) 
				rv = argp_parse(&ap_show_ld_allocations, state->argc-state->next+1, &state->argv[state->next-1], ARGP_IN_ORDER | ARGP_NO_HELP, 0, opts);

			else if (!strcmp(arg, "info")) 
				rv = argp_parse(&ap_show_ld_info, state->argc-state->next+1, &state->argv[state->next-1], ARGP_IN_ORDER | ARGP_NO_HELP, 0, opts);

			else 
				argp_error (state, "Invalid subcommand"); 

			// Stop current parser
			state->next = state->argc; 	
			break;

		// Last call. Verify parameters. Fill in missing values
		case ARGP_KEY_END:				
			// Fail if no command is set 
			if ( !opts[CLOP_CMD].set) {
				if (opts[CLOP_PRNT_OPTS].set)
					print_options_array(opts);
				print_help(CLAP_SHOW_LD);
				exit(0);
			}
			break;
	} 
	return rv;	
}

/**
 * Parse function for: port bind
 *
 * @return 0 success, non-zero to indicate a problem 
 */
static int pr_port_bind(int key, char *arg, struct argp_state *state)
{
	struct opt *opts, *o;
	int rv = 0;

	opts = (struct opt*) state->input;

	// Call common flag processing first
	rv = pr_common(key, arg, state, CLAP_PORT_BIND, ao_port_bind);

	// Set Command 
	o = &opts[CLOP_CMD];
	o->set = 1;
	o->val = CLCM_PORT_BIND;

	switch (key)
	{
		// Called for non option parameter
		case ARGP_KEY_ARG: 				
			argp_error (state, "Invalid subcommand"); 

			// Stop current parser
			state->next = state->argc; 	
			break;

		// Last call. Verify parameters. Fill in missing values
		case ARGP_KEY_END:				
			// Fail if one of the required options isn't set
			if ( !opts[CLOP_VCSID].set || !opts[CLOP_PPID].set || 
				!opts[CLOP_VPPBID].set) {
				if (opts[CLOP_PRNT_OPTS].set)
					print_options_array(opts);
				print_help(CLAP_PORT_BIND);
				exit(0);
			}
			break;
	} 
	return rv;	
}

/**
 * Parse function for: port unbind
 *
 * @return 0 success, non-zero to indicate a problem 
 */
static int pr_port_unbind(int key, char *arg, struct argp_state *state)
{
	struct opt *opts, *o;
	int rv = 0;

	opts = (struct opt*) state->input;

	// Call common flag processing first
	rv = pr_common(key, arg, state, CLAP_PORT_UNBIND, ao_port_unbind);

	// Set Command 
	o = &opts[CLOP_CMD];
	o->set = 1;
	o->val = CLCM_PORT_UNBIND;

	switch (key)
	{
		// Simulate Managed Hot-Remove
		case 'm': 
			o = &opts[CLOP_UNBIND_MODE];
			o->set = 1;
			o->val = CLPU_MANAGED;
			break;

		// Simulate Surpise Hot-Remove
		case 's': 
			o = &opts[CLOP_UNBIND_MODE];
			o->set = 1;
			o->val = CLPU_SURPRISE;
			break;

		// Wait for port link down before unbinding
		case 'w': 
			o = &opts[CLOP_UNBIND_MODE];
			o->set = 1;
			o->val = CLPU_WAIT;
			break;

		// Called for non option parameter
		case ARGP_KEY_ARG: 				
			argp_error (state, "Invalid subcommand"); 

			// Stop current parser
			state->next = state->argc; 	
			break;

		// Last call. Verify parameters. Fill in missing values
		case ARGP_KEY_END:				
			// Check for required parameters
			if (!opts[CLOP_VCSID].set || !opts[CLOP_VPPBID].set) {
				if (opts[CLOP_PRNT_OPTS].set)
					print_options_array(opts);
				print_help(CLAP_PORT_UNBIND);
				exit(0);
			}
			
			// Default to Surprise hot plug if not specified
			if (!opts[CLOP_UNBIND_MODE].set) {
				opts[CLOP_UNBIND_MODE].set = 1;
				opts[CLOP_UNBIND_MODE].val = CLPU_SURPRISE;
			}

			break;
	} 
	return rv;	
}

/**
 * Parse function for: port config
 *
 * @return 0 success, non-zero to indicate a problem 
 */
static int pr_port_config(int key, char *arg, struct argp_state *state)
{
	struct opt *opts, *o;
	int rv = 0;

	opts = (struct opt*) state->input;

	// Call common flag processing first
	rv = pr_common(key, arg, state, CLAP_PORT_CONFIG, ao_port_config);

	// Set Command 
	o = &opts[CLOP_CMD];
	o->set = 1;
	o->val = CLCM_PORT_CONFIG;

	switch (key)
	{
		// Extended Register Number
		case 'e': 
			o = &opts[CLOP_EXT_REGISTER];
			o->set = 1;
			o->u8 = hexordec_to_ul(arg);
			break;

		// First Dword Byte Enable
		case 'f': 
			o = &opts[CLOP_FDBE];
			o->set = 1;
			o->u8 = hexordec_to_ul(arg);
			break;

		// Register Number
		case 'r': 
			o = &opts[CLOP_REGISTER];
			o->set = 1;
			o->u8 = hexordec_to_ul(arg);
			break;

		// Perform a Write transaction
		case 'w': 
			o = &opts[CLOP_WRITE];
			o->set = 1;
			break;

		// Immediate data for write transaction
		case 703: 
			o = &opts[CLOP_DATA];
			o->set = 1;
			o->u32 = hexordec_to_ul(arg);
			break;

		// Called for non option parameter
		case ARGP_KEY_ARG: 				
			argp_error (state, "Invalid subcommand"); 

			// Stop current parser
			state->next = state->argc; 	
			break;

		// Last call. Verify parameters. Fill in missing values
		case ARGP_KEY_END:				
			// Check for mandatory options
			if ( !opts[CLOP_PPID].set ) {
				if (opts[CLOP_PRNT_OPTS].set)
					print_options_array(opts);
				print_help(CLAP_PORT_CONFIG);
				exit(0);
			}
			break;
	} 
	return rv;	
}

/**
 * Parse function for: port connect
 *
 * @return 0 success, non-zero to indicate a problem 
 */
static int pr_port_connect(int key, char *arg, struct argp_state *state)
{
	struct opt *opts, *o;
	int rv = 0;

	opts = (struct opt*) state->input;

	// Call common flag processing first
	rv = pr_common(key, arg, state, CLAP_PORT_CONN, ao_port_connect);

	// Set Command 
	o = &opts[CLOP_CMD];
	o->set = 1;
	o->val = CLCM_PORT_CONN;

	switch (key)
	{
		// Device ID 
		case 'd': 
			o = &opts[CLOP_DEVICE];
			o->set = 1;
			o->u8 = hexordec_to_ul(arg);
			break;

		// Called for non option parameter
		case ARGP_KEY_ARG: 				
			argp_error (state, "Invalid subcommand"); 

			// Stop current parser
			state->next = state->argc; 	
			break;

		// Last call. Verify parameters. Fill in missing values
		case ARGP_KEY_END:				
			// Check for mandatory options
			if ( !opts[CLOP_PPID].set ) {
				if (opts[CLOP_PRNT_OPTS].set)
					print_options_array(opts);
				print_help(CLAP_PORT_CONFIG);
				exit(0);
			}
			break;
	} 
	return rv;	
}

/**
 * Parse function for: port disconnect
 *
 * @return 0 success, non-zero to indicate a problem 
 */
static int pr_port_disconnect(int key, char *arg, struct argp_state *state)
{
	struct opt *opts, *o;
	int rv = 0;

	opts = (struct opt*) state->input;

	// Call common flag processing first
	rv = pr_common(key, arg, state, CLAP_PORT_DISCONN, ao_port_disconnect);

	// Set Command 
	o = &opts[CLOP_CMD];
	o->set = 1;
	o->val = CLCM_PORT_DISCONN;

	switch (key)
	{
		case 'a': 
			opts[CLOP_ALL].set = 1;
			break;

		// Called for non option parameter
		case ARGP_KEY_ARG: 				

			opts[CLOP_PPID].set = 1;
			opts[CLOP_PPID].u8 = hexordec_to_ul(arg);;

			// Stop current parser
			state->next = state->argc; 	
			break;

		// Last call. Verify parameters. Fill in missing values
		case ARGP_KEY_END:				
			// Check for mandatory options
			if ( !opts[CLOP_PPID].set && !opts[CLOP_ALL].set) {
				if (opts[CLOP_PRNT_OPTS].set)
					print_options_array(opts);
				print_help(CLAP_PORT_CONFIG);
				exit(0);
			}
			break;
	} 
	return rv;	
}

/**
 * Parse function for: port control
 *
 * @return 0 success, non-zero to indicate a problem 
 */
static int pr_port_ctrl(int key, char *arg, struct argp_state *state)
{
	struct opt *opts, *o;
	int rv = 0;

	opts = (struct opt*) state->input;

	// Call common flag processing first
	rv = pr_common(key, arg, state, CLAP_PORT_CTRL, ao_port_ctrl);

	// Set Command 
	o = &opts[CLOP_CMD];
	o->set = 1;
	o->val = CLCM_PORT_CTRL;

	switch (key)
	{
		// Assert PERST
		case 'a': 
			o = &opts[CLOP_PORT_CONTROL];
			o->set = 1;
			o->val = CLPC_ASSERT;
			break;

		// Deassert PERST
		case 'd': 
			o = &opts[CLOP_PORT_CONTROL];
			o->set = 1;
			o->val = CLPC_DEASSERT;
			break;

		// Reset PCIe-to-PCIe Bridge
		case 'r': 
			o = &opts[CLOP_PORT_CONTROL];
			o->set = 1;
			o->val = CLPC_RESET;
			break;

		// Called for non option parameter
		case ARGP_KEY_ARG: 				
			argp_error (state, "Invalid subcommand"); 

			// Stop current parser
			state->next = state->argc; 	
			break;

		// Last call. Verify parameters. Fill in missing values
		case ARGP_KEY_END:				
			// Check for mandatory options
			if ( !opts[CLOP_PPID].set ) {
				if (opts[CLOP_PRNT_OPTS].set)
					print_options_array(opts);
				print_help(CLAP_PORT_CTRL);
				exit(0);
			}

			// Default to reset action if opcode flag not provided 
			if ( !opts[CLOP_PORT_CONTROL].set ) {
				opts[CLOP_PORT_CONTROL].set = 1;
				opts[CLOP_PORT_CONTROL].val = CLPC_RESET;
			}
			break;
	} 
	return rv;	
}

/**
 * Parse function for: set ld
 *
 * @return 0 success, non-zero to indicate a problem 
 */
static int pr_set_ld(int key, char *arg, struct argp_state *state)
{
	struct opt *opts;
	int rv = 0;

	opts = (struct opt*) state->input;

	// Call common flag processing first
	rv = pr_common(key, arg, state, CLAP_SET_LD, ao_set_ld);

	switch (key)
	{
		// Called for non option parameter
		case ARGP_KEY_ARG: 				
			if (!strcmp(arg, "allocations") || !strcmp(arg, "alloc")) 
				rv = argp_parse(&ap_set_ld_allocations, state->argc-state->next+1, &state->argv[state->next-1], ARGP_IN_ORDER | ARGP_NO_HELP, 0, opts);

			else 
				argp_error (state, "Invalid subcommand"); 

			// Stop current parser
			state->next = state->argc; 	
			break;

		// Last call. Verify parameters. Fill in missing values
		case ARGP_KEY_END:				
			// Fail if no command is set 
			if ( !opts[CLOP_CMD].set) {
				if (opts[CLOP_PRNT_OPTS].set)
					print_options_array(opts);
				print_help(CLAP_SET_LD);
				exit(0);
			}
			break;
	} 
	return rv;	
}

/**
 * Parse function for: set qos
 *
 * @return 0 success, non-zero to indicate a problem 
 */
static int pr_set_qos(int key, char *arg, struct argp_state *state)
{
	struct opt *opts;
	int rv = 0;

	opts = (struct opt*) state->input;

	// Call common flag processing first
	rv = pr_common(key, arg, state, CLAP_SET_QOS, ao_set_qos);

	switch (key)
	{
		// Called for non option parameter
		case ARGP_KEY_ARG: 				
			if (!strcmp(arg, "allocated") || !strcmp(arg, "alloc")) 
				rv = argp_parse(&ap_set_qos_allocated, state->argc-state->next+1, &state->argv[state->next-1], ARGP_IN_ORDER | ARGP_NO_HELP, 0, opts);

			else if (!strcmp(arg, "control") || !strcmp(arg, "ctrl")) 
				rv = argp_parse(&ap_set_qos_control, state->argc-state->next+1, &state->argv[state->next-1], ARGP_IN_ORDER | ARGP_NO_HELP, 0, opts);

			else if (!strcmp(arg, "limit")) 
				rv = argp_parse(&ap_set_qos_limit, state->argc-state->next+1, &state->argv[state->next-1], ARGP_IN_ORDER | ARGP_NO_HELP, 0, opts);

			else 
				argp_error (state, "Invalid subcommand"); 

			// Stop current parser
			state->next = state->argc; 	
			break;

		// Last call. Verify parameters. Fill in missing values
		case ARGP_KEY_END:				
			// Fail if no command is set 
			if ( !opts[CLOP_CMD].set) {
				if (opts[CLOP_PRNT_OPTS].set)
					print_options_array(opts);
				print_help(CLAP_SET_QOS);
				exit(0);
			}
			break;
	} 
	return rv;	
}

/**
 * Parse function for: ld config
 *
 * @return 0 success, non-zero to indicate a problem 
 */
static int pr_ld_config(int key, char *arg, struct argp_state *state)
{
	struct opt *opts, *o;
	int rv = 0;

	opts = (struct opt*) state->input;

	// Call common flag processing first
	rv = pr_common(key, arg, state, CLAP_LD_CONFIG, ao_ld_config);

	// Set Command 
	o = &opts[CLOP_CMD];
	o->set = 1;
	o->val = CLCM_LD_CONFIG;

	switch (key)
	{
		// Extended Register Number
		case 'e': 
			o = &opts[CLOP_EXT_REGISTER];
			o->set = 1;
			o->u8 = hexordec_to_ul(arg);
			break;

		// First Dword Byte Enable
		case 'f': 
			o = &opts[CLOP_FDBE];
			o->set = 1;
			o->u8 = hexordec_to_ul(arg);
			break;

		// Register Number
		case 'r': 
			o = &opts[CLOP_REGISTER];
			o->set = 1;
			o->u8 = hexordec_to_ul(arg);
			break;

		// Write Data 
		case 'w': 
			o = &opts[CLOP_WRITE];
			o->set = 1;
			break;

		// Immediate data for write transaction
		case 703:
			o = &opts[CLOP_DATA];
			o->set = 1;
			o->u32 = hexordec_to_ul(arg);
			break;

		// Called for non option parameter
		case ARGP_KEY_ARG: 				
			argp_error (state, "Invalid subcommand"); 

			// Stop current parser
			state->next = state->argc; 	
			break;

		// Last call. Verify parameters. Fill in missing values
		case ARGP_KEY_END:				
			// Check for mandatory options
			if ( !opts[CLOP_PPID].set ) {
				if (opts[CLOP_PRNT_OPTS].set)
					print_options_array(opts);
				print_help(CLAP_LD_CONFIG);
				exit(0);
			}
			break;
	} 
	return rv;	
}

/**
 * Parse function for: ld mem
 *
 * @return 0 success, non-zero to indicate a problem 
 */
static int pr_ld_mem(int key, char *arg, struct argp_state *state)
{
	struct opt *opts, *o;
	int rv = 0;

	opts = (struct opt*) state->input;

	// Call common flag processing first
	rv = pr_common(key, arg, state, CLAP_LD_MEM, ao_ld_mem);

	// Set Command 
	o = &opts[CLOP_CMD];
	o->set = 1;
	o->val = CLCM_LD_MEM;

	switch (key)
	{
		// Last Dword Byte Enable
		case 'd': 
			o = &opts[CLOP_LDBE];
			o->set = 1;
			o->u8 = hexordec_to_ul(arg);
			break;

		// First Dword Byte Enable
		case 'f': 
			o = &opts[CLOP_FDBE];
			o->set = 1;
			o->u8 = hexordec_to_ul(arg);
			break;

		// Transaction Data Length (up to 4KB)
		case 'n': 
			o = &opts[CLOP_LEN];
			o->set = 1;
			o->len = hexordec_to_ul(arg);
			
			if (o->len > CLMR_MAX_LD_MEM_LEN ) {
				argp_error(state, "Length exceeds maximum supported value: CLMR_MAX_LD_MEM_LEN\n");
				exit(1);
			}
			break;

		// Transaction Offset in tareget's memory space
		case 'o': 
			o = &opts[CLOP_OFFSET];
			o->set = 1;
			o->u64 = hexordec_to_ull(arg);
			break;

		// Write Transacrtion
		case 'w': 
			o = &opts[CLOP_WRITE];
			o->set = 1;
			break;

		// Immediate data for write transaction
		case 703: 
			o = &opts[CLOP_DATA];
			o->set = 1;
			o->u32 = hexordec_to_ul(arg);
			break;

		// Filename for input file
		case 704: 
			o = &opts[CLOP_INFILE];
			o->set = 1;
			o->str = strdup(arg);
			break;

		// Called for non option parameter
		case ARGP_KEY_ARG: 				
			argp_error (state, "Invalid subcommand"); 

			// Stop current parser
			state->next = state->argc; 	
			break;

		// Last call. Verify parameters. Fill in missing values
		case ARGP_KEY_END:				

			// Check for mandatory options
			if ( !opts[CLOP_PPID].set || !opts[CLOP_LEN].set ) {
				if (opts[CLOP_PRNT_OPTS].set)
					print_options_array(opts);
				print_help(CLAP_LD_MEM);
				exit(0);
			}

			// Validate len
			if (opts[CLOP_LEN].set) {
				if (opts[CLOP_LEN].len == 0) {
					if (opts[CLOP_PRNT_OPTS].set)
						print_options_array(opts);
					argp_error(state, "Length must be greater than zero.");
					exit(1);
				}
			}

			// If input filename is provided, load into buffer in option
			o = &opts[CLOP_INFILE];
			if (o->set) {
				int rv; 
				FILE *fd;

				// Open File 
				fd = fopen(o->str, "rb");
				if (!fd) {
					if (opts[CLOP_PRNT_OPTS].set)
						print_options_array(opts);
					argp_error(state, "Could not open file");
					exit(1);
				}

				// Allocate memory to read the file into 
				o->buf = calloc(1, CLMR_MAX_LD_MEM_LEN);
				if (!o->buf) {
					if (opts[CLOP_PRNT_OPTS].set)
						print_options_array(opts);
					argp_error(state, "Could not allocation memory for file");
					fclose(fd);
					exit(1);
				}

				// Read in the file up to the max support buffer len
				rv = fread(o->buf, sizeof(o->buf[0]), CLMR_MAX_LD_MEM_LEN, fd);
				if (!rv) {
					if (opts[CLOP_PRNT_OPTS].set)
						print_options_array(opts);
					argp_error(state, "Could not read file");
					fclose(fd);
					exit(1);
				}
				
				// Close the file 
				fclose(fd);

				// Set the actual length of data in the buffer 
				o->len = rv;
			}
			break;
	} 
	return rv;	
}

/**
 * Parse function for: show qos allocated
 *
 * @return 0 success, non-zero to indicate a problem 
 */
static int pr_show_qos_allocated(int key, char *arg, struct argp_state *state)
{
	struct opt *opts, *o;
	int rv = 0;

	opts = (struct opt*) state->input;

	// Call common flag processing first
	rv = pr_common(key, arg, state, CLAP_SHOW_QOS_ALLOCATED, ao_show_qos_allocated);

	// Set Command 
	o = &opts[CLOP_CMD];
	o->set = 1;
	o->val = CLCM_SHOW_QOS_ALLOCATED;

	switch (key)
	{
		case 'n': 
			o = &opts[CLOP_NUM];
			o->set = 1;
			o->u8 = hexordec_to_ul(arg);
			break;

		// Called for non option parameter
		case ARGP_KEY_ARG: 				
			argp_error (state, "Invalid subcommand"); 

			// Stop current parser
			state->next = state->argc; 	
			break;

		// Last call. Verify parameters. Fill in missing values
		case ARGP_KEY_END:				
			// Fail if no port id was set
			if (!opts[CLOP_PPID].set) {
				if (opts[CLOP_PRNT_OPTS].set)
					print_options_array(opts);
				print_help(CLAP_SHOW_QOS_ALLOCATED);
				exit(0);
			}
			break;
	} 
	return rv;	
}

/**
 * Parse function for: show qos control
 *
 * @return 0 success, non-zero to indicate a problem 
 */
static int pr_show_qos_control(int key, char *arg, struct argp_state *state)
{
	struct opt *opts, *o;
	int rv = 0;

	opts = (struct opt*) state->input;

	// Call common flag processing first
	rv = pr_common(key, arg, state, CLAP_SHOW_QOS_CONTROL, ao_show_qos_control);

	// Set Command 
	o = &opts[CLOP_CMD];
	o->set = 1;
	o->val = CLCM_SHOW_QOS_CONTROL;

	switch (key)
	{
		// Called for non option parameter
		case ARGP_KEY_ARG: 				
			argp_error (state, "Invalid subcommand"); 

			// Stop current parser
			state->next = state->argc; 	
			break;

		// Last call. Verify parameters. Fill in missing values
		case ARGP_KEY_END:				
			// Fail if no port id was set
			if (!opts[CLOP_PPID].set) {
				if (opts[CLOP_PRNT_OPTS].set)
					print_options_array(opts);
				print_help(CLAP_SHOW_QOS_CONTROL);
				exit(0);
			}
			break;
	} 
	return rv;	
}

/**
 * Parse function for: show qos limit
 *
 * @return 0 success, non-zero to indicate a problem 
 */
static int pr_show_qos_limit(int key, char *arg, struct argp_state *state)
{
	struct opt *opts, *o;
	int rv = 0;

	opts = (struct opt*) state->input;

	// Call common flag processing first
	rv = pr_common(key, arg, state, CLAP_SHOW_QOS_LIMIT, ao_show_qos_limit);

	// Set Command 
	o = &opts[CLOP_CMD];
	o->set = 1;
	o->val = CLCM_SHOW_QOS_LIMIT;

	switch (key)
	{
		case 'n': 
			o = &opts[CLOP_NUM];
			o->set = 1;
			o->u8 = hexordec_to_ul(arg);
			break;

		// Called for non option parameter
		case ARGP_KEY_ARG: 				
			argp_error (state, "Invalid subcommand"); 

			// Stop current parser
			state->next = state->argc; 	
			break;

		// Last call. Verify parameters. Fill in missing values
		case ARGP_KEY_END:				
			// Fail if no port id was set
			if (!opts[CLOP_PPID].set) {
				if (opts[CLOP_PRNT_OPTS].set)
					print_options_array(opts);
				print_help(CLAP_SHOW_QOS_LIMIT);
				exit(0);
			}
			break;
	} 
	return rv;	
}

/**
 * Parse function for: show qos status 
 *
 * @return 0 success, non-zero to indicate a problem 
 */
static int pr_show_qos_status(int key, char *arg, struct argp_state *state)
{
	struct opt *opts, *o;
	int rv = 0;

	opts = (struct opt*) state->input;

	// Call common flag processing first
	rv = pr_common(key, arg, state, CLAP_SHOW_QOS_STATUS, ao_show_qos_status);

	// Set Command 
	o = &opts[CLOP_CMD];
	o->set = 1;
	o->val = CLCM_SHOW_QOS_STATUS;

	switch (key)
	{
		// Called for non option parameter
		case ARGP_KEY_ARG: 				
			argp_error (state, "Invalid subcommand"); 

			// Stop current parser
			state->next = state->argc; 	
			break;

		// Last call. Verify parameters. Fill in missing values
		case ARGP_KEY_END:				
			// Fail if no port id was set
			if (!opts[CLOP_PPID].set) {
				if (opts[CLOP_PRNT_OPTS].set)
					print_options_array(opts);
				print_help(CLAP_SHOW_QOS_STATUS);
				exit(0);
			}
			break;
	} 
	return rv;	
}

/**
 * Parse function for: show ld allocations
 *
 * @return 0 success, non-zero to indicate a problem 
 */
static int pr_show_ld_allocations(int key, char *arg, struct argp_state *state)
{
	struct opt *opts, *o;
	int rv = 0;

	opts = (struct opt*) state->input;

	// Call common flag processing first
	rv = pr_common(key, arg, state, CLAP_SHOW_LD_ALLOCATIONS, ao_show_ld_allocations);

	// Set Command 
	o = &opts[CLOP_CMD];
	o->set = 1;
	o->val = CLCM_SHOW_LD_ALLOCATIONS;

	switch (key)
	{
		// Called for non option parameterj
		case ARGP_KEY_ARG: 				
			argp_error (state, "Invalid subcommand"); 

			// Stop current parser
			state->next = state->argc; 	
			break;

		// Last call. Verify parameters. Fill in missing values
		case ARGP_KEY_END:				
			// Fail if no port id was set
			if (!opts[CLOP_PPID].set) {
				if (opts[CLOP_PRNT_OPTS].set)
					print_options_array(opts);
				print_help(CLAP_SHOW_LD_ALLOCATIONS);
				exit(0);
			}
			break;
	} 
	return rv;	
}

/**
 * Parse function for: show ld info
 *
 * @return 0 success, non-zero to indicate a problem 
 */
static int pr_show_ld_info(int key, char *arg, struct argp_state *state)
{
	struct opt *opts, *o;
	int rv = 0;

	opts = (struct opt*) state->input;

	// Call common flag processing first
	rv = pr_common(key, arg, state, CLAP_SHOW_LD_INFO, ao_show_ld_info);

	// Set Command 
	o = &opts[CLOP_CMD];
	o->set = 1;
	o->val = CLCM_SHOW_LD_INFO;

	switch (key)
	{
		// Called for non option parameter
		case ARGP_KEY_ARG: 				
			argp_error (state, "Invalid subcommand"); 

			// Stop current parser
			state->next = state->argc; 	
			break;

		// Last call. Verify parameters. Fill in missing values
		case ARGP_KEY_END:				
			// Fail if no port id was set
			if (!opts[CLOP_PPID].set) {
				if (opts[CLOP_PRNT_OPTS].set)
					print_options_array(opts);
				print_help(CLAP_SHOW_LD_INFO);
				exit(0);
			}
			break;
	} 
	return rv;	
}

/**
 * Parse function for: set ld allocations
 *
 * @return 0 success, non-zero to indicate a problem 
 */
static int pr_set_ld_allocations(int key, char *arg, struct argp_state *state)
{
	struct opt *opts, *o;
	int rv = 0;

	opts = (struct opt*) state->input;

	// Call common flag processing first
	rv = pr_common(key, arg, state, CLAP_SET_LD_ALLOCATIONS, ao_set_ld_allocations);

	// Set Command 
	o = &opts[CLOP_CMD];
	o->set = 1;
	o->val = CLCM_SET_LD_ALLOCATIONS;

	switch (key)
	{
		// Range 1
		case '1': 
		{
			o = &opts[CLOP_LD_RNG1];
			o->set = 1;

			// Allocate memory for the array of __u64 values 
			o->buf = calloc(sizeof(__u64), CLMR_MAX_LD);

			// Parse the commad separated string of hex values
			o->num = parse_u64_csv(o->buf, arg);
			o->len = o->num * sizeof(__u64);
		}
			break;

		// Range 2
		case '2':
		{
			o = &opts[CLOP_LD_RNG2];
			o->set = 1;

			// Allocate memory for the array of __u64 values 
			o->buf = calloc(sizeof(__u64), CLMR_MAX_LD);

			// Parse the commad separated string of hex values
			o->num = parse_u64_csv(o->buf, arg);;
			o->len = o->num * sizeof(__u64);
		}
			break;

		// Called for non option parameter
		case ARGP_KEY_ARG: 				
			argp_error (state, "Invalid subcommand"); 

			// Stop current parser
			state->next = state->argc; 	
			break;

		// Last call. Verify parameters. Fill in missing values
		case ARGP_KEY_END:				
			// If Range 1 or Range 2 was not specified then fail
			if (!opts[CLOP_LD_RNG1].set || !opts[CLOP_LD_RNG2].set) {
				if (opts[CLOP_PRNT_OPTS].set)
					print_options_array(opts);
				print_help(CLAP_SET_LD_ALLOCATIONS);
				exit(0);
			}

			// Fail if no port id was set
			if (!opts[CLOP_PPID].set) {
				if (opts[CLOP_PRNT_OPTS].set)
					print_options_array(opts);
				argp_error(state, "Insufficient Parameters");
			}
			break;
	} 
	return rv;	
}

/**
 * Parse function for: set limit
 *
 * @return 0 success, non-zero to indicate a problem 
 */
static int pr_set_limit(int key, char *arg, struct argp_state *state)
{
	struct opt *opts, *o;
	int rv = 0;

	opts = (struct opt*) state->input;

	// Call common flag processing first
	rv = pr_common(key, arg, state, CLAP_SET_MSG_LIMIT, ao_set_limit);

	// Set Command 
	o = &opts[CLOP_CMD];
	o->set = 1;
	o->val = CLCM_SET_MSG_LIMIT;

	switch (key)
	{
		// limit
		case 'n': 
		{
			o = &opts[CLOP_LIMIT];
			o->set = 1;
			o->u8 = strtoul(arg, NULL, 0);
		}
			break;

		// Called for non option parameter
		case ARGP_KEY_ARG: 				
			o = &opts[CLOP_LIMIT];
			o->set = 1;
			o->u8 = strtoul(arg, NULL, 0);

			// Stop current parser
			state->next = state->argc; 	
			break;

		// Last call. Verify parameters. Fill in missing values
		case ARGP_KEY_END:				
			// If required option not specified then fail
			if (!opts[CLOP_LIMIT].set) {
				if (opts[CLOP_PRNT_OPTS].set)
					print_options_array(opts);
				print_help(CLAP_SET_MSG_LIMIT);
				exit(0);
			}
			break;
	} 
	return rv;	
}

/**
 * Parse function for: set qos allocated
 *
 * @return 0 success, non-zero to indicate a problem 
 */
static int pr_set_qos_allocated(int key, char *arg, struct argp_state *state)
{
	struct opt *opts, *o;
	int rv = 0;

	opts = (struct opt*) state->input;

	// Call common flag processing first
	rv = pr_common(key, arg, state, CLAP_SET_QOS_ALLOCATED, ao_set_qos_allocated);

	// Set Command 
	o = &opts[CLOP_CMD];
	o->set = 1;
	o->val = CLCM_SET_QOS_ALLOCATED;

	switch (key)
	{
		// QoS BW Allocation Fraction list. Default: 0 [0-255] e.g. 1,2,3,.,n
		case 'f': 
		{
			o = &opts[CLOP_QOS_ALLOCATED];
			o->set = 1;

			// Allocate memory for the array of __u8 values 
			o->buf = calloc(sizeof(__u8), CLMR_MAX_LD);

			// Parse the commad separated string of hex values
			o->num = parse_u8_csv(o->buf, arg);
			o->len = o->num * sizeof(__u8);
		}
			break;

		// Called for non option parameter
		case ARGP_KEY_ARG: 				
			argp_error (state, "Invalid subcommand"); 

			// Stop current parser
			state->next = state->argc; 	
			break;

		// Last call. Verify parameters. Fill in missing values
		case ARGP_KEY_END:				
			// Fail if one of the required options isn't set
			if ( !opts[CLOP_QOS_ALLOCATED].set) {
				if (opts[CLOP_PRNT_OPTS].set)
					print_options_array(opts);
				print_help(CLAP_SET_QOS_ALLOCATED);
				exit(0);
			}

			// Fail if no port id was set
			if (!opts[CLOP_PPID].set) {
				if (opts[CLOP_PRNT_OPTS].set)
					print_options_array(opts);
				print_help(CLAP_SET_QOS_ALLOCATED);
				exit(0);
			}
			break;
	} 
	return rv;	
}

/**
 * Parse function for: set qos control
 *
 * @return 0 success, non-zero to indicate a problem 
 */
static int pr_set_qos_control(int key, char *arg, struct argp_state *state)
{
	struct opt *opts, *o;
	int rv = 0;

	opts = (struct opt*) state->input;

	// Call common flag processing first
	rv = pr_common(key, arg, state, CLAP_SET_QOS_CONTROL, ao_set_qos_control);

	// Set Command 
	o = &opts[CLOP_CMD];
	o->set = 1;
	o->val = CLCM_SET_QOS_CONTROL;

	switch (key)
	{
		// Backpressure Sample Interval x 100 ns. Default: 8 [0-15]
		case 'k': 
			opts[CLOP_BP_SAMPLE_INTVL].set = 1;
			opts[CLOP_BP_SAMPLE_INTVL].u8 = hexordec_to_ul(arg);
			break;

		// Egress Port Congestion Enable
		case 'e': 
			opts[CLOP_CONGEST_ENABLE].set = 1;
			break;

		// Completion Collection Interval. Default: 64 [0-255]
		case 'i': 
			opts[CLOP_CCINTERVAL].set = 1;
			opts[CLOP_CCINTERVAL].u8 = hexordec_to_ul(arg);
			break;

		// Egress Moderate Percentage. Default: 10 [1-100]
		case 'm': 
			opts[CLOP_EGRESS_MOD_PCNT].set = 1;
			opts[CLOP_EGRESS_MOD_PCNT].u8 = hexordec_to_ul(arg);;
			break;

		// ReqCmpBasisB. Default: 0 [0-65,535]
		case 'q': 
			opts[CLOP_REQCMPBASIS].set = 1;
			opts[CLOP_REQCMPBASIS].u16 = hexordec_to_ul(arg);
			break;

		// Egress Severe Percentage. Default: 25 [1-100]
		case 's': 
			opts[CLOP_EGRESS_SEV_PCNT].set = 1;
			opts[CLOP_EGRESS_SEV_PCNT].u8 = hexordec_to_ul(arg);;
			break;

		// Temporary Throughput Reduction Enable
		case 't': 
			opts[CLOP_TEMP_THROTTLE].set = 1;
			break;

		// Called for non option parameter
		case ARGP_KEY_ARG: 				
			argp_error (state, "Invalid subcommand"); 

			// Stop current parser
			state->next = state->argc; 	
			break;

		// Last call. Verify parameters. Fill in missing values
		case ARGP_KEY_END:				
			// Fail if no port id was set
			if (!opts[CLOP_PPID].set) {
				if (opts[CLOP_PRNT_OPTS].set)
					print_options_array(opts);
				print_help(CLAP_SET_QOS_CONTROL);
				exit(0);
			}
			break;
	} 
	return rv;	
}

/**
 * Parse function for: set qos limit
 *
 * @return 0 success, non-zero to indicate a problem 
 */
static int pr_set_qos_limit(int key, char *arg, struct argp_state *state)
{
	struct opt *opts, *o;
	int rv = 0;

	opts = (struct opt*) state->input;

	// Call common flag processing first
	rv = pr_common(key, arg, state, CLAP_SET_QOS_LIMIT, ao_set_qos_limit);

	// Set Command 
	o = &opts[CLOP_CMD];
	o->set = 1;
	o->val = CLCM_SET_QOS_LIMIT;

	switch (key)
	{
		// QoS BW Limit Fraction list. Default: 0 [0-255] e.g. 1,2,3,.,n
		case 'f': 
		{
			o = &opts[CLOP_QOS_LIMIT];
			o->set = 1;

			// Allocate memory for the array of __u8 values 
			o->buf = calloc(sizeof(__u8), CLMR_MAX_LD);

			// Parse the commad separated string of hex values
			o->num = parse_u8_csv(o->buf, arg);
			o->len = o->num * sizeof(__u8);
		}
			break;

		// Called for non option parameter
		case ARGP_KEY_ARG: 				
			argp_error (state, "Invalid subcommand"); 

			// Stop current parser
			state->next = state->argc; 	
			break;

		// Last call. Verify parameters. Fill in missing values
		case ARGP_KEY_END:				
			// Fail if one of the required options isn't set
			if ( !opts[CLOP_QOS_LIMIT].set) {
				if (opts[CLOP_PRNT_OPTS].set)
					print_options_array(opts);
				print_help(CLAP_SET_QOS_LIMIT);
				exit(0);
			}

			// Fail if no port id was set
			if (!opts[CLOP_PPID].set) {
				if (opts[CLOP_PRNT_OPTS].set)
					print_options_array(opts);
				print_help(CLAP_SET_QOS_LIMIT);
				exit(0);
			}
			break;
	} 
	return rv;	
}

/**
 * Obtain option defaults from environment if present 
 *
 * @return 	0 on Success, non zero otherwise
 */
int options_getenv()
{
	struct argp_state state;
	int rv;
	char *o;
	struct envopt *e;

	// Initialize variables
	rv = 1;
	state.input = opts;
	e = envopts;

	if (e == NULL)
		goto end;

	// Loop through global envopts array 
	while (e->key != 0)
	{
		// Get value from shell environment 
		o = getenv(e->name);

		// If the returned value is not NULL parse using pr_common()
		if (o != NULL)
			pr_common(e->key, o, &state, 0, NULL);

		// Advance to the next entry in the global envopts array 
		e++;
	}

	rv = 0;

end:
	
	return rv;
}

/**
 * Parse CLI options 
 *
 * @param argc Number of CLI parameters 
 * @param argv Array of string pointers to CLI parameters 
 * 
 * STEPS
 * 1: Store app name in global variable
 * 2: Allocate and clear memory for options array
 * 3: Obtain Option defaults from shell environment 
 * 4: Parse options 
 */ 
int options_parse(int argc, char *argv[])
{
	int rv, len;
	char default_name[] = "app";

	rv = 1;

	// STEP 1: Store app name in global variable
	app_name = NULL;
	if (argc > 0) 
	{
		len = strnlen(argv[0], CLMR_MAX_NAME_LEN);
		if (len > 0) 
		{
			app_name = malloc(len+1);
			if (app_name == NULL)
				goto end;
			memset(app_name, 0, len+1);

			if (!strncmp("./", argv[0], 2))
				memcpy(app_name, &argv[0][2], len);
			else
				memcpy(app_name, argv[0], len);
		}
	}
	else 
		app_name = default_name;

	// STEP 2: Allocate and clear memory for options array
	opts = calloc(CLOP_MAX, sizeof(struct opt));
	if (opts == NULL) 
		goto end_name;

	// STEP 3: Obtain Option defaults from shell environment 
	options_getenv();

	// STEP 4: Parse options 
  	rv = argp_parse(&ap_main, argc, argv, ARGP_IN_ORDER | ARGP_NO_HELP, 0, opts);
	if (rv != 0) 
		goto end_opt;

	return rv;

end_opt:

	free(opts);

end_name:

	if (app_name && app_name != default_name)
		free(app_name);

end:

	return rv;
}

