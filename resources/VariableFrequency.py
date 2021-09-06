# -*- coding: utf-8 -*-
"""
Created on Mon Mar 29 09:20:42 2021

@author: jcagle
"""

import numpy as np
import json

StimulationJsonDocument = dict()
StimulationJsonDocument["StimulationName"] = "Threshold Protocol"
StimulationJsonDocument["StimulationSequence"] = []
StimulationJsonDocument["AnalogWaveforms"] = []

StimulationJsonDocument["AnalogWaveforms"].append("ProbeStim/Probe60.bin")
StimulationJsonDocument["AnalogWaveforms"].append("ProbeStim/Probe90.bin")
StimulationJsonDocument["AnalogWaveforms"].append("ProbeStim/Probe120.bin")
StimulationJsonDocument["AnalogWaveforms"].append("ProbeStim/Probe150.bin")
StimulationJsonDocument["AnalogWaveforms"].append("ProbeStim/Probe180.bin")

StimulationDesign = dict()
StimulationDesign["Duration"] = 30
StimulationDesign["StimulationType"] = "Baseline"
StimulationDesign["RecordingFilename"] = "Baseline"
StimulationDesign["StimulationLead"] = 0
StimulationDesign["StimulationChannel"] = [0]
StimulationDesign["StimulationReturn"] = -1
StimulationJsonDocument["StimulationSequence"].append(StimulationDesign)

for f in range(len(StimulationJsonDocument["AnalogWaveforms"])):
    StimulationDesign = dict()
    StimulationDesign["Duration"] = 10
    StimulationDesign["StimulationType"] = "Novel"
    StimulationDesign["StimulationIndex"] = f+1
    StimulationDesign["RecordingFilename"] = "ERNA"
    StimulationDesign["StimulationLead"] = 0
    StimulationDesign["StimulationChannel"] = [1]
    StimulationDesign["StimulationReturn"] = -1
    StimulationJsonDocument["StimulationSequence"].append(StimulationDesign)

    StimulationDesign = dict()
    StimulationDesign["Duration"] = 2
    StimulationDesign["StimulationType"] = "Baseline"
    StimulationDesign["RecordingFilename"] = "ERNA"
    StimulationDesign["StimulationLead"] = 0
    StimulationDesign["StimulationChannel"] = [1]
    StimulationDesign["StimulationReturn"] = -1
    StimulationJsonDocument["StimulationSequence"].append(StimulationDesign)

for f in range(len(StimulationJsonDocument["AnalogWaveforms"])):
    StimulationDesign = dict()
    StimulationDesign["Duration"] = 10
    StimulationDesign["StimulationType"] = "Novel"
    StimulationDesign["StimulationIndex"] = f+1
    StimulationDesign["RecordingFilename"] = "ERNA"
    StimulationDesign["StimulationLead"] = 0
    StimulationDesign["StimulationChannel"] = [2]
    StimulationDesign["StimulationReturn"] = -1
    StimulationJsonDocument["StimulationSequence"].append(StimulationDesign)

    StimulationDesign = dict()
    StimulationDesign["Duration"] = 2
    StimulationDesign["StimulationType"] = "Baseline"
    StimulationDesign["RecordingFilename"] = "ERNA"
    StimulationDesign["StimulationLead"] = 0
    StimulationDesign["StimulationChannel"] = [2]
    StimulationDesign["StimulationReturn"] = -1
    StimulationJsonDocument["StimulationSequence"].append(StimulationDesign)

JsonObject = json.dumps(StimulationJsonDocument)

with open("ERNA_VariableFrequency_4Contacts.json", "w+") as file:
    file.write(JsonObject)
file.close()
