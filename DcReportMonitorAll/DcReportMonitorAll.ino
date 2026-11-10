/*
 * DcReportMonitorAll
 * QZSS L1S DC Report / DCX all-message monitor for Spresense.
 * Version 1.1 (2026-11-10).
 *
 * Based on DcReportMonitor (QZQSM library sample), extended to
 * receive ALL messages (filtering is a downstream decision):
 *   MT43 -> QZQSM library (JMA disaster prevention information,
 *           all 12 categories; enabled by default in QZQSM.h)
 *   MT44 -> QZSSDCX library (DCX: J-ALERT / L-ALERT / local
 *           government / overseas organizations)
 *   other MT -> raw hex dump (nothing is dropped at reception)
 * Portions derived from the DcReportMonitor sample sketch of the
 * QZQSM library:
 *   Copyright (c) 2019, baggio63446333
 *   Licensed under the BSD 3-Clause License (see the LICENSE file
 *   in the QZQSM library). Redistribution must retain that notice.
 * The QZSSDCX library (MIT License, Copyright (c) 2024 Switch
 * Science) is used via its API; no code is copied from it.
 * Dedupe: MT43 uses the original 20-entry decoded-report history.
 *         MT44 compares the message content bytes (excluding the
 *         rotating first byte and the counter/CRC tail), so a
 *         repeated alert is printed only once.
 * Reception and dedupe recording always run for everything.
 * Every display item below can be switched individually; the
 * switches gate only what is printed.
 *
 * ---------------- Serial commands ----------------
 * (Serial Monitor, newline ending; upper case is accepted)
 *   help                     this command list
 *   show                     list the mode and all item values
 *   status                   print the satellite / position
 *                            block once, on demand
 *   satellites period        show the periodic block interval
 *   satellites period 300    set the interval (seconds)
 *   satellites period off    stop the periodic block
 *   mode <name>              apply a mode (table below)
 *   set <item> on | off      switch one item
 *   set report.format full | first | off
 *   set dcx.format summary | full | off
 *   categories all           all MT43 categories (default)
 *   categories only 1,5      only these MT43 categories
 *   categories add 5         add one category
 *   categories remove 5      remove one category
 *   priority all             all MT43 report classifications
 *   priority only regular    only these (maximum, priority,
 *                            regular, training; or 1,2,3,7)
 *   priority add training    add one classification
 *   priority remove training remove one classification
 *   dcxclass all             all DCX classes (default)
 *   dcxclass only jalert     only these DCX classes (lalert,
 *                            jalert, local, outside)
 *   dcxclass add lalert      add one class
 *   dcxclass remove lalert   remove one class
 *   <item>                   show one item's current value
 *
 * ---------------- Display items and modes ----------------
 * R=recommended D=disaster G=gnss J=jalert E=eew F=full M=min
 * (o = on, - = off; modes are applied with "mode <name>")
 *   item                  meaning                    R D G J E F M
 *   header                report header line         o o o o o o o
 *   raw                   raw hex (MT44, other MT)   - - o - - o -
 *   report.format         MT43: full/first/off       F F F - F F f
 *   report.categories     MT43 categories (above)    a a a a 1 a a
 *   report.priority       MT43 classifications       a a a a a a a
 *   dcx.format            MT44: summary/full/off     S S S S - F S
 *   dcx.classes           DCX classes (above)        a a a L a a a
 *   dcx.empty             print empty DCX messages   - - - - - o -
 *   dcx.emptycount        DCX empty count (periodic) o o o - - o -
 *   dcx.library           DCX A9-A11 guidance        - - - - - o -
 *   dcx.area              DCX A12-A16, Ex3-Ex7 area  - - - - - o -
 *   dcx.target            DCX Ex1, Ex8/Ex9 targets   - o - o - o -
 *   dcx.settings          DCX A17/A18, Ex2           - - - - - o -
 *   dcx.meta              DCX class, SD, Vn, raw A1-A8 - - - - - o -
 *   satellites.count      visible / used count       o o o - - o -
 *   satellites.systems    per-system counts          o - o - - o -
 *   satellites.list       per-satellite lines        o - o - - o -
 *   satellites.elevation  elevation in the list      o - o - - o -
 *   satellites.azimuth    azimuth in the list        o - o - - o -
 *   satellites.signal     C/N in the list            o - o - - o -
 *   position.time         time (JST, periodic block) - - o - - o -
 *   position.fix          fix mode and type          - - o - - o -
 *   position.used         satellites used            - - o - - o -
 *   position.latitude     latitude                   - - o - - o -
 *   position.longitude    longitude                  - - o - - o -
 *   position.altitude     altitude                   - - o - - o -
 *   position.velocity     velocity                   - - o - - o -
 *   position.direction    direction                  - - o - - o -
 *   position.dop          DOP values                 - - o - - o -
 *   position.data         position data exists       - - o - - o -
 * (F = full, f = first line, S = summary, a = all categories,
 *  1 = category 1 only, L = lalert + jalert only. The satellite
 *  and position blocks print at the mode's period
 *  (recommended and disaster 600 s, gnss and full 60 s) when
 *  at least one of their items is on; the period is changed
 *  with "satellites period", and "status" prints the block
 *  once on demand.
 *  dcx.library to dcx.meta add detail groups to the summary;
 *  with dcx.format full, printAll includes every field except
 *  SD, which only dcx.meta prints. Changing any item or the
 *  categories after a mode makes the mode "custom".)
 *
 * Modes: recommended = AI-recommended general set (default),
 *        disaster = reports first, target areas shown, quiet,
 *        gnss = satellite / position diagnostics,
 *        jalert = DCX only (J-ALERT / L-ALERT), no MT43,
 *        eew = Earthquake Early Warning only (category 1),
 *        full = everything on (complete output, for JPEQ),
 *        min = minimal (report first lines + DCX summary).
 * Aliases: rec, dis, gnss, jal, eew, full, min.
 *
 * MT43 categories (Dc codes, IS-QZSS-DCR-017): 1 EEW,
 *   2 Hypocenter, 3 Seismic Intensity, 4 Nankai Trough,
 *   5 Tsunami, 6 NW Pacific Tsunami, 8 Volcano, 9 Ash Fall,
 *   10 Weather, 11 Flood, 12 Typhoon, 14 Maritime
 *   (7 and 13 are unused).
 *
 * Required hardware:
 *   Sony SPRESENSE main board (CXD5602PWBMAIN1), built-in GNSS
 *   antenna; a data-capable micro-USB cable.
 * Required driver (Windows):
 *   Silicon Labs CP210x USB to UART Bridge driver
 *   (board appears as a COM port; COM13 on the dev PC).
 * Required Arduino environment:
 *   Board package: SPRESENSE (sonydevworld) 3.4.7
 *     Tools > Board > Spresense Boards > Spresense
 *     Bootloader must be written once via Spresense Firmware
 *     Updater before the first upload.
 *   Libraries (install via Library Manager):
 *     QZQSM   - QZSS DC Report decode library (by baggio63446333)
 *     QZSSDCX - QZSS DCX decode library (by Switch Science)
 *     GNSS, GNSSPositionData, RTC - bundled with the board package
 * Serial monitor: 115200 bps.
 * References: IS-QZSS-DCR-017 (MT43), IS-QZSS-DCX-004 (MT44/CAMF),
 *             IS-QZSS-L1S-009 (signal and common section).
 */

