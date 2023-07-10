/*
**  Copyright 2022 bitValence, Inc.
**  All Rights Reserved.
**
**  This program is free software; you can modify and/or redistribute it
**  under the terms of the GNU Affero General Public License
**  as published by the Free Software Foundation; version 3 with
**  attribution addendums as found in the LICENSE.txt
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU Affero General Public License for more details.
**
**  Purpose:
**    Define application configurations for the Payload Simulator app
**
**  Notes:
**    1. These configurations should have an application scope and define
**       parameters that shouldn't need to change across deployments. If
**       a change is made to this file or any other app source file during
**       a deployment then the definition of the PL_SIM_PLATFORM_REV
**       macro in pl_sim_platform_cfg.h should be updated.
**
**  References:
**    1. OpenSatKit Object-based Application Developer's Guide.
**    2. cFS Application Developer's Guide.
**
*/
#ifndef _app_cfg_
#define _app_cfg_

/*
** Includes
*/

#include "pl_sim_eds_typedefs.h"
#include "app_c_fw.h"
#include "pl_sim_platform_cfg.h"


/******************************************************************************
** Application Macros
*/

/*
** Versions:
**
** 1.0 - Initial version
*/

#define  PL_SIM_MAJOR_VER   1
#define  PL_SIM_MINOR_VER   0


/******************************************************************************
** JSON init file definitions/declarations.
**    
*/

#define CFG_APP_CFE_NAME    APP_CFE_NAME

#define CFG_PL_SIM_CMD_TOPICID        PL_SIM_CMD_TOPICID
#define CFG_BC_SCH_1_HZ_TOPICID       BC_SCH_1_HZ_TOPICID
#define CFG_PL_SIM_STATUS_TLM_TOPICID PL_SIM_STATUS_TLM_TOPICID
#define CFG_TLM_SLOW_RATE             TLM_SLOW_RATE
      
#define CFG_CMD_PIPE_DEPTH  CMD_PIPE_DEPTH
#define CFG_CMD_PIPE_NAME   CMD_PIPE_NAME

#define APP_CONFIG(XX) \
   XX(APP_CFE_NAME,char*) \
   XX(PL_SIM_CMD_TOPICID,uint32) \
   XX(BC_SCH_1_HZ_TOPICID,uint32) \
   XX(PL_SIM_STATUS_TLM_TOPICID,uint32) \
   XX(TLM_SLOW_RATE,uint32) \
   XX(CMD_PIPE_DEPTH,uint32) \
   XX(CMD_PIPE_NAME,char*) \

DECLARE_ENUM(Config,APP_CONFIG)


/******************************************************************************
** Command Macros
**
*/

#define PL_SIM_POWER_ON_CMD_FC    (CMDMGR_APP_START_FC + 0)
#define PL_SIM_POWER_OFF_CMD_FC   (CMDMGR_APP_START_FC + 1)
#define PL_SIM_POWER_RESET_CMD_FC (CMDMGR_APP_START_FC + 2)
#define PL_SIM_SET_FAULT_CMD_FC   (CMDMGR_APP_START_FC + 3)
#define PL_SIM_CLEAR_FAULT_CMD_FC (CMDMGR_APP_START_FC + 4)


/******************************************************************************
** Event Macros
** 
** Define the base event message IDs used by each object/component used by the
** application. There are no automated checks to ensure an ID range is not
** exceeded so it is the developer's responsibility to verify the ranges. 
*/

#define PL_SIM_BASE_EID  (APP_C_FW_APP_BASE_EID + 0)


/*
** One event ID is used for all initialization debug messages. Uncomment one of
** the PL_MGR_INIT_EVS_TYPE definitions. Set it to INFORMATION if you want to
** see the events during initialization. This is opposite to what you'd expect 
** because INFORMATION messages are enabled by default when an app is loaded.
*/

#define PL_SIM_INIT_DEBUG_EID 999
#define PL_SIM_INIT_EVS_TYPE CFE_EVS_DEBUG
//#define PL_SIM_INIT_EVS_TYPE CFE_EVS_INFORMATION


#endif /* _app_cfg_ */
