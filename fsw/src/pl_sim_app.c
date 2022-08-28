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
**    Implement the Payload Simulator application
**
**  Notes:
**    1. See header notes. 
**
**  References:
**    1. OpenSatKit Object-based Application Developer's Guide.
**    2. cFS Application Developer's Guide.
**
*/

/*
** Includes
*/

#include <string.h>
#include "pl_sim_app.h"


/***********************/
/** Macro Definitions **/
/***********************/

/* Convenience macros */
#define  INITBL_OBJ    (&(PlSim.IniTbl))
#define  CMDMGR_OBJ    (&(PlSim.CmdMgr))


/*******************************/
/** Local Function Prototypes **/
/*******************************/

static int32 InitApp(void);
static int32 ProcessCommands(void);
static void SendStatusTlm(void);


/**********************/
/** File Global Data **/
/**********************/

/* 
** Must match DECLARE ENUM() declaration in app_cfg.h
** Defines "static INILIB_CfgEnum IniCfgEnum"
*/
DEFINE_ENUM(Config,APP_CONFIG)  


/*****************/
/** Global Data **/
/*****************/

PL_SIM_Class_t  PlSim;


/******************************************************************************
** Function: PL_SIM_AppMain
**
*/
void PL_SIM_AppMain(void)
{

   uint32 RunStatus = CFE_ES_RunStatus_APP_ERROR;
 
   CFE_EVS_Register(NULL, 0, CFE_EVS_NO_FILTER);

   if (InitApp() == CFE_SUCCESS) /* Performs initial CFE_ES_PerfLogEntry() call */
   {  
   
      RunStatus = CFE_ES_RunStatus_APP_RUN;
      
   }

   /*
   ** Main process loop
   */
   while (CFE_ES_RunLoop(&RunStatus))
   {

      /*
      ** ProcessCommands() pends indefinitely. The scheduler sends a message 
      ** to manage science files.
      */
      RunStatus = ProcessCommands();
      
   } /* End CFE_ES_RunLoop */

   CFE_ES_WriteToSysLog("PL_SIM App terminating, err = 0x%08X\n", RunStatus);   /* Use SysLog, events may not be working */

   CFE_EVS_SendEvent(PL_SIM_EXIT_EID, CFE_EVS_EventType_CRITICAL, "PL_SIM App terminating, err = 0x%08X", RunStatus);

   CFE_ES_ExitApp(RunStatus);  /* Let cFE kill the task (and any child tasks) */

} /* End of PL_SIM_AppMain() */


/******************************************************************************
** Functions: PL_SIM_ClearFaultCmd
**
** Set instrument fault state to FALSE.
**
** Note:
**  1. This function must comply with the CMDMGR_CmdFuncPtr definition
*/
bool PL_SIM_ClearFaultCmd (void* DataObjPtr, const CFE_MSG_Message_t *MsgPtr)
{
   
   PL_SIM_LIB_ClearFault();

   CFE_EVS_SendEvent (PL_SIM_CLEAR_FAULT_CMD_EID, CFE_EVS_EventType_INFORMATION, 
                      "Payload fault set to FALSE.");
               
   return true;

} /* End PL_SIM_ClearFaultCmd() */


/******************************************************************************
** Function: PL_SIM_NoOpCmd
**
*/
bool PL_SIM_NoOpCmd(void* ObjDataPtr, const CFE_MSG_Message_t *MsgPtr)
{

   CFE_EVS_SendEvent (PL_SIM_NOOP_CMD_EID, CFE_EVS_EventType_INFORMATION,
                      "No operation command received for PL_SIM App version %d.%d.%d",
                      PL_SIM_MAJOR_VER, PL_SIM_MINOR_VER, PL_SIM_PLATFORM_REV);

   return true;


} /* End PL_SIM_NoOpCmd() */


