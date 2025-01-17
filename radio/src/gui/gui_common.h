/*
 * Copyright (C) OpenTX
 *
 * Based on code named
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _GUI_COMMON_H_
#define _GUI_COMMON_H_

#include "lcd.h"
#include "keys.h"
#include "telemetry/telemetry_sensors.h"

#define READONLY_ROW                   ((uint8_t)-1)
#define TITLE_ROW                      READONLY_ROW
#define LABEL(...)                     READONLY_ROW
#define HIDDEN_ROW                     ((uint8_t)-2)

#if defined(ROTARY_ENCODER_NAVIGATION)
  #define CASE_EVT_ROTARY_LEFT         case EVT_ROTARY_LEFT:
  #define CASE_EVT_ROTARY_RIGHT        case EVT_ROTARY_RIGHT:
#else
  #define CASE_EVT_ROTARY_LEFT
  #define CASE_EVT_ROTARY_RIGHT
#endif

#if defined(NAVIGATION_X7) || defined(NAVIGATION_X9D) || defined(NAVIGATION_HORUS)
inline uint8_t MENU_FIRST_LINE_EDIT(const uint8_t * horTab, uint8_t horTabMax)
{
  if (horTab) {
    uint8_t result = 0;
    while (result < horTabMax && horTab[result] >= HIDDEN_ROW)
      ++result;
    return result;
  }
  else {
    return 0;
  }
}
#endif

typedef bool (*IsValueAvailable)(int);

int circularIncDec(int current, int inc, int min, int max, IsValueAvailable isValueAvailable=nullptr);
int getFirstAvailable(int min, int max, IsValueAvailable isValueAvailable);

bool isTrimModeAvailable(int mode);
bool isInputAvailable(int input);
bool isInputSourceAvailable(int source);
bool isThrottleSourceAvailable(int source);
bool isLogicalSwitchFunctionAvailable(int function);
bool isLogicalSwitchAvailable(int index);
bool isAssignableFunctionAvailable(int function);
bool isSourceAvailable(int source);
bool isSourceAvailableInGlobalFunctions(int source);
bool isSourceAvailableInCustomSwitches(int source);
bool isSourceAvailableInResetSpecialFunction(int index);
bool isSourceAvailableInGlobalResetSpecialFunction(int index);
bool isSwitchAvailableInLogicalSwitches(int swtch);
bool isSwitchAvailableInCustomFunctions(int swtch);
bool isSwitchAvailableInMixes(int swtch);
bool isSwitchAvailableInTimers(int swtch);
bool isR9MModeAvailable(int mode);
bool isPxx2IsrmChannelsCountAllowed(int channels);
bool isModuleUsingSport(uint8_t moduleBay, uint8_t moduleType);
bool isTrainerUsingModuleBay();
bool isExternalModuleAvailable(int moduleType);
bool isInternalModuleAvailable(int moduleType);
bool isRfProtocolAvailable(int protocol);
bool isTelemetryProtocolAvailable(int protocol);
bool isTrainerModeAvailable(int mode);

bool isSensorUnit(int sensor, uint8_t unit);
bool isCellsSensor(int sensor);
bool isGPSSensor(int sensor);
bool isAltSensor(int sensor);
bool isVoltsSensor(int sensor);
bool isCurrentSensor(int sensor);
bool isTelemetryFieldAvailable(int index);
bool isTelemetryFieldComparisonAvailable(int index);
bool isSensorAvailable(int sensor);
bool isRssiSensorAvailable(int sensor);

bool modelHasNotes();

#if defined(COLORLCD)
bool isSwitch2POSWarningStateAvailable(int state);
#endif

#if defined(GUI)
#define IS_INSTANT_TRIM_ALLOWED()      (IS_MAIN_VIEW_DISPLAYED() || IS_TELEMETRY_VIEW_DISPLAYED() || IS_OTHER_VIEW_DISPLAYED())
#else
#define IS_INSTANT_TRIM_ALLOWED()      true
#endif

#if defined(FLIGHT_MODES)
void drawFlightMode(coord_t x, coord_t y, int8_t idx, LcdFlags att=0);
#endif

swsrc_t checkIncDecMovedSwitch(swsrc_t val);

void drawCurveRef(coord_t x, coord_t y, CurveRef & curve, LcdFlags flags=0);
void drawDate(coord_t x, coord_t y, TelemetryItem & telemetryItem, LcdFlags flags=0);
void drawTelemScreenDate(coord_t x, coord_t y, source_t sensor, LcdFlags flags=0);
void drawGPSPosition(coord_t x, coord_t y, int32_t longitude, int32_t latitude, LcdFlags flags=0);
void drawGPSSensorValue(coord_t x, coord_t y, TelemetryItem & telemetryItem, LcdFlags flags=0);
void drawSensorCustomValue(coord_t x, coord_t y, uint8_t sensor, int32_t value, LcdFlags flags=0);
void drawSourceCustomValue(coord_t x, coord_t y, source_t channel, int32_t val, LcdFlags flags=0);
void drawSourceValue(coord_t x, coord_t y, source_t channel, LcdFlags flags=0);

void drawCurve(coord_t offset=0);

#if defined(COLORLCD)
void drawStringWithIndex(coord_t x, coord_t y, const char * str, int idx, LcdFlags flags=0, const char * prefix=nullptr, const char * suffix=nullptr);
uint8_t editCheckBox(uint8_t value, coord_t x, coord_t y, LcdFlags flags, event_t event);
swsrc_t editSwitch(coord_t x, coord_t y, swsrc_t value, LcdFlags flags, event_t event);
void drawFatalErrorScreen(const char * message);
void runFatalErrorScreen(const char * message);
#endif

#define IF_INTERNAL_MODULE_ON(x)                  (IS_INTERNAL_MODULE_ENABLED() ? (uint8_t)(x) : HIDDEN_ROW)
#define IF_MODULE_ON(moduleIndex, x)              (IS_MODULE_ENABLED(moduleIndex) ? (uint8_t)(x) : HIDDEN_ROW)

// model_setup Defines that are used in all uis in the same way

inline uint8_t MODULE_BIND_ROWS(int moduleIdx)
{
  if (isModuleXJTD8(moduleIdx) || isModuleSBUS(moduleIdx))
    return 1;

  if (isModulePPM(moduleIdx) || isModulePXX1(moduleIdx) || isModulePXX2(moduleIdx) || isModuleDSM2(moduleIdx) || isModuleMultimodule(moduleIdx))
    return 2;
  else
    return HIDDEN_ROW;
}

inline uint8_t MODULE_CHANNELS_ROWS(int moduleIdx)
{
  if (!IS_MODULE_ENABLED(moduleIdx))
    return HIDDEN_ROW;

  if (isModuleDSM2(moduleIdx) || isModuleCrossfire(moduleIdx) || isModuleSBUS(moduleIdx) || (isModuleMultimodule(moduleIdx) && g_model.moduleData[moduleIdx].getMultiProtocol(true) != MODULE_SUBTYPE_MULTI_DSM2))
    return 0;
  else
    return 1;
}


#if defined(MULTIMODULE)
// When using packed, the pointer in here end up not being aligned, which clang and gcc complain about
// Keep the order of the fields that the so that the size stays small
struct mm_protocol_definition {
    uint8_t protocol;
    uint8_t maxSubtype;
    bool failsafe;
    const char *subTypeString;
    const char *optionsstr;
};
const mm_protocol_definition *getMultiProtocolDefinition (uint8_t protocol);
#define MULTIMODULE_STATUS_ROWS(moduleIdx)      isModuleMultimodule(moduleIdx) ? TITLE_ROW : HIDDEN_ROW, (isModuleMultimodule(moduleIdx) && getMultiSyncStatus(moduleIdx).isValid()) ? TITLE_ROW : HIDDEN_ROW,
#define MULTIMODULE_MODULE_ROWS(moduleIdx)      isModuleMultimodule(moduleIdx) ? (uint8_t) 0 : HIDDEN_ROW,
#define MULTIMODULE_MODE_ROWS(moduleIdx)        (g_model.moduleData[moduleIdx].multi.customProto) ? (uint8_t) 3 :MULTIMODULE_HAS_SUBTYPE(g_model.moduleData[moduleIdx].getMultiProtocol(true)) ? (uint8_t)2 : (uint8_t)1

inline bool isMultiProtocolSelectable(int protocol)
{
  return protocol != MODULE_SUBTYPE_MULTI_SCANNER;
}

inline bool MULTIMODULE_HAS_SUBTYPE(uint8_t moduleIdx)
{
  return getMultiProtocolDefinition(moduleIdx)->maxSubtype > 0;
}
inline uint8_t MULTIMODULE_RFPROTO_COLUMNS(uint8_t moduleIdx)
{
#if LCD_W < 212
  return (g_model.moduleData[moduleIdx].multi.customProto ? (uint8_t) 1 : MULTIMODULE_HAS_SUBTYPE(g_model.moduleData[moduleIdx].getMultiProtocol(true)) ? (uint8_t) 0 : HIDDEN_ROW);
#else
  return (g_model.moduleData[moduleIdx].multi.customProto ? (uint8_t) 2 : MULTIMODULE_HAS_SUBTYPE(g_model.moduleData[moduleIdx].getMultiProtocol(true)) ? (uint8_t) 1 : 0);
#endif
}
#define MULTIMODULE_SUBTYPE_ROWS(moduleIdx)     isModuleMultimodule(moduleIdx) ? MULTIMODULE_RFPROTO_COLUMNS(moduleIdx) : HIDDEN_ROW,
#define MULTIMODULE_HASOPTIONS(moduleIdx)       (getMultiProtocolDefinition(moduleIdx)->optionsstr != nullptr)
#define MULTIMODULE_OPTIONS_ROW(moduleIdx)      (isModuleMultimodule(moduleIdx) && MULTIMODULE_HASOPTIONS(g_model.moduleData[moduleIdx].getMultiProtocol(true))) ? (uint8_t) 0: HIDDEN_ROW

#else
#define MULTIMODULE_STATUS_ROWS(moduleIdx)
#define MULTIMODULE_MODULE_ROWS(moduleIdx)
#define MULTIMODULE_SUBTYPE_ROWS(moduleIdx)
#define MULTIMODULE_MODE_ROWS(moduleIdx)        (uint8_t)0
#define MULTIMODULE_OPTIONS_ROW(moduleIdx)      HIDDEN_ROW
#endif

#define FAILSAFE_ROWS(moduleIdx)               isModuleFailsafeAvailable(moduleIdx) ? (g_model.moduleData[moduleIdx].failsafeMode==FAILSAFE_CUSTOM ? (uint8_t)1 : (uint8_t)0) : HIDDEN_ROW
#define MODULE_OPTION_ROW(moduleIdx)           (isModuleR9MNonAccess(moduleIdx) || isModuleSBUS(moduleIdx)  ? TITLE_ROW : MULTIMODULE_OPTIONS_ROW(moduleIdx))
#define MODULE_POWER_ROW(moduleIdx)            (isModuleMultimodule(moduleIdx) || isModuleR9MNonAccess(moduleIdx)) ? (isModuleR9MLiteNonPro(moduleIdx) ? (isModuleR9M_FCC_VARIANT(moduleIdx) ? READONLY_ROW : (uint8_t)0) : (uint8_t)0) : HIDDEN_ROW

void editStickHardwareSettings(coord_t x, coord_t y, int idx, event_t event, LcdFlags flags);

const char * writeScreenshot();

#endif // _GUI_COMMON_H_
