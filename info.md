Speed 0:
[14:28:34] - serial2:  0x02 0x26 0x50 0x20 0xc1 0xff // ? Sends motor on boot.
[14:28:34] - serial1:  0x16 0x0b 0x21
[14:28:34] - serial1:  0x11 0x20
[14:28:34] - serial1:  0x11 0x0a
[14:28:35] - serial1:  0x16 0x1a 0xf1
[14:28:35] - serial1:  0x11 0x20
[14:28:35] - serial1:  0x11 0x0a
[14:28:35] - serial1:  0x11 0x08
[14:28:35] - serial1:  0x16 0x0b 0x21
[14:28:35] - serial1:  0x11 0x20
[14:28:35] - serial2:  0x20
[14:28:35] - serial1:  0x11 0x0a
[14:28:36] - serial1:  0x11 0x08
[14:28:36] - serial2:  0x01
[14:28:36] - serial1:  0x16 0x0c 0x02 0x24
[14:28:36] - serial1:  0x11 0x20
[14:28:36] - serial2:  0x20
[14:28:36] - serial1:  0x11 0x0a
[14:28:36] - serial1:  0x11 0x08
[14:28:36] - serial2:  0x01
[14:28:37] - serial1:  0x16 0x1f 0x4a 0x7f
[14:28:37] - serial1:  0x11 0x20
[14:28:37] - serial2:  0x20
[14:28:37] - serial1:  0x11 0x0a
[14:28:37] - serial1:  0x11 0x08
[14:28:37] - serial2:  0x01
[14:28:37] - serial1:  0x11 0x11
[14:28:37] - serial1:  0x11 0x20
[14:28:38] - serial2:  0x20
[14:28:38] - serial1:  0x11 0x0a
[14:28:38] - serial1:  0x11 0x08
[14:28:38] - serial2:  0x01
[14:28:38] - serial1:  0x11 0x22 0x33
[14:28:38] - serial1:  0x11 0x20
[14:28:38] - serial2:  0x20
[14:28:39] - serial1:  0x11 0x0a
[14:28:39] - serial1:  0x11 0x08
[14:28:39] - serial2:  0x01
[14:28:39] - serial1:  0x16 0x1a 0xf1 // Turns on LED
[14:28:39] - serial1:  0x16 0x1a 0xf0 // Turns off LED
[14:28:39] - serial1:  0x11 0x20
[14:28:39] - serial2:  0x20
[14:28:39] - serial1:  0x11 0x0a
[14:28:40] - serial1:  0x11 0x08
[14:28:40] - serial2:  0x01
[14:28:40] - serial1:  0x16 0x0b 0x01 0x22
[14:28:40] - serial1:  0x11 0x20
[14:28:40] - serial2:  0x20
[14:28:40] - serial1:  0x11 0x0a
[14:28:40] - serial1:  0x11 0x08
[14:28:41] - serial2:  0x01
[14:28:41] - serial1:  0x16 0x0c 0x02 0x24
[14:28:41] - serial1:  0x11 0x20
[14:28:41] - serial2:  0x20
[14:28:41] - serial1:  0x11 0x0a
[14:28:41] - serial1:  0x11 0x08
[14:28:41] - serial2:  0x01

## Protocol

### Basic Read Request

| Byte Number | Byte | Meaning               |
| :---------- | :--- | :-------------------- |
| 0           | 0x11 | Read                  |
| 1           | 0x51 | Basic Data            |
| 2           | 0x04 | Command Length        |
| 3           | 0xB0 | None?                 |
| 4           | 0x05 | Sum(bytes 2..4) % 256 |

### General Read Request

| Byte Number | Byte | Meaning          |
| :---------- | :--- | :--------------- |
| 0           | 0x11 | Read             |
| 1           | 0x52 | General Settings |

### Pedal Read Request

| Byte Number | Byte | Meaning        |
| :---------- | :--- | :------------- |
| 0           | 0x11 | Read           |
| 1           | 0x53 | Pedal Settings |

### Throttle Read Request

| Byte Number | Byte | Meaning           |
| :---------- | :--- | :---------------- |
| 0           | 0x11 | Read              |
| 1           | 0x54 | Throttle Settings |

###
