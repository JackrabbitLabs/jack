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

#include <cxlstate.h>
#include <ptrqueue.h>
#include <pciutils.h>
#include <fmapi.h>

/* mctp_init()
 * mctp_set_mh()
 * mctp_run()
 */
#include <mctp.h>

#include "fmapi_handler.h"
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
#endif // JACK_VERBOSE

/* ENUMERATIONS ==============================================================*/

/* STRUCTS ===================================================================*/

/* PROTOTYPES ================================================================*/

/* GLOBAL VARIABLES ==========================================================*/

/* FUNCTIONS =================================================================*/

//0    1  2           3     4   5    6           7    8    9    10   11          12          13  14 
//#    @  Port State  Type  LD  Ver  CXL Ver     MLW  NLW  MLS  CLS  Dev Speeds  LTSSM       Ln  Flags
//===  =  ==========  ====  ==  ===  ==========  ===  ===  ===  ===  ==========  ==========  ==  ========
//0    +  USP         T1    -   1.1  A---------  16   16   5.0  -    12345-----  L0          0   LRPC----
//1    +  DSP         T3    16  2.0  AB--------  16    8   5.0  -    --345-----  L0          0   --PC----
//2    +  Unbound     T3    -   3.0  ABC-------  16    8   5.0  -    ---456----  Disabled    0   --P-----
//3    -  -           -     -   -    -           16    -   5.0  -    ----------  Detect      0   --------

struct col 
{
	int width;
	char *title;
};

struct col cols[] = 
{
	{3, "#"},
	{1, "@"},
	{10, "Port State"},
	{6, "Type"},
	{2, "LD"},
	{3, "Ver"},
	{8, "CXL Ver"},
	{3, "MLW"},
	{3, "NLW"},
	{3, "MLS"},
	{3, "CLS"},
	{8, "Speeds"},
	{8, "LTSSM"},
	{2, "LN"},
	{16, "Flags"},
	{0,0}
};

