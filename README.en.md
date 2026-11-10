# DcReportMonitorAll

An Arduino sketch for the SPRESENSE main board that receives **all** QZSS (Michibiki) L1S disaster and crisis management reports (DC Report) and prints them in plain text.

- MT43 (DC Report): all 12 categories of JMA disaster prevention information, decoded with the QZQSM library
- MT44 (DC Report Extended / DCX): J-ALERT, L-ALERT, municipality messages and messages from organizations outside Japan, decoded with the QZSSDCX library
- Reception and duplicate recording always cover everything. Only the display is filtered, item by item, with recommended preset modes for common uses.

Japanese version: [README.md](README.md)

## Features

- Receive everything, by design. Nothing is thinned out at reception; filtering is a display-layer (and future downstream) decision.
- Repeated transmissions of the same report are suppressed automatically (MT43: against a history of the last 20 decoded reports; MT44: by content bytes).
- Empty DCX messages (the steady-state message when there is no alert) are counted but not printed by default.
- Satellite status (visible satellites, per-system counts, systems used for positioning, per-satellite elevation / azimuth / C/N) and position information (fix status, latitude/longitude, altitude, velocity, direction, DOP) can be printed at an interval set by the mode (600 seconds in recommended). The `status` command prints the block once, on demand.
- 27 display items (25 on/off items plus 2 format items) and 3 selection filters (categories, report classification, DCX class), all controlled by commands from the serial monitor.
- Starts in the recommended mode.

## Requirements

### Hardware

| Item | Details |
|---|---|
| Board | SPRESENSE main board (CXD5602PWBMAIN1) |
| Antenna | The on-board chip antenna works (verified at a window side). No external antenna is required. |
| Cable | A MicroUSB cable that supports data |
| PC | A PC running the Arduino IDE (developed on Windows) |

This sketch targets the main board's built-in GNSS. It is not for the GNSS add-on board.

### Software

| Item | Details |
|---|---|
| Driver | Silicon Labs CP210x USB to UART Bridge driver (Windows) |
| Arduino IDE | Verified with 2.x |
| Board package | SPRESENSE 3.4.7 (Boards Manager; burn the bootloader once, on first use) |
| Board setting | Tools → Board → Spresense Boards → Spresense; select the board's COM port |
| Serial | 115200 bps |

### Libraries

Install both from the Library Manager.

| Library | Author | License | Purpose |
|---|---|---|---|
| QZQSM | baggio63446333 | BSD 3-Clause | MT43 decoding |
| QZSSDCX | Switch Science | MIT | MT44 (DCX) decoding |

GNSS, GNSSPositionData and RTC are bundled with the board package.

Note: QZSSDCX declares esp32 as its architecture, so the compiler prints an architecture warning. It works on SPRESENSE (verified on real hardware).

## Setup

1. Install the CP210x driver and confirm the board appears as a COM port.
2. Add the Spresense board manager URL in the Arduino IDE and install SPRESENSE from the Boards Manager.
3. First time only: burn the bootloader via Tools → Programmer → Spresense Firmware Updater.
4. Install QZQSM and QZSSDCX from the Library Manager.
5. Open `DcReportMonitorAll/DcReportMonitorAll.ino` from this repository.
6. Select the board and port, then upload.
7. Open the serial monitor at 115200 bps and press the board's reset button so you see the output from the start.

## Usage

Once running, the sketch acquires satellites and prints each report as it arrives. With no configuration at all, the recommended mode shows every category.

Commands are typed into the serial monitor's input box and sent with Enter (newline). They are case-insensitive. Replies from the sketch start with `>>`.

### See the current settings

```
show
```

Prints the mode name and the current value of every item.

```
help
```

Prints the command list.

### Satellite / position block interval and on-demand print

```
status                 print the satellite/position block once
satellites period      show the current interval
satellites period 300  set the interval to 300 seconds
satellites period off  stop the periodic block (print only via status)
```