#include <Arduino.h>
#include <GNSS.h>
#include <GNSSPositionData.h>
#include <QZQSM.h>
#include <QZSSDCX.h>
#include <RTC.h>

const int HISTORY_COUNT = 20;
QZQSM s_history[HISTORY_COUNT];
int s_index = 0;

/* MT44 dedupe key: message bytes 1..25 (content part) */
const int DCX_KEY_LEN = 25;
uint8_t s_raw_history[HISTORY_COUNT][DCX_KEY_LEN];
int s_raw_index = 0;

/* Number of empty DCX messages received */
static unsigned long s_dcxEmptyCount = 0;

/* ---------------- Display items ---------------- */
enum ItemId {
  IT_HEADER, IT_RAW,
  IT_DCX_EMPTY, IT_DCX_EMPTYCOUNT, IT_DCX_LIBRARY, IT_DCX_AREA,
  IT_DCX_TARGET, IT_DCX_SETTINGS, IT_DCX_META,
  IT_SAT_COUNT, IT_SAT_SYSTEMS, IT_SAT_LIST, IT_SAT_ELEVATION,
  IT_SAT_AZIMUTH, IT_SAT_SIGNAL,
  IT_POS_TIME, IT_POS_FIX, IT_POS_USED, IT_POS_LATITUDE,
  IT_POS_LONGITUDE, IT_POS_ALTITUDE, IT_POS_VELOCITY,
  IT_POS_DIRECTION, IT_POS_DOP, IT_POS_DATA,
  IT_COUNT
};
static const char *ITEM_NAME[IT_COUNT] = {
  "header", "raw",
  "dcx.empty", "dcx.emptycount", "dcx.library", "dcx.area",
  "dcx.target", "dcx.settings", "dcx.meta",
  "satellites.count", "satellites.systems", "satellites.list",
  "satellites.elevation", "satellites.azimuth", "satellites.signal",
  "position.time", "position.fix", "position.used",
  "position.latitude", "position.longitude", "position.altitude",
  "position.velocity", "position.direction", "position.dop",
  "position.data"
};
/* Boot state = recommended mode */
static bool s_item[IT_COUNT] = {
  1, 0,
  0, 1, 0, 0,
  0, 0, 0,
  1, 1, 1, 1,
  1, 1,
  0, 0, 0, 0,
  0, 0, 0,
  0, 0, 0
};
/* Format items: report 0=off 1=first 2=full; dcx 0=off 1=summary 2=full */
static int s_reportFormat = 2;
static int s_dcxFormat = 1;

/* Periodic satellite / position block interval, in seconds.
 * 0 = periodic block off (on demand only, "status" command).
 * Set by modes; changed with "satellites period". */
static int s_periodSec = 600;
static int s_tickCounter = 0;

/* Set by the "status" command; consumed in loop() once fresh
 * NavData is available */
static bool s_statusRequested = false;

/* MT43 category filter: bit (1 << Dc) set = print that category.
 * Dc is read from the raw message bits 18-21 (IS-QZSS-DCR-017):
 * PAB 8 + MT 6 + Rc 3, then Dc 4 -> (sf[2] >> 3) & 0x0F. */
static const uint16_t CAT_ALL = 0x7FFE; /* Dc 1..14 */
static uint16_t s_catMask = CAT_ALL;

/* MT43 report classification (Rc) filter: bits by Rc value.
 * Rc is read from the raw message bits 15-17:
 * ((sf[1] & 0x03) << 1) | (sf[2] >> 7).
 * Values (IS-QZSS-DCR-017): 1 maximum, 2 priority, 3 regular,
 * 7 training/test. */
static const uint8_t RC_ALL = 0x8E; /* bits 1, 2, 3, 7 */
static uint8_t s_rcMask = RC_ALL;

/* DCX class filter: bits by dcx_msg_type (QZSSDCX):
 * 1 lalert, 2 jalert, 3 local (municipality), 4 outside. */
static const uint8_t DCX_CLASS_ALL = 0x1E; /* bits 1..4 */
static uint8_t s_dcxClassMask = DCX_CLASS_ALL;

static char s_mode[16] = "recommended";