/******************************************************************************
** Functions: PL_SIM_PowerOffCmd
**
** Power off science instrument regardless of current state. The science state
** is unmodified and the PL_SIM_Execute() function takes care of any science
** data cleanup activities.
**
** Note:
**  1. This function must comply with the CMDMGR_CmdFuncPtr definition
**  2. The PL_SIM library outputs an event message power state transitions
*/
bool PL_SIM_PowerOffCmd(void* DataObjPtr, const CFE_MSG_Message_t *MsgPtr)
{

   PL_SIM_LIB_PowerOff();
      
   return true;

} /* End PL_SIM_PowerOffCmd() */


/******************************************************************************
** Functions: PL_SIM_PowerOnCmd
**
** Power on the payload
**
** Note:
**  1. This function must comply with the CMDMGR_CmdFuncPtr definition
**  2. The PL_SIM_LIB outputs an event message power state transitions
*/
bool PL_SIM_PowerOnCmd(void* DataObjPtr, const CFE_MSG_Message_t *MsgPtrr)
{

   bool RetStatus = false;

   if (PlSim.Lib.State.Power == PL_SIM_LIB_POWER_OFF)
   {
      PL_SIM_LIB_PowerOn();      
      RetStatus = true;
   
   }  
   else
   { 
      CFE_EVS_SendEvent (PL_SIM_PWR_ON_CMD_ERR_EID, CFE_EVS_EventType_ERROR, 
                         "Power on payload cmd rejected. Payload must be in OFF state and it's in the %s state.",
                         PL_SIM_LIB_GetPowerStateStr(PlSim.Lib.State.Power));
   }
   
   return RetStatus;

} /* End PL_SIM_PowerOnCmd() */


/******************************************************************************
** Functions: PL_SIM_PowerResetCmd
**
** Initiate a power reset. The PL_SIM_ExecuteStep() method defines how the
** simple responds to a reset.
**
** Note:
**  1. This function must comply with the CMDMGR_CmdFuncPtr definition
**  2. The PL_SIM library outputs an event message power state transitions
*/
bool PL_SIM_PowerResetCmd (void* DataObjPtr, const CFE_MSG_Message_t *MsgPtr)
{

   bool RetStatus = false;

   if (PlSim.Lib.State.Power == PL_SIM_LIB_POWER_READY)
   {
      PL_SIM_LIB_PowerReset();
      RetStatus = true;
   
   }  
   else
   { 
      CFE_EVS_SendEvent (PL_SIM_PWR_RESET_CMD_ERR_EID, CFE_EVS_EventType_ERROR, 
                         "Reset payload power cmd rejected. Payload must be in READY state and it's in the %s state.",
                         PL_SIM_LIB_GetPowerStateStr(PlSim.Lib.State.Power));
   }
   
   return RetStatus;

} /* End PL_SIM_PowerResetCmd() */


/******************************************************************************
** Function: PL_SIM_ResetAppCmd
**
** Notes:
**   1. Framework objects require an object reference since they are
**      reentrant. Applications use the singleton pattern and store a
**      reference pointer to the object data during construction.
*/
bool PL_SIM_ResetAppCmd(void* ObjDataPtr, const CFE_MSG_Message_t *MsgPtr)
{

   CMDMGR_ResetStatus(CMDMGR_OBJ);
   
   /* Leave the PL_SIM library state intact */
	  
   return true;

} /* End PL_SIM_ResetAppCmd() */


/******************************************************************************
** Functions: PL_SIM_SetFaultCmd
**
** Set instrument fault state to TRUE.
**
** Note:
**  1. This function must comply with the CMDMGR_CmdFuncPtr definition
*/
bool PL_SIM_SetFaultCmd (void* DataObjPtr, const CFE_MSG_Message_t *MsgPtr)
{
   
   PL_SIM_LIB_SetFault();

   CFE_EVS_SendEvent (PL_SIM_SET_FAULT_CMD_EID, CFE_EVS_EventType_INFORMATION, 
                      "Payload fault set to TRUE.");
               
   return true;

} /* End PL_SIM_SetFaultCmd() */