The interval is set per mode (600 seconds for recommended and disaster, 60 seconds for gnss and full). Applying a mode restores its interval. Changing the interval by hand switches the mode display to custom.

### Modes (recommended presets)

```
mode <name>
```

| Mode | Alias | Use | Behavior |
|---|---|---|---|
| recommended | rec | Everyday monitoring (default) | MT43 full text, DCX summary. Satellite block on, position off. No raw hex. |
| disaster | dis | During a disaster | MT43 full text, DCX summary + target areas. Satellites: count only. |
| gnss | gnss | Reception diagnostics | All satellite and position items on, raw hex on. |
| jalert | jal | Only J-ALERT and L-ALERT | MT43 hidden. DCX summary + target areas. Classes limited to lalert and jalert. |
| eew | eew | Only Earthquake Early Warning | MT43 category 1, full text. No DCX. No periodic blocks. |
| full | full | JPEQ integration / logging | Every item on, both formats full. |
| min | min | Minimal display | MT43 first line only, DCX summary, header only. |

After applying a mode, changing any item individually switches the mode display to custom.

### Turning items on and off

```
set <item> on
set <item> off
```

Sending an item name alone prints its current value (for example `satellites.list`).

#### Common

| Item | Meaning |
|---|---|
| header | The report header line (date/time, MT, satellite counts; for MT43 also RC and DC) |
| raw | Raw received data in hexadecimal |

#### MT43 (DC Report)

| Item | Meaning | Values |
|---|---|---|
| report.format | Display format | full = full text, first = first line only, off = hidden |

Category and classification selection use the `categories` and `priority` commands below.

#### MT44 (DCX)

