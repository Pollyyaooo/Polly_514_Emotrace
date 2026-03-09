# EmoTrace
#### MSTI TECHIN 514 Final Project           
#### 01/06/2026-03/06/2026
![overview](assets/overview.png)

# OVERVIEW
## One Sentence Explanation
EmoTrace uses GSR to sense physiological arousal and express a person’s current emotional state in real time.

## Problem being Solved
Emotion is a continuously changing internal state, but it is difficult to observe directly. Most of the time, we only understand our emotions afterward, through memory or language, rather than seeing how they change in the moment.

If emotional states could be __externalized__, they might become something that both the individual and others can notice in real time.

## The Proposed Solution
EmoTrace measures physiological arousal through galvanic skin response (GSR) and expresses these changes using a stepper-motor-driven physical needle.

The system consists of two physically separated devices connected wirelessly via BLE: a sensing device, which captures and processes physiological signals, and a display device, which translates these signals into slow mechanical movement.

This separation allows the relationship between the person being sensed and the person observing the display to remain flexible. They can be the same person or different people. In this way, emotional states are externalized as a physical trace that can be noticed and reflected upon.



## System Architecture

![System Architecture](assets/system_architecture.png)

**Sensing Device**
- Captures electrodermal activity via finger electrodes and a GSR sensor module
- Performs basic digital signal processing (filtering, baseline normalization)
- Transmits a continuous arousal indicator via BLE

**Display Device**
- Receives arousal data wirelessly
- Drives a stepper-motor-based gauge needle as the primary expressive output
- Provides minimal user input and status indication
- Is implemented on a custom-designed PCB

## Battery Estimation

### Display Device
Spreadsheet link:  
https://1drv.ms/x/c/3520740a59bca320/IQBtffllcoQcSJlxG03brPWNAZ11j2MDRlCTxOjXR5vgqAk?e=tpQcXu


### Sensing Device
Spreadsheet link:  
https://1drv.ms/x/c/3520740a59bca320/IQDMDjtJkVfMQYsOUzpoie-YAZGBVunHImL63yKkG8l4-dY?e=yHCJq8