#define SHOW_PORT_BUF_LEN 110
void print_ports(struct fmapi_psc_port_rsp *o)
{
	int i, k;
	struct fmapi_psc_port_info *p;
	char buf[SHOW_PORT_BUF_LEN+1];
	struct col *col;

	memset(buf, ' ', SHOW_PORT_BUF_LEN);
	buf[SHOW_PORT_BUF_LEN] = 0;

	// Print header
	{
		col = &cols[0];
	 	i = 0;

		while (col->width != 0)
		{
			memcpy(&buf[i], col->title, strlen(col->title));
			i += (col->width + 2);
			col++;
		}
		buf[i] = 0;
		printf("%s\n", buf);
	}

	// Reset buffer
	memset(buf, ' ', SHOW_PORT_BUF_LEN);

	// Print header line 
	{
		col = &cols[0];
	 	i = 0;

		while (col->width != 0)
		{
			memset(&buf[i], '-', col->width);
			i += (col->width + 2);
			col++;
		}
		buf[i] = 0;
		printf("%s\n", buf);
	}

	// Print port rows 

	for ( int j = 0 ; j < o->num ; j++ ) 
	{
		p = &o->list[j];
		i = 0;
		k = 0;

		// Reset buffer 
		memset(buf, ' ', SHOW_PORT_BUF_LEN);
		buf[SHOW_PORT_BUF_LEN] = 0;

		// COL 0: Port number
		sprintf(&buf[i],   "%d", p->ppid);
		i += (cols[k++].width + 2);

		// COL 1: Present bit 
		if (p->prsnt) 
			buf[i] = '+';
		else 
			buf[i] = '-';
		i += (cols[k++].width + 2);

		// COL 2: Port state
		//if (!p->prsnt)
		//	sprintf(&buf[i], "-");
		//else
			sprintf(&buf[i], "%s", fmps(p->state));
		i += (cols[k++].width + 2);

		// COL 3: Type 
		if (!p->prsnt)
			sprintf(&buf[i], "-");
		else 
			sprintf(&buf[i], "%s", fmdt(p->dt));
		i += (cols[k++].width + 2);

		// COL 4: LD
		if (!p->prsnt || (p->dt != FMDT_CXL_TYPE_3 && p->dt != FMDT_CXL_TYPE_3_POOLED) )
			sprintf(&buf[i], "-");
		else 
			sprintf(&buf[i], "%d", p->num_ld);
		i += (cols[k++].width + 2);

		// COL 5: Ver
		if (!p->prsnt)
			sprintf(&buf[i], "-");
		else 
			sprintf(&buf[i], "%s", fmdv(p->dv));
		i += (cols[k++].width + 2);

		// COL 6: CXL Versions 
		if (!p->prsnt)
			sprintf(&buf[i], "-");
		else 
		{
			char c = 'A';
			for ( int v = 0 ; v < 8 ; v++ )
			{
				if ((p->cv >> v) & 0x01)
					buf[i+v] = c;
				else 
					buf[i+v] = ' ';
				c++;
			}
		}
		i += (cols[k++].width + 2);

		// COL 7: MLW
		sprintf(&buf[i], "%d", p->mlw);
		i += (cols[k++].width + 2);

		// COL 8: NLW
		if (!p->prsnt)
			sprintf(&buf[i], "-");
		else 
		{
			if ((p->nlw >> 0) == 0)
				sprintf(&buf[i], "%d", p->mlw);
			else 
				sprintf(&buf[i], "%d", p->nlw >> 4);
		}
		i += (cols[k++].width + 2);

		// COL 9: MLS 
		sprintf(&buf[i], "%s", fmms(p->mls));
		i += (cols[k++].width + 2);

		// COL 10: CLS 
		if (!p->prsnt)
			sprintf(&buf[i], "-");
		else 
			sprintf(&buf[i], "%s", fmms(p->cls));
		i += (cols[k++].width + 2);

		// COL 11: Dev Speeds 
		if (!p->prsnt)
			sprintf(&buf[i], "-");
		else 
		{
			char c = '0';
			for ( int v = 0 ; v < 8 ; v++ )
			{
				if ((p->speeds >> v) & 0x01)
					buf[i+v] = c;
				else 
					buf[i+v] = ' ';
				c++;
			}
		}
		i += (cols[k++].width + 2);

		// COL 12: LTSSM
		if (!p->prsnt)
			sprintf(&buf[i], "-");
		else 
			sprintf(&buf[i], "%s", fmls(p->ltssm));
		i += (cols[k++].width + 2);

		// COL 13: First Lane 
		if (!p->prsnt)
			sprintf(&buf[i], "-");
		else 
			sprintf(&buf[i], "%d", p->lane);
		i += (cols[k++].width + 2);

		// COL 14: Flags
		if (p->lane_rev)
			buf[i+0] = 'L';
		if (p->perst)
			buf[i+1] = 'R';
		if (p->prsnt)
			buf[i+2] = 'P';
		if (p->pwrctrl)
			buf[i+3] = 'W';
		i += (cols[k++].width + 2);

		// Unset null termination zeros 
		for ( int k = 0 ; k < SHOW_PORT_BUF_LEN ; k++) 
		{
			if (buf[k] == 0)
				buf[k] = ' ';
		}
		buf[SHOW_PORT_BUF_LEN] = 0;
	
		printf("%s\n", buf);
	}
}

/**
 * Handle Responses of Tunneled CXL FM API MLD Component Command Set Messages
 *
 * @return 0 upon success. Non zero otherwise.
 *
 * STEPS
 * 1: Set buffer pointers 
 * 2: Deserialize Header
 * 3: Verify Response 
 * 4: Deserialize Object
 * 5: Handle opcode
 */
