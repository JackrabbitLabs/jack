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
#include <stdlib.h>

/* printf()
 */
#include <stdio.h>

/* memset()
 */
#include <string.h>

/* sem_t
 * sem_init()
 * sem_timedwait()
 */
#include <semaphore.h>

#include <errno.h>

/* autl_prnt_buf()
 */
#include <arrayutils.h>

#include <ptrqueue.h>
#include <timeutils.h>
#include <fmapi.h>
#include <emapi.h>
#include <cxlstate.h>

/* mctp_init()
 * mctp_set_mh()
 * mctp_run()
 */
#include <mctp.h>

#include "ctrl_handler.h"
#include "emapi_handler.h"
#include "fmapi_handler.h"
#include "cmd_encoder.h"
#include "options.h"

/* MACROS ====================================================================*/

#ifdef JACK_VERBOSE
 #define VERBOSE(v, m, t) 			({ if(opts[CLOP_VERBOSITY].u64 & v) printf("%d:%s %s\n",    t, __FUNCTION__, m   ); })
 #define VERBOSE_INT(v, m, t, i)	({ if(opts[CLOP_VERBOSITY].u64 & v) printf("%d:%s %s %d\n", t, __FUNCTION__, m, i); })
 #define VERBOSE_STR(v, m, t, s)	({ if(opts[CLOP_VERBOSITY].u64 & v) printf("%d:%s %s %s\n", t, __FUNCTION__, m, s); })
#else
 #define VERBOSE(v, m, t)
 #define VERBOSE_INT(v, m, t, i)
 #define VERBOSE_STR(v, m, t, s) 
#endif // JACK_VERBOSE

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
#endif // JACK_VERBOSE

#define JKLN_PORTS 			32
#define JKLN_VCSS 			32
#define JKLN_VPPBS  		256
#define JKLN_RSP_MSG_N 		13

/* ENUMERATIONS ==============================================================*/

/* STRUCTS ===================================================================*/

/* PROTOTYPES ================================================================*/


/* GLOBAL VARIABLES ==========================================================*/

struct cxl_switch *cxls;

/* FUNCTIONS =================================================================*/


int simple_handler(struct mctp *m, struct mctp_action *ma)
{
	m->dummy = 0;
	if (ma->sem != NULL)
		sem_post(ma->sem);
	return 0;
}


int init_switch(struct mctp *m)
{
	INIT
	struct mctp_action *ma;
	struct cxl_port *p;
	struct fmapi_msg msg, sub;

	ENTER 

	int rv;

	rv = 1;
	ma = NULL;

	STEP // 1: ISC - Identity 
	fmapi_fill_isc_id(&msg);
	if ( (ma = submit_fmapi(m, &msg, 0, NULL, NULL, NULL, NULL)) == NULL)
		goto fail;
	fmapi_update(m, ma);
	
	STEP // 2: ISC - Set msg limit 
	fmapi_fill_isc_set_msg_limit(&msg, JKLN_RSP_MSG_N);
	if ( (ma = submit_fmapi(m, &msg, 0, NULL, NULL, NULL, NULL)) == NULL)
		goto fail;

	STEP // 3: ISC - BOS 
	fmapi_fill_isc_bos(&msg);
	if ( (ma = submit_fmapi(m, &msg, 0, NULL, NULL, NULL, NULL)) == NULL)
		goto fail;
	fmapi_update(m, ma);

	STEP // 4: PSC - Identify Switch Device
	fmapi_fill_psc_id(&msg);
	if ( (ma = submit_fmapi(m, &msg, 0, NULL, NULL, NULL, NULL)) == NULL)
		goto fail;
	fmapi_update(m, ma);

	STEP // 5: PSC - Get Port Status 
	for ( int i = 0 ; i < cxls->num_ports ; i++)
	{
		fmapi_fill_psc_get_port(&msg, i);
		if ( (ma = submit_fmapi(m, &msg, 0, NULL, NULL, NULL, NULL)) == NULL)
			goto fail;
		fmapi_update(m, ma);
	}

	STEP // 6: VSC - Get VCS Status 
	for ( int i = 0 ; i < cxls->num_vcss ; i++)
	{
		fmapi_fill_vsc_get_vcs(&msg, i, 0, 255);
		if ( (ma = submit_fmapi(m, &msg, 0, NULL, NULL, NULL, NULL)) == NULL)
			goto fail;
		fmapi_update(m, ma);
	}

	STEP // 7; PCI Config Space - For each port, get first 64 Bytes of config space
	for ( int i = 0 ; i < cxls->num_ports ; i++)
	{
		p = &cxls->ports[i];

		if (!p->prsnt)
			continue;

		for ( int k = 0 ; k < 64 ; k+=4)
		{
			fmapi_fill_psc_cfg(&msg, i, k, 0, 0xF, FMCT_READ, NULL);
			if ( (ma = submit_fmapi(m, &msg, 0, NULL, NULL, NULL, NULL)) == NULL) 
				goto fail;
			fmapi_update(m, ma);
		}
	}

	STEP // 8: MCC - For each port, if an MLD device, fetch MCC data
	for ( int i = 0 ; i < cxls->num_ports ; i++)
	{
		p = &cxls->ports[i];

		if (p->dt != FMDT_CXL_TYPE_3_POOLED)
			continue;

		// MCC - Get Info 
		fmapi_fill_mcc_get_info(&sub);
		fmapi_fill_mpc_tmc(&msg, i, MCMT_CXLCCI, &sub);
		if ( (ma = submit_fmapi(m, &msg, 0, NULL, NULL, NULL, NULL)) == NULL) 
			goto fail;
		fmapi_update(m, ma);

		// MCC - Get LD Alloc 
		fmapi_fill_mcc_get_alloc(&sub, 0, 0);
		fmapi_fill_mpc_tmc(&msg, i, MCMT_CXLCCI, &sub);
		if ( (ma = submit_fmapi(m, &msg, 0, NULL, NULL, NULL, NULL)) == NULL) 
			goto fail;
		fmapi_update(m, ma);

		// MCC - Get QoS Control
		fmapi_fill_mcc_get_qos_ctrl(&sub);
		fmapi_fill_mpc_tmc(&msg, i, MCMT_CXLCCI, &sub);
		if ( (ma = submit_fmapi(m, &msg, 0, NULL, NULL, NULL, NULL)) == NULL) 
			goto fail;
		fmapi_update(m, ma);
		
		// MCC - Get QoS BW Alloc
		fmapi_fill_mcc_get_qos_alloc(&sub, 0, 0);
		fmapi_fill_mpc_tmc(&msg, i, MCMT_CXLCCI, &sub);
		if ( (ma = submit_fmapi(m, &msg, 0, NULL, NULL, NULL, NULL)) == NULL) 
			goto fail;
		fmapi_update(m, ma);

		// MCC - Get QoS BW Limit
		fmapi_fill_mcc_get_qos_limit(&sub, 0, 0);
		fmapi_fill_mpc_tmc(&msg, i, MCMT_CXLCCI, &sub);
		if ( (ma = submit_fmapi(m, &msg, 0, NULL, NULL, NULL, NULL)) == NULL) 
			goto fail;
		fmapi_update(m, ma);

		// MCC- - Get QoS Status 
		fmapi_fill_mcc_get_qos_status(&sub);
		fmapi_fill_mpc_tmc(&msg, i, MCMT_CXLCCI, &sub);
		if ( (ma = submit_fmapi(m, &msg, 0, NULL, NULL, NULL, NULL)) == NULL) 
			goto fail;
		fmapi_update(m, ma);
	}

	rv = 0;

	goto end;

fail:
	
	printf("ERR: submit_fmapi() returned NULL. rv: %d\n", rv);

end:

	EXIT(rv)
	
	return rv;
}

