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
**    Define the Payload Simulator application
**
**  Notes:
**    1. PL_SIM does not use performance monitor IDs because simulator apps
**       are not part of a flight build.
**
**  References:
**    1. OpenSatKit Object-based Application Developer's Guide.
**    2. cFS Application Developer's Guide.
**
*/
#ifndef _pl_sim_app_
#define _pl_sim_app_

/*
** Includes
*/

#include "app_cfg.h"
#include "pl_sim_lib.h"


/***********************/
/** Macro Definitions **/
/***********************/

/*
** Events
*/

#define PL_SIM_INIT_APP_EID          (PL_SIM_BASE_EID +  0)
#define PL_SIM_NOOP_CMD_EID          (PL_SIM_BASE_EID +  1)
#define PL_SIM_EXIT_EID              (PL_SIM_BASE_EID +  2)
#define PL_SIM_INVALID_CMD_EID       (PL_SIM_BASE_EID +  3)
#define PL_SIM_PWR_ON_CMD_EID        (PL_SIM_BASE_EID +  4)
#define PL_SIM_PWR_ON_CMD_ERR_EID    (PL_SIM_BASE_EID +  5)
#define PL_SIM_PWR_OFF_CMD_EID       (PL_SIM_BASE_EID +  6)
#define PL_SIM_PWR_RESET_CMD_ERR_EID (PL_SIM_BASE_EID +  7)
#define PL_SIM_PWR_RESET_CMD_EID     (PL_SIM_BASE_EID +  8)
#define PL_SIM_SET_FAULT_CMD_EID     (PL_SIM_BASE_EID +  9)
#define PL_SIM_CLEAR_FAULT_CMD_EID   (PL_SIM_BASE_EID + 10)


/**********************/
/** Type Definitions **/
/**********************/


/******************************************************************************
** Command Packets
** - See EDS command definitions in pl_sim.xml
*/


/******************************************************************************
** Telemetry Packets
** - See EDS command definitions in pl_sim.xml
*/


/******************************************************************************
** PL_SIM_Class
*/
typedef struct
{

   /* 
   ** App Framework
   */ 
   
   INITBL_Class_t  IniTbl; 
   CFE_SB_PipeId_t CmdPipe;
   CMDMGR_Class_t  CmdMgr;
   
   /*
   ** Telemetry Packets
   */
   
   PL_SIM_StatusTlm_t  StatusTlm;
   
   /*
   ** PL_SIM State
   */ 
           
   uint32         PerfId;
   CFE_SB_MsgId_t CmdMid;
   CFE_SB_MsgId_t ExecuteMid;
   uint32         TlmSlowRate;
   uint32         TlmSlowRateCnt;
   
   PL_SIM_LIB_Class_t  Lib;
   
} PL_SIM_Class_t;


/*******************/
/** Exported Data **/
/*******************/

extern PL_SIM_Class_t  PlSim;


/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: PL_SIM_AppMain
**
*/
void PL_SIM_AppMain(void);


/******************************************************************************
** Function: PL_SIM_NoOpCmd
**
*/
bool PL_SIM_NoOpCmd(void* ObjDataPtr, const CFE_MSG_Message_t *MsgPtr);


/******************************************************************************
** Functions: PL_SIM_PowerOnCmd, PL_SIM_PowerOffCmd, PL_SIM_PowerResetCmd
**
** Power on/off/reset payload.
**
** Notes:
**  1. This function must comply with the CMDMGR_CmdFuncPtr definition
**  2. Use separate command function codes & functions as opposed to one 
**     command with a parameter that would need validation
**  3. Reset allows an intermediate level of initialization to be simulated
**     that allows some system state to persist across the reset. For
**     science data may be allowed to resume immediately after a reset.
**
*/
bool PL_SIM_PowerOnCmd (void* DataObjPtr,   const CFE_MSG_Message_t *MsgPtr);
bool PL_SIM_PowerOffCmd(void* DataObjPtr,   const CFE_MSG_Message_t *MsgPtr);
bool PL_SIM_PowerResetCmd(void* DataObjPtr, const CFE_MSG_Message_t *MsgPtr);


/******************************************************************************
** Function: PL_SIM_ResetAppCmd
**
*/
bool PL_SIM_ResetAppCmd(void* ObjDataPtr, const CFE_MSG_Message_t *MsgPtr);


/******************************************************************************
** Functions: PL_SIM_SetFaultCmd, PL_SIM_ClearFaultCmd
**
** Set/clear fault state.
**
** Notes:
**  1. This function must comply with the CMDMGR_CmdFuncPtr definition
**  2. Use separate command function codes & functions as opposed to one 
**     command with a parameter that would need validation
**
*/
bool PL_SIM_SetFaultCmd (void* DataObjPtr,   const CFE_MSG_Message_t *MsgPtr);
bool PL_SIM_ClearFaultCmd (void* DataObjPtr, const CFE_MSG_Message_t *MsgPtr);


#endif /* _pl_sim_app_ */