int cci_handler(struct mctp *m, __u8 *payload)
{
	INIT
	struct fmapi_msg msg;
	int rv;

	ENTER 

	// Initialize variables 
	rv = 1;

	STEP // 1: Set buffer pointers 
	msg.buf = (struct fmapi_buf*) payload;

	STEP // 2: Deserialize Header
	fmapi_deserialize(&msg.hdr, msg.buf->hdr, FMOB_HDR, NULL);

	STEP // 3: Verify Response 

	// Verify msg category 
	if (msg.hdr.category != FMMT_RESP) 
	{
		printf("Error: Received a tunneled FM API message that was not a response: %s\n", fmmt(msg.hdr.category));
		rv = 1;
		goto end;
	}

	// Verify msg return code
	if (msg.hdr.return_code != FMRC_SUCCESS && msg.hdr.return_code != FMRC_BACKGROUND_OP_STARTED) 
	{
		printf("Error: %s\n", fmrc(msg.hdr.return_code));
		rv = msg.hdr.return_code;
		goto end;
	}

	STEP // 4: Deserialize Object
	fmapi_deserialize(&msg.obj, msg.buf->payload, fmapi_fmob_rsp(msg.hdr.opcode), NULL);

	STEP // 5: Handle opcode
	switch(msg.hdr.opcode)
	{
		case FMOP_MCC_INFO:
		{			
			struct fmapi_mcc_info_rsp *o = &msg.obj.mcc_info_rsp;
			double size;

			size = msg.obj.mcc_info_rsp.size / (double) (1024*1024*1024);	

			printf("Memory Size                 : 0x%llx - %.1f GiB\n", o->size, size);
			printf("LD Count                    : %d\n", o->num);
			printf("QoS: Port Congestion        : %d\n", o->epc);
			printf("QoS: Temporary BW Reduction : %d\n", o->ttr);
		}
			break;

		case FMOP_MCC_ALLOC_GET:
		{			
			struct fmapi_mcc_alloc_get_rsp *o = &msg.obj.mcc_alloc_get_rsp;

			printf("Total LDs on Device: %u\n", 		o->total);
			printf("Memory Granularity : %d - %s\n", 	o->granularity, fmmg(o->granularity));
			printf("Start LD ID of list: %u\n", 		o->start);
			printf("Num LDs in list    : %u\n", 		o->num);
			printf("\n");
			printf("LDID  Range1             Range2\n");
			printf("----  ------------------ ------------------\n");
			for ( int i = 0 ; i < o->num ; i++) {
				printf("%4d: 0x%016llx 0x%016llx\n", i+o->start, o->list[i].rng1, o->list[i].rng2);
			}
		}
			break;

		case FMOP_MCC_ALLOC_SET:
		{			
			struct fmapi_mcc_alloc_set_rsp *o = &msg.obj.mcc_alloc_set_rsp;

			printf("Number of LDs      : %u\n", o->num);
			printf("Starting LD ID     : %u\n", o->start);
			printf("\n");
			printf("LDID  Range1             Range2\n");
			printf("----  ------------------ ------------------\n");
			for ( int i = 0 ; i < o->num ; i++) {
				printf("%4d: 0x%016llx 0x%016llx\n", i+o->start, o->list[i].rng1, o->list[i].rng2);
			}
		}
			break;

		case FMOP_MCC_QOS_CTRL_GET:
		case FMOP_MCC_QOS_CTRL_SET:
		{			
			struct fmapi_mcc_qos_ctrl *o = &msg.obj.mcc_qos_ctrl;

			printf("Port Congestion                : %d\n", o->epc_en);
			printf("Temporary BW Reduction         : %d\n", o->ttr_en);
			printf("Egress Moderage Pcnt           : %d\n", o->egress_mod_pcnt);
			printf("Egress Severe Pcnt             : %d\n", o->egress_sev_pcnt);
			printf("Backpressure Sample Interval   : %d\n", o->sample_interval);
			printf("ReqCmpBasis                    : %d\n", o->rcb);
			printf("Completion Collection Internal : %d\n", o->comp_interval);
		}
			break;

		case FMOP_MCC_QOS_STAT:
		{			
			struct fmapi_mcc_qos_stat_rsp *o = &msg.obj.mcc_qos_stat_rsp;

			printf("Backpressure Avg Pcnt :  %d\n", o->bp_avg_pcnt);
		}
			break;

		case FMOP_MCC_QOS_BW_ALLOC_GET:
		case FMOP_MCC_QOS_BW_ALLOC_SET:
		{			
			struct fmapi_mcc_qos_bw_alloc *o = &msg.obj.mcc_qos_bw_alloc;

			printf("LDID  Val        PCNT\n");
			printf("----  ---------- ------\n");
			for (int i = 0 ; i < o->num ; i++ ){
				printf("%4d: %4d / 256 %5.1f%%\n", i+o->start, o->list[i], 100.0 * ((double)o->list[i])/256.0);
			}
		}
			break;

		case FMOP_MCC_QOS_BW_LIMIT_GET:
		case FMOP_MCC_QOS_BW_LIMIT_SET:
		{			
			struct fmapi_mcc_qos_bw_limit *o = &msg.obj.mcc_qos_bw_limit;

			printf("LDID  Val        PCNT\n");
			printf("----  ---------- ------\n");
			for (int i = 0 ; i < o->num ; i++ ){
				printf("%4d: %4d / 256 %5.1f%%\n", i+o->start, o->list[i], 100.0 * ((double)o->list[i])/256.0);
			}
		}
			break;

		default: rv = 1; break;
	}

	rv = 0;

end:

	EXIT(rv);

	return rv;
}

/**
 * Handle Responses of Tunneled CXL FM API MLD Component Command Set Messages
 *
 * @return 0 upon success. Non zero otherwise.
 *
 * STEPS
 * 1: Set buffer pointers 
 * 2: Deserialize Header
 * 3: Verify Response 
 * 4: Deserialize Object
 * 5: Handle opcode
 */
