# Generic Fine Sun Sensor - NOS3 Component
This repository contains the NOS3 Generic Fine Sun Sensor Component.
This includes flight software (fsw), ground software (gsw), and simulation(sim) directories.

## Overview
The generic fine sun sensor is an SPI device that accepts a command to retrieve the sun angles and returns the sun angles and an error code.
The available flight software is for use in the core Flight System (cFS) while the ground software supports COSMOS.
A NOS3 simulation is available which uses a 42 data provider.

## Technical Specifications
Angles reading
- Sensor type:  2 axes
- Field of view (FOV):  +/- 60 degrees

## Mechanical
### Reference System
The +z axis is perpendicular to the sensor diodes.  The +x axis is aligned with the long part of the sensor body and is stamped on the body.  The +y axis is aligned with the short part of the sensor body and is stamped on the body.  The x, y, and z axis make a mutually orthogonal, right hand coordinate system.  

Given that the sun unit vector in the sensor coordinate system is (xa, ya, za), the sensor reads out angles alpha and beta, where alpha is the angle from the +z axis in the x-z plane and beta is the angle from the +z axis in the y-z plane.  In other words, for the given unit vector (xa, ya, za):
- tan(alpha) = xa/za
- tan(beta) = ya/za.

## Communications
The NOS3 Generic Fine Sun Sensor Component can communicate via the SPI protocol.

### Data Format
The data structure for communications is explained below:
- Character transmission (unsigned char):  (Byte_0(LSB)) --> to communications channel
- Floating data transmission (float):  Float codification according to IEEE 754-1985 standard for single-precision floating 32 bits:  (sign|exponent|fraction) = (Byte_3(MSB)|Byte_2|Byte_1|Byte_0(LSB)) --> to communications channel
    - Floating value = s x 2^e x m, where:
        - Sign(s): 1 bit (0 = positive, 1 = negative)
        - Exponent: 8 bits (e = Exponent - 127)
        - Fraction: 23 bits (m = 1.Fraction)

### Frame Format
This protocol uses two messages:
- Request message: from master to slave
- Response message: from slave to master

The master is the on board computer of the satellite, or the master of the communication bus, and the slave is the generic fine sun sensor device.

Every command sent to the sensor shall comply with the format below:
- Sync Word = 0xDEADBEEF (4 bytes) - Synchronization word that indicates the start of a new frame.
- Command Code = 0xXX (1 byte) - Corresponds to the code of the incoming command.
- Length = 0x01 (1 byte) - For commands it is fixed to 0x01.
- Checksum = 0xXX (1 byte) - For command 0x01, the value of the checksum field is 0x02.

Every response sent to the master shall comply with the format described below:
- Sync Word = 0xDEADBEEF (4 bytes) - Synchronization word that indicates the start of a new frame.
- Command Code = 0xXX (1 byte) - Corresponds to the code of the command which this response refers to.
- Length = 0xXX (1 byte) - The sum of the number of bytes of the fields "Application Data" and "Checksum".
- Application Data = 0xXX ... 0xXX (2-16 bytes) - The response with the data requested by the corresponding command.
- Checksum = 0xXX (1 byte) - Used to check the integrity of the packet.  It is calculated by adding all bytes in "Command Code", "Length", and "Application Data" fields and extracting the least significant byte of the result.
    - E.g. Successful calculation of angular position with angles alpha = 5 degrees (0x40 A0 00 00) and beta = 10 degrees (0x41 20 00 00)
        - DE AD BE EF 01 0A 40 A0 00 00 41 20 00 00 00 
        - 01 + 0A + 40 + A0 + 00 + 00 + 41 + 20 + 00 + 00 + 00 = 0x14C.  Least significant byte of 0x14C is 0x4C.
        - So checksum is 0x4C

### SPI Protocol Format
The following describes an example of a command/response communication for command code 0x01 via SPI.  Note that the IDLE byte value is 0xFF:

- SPI Request:
    - On board computer to generic fine sun sensor device:
        - Sync Word = 0xDEADBEEF (4 bytes)
        - Command Code - 0x01 (1 byte)
        - Length = 0x01 (1 byte)
        - Checksum = 0x02 (1 byte)
    - Generic fine sun sensor device to on board computer:
        - Idle message = IDLE x 7 (7 bytes)
- SPI Response:
    - On board computer to generic fine sun sensor device:
        - Idle message = IDLE x 16 (16 bytes)
    - Generic fine sun sensor device to on board computer:
        - Sync Word = 0xDEADBEEF (4 bytes)
        - Command Code = 0x01 (1 byte)
        - Length = 0x0A (1 byte)
        - Application Data
            - Angle alpha = 0xXX 0xXX 0xXX 0xXX (float, 4 bytes)
            - Angle beta = 0xXX 0xXX 0xXX 0xXX (float, 4 bytes)
            - Error code = 0xXX (1 byte)
        - Checksum = 0xXX (1 byte)

### Commands
The command codes implemented are:
- Command Code = 0x01, Name = Angular position, Functionality = Request for the angular position (alpha, beta) and the error code.

#### Command 0x01:  Angular position
Request for the angular position and corresponding error code.  The two angles which determine the angular position (alpha and beta, see above) are represented by a single-precision floating-point format.  The error code is represented in a char.
- Command
    - Sync Word = 0xDEADBEEF (4 bytes)
    - Command Code = 0x01 (1 byte)
    - Length = 0x01 (1 byte)
    - Checksum = 0x02 (1 byte)

- Response
    - Sync Word = 0xDEADBEEF (4 bytes)
    - Command Code = 0x01 (1 byte)
    - Length = 0x0A (1 byte)
    - Application Data:
        - Angle alpha = 0xXX 0xXX 0xXX 0xXX (float, 4 bytes)
        - Angle beta = 0xXX 0xXX 0xXX 0xXX (float, 4 bytes)
        - Error code = 0xXX (1 byte)
    - Checksum = 0xXX (1 byte)

Error codes:
- 0 = No error.  Angles were calculated successfully.
- 1 = Error.  Sun not in field of view or earth albedo affected measurement.
