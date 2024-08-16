### Serial Communication Logs

- [14:28:34] MOTOR: 0x02 0x26 0x50 0x20 0xc1 0xff // ? False readings from startup
- [14:28:34] DISPLAY: 0x16 0x0b 0x21
- [14:28:34] DISPLAY: 0x11 0x20
- [14:28:34] DISPLAY: 0x11 0x0a
- [14:28:35] DISPLAY: 0x16 0x1a 0xf1
- [14:28:35] DISPLAY: 0x11 0x20
- [14:28:35] DISPLAY: 0x11 0x0a
- [14:28:35] DISPLAY: 0x11 0x08
- [14:28:35] DISPLAY: 0x16 0x0b 0x21
- [14:28:35] DISPLAY: 0x11 0x20
- [14:28:35] MOTOR: 0x20
- [14:28:35] DISPLAY: 0x11 0x0a
- [14:28:36] DISPLAY: 0x11 0x08
- [14:28:36] MOTOR: 0x01
- [14:28:36] DISPLAY: 0x16 0x0c 0x02 0x24
- [14:28:36] DISPLAY: 0x11 0x20
- [14:28:36] MOTOR: 0x20
- [14:28:36] DISPLAY: 0x11 0x0a
- [14:28:36] DISPLAY: 0x11 0x08
- [14:28:36] MOTOR: 0x01
- [14:28:37] DISPLAY: 0x16 0x1f 0x4a 0x7f
- [14:28:37] DISPLAY: 0x11 0x20
- [14:28:37] MOTOR: 0x20
- [14:28:37] DISPLAY: 0x11 0x0a
- [14:28:37] DISPLAY: 0x11 0x08
- [14:28:37] MOTOR: 0x01
- [14:28:37] DISPLAY: 0x11 0x11
- [14:28:37] DISPLAY: 0x11 0x20
- [14:28:38] MOTOR: 0x20
- [14:28:38] DISPLAY: 0x11 0x0a
- [14:28:38] DISPLAY: 0x11 0x08
- [14:28:38] MOTOR: 0x01
- [14:28:38] DISPLAY: 0x11 0x22 0x33
- [14:28:38] DISPLAY: 0x11 0x20
- [14:28:38] MOTOR: 0x20
- [14:28:39] DISPLAY: 0x11 0x0a
- [14:28:39] DISPLAY: 0x11 0x08
- [14:28:39] MOTOR: 0x01
- [14:28:39] DISPLAY: 0x16 0x1a 0xf1 // Turns on LED
- [14:28:39] DISPLAY: 0x16 0x1a 0xf0 // Turns off LED
- [14:28:39] DISPLAY: 0x11 0x20
- [14:28:39] MOTOR: 0x20
- [14:28:39] DISPLAY: 0x11 0x0a
- [14:28:40] DISPLAY: 0x11 0x08
- [14:28:40] MOTOR: 0x01
- [14:28:40] DISPLAY: 0x16 0x0b 0x01 0x22
- [14:28:40] DISPLAY: 0x11 0x20
- [14:28:40] MOTOR: 0x20
- [14:28:40] DISPLAY: 0x11 0x0a
- [14:28:40] DISPLAY: 0x11 0x08
- [14:28:41] MOTOR: 0x01
- [14:28:41] DISPLAY: 0x16 0x0c 0x02 0x24
- [14:28:41] DISPLAY: 0x11 0x20
- [14:28:41] MOTOR: 0x20
- [14:28:41] DISPLAY: 0x11 0x0a
- [14:28:41] DISPLAY: 0x11 0x08
- [14:28:41] MOTOR: 0x01

# Protocol