int cci_update(struct mctp *m, unsigned ppid, __u8 *payload)
{
	INIT
	struct fmapi_msg msg;
	struct cxl_port *p;
	struct cxl_mld *mld;
	int rv;

	ENTER 

	// Initialize variables 
	rv = 1;

	STEP // 1: Set buffer pointers 
	msg.buf = (struct fmapi_buf*) payload;
	p = &cxls->ports[ppid];
	mld = p->mld;

	STEP // 2: Deserialize Header
	fmapi_deserialize(&msg.hdr, msg.buf->hdr, FMOB_HDR, NULL);

	STEP // 3: Verify Response 

	// Verify msg category 
	if (msg.hdr.category != FMMT_RESP) 
	{
		printf("Error: Received a tunneled FM API message that was not a response: %s\n", fmmt(msg.hdr.category));
		rv = 1;
		goto end;
	}

	// Verify msg return code
	if (msg.hdr.return_code != FMRC_SUCCESS && msg.hdr.return_code != FMRC_BACKGROUND_OP_STARTED) 
	{
		printf("Error: %s\n", fmrc(msg.hdr.return_code));
		rv = msg.hdr.return_code;
		goto end;
	}

	STEP // 4: Deserialize Object
	fmapi_deserialize(&msg.obj, msg.buf->payload, fmapi_fmob_rsp(msg.hdr.opcode), NULL);

	STEP // 5: Handle opcode
	switch(msg.hdr.opcode)
	{
		case FMOP_MCC_INFO:
		{			
			struct fmapi_mcc_info_rsp *o = &msg.obj.mcc_info_rsp;

			// Allocate memory for MLD object in the port if needed 
			if (p->mld == NULL)
			{
				p->mld = calloc(1, sizeof(struct cxl_mld));
				mld = p->mld;
			}
			mld->memory_size 	= o->size;
			mld->num 			= o->num;
			p->ld 				= o->num;
			mld->epc 			= o->epc;
			mld->ttr 			= o->ttr;

			// Allocate memory for the PCI Config Space for each LD if needed 
			for (int i = 0 ; i < o->num ; i++)
				if (mld->cfgspace[i] == NULL)
					mld->cfgspace[i] = calloc(1, sizeof(PCLN_CFG));
		}
			break;

		case FMOP_MCC_ALLOC_GET:
		{			
			struct fmapi_mcc_alloc_get_rsp *o = &msg.obj.mcc_alloc_get_rsp;

			mld->granularity 	= o->granularity;
			for ( int i = 0 ; i < o->num ; i++ ) 
			{
				mld->rng1[i+o->start] = o->list[i].rng1;
				mld->rng2[i+o->start] = o->list[i].rng2;
			}
		}
			break;

		case FMOP_MCC_ALLOC_SET:
		{			
			struct fmapi_mcc_alloc_set_rsp *o = &msg.obj.mcc_alloc_set_rsp;

			for ( int i = 0 ; i < o->num ; i++) 
			{
				mld->rng1[i+o->start] = o->list[i].rng1;
				mld->rng2[i+o->start] = o->list[i].rng2;
			}
		}
			break;

		case FMOP_MCC_QOS_CTRL_GET:
		case FMOP_MCC_QOS_CTRL_SET:
		{			
			struct fmapi_mcc_qos_ctrl *o = &msg.obj.mcc_qos_ctrl;
			mld->epc_en				= o->epc_en;
			mld->ttr_en				= o->ttr_en;
			mld->egress_mod_pcnt 	= o->egress_mod_pcnt;
			mld->egress_sev_pcnt 	= o->egress_sev_pcnt;
			mld->sample_interval 	= o->sample_interval;
			mld->rcb 				= o->rcb;
			mld->comp_interval 		= o->comp_interval;
		}
			break;

		case FMOP_MCC_QOS_STAT:
		{			
			struct fmapi_mcc_qos_stat_rsp *o = &msg.obj.mcc_qos_stat_rsp;

			mld->bp_avg_pcnt 	= o->bp_avg_pcnt;
		}
			break;

		case FMOP_MCC_QOS_BW_ALLOC_GET:
		case FMOP_MCC_QOS_BW_ALLOC_SET:
		{			
			struct fmapi_mcc_qos_bw_alloc *o = &msg.obj.mcc_qos_bw_alloc;

			for ( int i = 0 ; i < o->num ; i++ )
				mld->alloc_bw[i+o->start] = o->list[i];
		}
			break;

		case FMOP_MCC_QOS_BW_LIMIT_GET:
		case FMOP_MCC_QOS_BW_LIMIT_SET:
		{			
			struct fmapi_mcc_qos_bw_limit *o = &msg.obj.mcc_qos_bw_limit;

			for ( int i = 0 ; i < o->num ; i++ )
				mld->bw_limit[i+o->start] = o->list[i];
		}
			break;

		default: rv = 1; break;
	}

	rv = 0;

end:

	EXIT(rv);

	return rv;
}

