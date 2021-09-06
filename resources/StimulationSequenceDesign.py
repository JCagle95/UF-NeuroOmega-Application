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

StimulationJsonDocument["AnalogWaveforms"].append("ProbeStim/Probe135.bin")

StimulationDesign = dict()
StimulationDesign["Duration"] = 30
StimulationDesign["StimulationType"] = "Baseline"
StimulationDesign["RecordingFilename"] = "Baseline"
StimulationDesign["StimulationLead"] = 0
StimulationDesign["StimulationChannel"] = [0]
StimulationDesign["StimulationReturn"] = -1
StimulationJsonDocument["StimulationSequence"].append(StimulationDesign)

"""
for contact in range(4):
    for amplitude in np.array(range(9))/2:
        StimulationDesign = dict()
        StimulationDesign["Duration"] = 20
        StimulationDesign["StimulationType"] = "Standard"
        StimulationDesign["RecordingFilename"] = "Threshold"
        StimulationDesign["StimulationLead"] = 0
        StimulationDesign["StimulationChannel"] = [contact]
        StimulationDesign["StimulationReturn"] = -1
        StimulationDesign["Amplitude"] = amplitude
        StimulationDesign["Pulsewidth"] = 90
        StimulationDesign["Frequency"] = 135
        StimulationJsonDocument["StimulationSequence"].append(StimulationDesign)
    
"""
for i in range(4):
    StimulationDesign = dict()
    StimulationDesign["Duration"] = 10
    StimulationDesign["StimulationType"] = "Novel"
    StimulationDesign["StimulationIndex"] = 1
    StimulationDesign["RecordingFilename"] = "ERNA"
    StimulationDesign["StimulationLead"] = 0
    StimulationDesign["StimulationChannel"] = [i]
    StimulationDesign["StimulationReturn"] = -1
    StimulationJsonDocument["StimulationSequence"].append(StimulationDesign)

    StimulationDesign = dict()
    StimulationDesign["Duration"] = 2
    StimulationDesign["StimulationType"] = "Baseline"
    StimulationDesign["RecordingFilename"] = "ERNA"
    StimulationDesign["StimulationLead"] = 0
    StimulationDesign["StimulationChannel"] = [0]
    StimulationDesign["StimulationReturn"] = -1
    StimulationJsonDocument["StimulationSequence"].append(StimulationDesign)

for i in range(4):
    StimulationDesign = dict()
    StimulationDesign["Duration"] = 10
    StimulationDesign["StimulationType"] = "Standard"
    StimulationDesign["RecordingFilename"] = "EP"
    StimulationDesign["StimulationLead"] = 0
    StimulationDesign["StimulationChannel"] = [i]
    StimulationDesign["StimulationReturn"] = -1
    StimulationDesign["Amplitude"] = 2.0
    StimulationDesign["Pulsewidth"] = 90
    StimulationDesign["Frequency"] = 10
    StimulationJsonDocument["StimulationSequence"].append(StimulationDesign)
    
    StimulationDesign = dict()
    StimulationDesign["Duration"] = 2
    StimulationDesign["StimulationType"] = "Baseline"
    StimulationDesign["RecordingFilename"] = "EP"
    StimulationDesign["StimulationLead"] = 0
    StimulationDesign["StimulationChannel"] = [0]
    StimulationDesign["StimulationReturn"] = -1
    StimulationJsonDocument["StimulationSequence"].append(StimulationDesign)


JsonObject = json.dumps(StimulationJsonDocument)

with open("StimulationSequences4_New.json", "w+") as file:
    file.write(JsonObject)
file.close()