struct ModeDef {
  const char *name;
  const char *alias;
  bool v[IT_COUNT];
  int reportFormat;
  int dcxFormat;
  uint16_t cat;
  uint8_t rc;
  uint8_t dcxClass;
  int period;
};
static const ModeDef MODES[] = {
  /* name          alias   header raw | dcx: empty emptycount library area target settings meta | sat: count systems list elevation azimuth signal | pos: time fix used latitude longitude altitude velocity direction dop | rf df cat rc dcx period */
  { "recommended", "rec",
    { 1, 0,   0, 1, 0, 0, 0, 0, 0,   1, 1, 1, 1, 1, 1,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 2, 1, CAT_ALL, RC_ALL, DCX_CLASS_ALL, 600 },
  { "disaster", "dis",
    { 1, 0,   0, 1, 0, 0, 1, 0, 0,   1, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 2, 1, CAT_ALL, RC_ALL, DCX_CLASS_ALL, 600 },
  { "gnss", "gnss",
    { 1, 1,   0, 1, 0, 0, 0, 0, 0,   1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, 2, 1, CAT_ALL, RC_ALL, DCX_CLASS_ALL, 60 },
  { "jalert", "jal",
    { 1, 0,   0, 0, 0, 0, 1, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0, 1, CAT_ALL, RC_ALL, (uint8_t)((1u << DCX_MSG_L_ALERT) | (1u << DCX_MSG_J_ALERT)), 60 },
  { "eew", "eew",
    { 1, 0,   0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 2, 0, (uint16_t)(1u << 1), RC_ALL, DCX_CLASS_ALL, 60 },
  { "full", "full",
    { 1, 1,   1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, 2, 2, CAT_ALL, RC_ALL, DCX_CLASS_ALL, 60 },
  { "min", "min",
    { 1, 0,   0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 1, 1, CAT_ALL, RC_ALL, DCX_CLASS_ALL, 60 },
};
static const int MODE_COUNT = sizeof(MODES) / sizeof(MODES[0]);

static const char *reportFormatName(int f)
{
  return f == 2 ? "full" : (f == 1 ? "first" : "off");
}
static const char *dcxFormatName(int f)
{
  return f == 2 ? "full" : (f == 1 ? "summary" : "off");
}

static void printPeriodValue()
{
  Serial.print(">> satellites.period = ");
  if (s_periodSec == 0) {
    Serial.println("off");
  } else {
    Serial.print(s_periodSec);
    Serial.println(" s");
  }
}

static void printCatMask()
{
  if (s_catMask == CAT_ALL) {
    Serial.print("all");
    return;
  }
  int dc;
  bool first = true;
  for (dc = 1; dc <= 14; dc++) {
    if (s_catMask & (1u << dc)) {
      if (!first) Serial.print(",");
      first = false;
      Serial.print(dc);
    }
  }
}

static void printItemGroup(const char *label, int first, int last)
{
  Serial.print(">> [");
  Serial.print(label);
  Serial.print("] ");
  int i;
  for (i = first; i <= last; i++) {
    const char *n = ITEM_NAME[i];
    const char *dot = strchr(n, '.');
    Serial.print(dot ? dot + 1 : n);
    Serial.print(s_item[i] ? "=on " : "=off ");
  }
  Serial.println();
}

static void showStatus()
{
  Serial.print(">> Mode: ");
  Serial.println(s_mode);
  Serial.print(">> [report] format=");
  Serial.print(reportFormatName(s_reportFormat));
  Serial.print(" categories=");
  printCatMask();
  Serial.print(" priority=");
  printRcMask();
  Serial.println();
  Serial.print(">> [dcx] format=");
  Serial.print(dcxFormatName(s_dcxFormat));
  Serial.print(" classes=");
  printDcxClassMask();
  Serial.print(" ");
  int i;
  for (i = IT_DCX_EMPTY; i <= IT_DCX_META; i++) {
    const char *n = ITEM_NAME[i];
    Serial.print(strchr(n, '.') + 1);
    Serial.print(s_item[i] ? "=on " : "=off ");
  }
  Serial.println();
  printItemGroup("common", IT_HEADER, IT_RAW);
  printItemGroup("satellites", IT_SAT_COUNT, IT_SAT_SIGNAL);
  printPeriodValue();
  printItemGroup("position", IT_POS_TIME, IT_POS_DATA);
}

static void printHelp()
{
  Serial.println(">> Commands:");
  Serial.println(">>   show                list the mode and all item values");
  Serial.println(">>   status              print the satellite/position block once");
  Serial.println(">>   satellites period [sec|off]  periodic block interval");
  Serial.println(">>   mode <name>         recommended|disaster|gnss|jalert|eew|full|min");
  Serial.println(">>   set <item> on|off   switch one item (names: see header)");
  Serial.println(">>   set report.format full|first|off");
  Serial.println(">>   set dcx.format summary|full|off");
  Serial.println(">>   categories all | only 1,5 | add 5 | remove 5");
  Serial.println(">>   priority all | only regular | add training | remove training");
  Serial.println(">>   dcxclass all | only jalert | add lalert | remove lalert");
  Serial.println(">>   <item>              show one item's current value");
}

static bool applyMode(const char *name)
{
  int i;
  for (i = 0; i < MODE_COUNT; i++) {
    if (strcmp(MODES[i].name, name) == 0 ||
        strcmp(MODES[i].alias, name) == 0) {
      int j;
      for (j = 0; j < IT_COUNT; j++) s_item[j] = MODES[i].v[j];
      s_reportFormat = MODES[i].reportFormat;
      s_dcxFormat = MODES[i].dcxFormat;
      s_catMask = MODES[i].cat;
      s_rcMask = MODES[i].rc;
      s_dcxClassMask = MODES[i].dcxClass;
      s_periodSec = MODES[i].period;
      s_tickCounter = 0;
      strncpy(s_mode, MODES[i].name, sizeof(s_mode) - 1);
      s_mode[sizeof(s_mode) - 1] = '\0';
      Serial.print(">> Mode: ");
      Serial.println(s_mode);
      return true;
    }
  }
  return false;
}

static void markCustom()
{
  strcpy(s_mode, "custom");
}

static int findItem(const char *name)
{
  int i;
  for (i = 0; i < IT_COUNT; i++) {
    if (strcmp(ITEM_NAME[i], name) == 0) return i;
  }
  return -1;
}

static const char *rcName(int v)
{
  switch (v) {
    case 1: return "maximum";
    case 2: return "priority";
    case 3: return "regular";
    case 7: return "training";
    default: return "unknown";
  }
}

static int rcValueOf(const char *s)
{
  if (strcmp(s, "maximum") == 0) return 1;
  if (strcmp(s, "priority") == 0) return 2;
  if (strcmp(s, "regular") == 0) return 3;
  if (strcmp(s, "training") == 0) return 7;
  int v = atoi(s);
  if (v == 1 || v == 2 || v == 3 || v == 7) return v;
  return -1;
}

static void printRcMask()
{
  if ((s_rcMask & RC_ALL) == RC_ALL) {
    Serial.print("all");
    return;
  }
  static const int vals[4] = { 1, 2, 3, 7 };
  bool first = true;
  int i;
  for (i = 0; i < 4; i++) {
    if (s_rcMask & (1u << vals[i])) {
      if (!first) Serial.print(",");
      first = false;
      Serial.print(rcName(vals[i]));
    }
  }
  if (first) Serial.print("none");
}

static const char *dcxClassName(int t)
{
  switch (t) {
    case DCX_MSG_L_ALERT: return "lalert";
    case DCX_MSG_J_ALERT: return "jalert";
    case DCX_MSG_MT_INFO: return "local";
    case DCX_MSG_OUTSIDE_JAPAN: return "outside";
    default: return "unknown";
  }
}

static int dcxClassOf(const char *s)
{
  if (strcmp(s, "lalert") == 0) return DCX_MSG_L_ALERT;
  if (strcmp(s, "jalert") == 0) return DCX_MSG_J_ALERT;
  if (strcmp(s, "local") == 0) return DCX_MSG_MT_INFO;
  if (strcmp(s, "outside") == 0) return DCX_MSG_OUTSIDE_JAPAN;
  return -1;
}

static void printDcxClassMask()
{
  if (s_dcxClassMask == DCX_CLASS_ALL) {
    Serial.print("all");
    return;
  }
  bool first = true;
  int t;
  for (t = 1; t <= 4; t++) {
    if (s_dcxClassMask & (1u << t)) {
      if (!first) Serial.print(",");
      first = false;
      Serial.print(dcxClassName(t));
    }
  }
  if (first) Serial.print("none");
}

static void printItemValue(const char *name)
{
  if (strcmp(name, "report.format") == 0) {
    Serial.print(">> report.format = ");
    Serial.println(reportFormatName(s_reportFormat));
    return;
  }
  if (strcmp(name, "report.priority") == 0) {
    Serial.print(">> report.priority = ");
    printRcMask();
    Serial.println();
    return;
  }
  if (strcmp(name, "dcx.classes") == 0) {
    Serial.print(">> dcx.classes = ");
    printDcxClassMask();
    Serial.println();
    return;
  }
  if (strcmp(name, "dcx.format") == 0) {
    Serial.print(">> dcx.format = ");
    Serial.println(dcxFormatName(s_dcxFormat));
    return;
  }
  if (strcmp(name, "satellites.period") == 0) {
    printPeriodValue();
    return;
  }
  if (strcmp(name, "report.categories") == 0) {
    Serial.print(">> report.categories = ");
    printCatMask();
    Serial.println();
    return;
  }
  int id = findItem(name);
  if (id >= 0) {
    Serial.print(">> ");
    Serial.print(ITEM_NAME[id]);
    Serial.println(s_item[id] ? " = on" : " = off");
    return;
  }
  Serial.print(">> Unknown item: ");
  Serial.println(name);
}

/* Parse a comma-separated category list into a mask */
static uint16_t parseCatList(const char *arg)
{
  uint16_t m = 0;
  const char *p = arg;
  while (p && *p) {
    int v = atoi(p);
    if (v >= 1 && v <= 14) m |= (uint16_t)(1u << v);
    p = strchr(p, ',');
    if (p) p++;
  }
  return m;
}

/* Serial command parser: up to 3 space-separated words */
static void execCommand(char *line)
{
  char *tok[3];
  int ntok = 0;
  char *p = line;
  while (*p && ntok < 3) {
    while (*p == ' ' || *p == '\t') p++;
    if (!*p) break;
    tok[ntok++] = p;
    while (*p && *p != ' ' && *p != '\t') p++;
    if (*p) {
      *p = '\0';
      p++;
    }
  }
  if (ntok == 0) return;

  if (strcmp(tok[0], "help") == 0) {
    printHelp();
    return;
  }
  if (strcmp(tok[0], "show") == 0) {
    showStatus();
    return;
  }
  if (strcmp(tok[0], "status") == 0) {
    /* Printed in loop() with the next fresh NavData */
    s_statusRequested = true;
    return;
  }
  if (strcmp(tok[0], "satellites") == 0) {
    if (ntok >= 2 && strcmp(tok[1], "period") == 0) {
      if (ntok == 2) {
        printPeriodValue();
        return;
      }
      const char *a = tok[2];
      if (strcmp(a, "off") == 0) {
        s_periodSec = 0;
      } else if (a[0] >= '0' && a[0] <= '9') {
        int v = atoi(a);
        if (v != 0 && (v < 10 || v > 3600)) {
          Serial.println(">> satellites period: off | 0 | 10-3600 (seconds)");
          return;
        }
        s_periodSec = v;
      } else {
        Serial.println(">> satellites period: off | 0 | 10-3600 (seconds)");
        return;
      }
      s_tickCounter = 0;
      markCustom();
      printPeriodValue();
      return;
    }
    Serial.println(">> Use: satellites period [seconds | off]");
    return;
  }
  if (strcmp(tok[0], "mode") == 0) {
    if (ntok == 1) {
      Serial.print(">> Mode: ");
      Serial.println(s_mode);
    } else if (!applyMode(tok[1])) {
      Serial.print(">> Unknown mode: ");
      Serial.println(tok[1]);
      Serial.println(">> Modes: recommended | disaster | gnss | jalert | eew | full | min");
    }
    return;
  }
  if (strcmp(tok[0], "categories") == 0) {
    if (ntok == 1) {
      Serial.print(">> report.categories = ");
      printCatMask();
      Serial.println();
      return;
    }
    if (strcmp(tok[1], "all") == 0) {
      s_catMask = CAT_ALL;
      markCustom();
    } else if (strcmp(tok[1], "only") == 0 && ntok == 3) {
      uint16_t m = parseCatList(tok[2]);
      if (m == 0) {
        Serial.println(">> No valid category (1-14)");
        return;
      }
      s_catMask = m;
      markCustom();
    } else if (strcmp(tok[1], "add") == 0 && ntok == 3) {
      int v = atoi(tok[2]);
      if (v < 1 || v > 14) {
        Serial.println(">> Category must be 1-14");
        return;
      }
      s_catMask |= (uint16_t)(1u << v);
      markCustom();
    } else if (strcmp(tok[1], "remove") == 0 && ntok == 3) {
      int v = atoi(tok[2]);
      if (v < 1 || v > 14) {
        Serial.println(">> Category must be 1-14");
        return;
      }
      s_catMask &= (uint16_t)~(1u << v);
      markCustom();
    } else {
      Serial.println(">> Use: categories all | only 1,5 | add 5 | remove 5");
      return;
    }
    Serial.print(">> report.categories = ");
    printCatMask();
    Serial.println();
    return;
  }
  if (strcmp(tok[0], "priority") == 0) {
    if (ntok == 1) {
      printItemValue("report.priority");
      return;
    }
    if (strcmp(tok[1], "all") == 0) {
      s_rcMask = RC_ALL;
      markCustom();
    } else if (strcmp(tok[1], "only") == 0 && ntok == 3) {
      uint8_t m = 0;
      char *p = tok[2];
      bool ok = true;
      while (p && *p) {
        /* split one token at a time on commas */
        char word[16];
        int wi = 0;
        while (*p && *p != ',' && wi < 15) word[wi++] = *p++;
        word[wi] = '\0';
        if (*p == ',') p++;
        int v = rcValueOf(word);
        if (v < 0) ok = false;
        else m |= (uint8_t)(1u << v);
      }
      if (!ok || m == 0) {
        Serial.println(">> Use names: maximum, priority, regular, training");
        return;
      }
      s_rcMask = m;
      markCustom();
    } else if ((strcmp(tok[1], "add") == 0 || strcmp(tok[1], "remove") == 0) && ntok == 3) {
      int v = rcValueOf(tok[2]);
      if (v < 0) {
        Serial.println(">> Use names: maximum, priority, regular, training");
        return;
      }
      if (tok[1][0] == 'a') s_rcMask |= (uint8_t)(1u << v);
      else s_rcMask &= (uint8_t)~(1u << v);
      markCustom();
    } else {
      Serial.println(">> Use: priority all | only regular | add training | remove training");
      return;
    }
    printItemValue("report.priority");
    return;
  }
  if (strcmp(tok[0], "dcxclass") == 0) {
    if (ntok == 1) {
      printItemValue("dcx.classes");
      return;
    }
    if (strcmp(tok[1], "all") == 0) {
      s_dcxClassMask = DCX_CLASS_ALL;
      markCustom();
    } else if (strcmp(tok[1], "only") == 0 && ntok == 3) {
      uint8_t m = 0;
      char *p = tok[2];
      bool ok = true;
      while (p && *p) {
        char word[16];
        int wi = 0;
        while (*p && *p != ',' && wi < 15) word[wi++] = *p++;
        word[wi] = '\0';
        if (*p == ',') p++;
        int v = dcxClassOf(word);
        if (v < 0) ok = false;
        else m |= (uint8_t)(1u << v);
      }
      if (!ok || m == 0) {
        Serial.println(">> Use names: lalert, jalert, local, outside");
        return;
      }
      s_dcxClassMask = m;
      markCustom();
    } else if ((strcmp(tok[1], "add") == 0 || strcmp(tok[1], "remove") == 0) && ntok == 3) {
      int v = dcxClassOf(tok[2]);
      if (v < 0) {
        Serial.println(">> Use names: lalert, jalert, local, outside");
        return;
      }
      if (tok[1][0] == 'a') s_dcxClassMask |= (uint8_t)(1u << v);
      else s_dcxClassMask &= (uint8_t)~(1u << v);
      markCustom();
    } else {
      Serial.println(">> Use: dcxclass all | only jalert | add lalert | remove lalert");
      return;
    }
    printItemValue("dcx.classes");
    return;
  }
  if (strcmp(tok[0], "set") == 0) {
    if (ntok < 3) {
      Serial.println(">> Use: set <item> on|off (or a format value)");
      return;
    }
    if (strcmp(tok[1], "report.format") == 0) {
      if (strcmp(tok[2], "full") == 0) s_reportFormat = 2;
      else if (strcmp(tok[2], "first") == 0) s_reportFormat = 1;
      else if (strcmp(tok[2], "off") == 0) s_reportFormat = 0;
      else {
        Serial.println(">> report.format: full | first | off");
        return;
      }
      markCustom();
      printItemValue("report.format");
      return;
    }
    if (strcmp(tok[1], "dcx.format") == 0) {
      if (strcmp(tok[2], "full") == 0) s_dcxFormat = 2;
      else if (strcmp(tok[2], "summary") == 0) s_dcxFormat = 1;
      else if (strcmp(tok[2], "off") == 0) s_dcxFormat = 0;
      else {
        Serial.println(">> dcx.format: summary | full | off");
        return;
      }
      markCustom();
      printItemValue("dcx.format");
      return;
    }
    int id = findItem(tok[1]);
    if (id < 0) {
      Serial.print(">> Unknown item: ");
      Serial.println(tok[1]);
      return;
    }
    if (strcmp(tok[2], "on") == 0) {
      s_item[id] = true;
    } else if (strcmp(tok[2], "off") == 0) {
      s_item[id] = false;
    } else {
      Serial.println(">> Use: set <item> on | off");
      return;
    }
    markCustom();
    printItemValue(tok[1]);
    return;
  }
  if (ntok == 1) {
    /* A bare item name shows its current value */
    printItemValue(tok[0]);
    return;
  }
  Serial.print(">> Unknown command: ");
  Serial.println(tok[0]);
  Serial.println(">> Type 'help' for the command list");
}

static void pollSerialCommand()
{
  static char buf[64];
  static int len = 0;
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      buf[len] = '\0';
      if (len > 0) execCommand(buf);
      len = 0;
    } else if (len < (int)sizeof(buf) - 1) {
      if (c >= 'A' && c <= 'Z') c = c + ('a' - 'A');
      buf[len++] = c;
    }
  }
}

/* ---------------- Output helpers ---------------- */
SpGnss Gnss;

static void printHex(const uint8_t *data, unsigned int len)
{
  for (unsigned int i = 0; i < len; i++) {
    if (data[i] < 16) Serial.print("0");
    Serial.print(data[i], HEX);
  }
  Serial.println();
}

/* Print only the first line of a multi-line report text */
static void printFirstLine(const char *text)
{
  char buf[200];
  int i = 0;
  while (text[i] && text[i] != '\n' && i < (int)sizeof(buf) - 1) {
    buf[i] = text[i];
    i++;
  }
  buf[i] = '\0';
  Serial.println(buf);
}

/* DCX detail groups, added to the summary display.
 * Each group is gated by its own item (dcx.library, dcx.area,
 * dcx.target, dcx.settings, dcx.meta). Labels and conversions
 * follow the library's printAll. */
static void printDcxDetail(DCXMessage &r)
{
  bool isL = (r.dcx_msg_type == DCX_MSG_L_ALERT);
  bool isMT = (r.dcx_msg_type == DCX_MSG_MT_INFO);
  bool isJ = (r.dcx_msg_type == DCX_MSG_J_ALERT);

  if (s_item[IT_DCX_META]) {
    Serial.print("Class: ");
    Serial.print((int)r.dcx_msg_type);
    Serial.print(" (");
    if (isL) Serial.print("L-Alert");
    else if (isJ) Serial.print("J-Alert");
    else if (isMT) Serial.print("Municipality");
    else if (r.dcx_msg_type == DCX_MSG_OUTSIDE_JAPAN) Serial.print("Outside Japan");
    else if (r.dcx_msg_type == DCX_MSG_NULL) Serial.print("Null");
    else Serial.print("Unknown");
    Serial.println(")");
    Serial.print("SD: sdmt=");
    Serial.print((int)r.sdmt);
    Serial.print(" sdm=");
    Serial.println((int)r.sdm);
    Serial.print("Vn - Version number: ");
    Serial.println((int)r.vn_version_number);
    Serial.print("A1-A8 raw: A1=");
    Serial.print((int)r.a1_message_type);
    Serial.print(" A2=");
    Serial.print((int)r.a2_country_region_name);
    Serial.print(" A3=");
    Serial.print((int)r.a3_provider_identifier);
    Serial.print(" A4=");
    Serial.print((int)r.a4_hazard_category_type);
    Serial.print(" A5=");
    Serial.print((int)r.a5_severity);
    Serial.print(" A6=");
    Serial.print((int)r.a6_hazard_onset_week_number);
    Serial.print(" A7=");
    Serial.print((int)r.a7_hazard_onset_ToW);
    Serial.print(" A8=");
    Serial.println((int)r.a8_hazard_duration);
  }

  if (s_item[IT_DCX_LIBRARY]) {
    Serial.print("A9 - Selection of library: ");
    Serial.println((int)r.a9_selection_of_Library);
    Serial.print("A10 - Version of library: ");
    Serial.println((int)r.a10_version_of_library);
    Serial.print("A11 - Guidance to react library: ");
    Serial.print((int)r.a11_guidance_to_react_library);
    if (isL || isMT || isJ) {
      Serial.print(" --> ");
      Serial.print(DCXDecoder::get_guidance_instruction_library_ja(
          r.a11_guidance_to_react_library));
    }
    Serial.println();
  }

  if (s_item[IT_DCX_AREA]) {
    Serial.print("A12 - Ellipse centre latitude: ");
    Serial.print((int)r.a12_ellipse_centre_latitude);
    Serial.print(" --> ");
    Serial.println(DCXDecoder::calc_latitude_deg(
        r.a12_ellipse_centre_latitude));
    Serial.print("A13 - Ellipse centre longitude: ");
    Serial.print((int)r.a13_ellipse_centre_longitude);
    Serial.print(" --> ");
    Serial.println(DCXDecoder::calc_longitude_deg(
        r.a13_ellipse_centre_longitude));
    Serial.print("A14 - Ellipse semi major axis: ");
    Serial.print((int)r.a14_ellipse_semi_major_axis);
    Serial.print(" --> ");
    Serial.println(DCXDecoder::calc_radius(
        r.a14_ellipse_semi_major_axis));
    Serial.print("A15 - Ellipse semi minor axis: ");
    Serial.print((int)r.a15_ellipse_semi_minor_axis);
    Serial.print(" --> ");
    Serial.println(DCXDecoder::calc_radius(
        r.a15_ellipse_semi_minor_axis));
    Serial.print("A16 - Ellipse azimuth angle: ");
    Serial.print((int)r.a16_ellipse_azimuth_angle);
    Serial.print(" --> ");
    Serial.println(DCXDecoder::calc_azimuth_angle(
        r.a16_ellipse_azimuth_angle));
    if (isL || isMT) {
      Serial.print("EX3 - Additional Ellipse Centre Latitude: ");
      Serial.print((int)r.ex3_additional_ellipse_centre_latitude);
      Serial.print(" --> ");
      Serial.println(DCXDecoder::calc_additional_latitude_deg(
          r.ex3_additional_ellipse_centre_latitude));
      Serial.print("EX4 - Additional Ellipse Centre Longitude: ");
      Serial.print((int)r.ex4_additional_ellipse_centre_longitude);
      Serial.print(" --> ");
      Serial.println(DCXDecoder::calc_additional_longitude_deg(
          r.ex4_additional_ellipse_centre_longitude));
      Serial.print("EX5 - Additional Ellipse Semi-Major Axis: ");
      Serial.print((int)r.ex5_additional_ellipse_semi_major_axis);
      Serial.print(" --> ");
      Serial.println(DCXDecoder::calc_radius(
          r.ex5_additional_ellipse_semi_major_axis));
      Serial.print("EX6 - Additional Ellipse Semi-Minor Axis: ");
      Serial.print((int)r.ex6_additional_ellipse_semi_minor_axis);
      Serial.print(" --> ");
      Serial.println(DCXDecoder::calc_radius(
          r.ex6_additional_ellipse_semi_minor_axis));
      Serial.print("EX7 - Additional Ellipse Azimuth: ");
      Serial.print((int)r.ex7_additional_ellipse_azimuth);
      Serial.print(" --> ");
      Serial.println(DCXDecoder::calc_additional_azimuth_angle(
          r.ex7_additional_ellipse_azimuth));
    }
  }

  if (s_item[IT_DCX_TARGET]) {
    if (isL || isMT) {
      Serial.print("Ex1 - Target area code: ");
      Serial.print((int)r.ex1_target_area_code);
      Serial.print(" --> ");
      Serial.println(DCXDecoder::get_municipality_code_japan(
          r.ex1_target_area_code));
    }
    if (isJ) {
      Serial.print("Ex8 - Target area code list type: ");
      Serial.println((int)r.ex8_target_area_code_list_type);
      Serial.print("Ex9 - Target area code list: ");
      if (r.ex8_target_area_code_list_type == 0) {
        /* Prefecture code: two 32-bit masks, MSB first */
        uint32_t code1 = r.ex9_target_area_code_list[0];
        uint32_t code2 = r.ex9_target_area_code_list[1];
        bool sepa = false;
        DCXDecoder::binary_print(Serial, code1, 32);
        DCXDecoder::binary_print(Serial, code2, 32);
        Serial.print(" --> ");
        int i;
        for (i = 0; i < 32; i++) {
          if (code1 & 0x80000000) {
            if (sepa) Serial.print(" ");
            sepa = true;
            Serial.print(DCXDecoder::get_prefecture_name_ja(i));
          }
          code1 <<= 1;
        }
        for (i = 0; i < 32; i++) {
          if (code2 & 0x80000000) {
            if (sepa) Serial.print(" ");
            sepa = true;
            Serial.print(DCXDecoder::get_prefecture_name_ja(i + 32));
          }
          code2 <<= 1;
        }
        Serial.println();
      } else {
        /* Municipality codes: four 16-bit codes */
        uint32_t mcodes[4] = {
          r.ex9_target_area_code_list[0] >> 16,
          r.ex9_target_area_code_list[0] & 0xffff,
          r.ex9_target_area_code_list[1] >> 16,
          r.ex9_target_area_code_list[1] & 0xffff
        };
        bool sepa = false;
        int i;
        for (i = 0; i < 4; i++) {
          if (mcodes[i]) {
            if (sepa) Serial.print(" ");
            sepa = true;
            Serial.print(DCXDecoder::get_municipality_code_japan(mcodes[i]));
          }
        }
        Serial.println();
      }
    }
  }

  if (s_item[IT_DCX_SETTINGS]) {
    Serial.print("A17 - Main subject for specific settings: ");
    Serial.println((int)r.a17_main_subject_for_specific_settings);
    Serial.print("A18 - Specific settings: ");
    Serial.println((int)r.a18_specific_settings);
    if (isL || isMT) {
      Serial.print("EX2 - Evacuate Direction Type: ");
      Serial.print((int)r.ex2_evacuate_direction_type);
      Serial.print(" --> ");
      Serial.println(DCXDecoder::get_evacuate_direction_type_ja(
          r.ex2_evacuate_direction_type));
    }
  }
}

static const char *satTypeName(SpNavData *nav, int i)
{
  if (nav->isSatelliteType(i, QZ_L1S)) return "QZSS-L1S";
  if (nav->isSatelliteType(i, QZ_L1CA)) return "QZSS-L1CA";
  if (nav->isSatelliteType(i, GPS)) return "GPS";
  if (nav->isSatelliteType(i, GLONASS)) return "GLONASS";
  if (nav->isSatelliteType(i, SBAS)) return "SBAS";
  if (nav->isSatelliteType(i, BEIDOU)) return "BeiDou";
  if (nav->isSatelliteType(i, GALILEO)) return "Galileo";
  return "UNKNOWN";
}

static bool anySatItem()
{
  return s_item[IT_SAT_COUNT] || s_item[IT_SAT_SYSTEMS] ||
         s_item[IT_SAT_LIST] || s_item[IT_DCX_EMPTYCOUNT];
}

static void printSatelliteStatus(SpNavData *nav)
{
  Serial.println("-------- Satellite status --------");
  if (s_item[IT_SAT_COUNT]) {
    Serial.print("Visible: ");
    Serial.print((int)nav->numSatellites);
    Serial.print("  Used for position: ");
    Serial.println((int)nav->numSatellitesCalcPos);
  }
  int n = nav->numSatellites;
  if (n > 24) n = 24;
  if (s_item[IT_SAT_SYSTEMS]) {
    int cGps = 0, cGlo = 0, cSbas = 0, cL1ca = 0, cL1s = 0, cBds = 0, cGal = 0;
    int i;
    for (i = 0; i < n; i++) {
      if (nav->isSatelliteType(i, GPS)) cGps++;
      if (nav->isSatelliteType(i, GLONASS)) cGlo++;
      if (nav->isSatelliteType(i, SBAS)) cSbas++;
      if (nav->isSatelliteType(i, QZ_L1CA)) cL1ca++;
      if (nav->isSatelliteType(i, QZ_L1S)) cL1s++;
      if (nav->isSatelliteType(i, BEIDOU)) cBds++;
      if (nav->isSatelliteType(i, GALILEO)) cGal++;
    }
    Serial.print("GPS=");
    Serial.print(cGps);
    Serial.print(" GLONASS=");
    Serial.print(cGlo);
    Serial.print(" SBAS=");
    Serial.print(cSbas);
    Serial.print(" QZSS-L1CA=");
    Serial.print(cL1ca);
    Serial.print(" QZSS-L1S=");
    Serial.print(cL1s);
    Serial.print(" BeiDou=");
    Serial.print(cBds);
    Serial.print(" Galileo=");
    Serial.println(cGal);
    Serial.print("Used systems:");
    unsigned short pst = nav->posSatelliteType;
    bool anyU = false;
    if (pst & GPS) { Serial.print(" GPS"); anyU = true; }
    if (pst & GLONASS) { Serial.print(" GLONASS"); anyU = true; }
    if (pst & SBAS) { Serial.print(" SBAS"); anyU = true; }
    if (pst & QZ_L1CA) { Serial.print(" QZSS-L1CA"); anyU = true; }
    if (pst & QZ_L1S) { Serial.print(" QZSS-L1S"); anyU = true; }
    if (pst & BEIDOU) { Serial.print(" BeiDou"); anyU = true; }
    if (pst & GALILEO) { Serial.print(" Galileo"); anyU = true; }
    if (!anyU) Serial.print(" none");
    Serial.println();
  }
  if (s_item[IT_SAT_LIST]) {
    int i;
    for (i = 0; i < n; i++) {
      SpSatellite *s = &nav->satellite[i];
      Serial.print("  ");
      Serial.print(satTypeName(nav, i));
      Serial.print(" ID=");
      Serial.print((int)s->svid);
      if (s_item[IT_SAT_ELEVATION]) {
        Serial.print(" EL=");
        Serial.print((int)s->elevation);
      }
      if (s_item[IT_SAT_AZIMUTH]) {
        Serial.print(" AZ=");
        Serial.print((int)s->azimuth);
      }
      if (s_item[IT_SAT_SIGNAL]) {
        Serial.print(" C/N=");
        Serial.print(s->sigLevel, 1);
        Serial.print(" dBHz");
      }
      Serial.println();
    }
  }
  if (s_item[IT_DCX_EMPTYCOUNT]) {
    Serial.print("DCX empty messages received: ");
    Serial.println(s_dcxEmptyCount);
  }
}

static bool anyPosItem()
{
  int i;
  for (i = IT_POS_TIME; i <= IT_POS_DATA; i++) {
    if (s_item[i]) return true;
  }
  return false;
}

static void printPosition(SpNavData *nav)
{
  Serial.println("-------- Position --------");
  if (s_item[IT_POS_TIME]) {
    RtcTime t = RTC.getTime();
    char buf[40];
    snprintf(buf, sizeof(buf), "Time: %04d/%02d/%02d %02d:%02d:%02d JST",
             t.year(), t.month(), t.day(),
             t.hour(), t.minute(), t.second());
    Serial.println(buf);
  }
  if (s_item[IT_POS_DATA]) {
    Serial.print("Position data: ");
    Serial.println(nav->posDataExist ? "yes" : "no");
  }
  if (nav->posDataExist && nav->posFixMode >= 2) {
    if (s_item[IT_POS_FIX]) {
      static const char *typeName[5] = {
        "Invalid", "GNSS", "Reserved", "User set", "Previous"
      };
      Serial.print("Fix: ");
      Serial.print(nav->posFixMode == 3 ? "3D" : "2D");
      Serial.print("  Type: ");
      Serial.println(typeName[nav->type <= 4 ? nav->type : 0]);
    }
    if (s_item[IT_POS_USED]) {
      Serial.print("Used: ");
      Serial.println((int)nav->numSatellitesCalcPos);
    }
    if (s_item[IT_POS_LATITUDE]) {
      Serial.print("Lat: ");
      Serial.println(nav->latitude, 6);
    }
    if (s_item[IT_POS_LONGITUDE]) {
      Serial.print("Lon: ");
      Serial.println(nav->longitude, 6);
    }
    if (s_item[IT_POS_ALTITUDE]) {
      Serial.print("Alt: ");
      Serial.print(nav->altitude, 1);
      Serial.println(" m");
    }
    if (s_item[IT_POS_VELOCITY]) {
      Serial.print("Vel: ");
      Serial.print(nav->velocity, 1);
      Serial.println(" m/s");
    }
    if (s_item[IT_POS_DIRECTION]) {
      Serial.print("Dir: ");
      Serial.print(nav->direction, 1);
      Serial.println(" deg");
    }
    if (s_item[IT_POS_DOP]) {
      Serial.print("PDOP: ");
      Serial.print(nav->pdop, 2);
      Serial.print("  HDOP: ");
      Serial.print(nav->hdop, 2);
      Serial.print("  VDOP: ");
      Serial.print(nav->vdop, 2);
      Serial.print("  TDOP: ");
      Serial.println(nav->tdop, 2);
    }
  } else {
    Serial.print("No fix (visible satellites: ");
    Serial.print((int)nav->numSatellites);
    Serial.println(")");
  }
}

void setup()
{
  /* Initialize Serial */
  Serial.begin(115200);

  Serial.println("DcReportMonitorAll (DC Report / DCX Receiver)");
  Serial.println("Display mode: recommended - 'help' for commands, 'show' for settings");

  /* Initialize GNSS */
  if (Gnss.begin()) {
    Serial.println("begin error!");
  }

  /* select satellite system */
  Gnss.select(GPS);
  Gnss.select(QZ_L1CA);
  Gnss.select(QZ_L1S);

  /* set interval */
  Gnss.setInterval(1);

  /* Start GNSS */
  if (Gnss.start(COLD_START)) {
    Serial.println("start error!");
  }
}

void loop()
{
  /* Display commands from the Serial Monitor */
  pollSerialCommand();

  /* Check update. */
  if (Gnss.waitUpdate(1000)) {
    RtcTime now;

    // LED Heartbeat
    static int toggle = 0;
    if (toggle++ % 2) {
      ledOn(LED0);
    } else {
      ledOff(LED0);
    }

    // Get time and position data with QZQSM
    SpNavData NavData;
    Gnss.getNavData(&NavData);

    // Periodic blocks at the satellites period (0 = off),
    // each item gated
    if (s_periodSec > 0 && ++s_tickCounter >= s_periodSec) {
      s_tickCounter = 0;
      if (anySatItem()) printSatelliteStatus(&NavData);
      if (anyPosItem()) printPosition(&NavData);
    }

    // On-demand block ("status" command), same gating
    if (s_statusRequested) {
      s_statusRequested = false;
      bool printed = false;
      if (anySatItem()) { printSatelliteStatus(&NavData); printed = true; }
      if (anyPosItem()) { printPosition(&NavData); printed = true; }
      if (!printed) {
        Serial.println(">> status: no satellite/position items are on (see 'show')");
      }
    }

    // Check if UTC time is acquired
    SpGnssTime *time = &NavData.time;
    if (time->year >= 2000) {

      // Time fix
      ledOn(LED1);

      // Convert SpGnssTime to RtcTime
      RtcTime gps(time->year, time->month, time->day,
                  time->hour, time->minute, time->sec, time->usec * 1000);
      // Convert UTC to JST time
      gps += 9 * 60 * 60;

      // Compare with the current time
      now = RTC.getTime();
      int diff = now - gps;
      if (abs(diff) >= 1) {
        RTC.setTime(gps);
      }
    }

    // Get DC report
    void *handle = Gnss.getDCReport();
    if (handle) {

      // begin DC report
      ledOn(LED3);

      struct cxd56_gnss_dcreport_data_s *dcr =
          (struct cxd56_gnss_dcreport_data_s *)handle;
      uint8_t *sf = dcr->sf;
      unsigned int sfLen = sizeof(dcr->sf);

      /* Message type: bits 9-14 of the 250-bit message
         (byte 0 = preamble 8 bits, byte 1 upper 6 bits = MT) */
      int mt = (sf[1] >> 2) & 0x3F;

      /* Count QZSS L1S satellites currently visible */
      int qzL1s = 0;
      int si;
      int snum = NavData.numSatellites;
      if (snum > 24) snum = 24;
      for (si = 0; si < snum; si++) {
        if (NavData.isSatelliteType(si, QZ_L1S)) qzL1s++;
      }

      now = RTC.getTime();
      char datetime[96];
      snprintf(datetime, sizeof(datetime),
               "================================ %04d/%02d/%02d %02d:%02d:%02d MT=%d SAT=%d QZ_L1S=%d",
               now.year(), now.month(), now.day(),
               now.hour(), now.minute(), now.second(), mt,
               (int)NavData.numSatellites, qzL1s);

      if (mt == 43) {
        /* JMA-DC Report: original QZQSM path */
        QZQSM report;
        report.SetYear(now.year());
        report.Decode(sf);

        int reported = 0;
        int i;
        for (i = 0; i < HISTORY_COUNT; i++) {
          if (s_history[i] == report) {
            reported = 1;
            break;
          }
        }

        if (!reported) {
          /* New report: print per format, category filter and
             classification filter; always record.
             Dc = message bits 18-21, Rc = bits 15-17. */
          int dc = (sf[2] >> 3) & 0x0F;
          int rc = ((sf[1] & 0x03) << 1) | (sf[2] >> 7);
          if (s_reportFormat != 0 && (s_catMask & (1u << dc)) &&
              (s_rcMask & (1u << rc))) {
            if (s_item[IT_HEADER]) {
              Serial.print(datetime);
              Serial.print(" RC=");
              Serial.print(rc);
              Serial.print(" DC=");
              Serial.println(dc);
            }
            if (s_reportFormat == 2) {
              Serial.println(report.GetReport());
            } else {
              printFirstLine(report.GetReport());
            }
          }
          s_history[s_index] = report;
          s_index++;
          s_index %= HISTORY_COUNT;
        }
      } else if (mt == 44) {
        /* DCX */
        DCXDecoder dec;
        dec.decode(sf);

        if (dec.r.dcx_msg_type == DCX_MSG_NULL) {
          /* Empty message: always counted, printed only if set */
          s_dcxEmptyCount++;
          if (s_item[IT_DCX_EMPTY] && s_dcxFormat != 0) {
            if (s_item[IT_HEADER]) Serial.println(datetime);
            dec.printSummary(Serial, dec.r);
          }
        } else {
          /* Real alert: dedupe by content bytes 1..25, so the
             rotating first byte and the counter/CRC tail do not
             defeat the check */
          int reported = 0;
          int i;
          for (i = 0; i < HISTORY_COUNT; i++) {
            if (memcmp(s_raw_history[i], sf + 1, DCX_KEY_LEN) == 0) {
              reported = 1;
              break;
            }
          }

          if (!reported) {
            /* New DCX alert: print per format and class filter,
               always record */
            if (s_dcxFormat != 0 &&
                (s_dcxClassMask & (1u << dec.r.dcx_msg_type))) {
              if (s_item[IT_HEADER]) Serial.println(datetime);
              if (s_item[IT_RAW]) {
                Serial.print("RAW: ");
                printHex(sf, sfLen);
              }
              if (s_dcxFormat == 2) {
                dec.printAll(Serial, dec.r);
              } else {
                dec.printSummary(Serial, dec.r);
                printDcxDetail(dec.r);
              }
            }
            memcpy(s_raw_history[s_raw_index], sf + 1, DCX_KEY_LEN);
            s_raw_index++;
            s_raw_index %= HISTORY_COUNT;
          }
        }
      } else {
        /* Any other message type: raw dump only, gated by raw */
        if (s_item[IT_RAW]) {
          if (s_item[IT_HEADER]) Serial.println(datetime);
          Serial.print("RAW: ");
          printHex(sf, sfLen);
        }
      }

      // end DC report
      ledOff(LED3);
    }
  }
}