Thanks for philippsandhaus for providing some protocol information. [link](https://github.com/philippsandhaus/bafang-python)

## Command: Read Basic

- Send: 0x11 0x51 0x04 0xB0 0x05
- Response: 0x51 0x10 0x48 0x5A 0x58 0x54 0x53 0x5A 0x5A 0x36 0x32 0x32 0x32 0x30 0x31 0x31 0x01 0x14 0x1B

### Send (Description)

- 00: 0x11 (Read)
- 01: 0x51 (Basic Data)
- 02-04: 0x04 0xB0 0x05 (?)

### Response (Description)

- 00: 0x51 (Response of the type read)
- 01: 0x10 (Length of the response)
- 02-05: 0x48 0x5A 0x58 0x54 (Manufactor in ascii -> HZXT)
- 06-09: 0x53 0x5A 0x5A 0x36 (Model in ascii -> SZZ6)
- 10-11: 0x32 0x32 (Hardware version ASCII -> 2.2)
- 12-15: 0x32 0x30 0x31 0x31 (Firmware version ASCII -> 2.0.1.1)
- 16: Voltage:

  | Byte | Voltage |
  | :--- | :------ |
  | 0x00 | 24V     |
  | 0x01 | 36V     |
  | 0x02 | 48V     |
  | 0x03 | 60V     |
  | 0x04 | 24-48V  |
  | else | 24-60V  |

- 17: 0x14 (Max current -> 20A)
  - Integer: The maximum current in A
- 18: 0x1B (Checksum?)

## Command: Read General

- Send: 0x11 0x52
- Response: 0x52 0x18 0x1F 0x0F 0x00 0x1C 0x25 0x2E 0x37 0x40 0x49 0x52 0x5B 0x64 0x64 0x64 0x64 0x64 0x64 0x64 0x64 0x64 0x64 0x64 0x34 0x01 0xDF

### Send (Description)

- 00: 0x11 (Read)
- 01: 0x52 (General Settings)

### Response (Description)

- 00: 0x52 (Response of the type read)
- 01: 0x18 (Length of the response)
- 02: 0x1F (Low battery voltage -> 31V)
- 03: 0x0F (Limited current (A) -> 15A)
- 04: 0x00 (Limit current (%) Assist0 -> 0%)
- 05: 0x1C (Limit current (%) Assist1 -> 28%)
- 06: 0x25 (Limit current (%) Assist2 -> 37%)
- 07: 0x2E (Limit current (%) Assist3 -> 46%)
- 08: 0x37 (Limit current (%) Assist4 -> 55%)
- 09: 0x40 (Limit current (%) Assist5 -> 64%)
- 10: 0x49 (Limit current (%) Assist6 -> 73%)
- 11: 0x52 (Limit current (%) Assist7 -> 82%)
- 12: 0x5B (Limit current (%) Assist8 -> 91%)
- 13: 0x64 (Limit current (%) Assist9 -> 100%)
- 14: 0x64 (Limit speed (%) Assist0 -> 100%)
- 15: 0x64 (Limit speed (%) Assist1 -> 100%)
- 16: 0x64 (Limit speed (%) Assist2 -> 100%)
- 17: 0x64 (Limit speed (%) Assist3 -> 100%)
- 18: 0x64 (Limit speed (%) Assist4 -> 100%)
- 19: 0x64 (Limit speed (%) Assist5 -> 100%)
- 20: 0x64 (Limit speed (%) Assist6 -> 100%)
- 21: 0x64 (Limit speed (%) Assist7 -> 100%)
- 22: 0x64 (Limit speed (%) Assist8 -> 100%)
- 23: 0x64 (Limit speed (%) Assist9 -> 100%)
- 24: 0x34 (Wheel Diameter -> 20")

  | Byte      | Inch |
  | :-------- | :--- |
  | 0x1F,0x20 | 16"  |
  | 0x21,0x22 | 17"  |
  | 0x23,0x24 | 18"  |
  | 0x25,0x26 | 19"  |
  | 0x27,0x28 | 20"  |
  | 0x29,0x2A | 21"  |
  | 0x2B,0x2C | 22"  |
  | 0x2D,0x2E | 23"  |
  | 0x2F,0x30 | 24"  |
  | 0x31,0x32 | 25"  |
  | 0x33,0x34 | 26"  |
  | 0x35,0x36 | 27"  |
  | 0x37      | 700C |
  | 0x38      | 28"  |
  | 0x39,0x3A | 29"  |
  | 0x3B,0x3C | 30"  |

- 25: Speedmeter Model/Speedmeter Signal 01:
  - Bits 1-2 (Model)
  - 0x00: External
  - 0x01: Internal
  - 0x02: Motorphase
  - Bits 3-6 (Speedmeter Signals)
- 26: 0xDF (Checksum?)

## Command: Read Pedal

- Send: 0x11 0x53
- Response: 0x53 0x0B 0x03 0xFF 0xFF 0x64 0x06 0x14 0x0A 0x19 0x08 0x14 0x14 0x27

### Send (Description)

- 00: 0x11 (Read)
- 01: 0x53 (Pedal Settings)

### Response (Description)

- 00: 0x53 (Response of the type read)
- 01: 0x0B (Length of the response)
- 02: 0x03 (Pedal Type):

  | Byte | Pedal Type      |
  | :--- | :-------------- |
  | 0x00 | No Pedal        |
  | 0x01 | DH-Sensor-12    |
  | 0x02 | DH-Sensor-32    |
  | 0x03 | DoubleSignal-24 |

- 03: 0xFF (Designated Assist Level):

  | Byte      | Assist Level         |
  | :-------- | :------------------- |
  | 0x00-0x09 | Assist Mode Number   |
  | 0xFF      | By Display's Command |

- 04: 0xFF (Speed Limit):

  | Byte      | Speed Limit                 |
  | :-------- | :-------------------------- |
  | 0x0F-0x28 | Speed Limit in km/h (15-40) |
  | 0xFF      | By Display's Command        |

- 05: 0x64 (Start Current (%) -> 100%)
  | Byte | Current (%) |
  | :-------- | :---------- |
  | 0x00-0x64 | 0-100% |
- 06: 0x06 (Slow-Start Mode -> 6):

  | Byte      | Mode Number |
  | :-------- | :---------- |
  | 0x01-0x08 | 0-8         |

- 07: 0x14 (Start Degree -> 20°)
  - Integer: Number of Signal before start
- 08: 0x0A (Work Mode -> 10):

  | Byte      | Work Mode                             |
  | :-------- | :------------------------------------ |
  | 0x01-0x0A | 1-10 Angular speed of pedal/wheel\*10 |
  | 0xff      | Undetermined                          |

- 09: 0x19 (Time to stop -> 25)
  - Integer: \*10ms
- 10: 0x08 (Current Decay -> 8)
  - 0x01-0x08
- 11: 0x14 (Stop Decay -> 20)
  - Integer: \*10ms
- 12: 0x14 (Keep current (%) -> 20%)
  - 0x00-0x64
- 13: 0x27 (Checksum?)

## Command: Read Throttle

- Send: 0x11 0x54
- Response: 0x54 0x06 0x0B 0x23 0x00 0x03 0x11 0x14 0xAC

### Send (Description)

- 00: 0x11 (Read)
- 01: 0x54 (Throttle Settings)

### Response (Description)

- 00: 0x54 (Response of the type read)
- 01: 0x06 (Length of the response)
- 02: 0x0B (Start voltage (\*100mv) -> 1.1V)
  - Integer: Voltage in 100mV
- 03: 0x23 (End voltage (\*100mv) -> 3.5V)
  - Integer: Voltage in 100mV
- 04: 0x00 (Mode -> 0):

  | Byte | Mode    |
  | :--- | :------ |
  | 0x00 | Current |
  | 0x01 | Speed   |

- 05: 0x03 (Designated Assist Level):

  | Byte      | Assist Level         |
  | :-------- | :------------------- |
  | 0x00-0x09 | Assist Mode Number   |
  | 0xFF      | By Display's Command |

- 06: 0x11 (Speed Limited (14 - 20))

  | Byte      | Speed Limit                 |
  | :-------- | :-------------------------- |
  | 0x0F-0x28 | Speed Limit in km/h (15-40) |
  | 0xFF      | By Display's Command        |

- 07: 0x14 (Start Current (%) -> 20%)

  | Byte      | Current (%) |
  | :-------- | :---------- |
  | 0x00-0x64 | 0-100%      |

- 08: 0xAC (Checksum?)

## Command: Read Speed

- Send: 0x11 0x20
- Response 0x00 0x00 0x20

### Send (Description)

- 00: 0x11 (Read)
- 01: 0x22 (Speed)

### Response (Description)

- 00: 0x00 (?)
- 01: 0x00 (Wheel?)
- 02: 0x20 (Wheel + 0x20?)

## Command: Read Brakes

- Send: 0x11 0x08
- Response: 0x01

### Send (Description)

- 00: 0x11 (Read)
- 01: 0x08 (Brakes)

### Response (Description)

- 00: 0x01 (Status of the brakes -> Not braking):

  | Byte | Description |
  | :--- | :---------- |
  | 0x01 | Not braking |
  | 0x03 | braking     |

## Command: Set Light

- Send: 0x16 0x1a 0xf1
- Response: 0x20

### Send (Description)

- 00: 0x16 (Write)
- 01: 0x1a (Length?)
- 02: 0xf1 (Set light -> On):

  | Byte | Description    |
  | :--- | :------------- |
  | 0xf0 | Turn off light |
  | 0xf1 | Turn on light  |

### Response (Description)

- 00: 0x20 (OK?)
