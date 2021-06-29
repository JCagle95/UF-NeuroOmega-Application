#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Python Decoder for Alpha Omega MPX File Format (v4)
https://github.com/JCagle95/UF-NeuroOmega-Application/wiki/MPX-File-Format-Overview

Usage 1 - Only Analog/Digital Channel Decoding: 
    from decodeMPX import *
    Content = decodeMPX("Test.mpx")
    
Usage 2 - Analog/Digital Channel Decoding with Custom Stream Format Decoding:
    
    from decodeMPX import *
    Parsers = [parseCommandMotorSetPos, parseCommandModuleStimulus, parseCommandMessageTextMessage]
    Content = decodeMPX("Test.mpx", Parsers)

Look for "Defined Parsers" header for available parsers
    
@author: Jackson Cagle, University of Florida, ©2021
@email: jackson.cagle@neurology.ufl.edu
"""

import sys, os
import numpy as np
from datetime import datetime

######################################
######### Defined Parsers ############
######################################
"""
def parseCommandMessageChannelDownSample
def parseCommandMessagePortAsStrobe
def parseCommandMessageChannelState
def parseCommandMessageTextMessage
def parseCommandTemplMatchSpikesSelector
def parseCommandTemplMatchThreshold
def parseCommandTemplMatchTemplChange
def parseCommandImpedanceValues
def parseCommandFilterParams
def parseCommandChannelChange
def parseCommandMotorConfig
def parseCommandMotorSetSpeed
def parseCommandMotorSetPos
def parseCommandModuleStimulus
def parseCommandModuleElectrodeParam
def parseCommandStimStop
def parseCommandStimStart
def parseCommandTrajSettings
def parseStatusStimStatus
"""

"""""""""""""""""""""""""""
# Command Bytes Definitions
"""""""""""""""""""""""""""

C_Command_Type                                  = 77
E_Command_Generic_Message                       = 7 
E_Command_Module_Params                         = 8 
E_Command_Stim_Start                            = 10 
E_Command_Stim_Stop                             = 11 
E_Command_Motor_SetPos                          = 106  
E_Command_Motor_SetSpeed                        = 110 
E_Command_Motor_Config                          = 115 
E_Command_WirelessMap_ChannelChange             = 200 
E_Command_WirelessMap_TemplMatchTemplChange     = 230 
E_Command_WirelessMap_TemplMatchThreshold       = 231 
E_Command_WirelessMap_TemplMatchSpikesSelector  = 232 
E_Command_MGPlus_Imp_Values                     = 411 
E_Command_Traj_Settings                         = 522
E_Command_Filter_Params                         = 867

"""""""""""""""""""""""""""
# Status Bytes Definitions
"""""""""""""""""""""""""""
C_Status_Type                  = 83  
E_Status_Stim_Status           = 4 

"""""""""""""""""""""""""""
# Generic Message Definitions
"""""""""""""""""""""""""""

E_GenMes_HeadStageStatuses        = 6 
E_GenMes_TextMessage              = 20
E_GenMes_PortAsStrobe             = 21
E_GenMes_ChannelState             = 25
E_GenMes_ChannelDownSample        = 26
E_GenMes_ReferenceChanged         = 34
"""""""""""""""""""""""""""
# Module Parameters
"""""""""""""""""""""""""""

E_Module_ModuleStimulus          = 1 
E_Module_AnalogOutputParam       = 3 
E_Module_ElectrodeParam          = 5 

"""""""""""""""""""""""""""
# Main Function Parameters
"""""""""""""""""""""""""""
def decodeMPX(Filename, StreamToParse=list()):
    with open(Filename, "rb") as file:
        rawBytes = file.read()
    
    Content = {"Header": dict(), "Data": dict(), "Stream": list()}
    
    ChannelDefinitionPackage = list()
    ChannelDataPackage = list()
    ChannelDataLength = dict()
    ChannelNameMap = dict()
    
    Stream = dict()
    
    blockOffset = 0
    while blockOffset < len(rawBytes) - 5:
        blockLength = int.from_bytes(rawBytes[slice(blockOffset + 0, blockOffset + 2)], byteorder="little")
        blockType = str(bytes([rawBytes[blockOffset + 2]]), "utf-8")
        
        # ASCII 'h' = 104
        if rawBytes[blockOffset + 2] == 104:
            Content["Header"]["ProgramVersion"] = np.frombuffer(rawBytes[blockOffset+8:blockOffset+10], dtype=np.uint16)[0]
            Content["Header"]["SessionDateTime"] = datetime(int.from_bytes(rawBytes[slice(blockOffset+16, blockOffset+18)], byteorder="little"), rawBytes[blockOffset+15], rawBytes[blockOffset+14],
                                                            rawBytes[blockOffset+10], rawBytes[blockOffset+11], rawBytes[blockOffset+12])
            Content["Header"]["MinimumAcquisitionTime"] = np.frombuffer(rawBytes[blockOffset+20:blockOffset+28], dtype=float)[0]
            Content["Header"]["MaximumAcquisitionTime"] = np.frombuffer(rawBytes[blockOffset+28:blockOffset+36], dtype=float)[0]
            Content["Header"]["EraseCount"] = np.frombuffer(rawBytes[blockOffset+36:blockOffset+40], dtype=np.int32)[0]
            Content["Header"]["DataFormatVersion"] = rawBytes[40]
            Content["Header"]["ApplicationName"] = rawBytes[blockOffset+41:blockOffset+51].rstrip(b'\x00').decode("utf-8")
            Content["Header"]["ResourceVersion"] = rawBytes[blockOffset+51:blockOffset+55].rstrip(b'\x00').decode("utf-8")
        
        # ASCII '2' = 50
        elif rawBytes[blockOffset + 2] == 50:
            ChannelDefinitionPackage.append((blockOffset, blockLength))
        
        # ASCII '5' = 53
        elif rawBytes[blockOffset + 2] == 53:
            ChannelDataPackage.append((blockOffset, blockLength))
            ChannelID = np.frombuffer(rawBytes[blockOffset+4:blockOffset+6], dtype=np.int16)[0]
            if ChannelID in ChannelDataLength.keys():
                ChannelDataLength[ChannelID] += (blockLength - 10) / 2
            else:
                ChannelDataLength[ChannelID] = (blockLength - 10) / 2
        
        # ASCII 'S' = 83
        elif rawBytes[blockOffset + 2] == 83:
            Stream = {"ChannelName": "", "Channel": 0, "Data": list()}
            Stream["ChannelName"] = rawBytes[blockOffset+14:blockOffset+blockLength-4].rstrip(b'\x00').decode("utf-8")
            Stream["Channel"] = np.frombuffer(rawBytes[blockOffset+8:blockOffset+10], dtype=np.int16)[0]
            
        # ASCII 'E' = 69
        elif rawBytes[blockOffset + 2] == 69:
            for func in StreamToParse:
                StreamStruct = func(rawBytes[blockOffset:blockOffset+blockLength])
                if len(StreamStruct) > 0:
                    Stream["Data"].append(StreamStruct)
            
        blockOffset += blockLength
    
    Content["Stream"] = Stream
    
    for blockOffset, blockLength in ChannelDefinitionPackage:
        if np.frombuffer(rawBytes[blockOffset+12:blockOffset+14], dtype=np.int16)[0] in ChannelDataLength.keys():
            ChannelDefinition = dict()
            ChannelDefinition["isAnalog"] = np.frombuffer(rawBytes[blockOffset+8:blockOffset+10], dtype=np.int16)[0] == 1
            ChannelDefinition["isInput"] = np.frombuffer(rawBytes[blockOffset+10:blockOffset+12], dtype=np.int16)[0] == 1
            ChannelDefinition["ChannelID"] = np.frombuffer(rawBytes[blockOffset+12:blockOffset+14], dtype=np.int16)[0]
            
            # There are different headers for Analog vs Digital
            # Analog Channel
            if ChannelDefinition["isAnalog"]:
                ChannelDefinition["Mode"] = np.frombuffer(rawBytes[blockOffset+18:blockOffset+20], dtype=np.int16)[0]
                ChannelDefinition["BitResolution"] = np.frombuffer(rawBytes[blockOffset+20:blockOffset+24], dtype=np.float32)[0]
                ChannelDefinition["SamplingRate"] = np.frombuffer(rawBytes[blockOffset+24:blockOffset+28], dtype=np.float32)[0] * 1000
                ChannelDefinition["BlockSize"] = np.frombuffer(rawBytes[blockOffset+28:blockOffset+30], dtype=np.int16)[0]
                ChannelDefinition["Shape"] = np.frombuffer(rawBytes[blockOffset+30:blockOffset+32], dtype=np.int16)[0]
                
                # Continuous Analog Channel
                if ChannelDefinition["Mode"] == 0:
                    ChannelDefinition["SampleValues"] = np.zeros((int(ChannelDataLength[ChannelDefinition["ChannelID"]])),dtype=np.int16)
                    ChannelDefinition["Duration"] = np.frombuffer(rawBytes[blockOffset+32:blockOffset+36], dtype=np.float32)[0]
                    ChannelDefinition["TotalGain"] = np.frombuffer(rawBytes[blockOffset+36:blockOffset+38], dtype=np.int16)[0]
                    ChannelDefinition["ChannelName"] = rawBytes[blockOffset+38:blockOffset+blockLength].rsplit(b'\x00')[0].decode("utf-8")
                
                # Segmented Analog Channel
                elif ChannelDefinition["Mode"] == 1:
                    ChannelDefinition["Trigger"] = dict()
                    ChannelDefinition["Trigger"]["TimeRange"] = np.frombuffer(rawBytes[blockOffset+32:blockOffset+40], dtype=np.float32)
                    ChannelDefinition["Trigger"]["Level"] = np.frombuffer(rawBytes[blockOffset+40:blockOffset+42], dtype=np.int16)[0]
                    ChannelDefinition["Trigger"]["Mode"] = np.frombuffer(rawBytes[blockOffset+42:blockOffset+44], dtype=np.int16)[0]
                    ChannelDefinition["Trigger"]["isRMS"] = np.frombuffer(rawBytes[blockOffset+44:blockOffset+46], dtype=np.int16)[0] == 1
                    ChannelDefinition["TotalGain"] = np.frombuffer(rawBytes[blockOffset+46:blockOffset+48], dtype=np.int16)[0]
                    ChannelDefinition["ChannelName"] = rawBytes[blockOffset+48:blockOffset+blockLength].rsplit(b'\x00')[0].decode("utf-8")
                
                # Older MPX version contain other modes
                else:
                    ChannelDefinition["ChannelName"] = f"Unknown_{ChannelDefinition['ChannelID']}"
                    
            # Digital Channel
            else:
                ChannelDefinition["SampleValues"] = np.zeros((int(ChannelDataLength[ChannelDefinition["ChannelID"]]),2),dtype=np.uint32)
                ChannelDefinition["SamplingRate"] = np.frombuffer(rawBytes[blockOffset+18:blockOffset+22], dtype=np.float32)[0] * 1000
                ChannelDefinition["SaveTrigger"] = np.frombuffer(rawBytes[blockOffset+22:blockOffset+24], dtype=np.int16)[0]
                ChannelDefinition["Duration"] = np.frombuffer(rawBytes[blockOffset+24:blockOffset+28], dtype=np.float32)[0]
                ChannelDefinition["PreviousState"] = np.frombuffer(rawBytes[blockOffset+28:blockOffset+30], dtype=np.int16)[0]
                ChannelDefinition["ChannelName"] = rawBytes[blockOffset+30:blockOffset+blockLength].rsplit(b'\x00')[0].decode("utf-8")
            
            ChannelNameMap[ChannelDefinition["ChannelID"]] = ChannelDefinition["ChannelName"]
            Content["Data"][ChannelDefinition["ChannelID"]] = ChannelDefinition 
    
    for item in ChannelDataLength.keys():
        if not item in Content["Data"].keys():
            ChannelDefinition = dict()
            ChannelDefinition["ChannelID"] = item
            ChannelDefinition["isAnalog"] = False
            ChannelDefinition["Mode"] = -1
            ChannelDefinition["SampleValues"] = np.zeros((int(ChannelDataLength[ChannelDefinition["ChannelID"]]),2),dtype=np.uint32)
            Content["Data"][item] = ChannelDefinition 
        ChannelDataLength[item] = 0
        
    for blockOffset, blockLength in ChannelDataPackage:
        ChannelID = np.frombuffer(rawBytes[blockOffset+4:blockOffset+6], dtype=np.int16)[0]
    
        # Analog Data Structure
        if Content["Data"][ChannelID]["isAnalog"] and Content["Data"][ChannelID]["Mode"] == 0:
            chuck = slice(int(ChannelDataLength[ChannelID]), ChannelDataLength[ChannelID] + int((blockLength-10)/2))
            Content["Data"][ChannelID]["SampleValues"][chuck] = np.frombuffer(rawBytes[blockOffset+6:blockOffset+blockLength-4], dtype=np.int16)
            ChannelDataLength[ChannelID] += int((blockLength-10)/2)
            
        # Digital
        if not Content["Data"][ChannelID]["isAnalog"]:
            Content["Data"][ChannelID]["SampleValues"][int(ChannelDataLength[ChannelID]),0] = np.frombuffer(rawBytes[blockOffset+8:blockOffset+12], dtype=np.uint32)
            Content["Data"][ChannelID]["SampleValues"][int(ChannelDataLength[ChannelID]),1] = np.frombuffer(rawBytes[blockOffset+6:blockOffset+8], dtype=np.uint16)
            ChannelDataLength[ChannelID] += 1

    return Content

def parseCommandMessageChannelDownSample(rawBytes):
    StreamStruct = dict()
    
    if rawBytes[10] == C_Command_Type:
        PackageType = np.frombuffer(rawBytes[12:14], dtype=np.int16)[0]
        if PackageType == E_Command_Generic_Message:
            MessageType = np.frombuffer(rawBytes[14:16], dtype=np.int16)[0]
            if MessageType == E_GenMes_ChannelDownSample:
                StreamStruct["PackageType"] = PackageType
                StreamStruct["MessageType"] = MessageType
                StreamStruct["isStatus"] = False
                StreamStruct["StatusByte"] = rawBytes[11]
                StreamStruct["Timestamp"] = np.frombuffer(rawBytes[4:8], dtype=np.uint32)[0]
                
                StreamStruct["ChannelID"], _, StreamStruct["DownSampleFactor"] = np.frombuffer(rawBytes[16:22], dtype=np.int16)

    return StreamStruct

def parseCommandMessagePortAsStrobe(rawBytes):
    StreamStruct = dict()
    
    if rawBytes[10] == C_Command_Type:
        PackageType = np.frombuffer(rawBytes[12:14], dtype=np.int16)[0]
        if PackageType == E_Command_Generic_Message:
            MessageType = np.frombuffer(rawBytes[14:16], dtype=np.int16)[0]
            if MessageType == E_GenMes_PortAsStrobe:
                StreamStruct["PackageType"] = PackageType
                StreamStruct["MessageType"] = MessageType
                StreamStruct["isStatus"] = False
                StreamStruct["StatusByte"] = rawBytes[11]
                StreamStruct["Timestamp"] = np.frombuffer(rawBytes[4:8], dtype=np.uint32)[0]
                
                StreamStruct["ChannelID"] = np.frombuffer(rawBytes[20:22], dtype=np.int16)[0]
                StreamStruct["Strobe"] = rawBytes[16] == 1

    return StreamStruct

def parseCommandMessageChannelState(rawBytes):
    StreamStruct = dict()
    
    if rawBytes[10] == C_Command_Type:
        PackageType = np.frombuffer(rawBytes[12:14], dtype=np.int16)[0]
        if PackageType == E_Command_Generic_Message:
            MessageType = np.frombuffer(rawBytes[14:16], dtype=np.int16)[0]
            if MessageType == E_GenMes_ChannelState:
                StreamStruct["PackageType"] = PackageType
                StreamStruct["MessageType"] = MessageType
                StreamStruct["isStatus"] = False
                StreamStruct["StatusByte"] = rawBytes[11]
                StreamStruct["Timestamp"] = np.frombuffer(rawBytes[4:8], dtype=np.uint32)[0]
                
                StreamStruct["ChannelID"] = np.frombuffer(rawBytes[16:18], dtype=np.int16)[0]
                StreamStruct["AcquisitionOn"] = rawBytes[20] == 1

    return StreamStruct

def parseCommandMessageTextMessage(rawBytes):
    StreamStruct = dict()
    
    if rawBytes[10] == C_Command_Type:
        PackageType = np.frombuffer(rawBytes[12:14], dtype=np.int16)[0]
        if PackageType == E_Command_Generic_Message:
            MessageType = np.frombuffer(rawBytes[14:16], dtype=np.int16)[0]
            if MessageType == E_GenMes_TextMessage:
                StreamStruct["PackageType"] = PackageType
                StreamStruct["MessageType"] = MessageType
                StreamStruct["isStatus"] = False
                StreamStruct["StatusByte"] = rawBytes[11]
                StreamStruct["Timestamp"] = np.frombuffer(rawBytes[4:8], dtype=np.uint32)[0]
                
                StreamStruct["RealTimestamp"] = np.frombuffer(rawBytes[16:20], dtype=np.uint32)[0]
                StreamStruct["Message"] = rawBytes[22:].rstrip(b'\x00').decode("utf-8")

    return StreamStruct

def parseCommandTemplMatchTemplChange(rawBytes):
    StreamStruct = dict()
    
    if rawBytes[10] == C_Command_Type:
        PackageType = np.frombuffer(rawBytes[12:14], dtype=np.int16)[0]
        if PackageType == E_Command_WirelessMap_TemplMatchTemplChange:
            StreamStruct["PackageType"] = PackageType
            StreamStruct["isStatus"] = False
            StreamStruct["StatusByte"] = rawBytes[11]
            StreamStruct["Timestamp"] = np.frombuffer(rawBytes[4:8], dtype=np.uint32)[0]
            
            StreamStruct["ChannelID"], StreamStruct["TemplateID"], StreamStruct["nPoints"] = np.frombuffer(rawBytes[14:20], dtype=np.int16)
            StreamStruct["TemplatePoints"] = np.frombuffer(rawBytes[20:52], dtype=np.int16)
            StreamStruct["TemplateMode"] = np.frombuffer(rawBytes[52:54], dtype=np.int16)[0]

    return StreamStruct

def parseCommandTemplMatchThreshold(rawBytes):
    StreamStruct = dict()
    
    if rawBytes[10] == C_Command_Type:
        PackageType = np.frombuffer(rawBytes[12:14], dtype=np.int16)[0]
        if PackageType == E_Command_WirelessMap_TemplMatchThreshold:
            StreamStruct["PackageType"] = PackageType
            StreamStruct["isStatus"] = False
            StreamStruct["StatusByte"] = rawBytes[11]
            StreamStruct["Timestamp"] = np.frombuffer(rawBytes[4:8], dtype=np.uint32)[0]
            
            StreamStruct["ChannelID"], StreamStruct["TemplateID"] = np.frombuffer(rawBytes[14:18], dtype=np.int16)
            StreamStruct["Threshold"], StreamStruct["NoiseLevel"] = np.frombuffer(rawBytes[18:22], dtype=np.uint16)

    return StreamStruct

def parseCommandTemplMatchSpikesSelector(rawBytes):
    StreamStruct = dict()
    
    if rawBytes[10] == C_Command_Type:
        PackageType = np.frombuffer(rawBytes[12:14], dtype=np.int16)[0]
        if PackageType == E_Command_WirelessMap_TemplMatchSpikesSelector:
            StreamStruct["PackageType"] = PackageType
            StreamStruct["isStatus"] = False
            StreamStruct["StatusByte"] = rawBytes[11]
            StreamStruct["Timestamp"] = np.frombuffer(rawBytes[4:8], dtype=np.uint32)[0]
            
            StreamStruct["YCoord"] = [0,0]
            StreamStruct["ChannelID"], StreamStruct["Enabled"], StreamStruct["TemplateID"], StreamStruct["XCoord"], StreamStruct["YCoord"][0], StreamStruct["YCoord"][1], StreamStruct["SpikeSelector"] = np.frombuffer(rawBytes[14:28], dtype=np.int16)

    return StreamStruct

def parseCommandTrajSettings(rawBytes):
    StreamStruct = dict()
    
    if rawBytes[10] == C_Command_Type:
        PackageType = np.frombuffer(rawBytes[12:14], dtype=np.int16)[0]
        if PackageType == E_Command_Traj_Settings:
            StreamStruct["PackageType"] = PackageType
            StreamStruct["isStatus"] = False
            StreamStruct["StatusByte"] = rawBytes[11]
            StreamStruct["Timestamp"] = np.frombuffer(rawBytes[4:8], dtype=np.uint32)[0]
            
            StreamStruct["TrajectoryIndex"], StreamStruct["TrajectorySide"], StreamStruct["BenGunType"]  = np.frombuffer(rawBytes[14:20], dtype=np.int16)
            StreamStruct["BenGunElectrodeMap"] = np.frombuffer(rawBytes[20:30], dtype=np.int16)
            StreamStruct["MaxElectrode"] = np.frombuffer(rawBytes[30:32], dtype=np.int16)[0]
            StreamStruct["CenPosX"], StreamStruct["CenPosY"], StreamStruct["StartDepth"], StreamStruct["TargetDepth"], StreamStruct["MacroMicroDistance"] = np.frombuffer(rawBytes[32:52], dtype=np.float32)
            StreamStruct["LeadType"] = np.frombuffer(rawBytes[52:54], dtype=np.int16)[0]

    return StreamStruct

def parseCommandModuleStimulus(rawBytes):
    StreamStruct = dict()
    
    if rawBytes[10] == C_Command_Type:
        PackageType = np.frombuffer(rawBytes[12:14], dtype=np.int16)[0]
        if PackageType == E_Command_Module_Params:
            ModuleType, DestinationID = np.frombuffer(rawBytes[14:18], dtype=np.int16)
            if ModuleType == E_Module_ModuleStimulus:
                StreamStruct["PackageType"] = PackageType
                StreamStruct["ModuleType"] = ModuleType
                StreamStruct["DestinationID"] = DestinationID
                StreamStruct["isStatus"] = False
                StreamStruct["StatusByte"] = rawBytes[11]
                StreamStruct["Timestamp"] = np.frombuffer(rawBytes[4:8], dtype=np.uint32)[0]
                
                StreamStruct["StimulationChannel"], StreamStruct["StimulationReturn"], StreamStruct["StimulationType"] = np.frombuffer(rawBytes[18:24], dtype=np.int16)
                StreamStruct["Amplitudes"] = np.frombuffer(rawBytes[24:28], dtype=np.int16)
                StreamStruct["PulseWidths"] = np.frombuffer(rawBytes[28:32], dtype=np.int16)
                StreamStruct["Duration"] = np.frombuffer(rawBytes[32:36], dtype=np.int32)[0]
                StreamStruct["Frequency"], StreamStruct["StopRecChannelMask"], StreamStruct["StopRecGroupID"], StreamStruct["IncStepSize"] = np.frombuffer(rawBytes[36:44], dtype=np.int16)
                StreamStruct["PulseDelays"] = np.frombuffer(rawBytes[46:50], dtype=np.int16)
                StreamStruct["AnalogStim"], StreamStruct["AnalogWaveID"] = np.frombuffer(rawBytes[50:54], dtype=np.int16)
                    
    return StreamStruct

# TODO: We are still missing a lot of unknowns. See Wiki [77,3,8,0,5,0]
def parseCommandModuleElectrodeParam(rawBytes):
    StreamStruct = dict()
    
    if rawBytes[10] == C_Command_Type:
        PackageType = np.frombuffer(rawBytes[12:14], dtype=np.int16)[0]
        if PackageType == E_Command_Module_Params:
            ModuleType, DestinationID = np.frombuffer(rawBytes[14:18], dtype=np.int16)
            if ModuleType == E_Module_ElectrodeParam:
                StreamStruct["PackageType"] = PackageType
                StreamStruct["ModuleType"] = ModuleType
                StreamStruct["DestinationID"] = DestinationID
                StreamStruct["isStatus"] = False
                StreamStruct["StatusByte"] = rawBytes[11]
                StreamStruct["Timestamp"] = np.frombuffer(rawBytes[4:8], dtype=np.uint32)[0]
                
                StreamStruct["ImpedanceWave"] = [0,0]
                StreamStruct["ImpedanceWave"][0], StreamStruct["ChannelID"], StreamStruct["ImpedanceWave"][1] = np.frombuffer(rawBytes[26:32], dtype=np.int16)
                StreamStruct["HSGain"] = np.frombuffer(rawBytes[34:36], dtype=np.int16)[0]
                StreamStruct["ContactType"] = np.frombuffer(rawBytes[38:40], dtype=np.int16)[0]
                StreamStruct["PreGain"] = np.frombuffer(rawBytes[42:44], dtype=np.int16)[0]
                        
    return StreamStruct

def parseCommandStimStart(rawBytes):
    StreamStruct = dict()
    
    if rawBytes[10] == C_Command_Type:
        PackageType = np.frombuffer(rawBytes[12:14], dtype=np.int16)[0]
        if PackageType == E_Command_Stim_Start:
            StreamStruct["PackageType"] = PackageType
            StreamStruct["isStatus"] = False
            StreamStruct["StatusByte"] = rawBytes[11]
            StreamStruct["Timestamp"] = np.frombuffer(rawBytes[4:8], dtype=np.uint32)[0]
            
            StreamStruct["ChannelID"] = np.frombuffer(rawBytes[14:16], dtype=np.int16)[0]

    return StreamStruct

def parseCommandStimStop(rawBytes):
    StreamStruct = dict()
    
    if rawBytes[10] == C_Command_Type:
        PackageType = np.frombuffer(rawBytes[12:14], dtype=np.int16)[0]
        if PackageType == E_Command_Stim_Stop:
            StreamStruct["PackageType"] = PackageType
            StreamStruct["isStatus"] = False
            StreamStruct["StatusByte"] = rawBytes[11]
            StreamStruct["Timestamp"] = np.frombuffer(rawBytes[4:8], dtype=np.uint32)[0]
            
            StreamStruct["ChannelID"] = np.frombuffer(rawBytes[14:16], dtype=np.int16)[0]

    return StreamStruct

def parseCommandMotorSetPos(rawBytes):
    StreamStruct = dict()
    
    if rawBytes[10] == C_Command_Type:
        PackageType = np.frombuffer(rawBytes[12:14], dtype=np.int16)[0]
        if PackageType == E_Command_Motor_SetPos:
            StreamStruct["PackageType"] = PackageType
            StreamStruct["isStatus"] = False
            StreamStruct["StatusByte"] = rawBytes[11]
            StreamStruct["Timestamp"] = np.frombuffer(rawBytes[4:8], dtype=np.uint32)[0]
            
            StreamStruct["MotorID"] = np.frombuffer(rawBytes[14:16], dtype=np.int16)[0]
            StreamStruct["Position"] = np.frombuffer(rawBytes[16:20], dtype=np.int32)[0]

    return StreamStruct

def parseCommandMotorSetSpeed(rawBytes):
    StreamStruct = dict()
    
    if rawBytes[10] == C_Command_Type:
        PackageType = np.frombuffer(rawBytes[12:14], dtype=np.int16)[0]
        if PackageType == E_Command_Motor_SetSpeed:
            StreamStruct["PackageType"] = PackageType
            StreamStruct["isStatus"] = False
            StreamStruct["StatusByte"] = rawBytes[11]
            StreamStruct["Timestamp"] = np.frombuffer(rawBytes[4:8], dtype=np.uint32)[0]
            
            StreamStruct["MotorID"] = np.frombuffer(rawBytes[14:16], dtype=np.int16)[0]
            StreamStruct["Speed"] = np.frombuffer(rawBytes[16:20], dtype=np.int32)[0]

    return StreamStruct

def parseCommandMotorConfig(rawBytes):
    StreamStruct = dict()
    
    if rawBytes[10] == C_Command_Type:
        PackageType = np.frombuffer(rawBytes[12:14], dtype=np.int16)[0]
        if PackageType == E_Command_Motor_Config:
            StreamStruct["PackageType"] = PackageType
            StreamStruct["isStatus"] = False
            StreamStruct["StatusByte"] = rawBytes[11]
            StreamStruct["Timestamp"] = np.frombuffer(rawBytes[4:8], dtype=np.uint32)[0]
            
            StreamStruct["MotorID"] = np.frombuffer(rawBytes[14:16], dtype=np.int16)[0]
            StreamStruct["Position"], StreamStruct["ZeroPosition"], StreamStruct["TargetPosition"], StreamStruct["StartPosition"], StreamStruct["Speed"], StreamStruct["Range"] = np.frombuffer(rawBytes[16:40], dtype=np.int32)

    return StreamStruct

def parseCommandChannelChange(rawBytes):
    StreamStruct = dict()
    
    if rawBytes[10] == C_Command_Type:
        PackageType = np.frombuffer(rawBytes[12:14], dtype=np.int16)[0]
        if PackageType == E_Command_WirelessMap_ChannelChange:
            StreamStruct["PackageType"] = PackageType
            StreamStruct["isStatus"] = False
            StreamStruct["StatusByte"] = rawBytes[11]
            StreamStruct["Timestamp"] = np.frombuffer(rawBytes[4:8], dtype=np.uint32)[0]
            
            StreamStruct["ChannelID"], StreamStruct["Level"], StreamStruct["Direction"], StreamStruct["Gain"] = np.frombuffer(rawBytes[14:22], dtype=np.int16)
            StreamStruct["Enabled"] = rawBytes[22] == 1

    return StreamStruct

def parseCommandFilterParams(rawBytes):
    StreamStruct = dict()
    
    if rawBytes[10] == C_Command_Type:
        PackageType = np.frombuffer(rawBytes[12:14], dtype=np.int16)[0]
        if PackageType == E_Command_Filter_Params:
            StreamStruct["PackageType"] = PackageType
            StreamStruct["isStatus"] = False
            StreamStruct["StatusByte"] = rawBytes[11]
            StreamStruct["Timestamp"] = np.frombuffer(rawBytes[4:8], dtype=np.uint32)[0]
            
            StreamStruct["DownSampleFactor"] = np.frombuffer(rawBytes[14:16], dtype=np.int16)[0]
            StreamStruct["FilterParams"] = np.frombuffer(rawBytes[16:20], dtype=np.int32)[0]
            StreamStruct["ChannelID"], StreamStruct["FilterType"] = np.frombuffer(rawBytes[20:24], dtype=np.int16)
            StreamStruct["Coefficients"] = np.frombuffer(rawBytes[24:64], dtype=np.int16)
            StreamStruct["nCoefficient"] = np.frombuffer(rawBytes[64:66], dtype=np.int16)[0]

    return StreamStruct

def parseCommandImpedanceValues(rawBytes):
    StreamStruct = dict()
    
    if rawBytes[10] == C_Command_Type:
        PackageType = np.frombuffer(rawBytes[12:14], dtype=np.int16)[0]
        if PackageType == E_Command_MGPlus_Imp_Values:
            StreamStruct["PackageType"] = PackageType
            StreamStruct["isStatus"] = False
            StreamStruct["StatusByte"] = rawBytes[11]
            StreamStruct["Timestamp"] = np.frombuffer(rawBytes[4:8], dtype=np.uint32)[0]
            
            StreamStruct["ChannelMask"] = np.frombuffer(rawBytes[14:16], dtype=np.int16)[0]
            StreamStruct["Impedances"] = np.frombuffer(rawBytes[16:80], dtype=np.int32)
            StreamStruct["ChannelGroupID"] = np.frombuffer(rawBytes[80:82], dtype=np.int16)[0]

    return StreamStruct

def parseStatusStimStatus(rawBytes):
    StreamStruct = dict()
    
    if rawBytes[10] == C_Status_Type:
        PackageType = np.frombuffer(rawBytes[12:14], dtype=np.int16)[0]
        if PackageType == E_Status_Stim_Status:
            StreamStruct["PackageType"] = PackageType
            StreamStruct["isStatus"] = False
            StreamStruct["StatusByte"] = rawBytes[11]
            StreamStruct["Timestamp"] = np.frombuffer(rawBytes[4:8], dtype=np.uint32)[0]
            
            StreamStruct["ChannelID"] = np.frombuffer(rawBytes[14:16], dtype=np.int16)[0]
            StreamStruct["FrequencyDeviation"] = np.frombuffer(rawBytes[16:18], dtype=np.int16)[0]
            StreamStruct["StimStatus"] = np.frombuffer(rawBytes[18:20], dtype=np.int16)[0]
            StreamStruct["MeasuredAmplitudes"] = np.frombuffer(rawBytes[20:24], dtype=np.int16)
            
    return StreamStruct