void list(struct mctp *m)
{
	m->dummy = 0;
	printf("list\n");
}

/**
 * The Jack main run function 
 */
void run(struct mctp *m)
{
	struct mctp_action *ma;

	// Initialize variables
	ma = NULL;

	// 1: If no command then exit 
	if ( !opts[CLOP_CMD].set )
		goto end;

	// Initialize cached copy of remote switch state 
	if (opts[CLOP_NO_INIT].set == 0)
		init_switch(m);

	if (opts[CLOP_CMD].val == CLCM_LIST)
		list(m);
	else
	{
		// Submit Request 
		ma = submit_cli_request(m, NULL);
		if (ma == NULL)
		{
			printf("ma Was NULL\n");
			if (errno == ETIMEDOUT) 
				printf("CLI Submit call timed out\n");
			goto end;
		}

		// Print out response 
		switch(ma->rsp->type)
		{
			case MCMT_CXLFMAPI:		fmapi_handler(m, ma->rsp, ma->req); break;
			case MCMT_CSE:			emapi_handler(m, ma->rsp);			break;
			case MCMT_CONTROL: 		ctrl_handler(m, ma->rsp);			break;
			default:													break;
		}
	}

end:

	return;

}

/**
 * jack cli tool main function
 *
 * STEPS 
 * 1: Parse CLI options
 * 2: Verify Command was requested 
 * 3: MCTP Init
 * 4: Configure MCTP 
 * 5: Run MCTP
 * 6: Free memory
 */
int main(int argc, char* argv[]) 
{
	int rv;
	struct mctp *m;

	rv = 1;

	// STEP 1: Parse CLI options
	rv = options_parse(argc,argv);
	if (rv != 0) 
	{
		printf("Error: Parse options failed:\n");
		goto end;
	}

	// Initialize global state 
	cxls = cxls_init(JKLN_PORTS, JKLN_VCSS, JKLN_VPPBS);

	// Verify Command was requested 
	if (!opts[CLOP_CMD].set) 
	{
		printf("Error: No command was selected\n");
		rv = 1;
		goto end;
	}

	// MCTP Init
	m = mctp_init();
	if (m == NULL) 
	{
		printf("Error: mctp_init() failed\n");
		rv = 1;
		goto end;
	}

	// Configure MCTP 

	// Set Message handler functions
	mctp_set_handler(m, MCMT_CXLFMAPI, 	simple_handler);
	mctp_set_handler(m, MCMT_CSE, 		simple_handler);
	mctp_set_handler(m, MCMT_CONTROL, 	simple_handler);

	// Set MCTP verbosity levels
	mctp_set_verbosity(m, opts[CLOP_MCTP_VERBOSITY].u64);

	// Run MCTP
	rv = mctp_run(m, opts[CLOP_TCP_PORT].u16, opts[CLOP_TCP_ADDRESS].u32, MCRM_CLIENT, 1, 1);
	if (rv != 0)
	{
		printf("Error: mctp_run() failed: %d\n", rv);
		goto stop;
	}

	// Run Jack main sequence 
	run(m);

	mctp_stop(m);

	rv = 0;

stop:

	// STEP 6: Free memory
	mctp_free(m);
	cxls_free(cxls);
	options_free(opts);

	return rv;

end:

	return rv;
};

