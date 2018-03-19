
#ifndef __BPMCTL_COMMON_H_INCLUDED__
#define __BPMCTL_COMMON_H_INCLUDED__
/*
<:copyright-BRCM:2009:proprietary:standard

   Copyright (c) 2009 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
*/

/*
 *******************************************************************************
 * File Name  : bpmctl_common.h
 * Description: This file is common IOCTL interface between kernel and user
 *              space for BPM.
 *******************************************************************************
 */

#define BPM_DEVNAME             "bpm"

/* BPM Character Device */
#define BPM_DRV_NAME            BPM_DEVNAME
#define BPM_DRV_DEVICE_NAME     "/dev/" BPM_DRV_NAME

#define BPMCTL_ERROR               (-1)
#define BPMCTL_SUCCESS             0

/*
 * Ioctl definitions.
 */
typedef enum {
/* IOCTL cmd values 1 and 2 are mapped to FIBMAP and FIGETBSZ on ARM
   processor. Hence start all IOCTL values from 100 to prevent conflicts */
    BPMCTL_IOCTL_SYS = 100,
    BPMCTL_IOCTL_MAX
} bpmctl_ioctl_t;

typedef enum {
    BPMCTL_SUBSYS_STATUS,
    BPMCTL_SUBSYS_THRESH,
    BPMCTL_SUBSYS_BUFFERS,
    BPMCTL_SUBSYS_MAX
} bpmctl_subsys_t;

typedef enum {
    BPMCTL_OP_GET,
    BPMCTL_OP_SET,
    BPMCTL_OP_ADD,
    BPMCTL_OP_REM,
    BPMCTL_OP_DUMP,
    BPMCTL_OP_MAX
} bpmctl_op_t;

typedef struct {
    bpmctl_subsys_t subsys;
    bpmctl_op_t     op;
} bpmctl_data_t;


#endif /*  __BPMCTL_COMMON_H_INCLUDED__ */