/******************************************************************************
** Function: InitApp
**
*/
static int32 InitApp(void)
{

   int32 Status = OSK_C_FW_CFS_ERROR;
   
   /*
   ** Initialize objects 
   */

   if (INITBL_Constructor(&PlSim.IniTbl, PL_SIM_INI_FILENAME, &IniCfgEnum))
   {
   
      PlSim.CmdMid      = CFE_SB_ValueToMsgId(INITBL_GetIntConfig(INITBL_OBJ, CFG_PL_SIM_CMD_TOPICID));
      PlSim.ExecuteMid  = CFE_SB_ValueToMsgId(INITBL_GetIntConfig(INITBL_OBJ, CFG_PL_SIM_EXE_TOPICID));
      PlSim.TlmSlowRate = INITBL_GetIntConfig(INITBL_OBJ, CFG_TLM_SLOW_RATE);

      Status = CFE_SUCCESS; 
  
   } /* End if INITBL Constructed */
  
   if (Status == CFE_SUCCESS)
   {

      /*
      ** Constuct app's child objects
      */
            
        
      /*
      ** Initialize app level interfaces
      */
      
      CFE_SB_CreatePipe(&PlSim.CmdPipe, INITBL_GetIntConfig(INITBL_OBJ, CFG_CMD_PIPE_DEPTH), INITBL_GetStrConfig(INITBL_OBJ, CFG_CMD_PIPE_NAME));  
      CFE_SB_Subscribe(PlSim.CmdMid,     PlSim.CmdPipe);
      CFE_SB_Subscribe(PlSim.ExecuteMid, PlSim.CmdPipe);

      CMDMGR_Constructor(CMDMGR_OBJ);
      CMDMGR_RegisterFunc(CMDMGR_OBJ, CMDMGR_NOOP_CMD_FC,  NULL, PL_SIM_NoOpCmd,     0);
      CMDMGR_RegisterFunc(CMDMGR_OBJ, CMDMGR_RESET_CMD_FC, NULL, PL_SIM_ResetAppCmd, 0);

      CMDMGR_RegisterFunc(CMDMGR_OBJ, PL_SIM_POWER_ON_CMD_FC,    &PlSim,  PL_SIM_PowerOnCmd,    PL_SIM_POWER_ON_CMD_DATA_LEN);
      CMDMGR_RegisterFunc(CMDMGR_OBJ, PL_SIM_POWER_OFF_CMD_FC,   &PlSim,  PL_SIM_PowerOffCmd,   PL_SIM_POWER_OFF_CMD_DATA_LEN);
      CMDMGR_RegisterFunc(CMDMGR_OBJ, PL_SIM_POWER_RESET_CMD_FC, &PlSim,  PL_SIM_PowerResetCmd, PL_SIM_POWER_RESET_CMD_DATA_LEN);
      CMDMGR_RegisterFunc(CMDMGR_OBJ, PL_SIM_SET_FAULT_CMD_FC,   &PlSim,  PL_SIM_SetFaultCmd,   PL_SIM_SET_FAULT_CMD_DATA_LEN);
      CMDMGR_RegisterFunc(CMDMGR_OBJ, PL_SIM_CLEAR_FAULT_CMD_FC, &PlSim,  PL_SIM_ClearFaultCmd, PL_SIM_CLEAR_FAULT_CMD_DATA_LEN);

      /*
      ** Initialize app messages 
      */
 
      CFE_MSG_Init(CFE_MSG_PTR(PlSim.StatusTlm.TelemetryHeader), 
                   CFE_SB_ValueToMsgId(INITBL_GetIntConfig(INITBL_OBJ, CFG_PL_SIM_STATUS_TLM_TOPICID)), 
                   sizeof(PL_SIM_StatusTlm_t));

      /*
      ** Application startup event message
      */
      CFE_EVS_SendEvent(PL_SIM_INIT_APP_EID, CFE_EVS_EventType_INFORMATION,
                        "PL_SIM App Initialized. Version %d.%d.%d",
                        PL_SIM_MAJOR_VER, PL_SIM_MINOR_VER, PL_SIM_PLATFORM_REV);
                        
   } /* End if CHILDMGR constructed */
   
   return(Status);

} /* End of InitApp() */