/**
 * Handle Responses to FM API Messages
 *
 * @return 0 upon success. Non zero otherwise.
 *
 * STEPS:
 * 1: Set buffer pointers 
 * 2: Deserialize Request Header
 * 3: Deserialize Request Object 
 * 4: Deserialize Response Header
 * 5: Verify Response 
 * 6: Deserialize Response Payload using object from request
 * 7: Handle opcode 
 */
int fmapi_handler(struct mctp *m, struct mctp_msg *mr, struct mctp_msg *mm)
{
	INIT 
	int rv; 
	struct fmapi_msg req, rsp;

	ENTER 

	// Initialize varialbes
	rv = 1;

	STEP // 1: Set buffer pointers 
	req.buf = (struct fmapi_buf*) mm->payload;
	rsp.buf = (struct fmapi_buf*) mr->payload;
	
	STEP // 2: Deserialize Request Header
	fmapi_deserialize(&req.hdr, req.buf->hdr, FMOB_HDR, NULL);

	STEP // 3: Deserialize Request Object 
	fmapi_deserialize(&req.obj, req.buf->payload, fmapi_fmob_req(req.hdr.opcode), NULL);

	STEP // 4: Deserialize Response Header
	fmapi_deserialize(&rsp.hdr, rsp.buf->hdr, FMOB_HDR, NULL);

	STEP // 5: Verify Response 

	// Verify msg category 
	if (rsp.hdr.category != FMMT_RESP) 
	{
		printf("Error: Received an FM API message that was not a response: %s\n", fmmt(rsp.hdr.category));
		goto end;
	}
	
	// Verify return code
	if (rsp.hdr.return_code != FMRC_SUCCESS && rsp.hdr.return_code != FMRC_BACKGROUND_OP_STARTED) 
	{
		printf("Error: %s\n", fmrc(rsp.hdr.return_code));
		rv = rsp.hdr.return_code;
		goto end;
	}

	STEP // 6: Deserialize Response Payload using object from request
	fmapi_deserialize(&rsp.obj, rsp.buf->payload, fmapi_fmob_rsp(rsp.hdr.opcode), &req.obj);

	STEP // 7: Handle opcode 
	switch(rsp.hdr.opcode)
	{
		case FMOP_ISC_BOS:
	 	{
			struct fmapi_isc_bos *o = &rsp.obj.isc_bos;

			printf("Show Background Operation Status:\n");
			printf("Background Op. Running:   %d\n",          o->running);
			printf("Percent Complete:         %d%%\n",        o->pcnt);
			printf("Command Opcode:           0x%04x - %s\n", o->opcode, fmop(rsp.hdr.opcode));
			printf("Return Code:              0x%04x - %s\n", o->rc, fmrc(o->rc));
			printf("Vendor Specific Status:   0x%04x\n",      o->ext);
		}
			break;

		case FMOP_ISC_ID:
		{
			struct fmapi_isc_id_rsp *o = &rsp.obj.isc_id_rsp;

			printf("Show Identity:\n");
			printf("PCIe Vendor ID:           0x%x\n", 		o->vid);
			printf("PCIe Device ID:           0x%x\n", 		o->did);
			printf("PCIe Subsystem Vendor ID: 0x%x\n", 		o->svid);
			printf("PCIe Subsystem ID:        0x%x\n", 		o->ssid);
			printf("SN:                       0x%llx\n", 	o->sn);
			printf("Max Msg Size n of 2^n:    %d - %d B\n", o->size, 1 << o->size);
		}
			break;

		case FMOP_ISC_MSG_LIMIT_GET:
		case FMOP_ISC_MSG_LIMIT_SET:
		{
			struct fmapi_isc_msg_limit *o = &rsp.obj.isc_msg_limit;

			printf("Response Msg Limit (n of 2^n):  %d - %d B\n", o->limit, 1 << o->limit);
		}
			break;

		case FMOP_PSC_ID:
		{	
			struct fmapi_psc_id_rsp *o = &rsp.obj.psc_id_rsp;
			int active_ports, active_vcss;

			// Count number of active ports
			active_ports = 0;
			for ( int i = 0 ; i < 32 ; i++) 
				for ( int k = 0 ; k < 8 ; k++ )
					if ((o->active_ports[i] >> k) & 0x01)
						active_ports++;

			// Count number of active vcss
			active_vcss = 0;
			for ( int i = 0 ; i < 32 ; i++) 
				for ( int k = 0 ; k < 8 ; k++ )
					if ((o->active_vcss[i] >> k) & 0x01)
						active_vcss++;

			printf("Show Switch:\n");
			printf("Ingress Port ID       : %d\n", o->ingress_port);
			printf("Num Physical Ports    : %u\n", o->num_ports);
			printf("Active Physical Ports : %u\n", active_ports);
			printf("Num VCSs              : %u\n", o->num_vcss);
			printf("Active VCSs           : %u\n", active_vcss);
			printf("Num VPPBs             : %u\n", o->num_vppbs);
			printf("Num Active VPPBs      : %u\n", o->active_vppbs); 
			printf("Num HDM Decoders      : %u\n", o->num_decoders);
		}
			break;

		case FMOP_PSC_PORT:
		{
			struct fmapi_psc_port_rsp *o = &rsp.obj.psc_port_rsp;
			print_ports(o);
		}
			break;

		case FMOP_PSC_PORT_CTRL:
		{
			// Nothing to do upon success
		}
			break;

		case FMOP_PSC_CFG:
		{
			struct fmapi_psc_cfg_rsp *o = &rsp.obj.psc_cfg_rsp;
			printf("Data: 0x%02x%02x%02x%02x\n", o->data[0], o->data[1], o->data[2], o->data[3]);
		}
			break;

		case FMOP_VSC_INFO:
		{
			struct fmapi_vsc_info_rsp *o = &rsp.obj.vsc_info_rsp;
			struct fmapi_vsc_info_blk *v;
			struct fmapi_vsc_ppb_stat_blk *b;
			int i, k;

			printf("Show VCS:\n");
		
			for ( i = 0 ; i < o->num ; i++ ) 
			{
				v = &o->list[i];

				if ( i > 0 )
					printf("\n");

				printf("VCS ID  : %d\n", v->vcsid);
				printf("State   : %s\n", fmvs(v->state));
				printf("USP ID  : %d\n", v->uspid);
				printf("vPPBs   : %d\n", v->num);
				printf("\n");
				printf("vPPB  PPID LDID Status\n");
				printf("----  ---- ---- -----------\n");
				for ( k = 0 ; k < v->num ; k++)
				{
					b = &v->list[k];
					printf("%4d: ", k);
					switch(b->status)
					{
						case FMBS_UNBOUND:
							printf("   - ");
							printf("   - ");
							printf("%s", fmbs(b->status));
							break;

						case FMBS_INPROGRESS:
							printf("   ? ");
							printf("   ? ");
							printf("%s", fmbs(b->status));
							break;

						case FMBS_BOUND_PORT:
							printf("%4d ", b->ppid);
							printf("   - ");
							printf("%s", fmbs(b->status));
							break;

						case FMBS_BOUND_LD:
							printf("%4d ", b->ppid);
							printf("%4d ", b->ldid);
							printf("%s", fmbs(b->status));
							break;

						default: 
							break;
					}
					printf("\n");
				}
			}
		}
			break;

		case FMOP_VSC_BIND:
		{
			if (rsp.hdr.return_code == FMRC_BACKGROUND_OP_STARTED) 
			{
				printf("Bind operation started in the background\n");
			}
		}
			break;

		case FMOP_VSC_UNBIND:
		{
			if (rsp.hdr.return_code == FMRC_BACKGROUND_OP_STARTED) 
			{
				printf("Unbind operation started in the background\n");
			}
		}
			break;

		case FMOP_VSC_AER:
			break;

		case FMOP_MPC_TMC:
		{
			struct fmapi_mpc_tmc_rsp *o = &rsp.obj.mpc_tmc_rsp;

			if (o->type != MCMT_CXLCCI)
			{
				printf("Error: Tunneled command had incorrect MCTP Message Type: 0x%02x\n", o->type);
				goto end;
			}

			rv = cci_handler(m, o->msg);	
		}
			break;

		case FMOP_MPC_CFG:
		{
			struct fmapi_mpc_cfg_rsp *o = &rsp.obj.mpc_cfg_rsp;
			printf("Data: 0x%02x%02x%02x%02x\n", o->data[0], o->data[1], o->data[2], o->data[3]);
		}
			break;

		case FMOP_MPC_MEM:
		{
			struct fmapi_mpc_mem_rsp *o = &rsp.obj.mpc_mem_rsp;
			autl_prnt_buf(o->data, o->len, 4, 0);
		}
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

/**
 * Update cached switch state from Responses to FM API Messages
 *
 * @return 0 upon success. Non zero otherwise.
 *
 * STEPS:
 * 1: Set buffer pointers 
 * 2: Deserialize Request Header
 * 3: Deserialize Request Object 
 * 4: Deserialize Response Header
 * 5: Verify Response 
 * 6: Deserialize Response Payload using object from request
 * 7: Handle opcode 
 */
int fmapi_update(struct mctp *m, struct mctp_action *ma)
{
	INIT 
	int rv; 
	struct fmapi_msg req, rsp;

	ENTER 

	// Initialize varialbes
	rv = 1;

	STEP // 1: Set buffer pointers 
	req.buf = (struct fmapi_buf*) ma->req->payload;
	rsp.buf = (struct fmapi_buf*) ma->rsp->payload;
	
	STEP // 2: Deserialize Request Header
	fmapi_deserialize(&req.hdr, req.buf->hdr, FMOB_HDR, NULL);

	STEP // 3: Deserialize Request Object 
	fmapi_deserialize(&req.obj, req.buf->payload, fmapi_fmob_req(req.hdr.opcode), NULL);

	STEP // 4: Deserialize Response Header
	fmapi_deserialize(&rsp.hdr, rsp.buf->hdr, FMOB_HDR, NULL);

	STEP // 5: Verify Response 

	// Verify msg category 
	if (rsp.hdr.category != FMMT_RESP) 
	{
		printf("Error: Received an FM API message that was not a response: %s\n", fmmt(rsp.hdr.category));
		goto end;
	}
	
	// Verify return code
	if (rsp.hdr.return_code != FMRC_SUCCESS && rsp.hdr.return_code != FMRC_BACKGROUND_OP_STARTED) 
	{
		printf("Error: %s\n", fmrc(rsp.hdr.return_code));
		rv = rsp.hdr.return_code;
		goto end;
	}

	STEP // 6: Deserialize Response Payload using object from request
	fmapi_deserialize(&rsp.obj, rsp.buf->payload, fmapi_fmob_rsp(rsp.hdr.opcode), &req.obj);

	STEP // 8: Obtain lock on switch state 
	pthread_mutex_lock(&cxls->mtx);

	STEP // 7: Handle opcode 
	switch(rsp.hdr.opcode)
	{
		case FMOP_ISC_BOS:
	 	{
			struct fmapi_isc_bos *o = &rsp.obj.isc_bos;
			cxls->bos_opcode 	= o->opcode;
			cxls->bos_rc 		= o->rc;
			cxls->bos_running   = o->running;
			cxls->bos_pcnt 		= o->pcnt;
			cxls->bos_ext 		= o->ext;
		}
			break;

		case FMOP_ISC_ID:
		{
			struct fmapi_isc_id_rsp *o = &rsp.obj.isc_id_rsp;
			cxls->vid 				= o->vid;
			cxls->did 				= o->did;
			cxls->svid 				= o->svid;
			cxls->ssid 				= o->ssid;
			cxls->sn 				= o->sn;
			cxls->max_msg_size_n 	= o->size;
		}
			break;

		case FMOP_ISC_MSG_LIMIT_GET:
		case FMOP_ISC_MSG_LIMIT_SET:
		{
			struct fmapi_isc_msg_limit *o = &rsp.obj.isc_msg_limit;
			cxls->msg_rsp_limit_n = o->limit;
		}
			break;

		case FMOP_PSC_ID:
		{	
			struct fmapi_psc_id_rsp *o = &rsp.obj.psc_id_rsp;
			cxls->ingress_port 	= o->ingress_port;
			cxls->num_ports 	= o->num_ports;
			cxls->num_vcss 		= o->num_vcss;
			cxls->num_vppbs 	= o->num_vppbs;
			cxls->active_vppbs 	= o->active_vppbs;
			cxls->num_decoders 	= o->num_decoders;
		}
			break;

		case FMOP_PSC_PORT:
		{
			struct fmapi_psc_port_rsp *o = &rsp.obj.psc_port_rsp;
			struct fmapi_psc_port_info *x;
			struct cxl_port *p;
			
			for ( int i = 0 ; i < o->num ; i++ )
			{
				x = &o->list[i];
				p = &cxls->ports[x->ppid];
				p->state		= x->state;
    			p->dv			= x->dv;
    			p->dt			= x->dt;	
    			p->cv			= x->cv;	
    			p->mlw			= x->mlw;
    			p->nlw			= x->nlw;
    			p->speeds		= x->speeds;
    			p->mls			= x->mls;
    			p->cls			= x->cls;
    			p->ltssm		= x->ltssm;
				p->lane			= x->lane;
				p->lane_rev 	= x->lane_rev;
				p->perst		= x->perst;
				p->prsnt		= x->prsnt;
				p->pwrctrl		= x->pwrctrl;
    			p->ld			= x->num_ld;
			}
		}
			break;

		case FMOP_PSC_PORT_CTRL:
			break;

		case FMOP_PSC_CFG:
		{
			struct fmapi_psc_cfg_rsp *o = &rsp.obj.psc_cfg_rsp;

			if (req.obj.psc_cfg_req.type == FMCT_READ )
			{
				struct cxl_port *p = &cxls->ports[req.obj.psc_cfg_req.ppid];
				unsigned reg = (req.obj.psc_cfg_req.ext << 8) | req.obj.psc_cfg_req.reg;

				if (req.obj.psc_cfg_req.fdbe & 0x01)
					p->cfgspace[reg] = o->data[0];
				if (req.obj.psc_cfg_req.fdbe & 0x02)
					p->cfgspace[reg] = o->data[1];
				if (req.obj.psc_cfg_req.fdbe & 0x04)
					p->cfgspace[reg] = o->data[2];
				if (req.obj.psc_cfg_req.fdbe & 0x08)
					p->cfgspace[reg] = o->data[3];
			}
		}
			break;

		case FMOP_VSC_INFO:
		{
			struct fmapi_vsc_info_rsp *o = &rsp.obj.vsc_info_rsp;
			struct cxl_vcs *v;
			struct fmapi_vsc_info_blk *x;
			struct fmapi_vsc_ppb_stat_blk *b;
			int i, k;

			for ( i = 0 ; i < o->num ; i++ ) 
			{
				x = &o->list[i];
				v = &cxls->vcss[x->vcsid];

				v->vcsid = x->vcsid;
				v->state = x->state;
				v->uspid = x->uspid;
				v->num   = x->num;

				for ( k = 0 ; k < v->num ; k++)
				{
					b = &x->list[k];
					v->vppbs[k].bind_status = b->status;
					v->vppbs[k].ppid 		= b->ppid;
					v->vppbs[k].ldid 		= b->ldid;
				}
			}
		}
			break;

		case FMOP_VSC_BIND:
			break;

		case FMOP_VSC_UNBIND:
			break;

		case FMOP_VSC_AER:
			break;

		case FMOP_MPC_TMC:
		{
			struct fmapi_mpc_tmc_rsp *o = &rsp.obj.mpc_tmc_rsp;

			if (o->type != MCMT_CXLCCI)
			{
				printf("Error: Tunneled command had incorrect MCTP Message Type: 0x%02x\n", o->type);
				goto end;
			}

			rv = cci_update(m, req.obj.mpc_tmc_req.ppid, o->msg);	
		}
			break;

		case FMOP_MPC_CFG:
		{
			struct fmapi_mpc_cfg_rsp *o = &rsp.obj.mpc_cfg_rsp;

			if (req.obj.mpc_cfg_req.type == FMCT_READ )
			{
				struct cxl_port *p = &cxls->ports[req.obj.mpc_cfg_req.ppid];
				struct cxl_mld *m = p->mld;
				unsigned ldid = req.obj.mpc_cfg_req.ldid;
				unsigned reg = (req.obj.mpc_cfg_req.ext << 8) | req.obj.mpc_cfg_req.reg;

				if (req.obj.mpc_cfg_req.fdbe & 0x01)
					m->cfgspace[ldid][reg] = o->data[0];
				if (req.obj.mpc_cfg_req.fdbe & 0x02)
					m->cfgspace[ldid][reg] = o->data[1];
				if (req.obj.mpc_cfg_req.fdbe & 0x04)
					m->cfgspace[ldid][reg] = o->data[2];
				if (req.obj.mpc_cfg_req.fdbe & 0x08)
					m->cfgspace[ldid][reg] = o->data[3];
			}
		}
			break;

		case FMOP_MPC_MEM:
			break;

		default: 
			goto end;
	}

	rv = 0;

end:

	STEP // Release lock on switch state 
	pthread_mutex_unlock(&cxls->mtx);

	// Return mctp_msg to free pool
	mctp_retire(m, ma);

	EXIT(rv)

	return rv;
}

