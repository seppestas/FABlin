/**
 tools.cpp

 Tools configuration handler
*/

#include <stdint.h>
#include <Wire.h>

#if defined(SMART_COMM)
#include <SmartComm.h>
#endif

#include "Marlin.h"
#include "Configuration_heads.h"
#include "tools.h"

/**
 * tools.load
 *
 * Load a factory tool configuration into one of the user definable slots.
 */
void tools_s::load (uint8_t tool, uint8_t id)
{
   memcpy(&magazine[tool], &factory[id], sizeof factory[id]);
   /*magazine[tool].mode      = factory[id].mode;
   magazine[tool].extruders = factory[id].extruders;
   magazine[tool].heaters   = factory[id].heaters;
   magazine[tool].serial    = factory[id].serial;*/
}

/**
 * tools.define
 *
 * Define a logical tool that can be selected through `T` address
 *
 * A tool can be a physical addon (head) to be harnessed, or a different
 * configuration of equipped hardware.
 *
 * Max tools number is statically set to 3 at present.
 *
 */
void tools_s::define(uint8_t tool, int8_t drive, unsigned int heater, uint8_t serial)
{
   tool_extruder_mapping[tool] = drive;
   tool_heater_mapping[tool] = heater;
   tool_twi_support[tool] = serial != 0;

   magazine[tool].extruders = drive >= 0? 1 << drive : 0;
   magazine[tool].serial = serial;
   magazine[tool].heaters = 1 << heater;
}

/**
 * tools.change
 *
 * Loads the referenced tool and activate all of its defined hardware and
 * facilities.
 *
 */
uint8_t tools_s::change (uint8_t tool)
{
   // Load first selected extruder into map
   tool_extruder_mapping[tool] = -1;
   for (uint8_t e = 0; e < EXTRUDERS; e++) {
      if (magazine[tool].extruders & (1 << e)) {
         tool_extruder_mapping[tool] = e;
         break;
      }
   }

   // Load serial communication setting
   tool_twi_support[tool] = magazine[tool].serial != 0;

   // Set globals
   active_extruder = tool_extruder_mapping[tool];
   head_is_dummy = !tool_twi_support[tool];
   installed_head = &magazine[tool];

#ifdef SMART_COMM
   if (installed_head_id <= FAB_HEADS_laser_ID)
   {
#endif
      // For legacy heads we do not rely on SmartComm module
      if (head_is_dummy) {
         TWCR = 0;
      } else {
         Wire.begin();
      }
#ifdef SMART_COMM
   }
   else
   {
      if (head_is_dummy) {
         SmartHead.end();
      } else {
         SmartHead.begin();
         // SmartComm configuration should have already been done

         // WARNING: possibly breaking change for custom heads with id > 99
         // dependant on legacy behaviour. That is, FABlin comm support shall
         // be explicitely configured for them to continue to work.
      }
   }
#endif

   active_tool = tool;
   return active_extruder;
}

tools_s tools;