/******************************************************************************
** Function: ProcessCommands
**
*/
static int32 ProcessCommands(void)
{

   int32  RetStatus = CFE_ES_RunStatus_APP_RUN;
   int32  SysStatus;

   CFE_SB_Buffer_t* SbBufPtr;
   CFE_SB_MsgId_t   MsgId = CFE_SB_INVALID_MSG_ID;


   SysStatus = CFE_SB_ReceiveBuffer(&SbBufPtr, PlSim.CmdPipe, CFE_SB_PEND_FOREVER);

   if (SysStatus == CFE_SUCCESS)
   {
      
      SysStatus = CFE_MSG_GetMsgId(&SbBufPtr->Msg, &MsgId);
   
      if (SysStatus == CFE_SUCCESS)
      {
  
         if (CFE_SB_MsgId_Equal(MsgId, PlSim.CmdMid)) 
         {
            
            CMDMGR_DispatchFunc(CMDMGR_OBJ, &SbBufPtr->Msg);
         
         } 
         else if (CFE_SB_MsgId_Equal(MsgId, PlSim.ExecuteMid))
         {


            PL_SIM_LIB_ExecuteStep();
            PL_SIM_LIB_ReadState(&PlSim.Lib);
            if (PlSim.Lib.State.Power != PL_SIM_LIB_POWER_OFF)
            {
               SendStatusTlm();
            }
            else
            {
               if (PlSim.TlmSlowRateCnt >= PlSim.TlmSlowRate)
               {
                  SendStatusTlm();
                  PlSim.TlmSlowRateCnt = 0;
               }
               else
               {
                  PlSim.TlmSlowRateCnt++;
               }
            }
         }
         else
         {
            
            CFE_EVS_SendEvent(PL_SIM_INVALID_CMD_EID, CFE_EVS_EventType_ERROR,
                              "Received invalid command packet, MID = 0x%08X",
                              CFE_SB_MsgIdToValue(MsgId));
         } 

      }
      else
      {
         
         CFE_EVS_SendEvent(PL_SIM_INVALID_CMD_EID, CFE_EVS_EventType_ERROR,
                           "CFE couldn't retrieve message ID from the message, Status = %d", SysStatus);
      }
      
   } /* Valid SB receive */ 
   else 
   {
   
         CFE_ES_WriteToSysLog("PL_SIM software bus error. Status = 0x%08X\n", SysStatus);   /* Use SysLog, events may not be working */
         RetStatus = CFE_ES_RunStatus_APP_ERROR;
   }  
      
   return RetStatus;
   
} /* End ProcessCommands() */


/******************************************************************************
** Function: SendStatusTlm
**
*/
static void SendStatusTlm(void)
{

   /*
   ** Framework Data
   */
   
   PlSim.StatusTlm.ValidCmdCnt   = PlSim.CmdMgr.ValidCmdCnt;
   PlSim.StatusTlm.InvalidCmdCnt = PlSim.CmdMgr.InvalidCmdCnt;
   
   
   /*
   ** PL_SIM Library Data
   */

   PlSim.StatusTlm.LibPwrState           = PlSim.Lib.State.Power;
   PlSim.StatusTlm.LibPwrInitCycleCnt    = PlSim.Lib.State.PowerInitCycleCnt;
   PlSim.StatusTlm.LibPwrResetCycleCnt   = PlSim.Lib.State.PowerResetCycleCnt;
   PlSim.StatusTlm.LibDetectorFault      = PlSim.Lib.State.DetectorFaultPresent;
   PlSim.StatusTlm.LibDetectorReadoutRow = PlSim.Lib.Detector.ReadoutRow;
   PlSim.StatusTlm.LibDetectorImageCnt   = PlSim.Lib.Detector.ImageCnt;


   CFE_SB_TimeStampMsg(CFE_MSG_PTR(PlSim.StatusTlm.TelemetryHeader));
   CFE_SB_TransmitMsg(CFE_MSG_PTR(PlSim.StatusTlm.TelemetryHeader), true);

} /* End SendStatusTlm() */