| Item | Meaning | Values |
|---|---|---|
| dcx.format | Display format | summary, full (the library's full print), off |
| dcx.empty | Print empty messages themselves | on / off |
| dcx.emptycount | Show the empty-message count in the satellite block | on / off |
| dcx.library | Guidance library (A9), evacuate direction (A10), guidance (A11) | on / off |
| dcx.area | Target area (A12–A16) and additional target area (Ex3–Ex7) | on / off |
| dcx.target | Target area code (Ex1) and target areas (Ex8, Ex9) | on / off |
| dcx.settings | Usage settings (A17, A18) and target area code setting (Ex2) | on / off |
| dcx.meta | Class, satellite identifier SD, version, raw A1–A8 codes | on / off |

Class selection uses the `dcxclass` command below. The detail items from dcx.library onward are appended after the summary when dcx.format is summary. With full, the library's full print takes precedence.

#### Satellites (periodic block)

| Item | Meaning |
|---|---|
| satellites.count | Visible satellite count and satellites used for position |
| satellites.systems | Per-system counts and the systems used for positioning |
| satellites.list | Print one line per satellite |
| satellites.elevation | Elevation column of the per-satellite lines |
| satellites.azimuth | Azimuth column |
| satellites.signal | C/N column (dBHz) |

The satellite block prints at the configured interval when at least one satellites item is on. The empty-message count (dcx.emptycount) appears at the end of this block.

#### Position (periodic block)

| Item | Meaning |
|---|---|
| position.time | Time (JST) |
| position.fix | Fix mode (invalid, 2D, 3D) and fix type |
| position.used | Satellites used for position |
| position.latitude | Latitude (degrees, 6 decimals) |
| position.longitude | Longitude (degrees, 6 decimals) |
| position.altitude | Altitude (meters) |
| position.velocity | Velocity (m/s) |
| position.direction | Direction (degrees) |
| position.dop | DOP values (PDOP, HDOP, VDOP, TDOP) |
| position.data | Whether position data exists |

The position block prints at the same interval as the satellite block when at least one position item is on. Before a fix, it prints the time and data status followed by "No fix".

### Category selection (MT43)

The satellites broadcast all categories together; there is no way to filter at reception. Selection happens at the display layer (reception and duplicate recording always cover all categories).

```
categories            show the current selection
categories all        all categories (default)
categories only 1,5   only these categories
categories add 5      add one category
categories remove 5   remove one category
```

| No. | Category | No. | Category |
|---|---|---|---|
| 1 | Earthquake Early Warning | 8 | Volcano |
| 2 | Hypocenter | 9 | Ash Fall |
| 3 | Seismic Intensity | 10 | Weather |
| 4 | Nankai Trough Earthquake | 11 | Flood |
| 5 | Tsunami | 12 | Typhoon |
| 6 | Northwest Pacific Tsunami | 14 | Marine |

7 and 13 are unused.

### Report classification selection (MT43)

```
priority              show the current selection
priority all          all classifications (default)
priority only regular regular only
priority add training add training/test
priority remove training remove training/test
```

Values can be given as names (maximum, priority, regular, training) or numbers (1, 2, 3, 7).

| Value | Classification |
|---|---|
| 1 | Maximum priority |
| 2 | Priority |
| 3 | Regular |
| 7 | Training/Test |

### DCX class selection (MT44)

```
dcxclass              show the current selection
dcxclass all          all classes (default)
dcxclass only jalert  J-ALERT only
dcxclass only lalert,jalert  L-ALERT and J-ALERT
dcxclass add local    add municipality messages
dcxclass remove outside remove messages from outside Japan
```

| Value | Class |
|---|---|
| lalert | L-ALERT (evacuation information etc., provided via FMMC) |
| jalert | J-ALERT (missile launch information etc., provided by FDMA) |
| local | Municipality messages |
| outside | Messages from organizations outside Japan |

Empty messages are not affected by class selection; they are controlled by dcx.empty and dcx.emptycount.

### Examples

Earthquake Early Warning only, full text:

```
mode eew
```

J-ALERT and L-ALERT, summary with target areas:

```
mode jalert
```

Detailed GNSS diagnostics:

```
mode gnss
```

Recommended mode, plus raw hex:

```
mode recommended
set raw on
```

Typhoon and Marine categories only:

```
categories only 12,14
```

Stop training/test reports:

```
priority remove training
```

## Sample output

```
DcReportMonitorAll (DC Report / DCX Receiver)
Display mode: recommended - 'help' for commands, 'show' for settings
-------- Satellite status --------
Visible: 6  Used for position: 0
GPS=5 GLONASS=0 SBAS=0 QZSS-L1CA=0 QZSS-L1S=1 BeiDou=0 Galileo=0
Used systems: none
  GPS ID=28 EL=0 AZ=0 C/N=37.5 dBHz
  QZSS-L1S ID=189 EL=0 AZ=0 C/N=21.6 dBHz
DCX empty messages received: 12
==== 2026/11/10 15:38:08 (JST) MT=43 SAT=6 QZ_L1S=1 ==== RC=3 DC=12
```

The report body follows: plain Japanese text for MT43 (issue time, category, body), and a class-headed summary (or full print) for MT44.

## Specifications and references

- IS-QZSS-DCR-017 — DC Report specification (MT43)
- IS-QZSS-DCX-004 — DC Report Extended specification (MT44 and CAMF)
- IS-QZSS-L1S-009 — L1S signal specification

All are published on the official QZSS website (Cabinet Office, Japan) on the Performance Standard / Interface Specification download page.

DC Report is transmitted on the L1S signal (250 bps). Every 4 seconds, either one MT43 or one MT44 message is delivered. When there is no information to deliver, a null message (MT63) or an empty DCX message is transmitted instead.

## License

The sketch itself is released under the MIT License (see [LICENSE](LICENSE)).

Parts of the sketch are derived from the DcReportMonitor sample sketch of the QZQSM library and remain under the BSD 3-Clause License (Copyright (c) 2019, baggio63446333). The QZSSDCX library is MIT License (Copyright (c) 2024 Switch Science) and is used through its API only. See [NOTICE.md](NOTICE.md) for details.
