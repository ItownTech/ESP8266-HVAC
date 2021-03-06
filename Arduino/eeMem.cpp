#include "eeMem.h"
#include <EEPROM.h>

eeSet ee = { sizeof(eeSet), 0xAAAA,
  "",  // saved SSID (place your SSID and password here)
  "", // router password
  {840, 870},   // 87.0, 90.0 default cool temps
  {760, 780},   // default heat temps
  {20, 8},     // cycleThresh (cool 2.0, heat 1.0)
  0,            // Mode
  33,           // heatThresh (under 33F is gas)
  60*5,         // 5 mins minimum for a cycle
  60*30,        // 30 minutes maximun for a cycle
  60*8,         // idleMin 8 minutes minimum between cycles
  0,            // filterMinutes
  {60, 120},    // fanPostDelay {cool, heat}
  {60, 60},     // fanPreTime {cool, heat}
  60*10,        // 10 mins default for override
  0,            // heatMode
  -5,           // timeZone
  0,            // temp reading offset adjust
  0,            // humidMode
  {450, 550},   // rhLevel 45.0%, 55%
  {40, -40},    // awayDelta cool, heat
  9*60,         // awayTime (minutes)
  30*60,        // fanCycleTime 30 mins
  192 | (168<<8) | (105<<24), // hostIp 192.168.0.105
  85,           // host port
  "41042",      // zipCode
  "password",  // password for controlling thermostat
  false,        // bLock
  false,        // bNotLocal
  1540,         // price per KWH in cents * 10000 (0.154)
  1243,         // nat gas cost per 1000 cubic feet in 10th of cents * 1000 ($1.243)
  46,           // forecast range for in mapping to out mix/max (5, but 3 can be better)
  46,           // forecast range for display (5 of 7 day max)
  {0},       // fCostE month costs
  {0},       // fCostG
  {0},          // fCostDay day costs
  920,          // cubic feet per minute * 1000 of furnace (0.92)
  4500,         // compressorWatts
  350,          // fanWatts
  100,          // furnaceWatts
  150,          // humidWatts
  120,          // furnacePost (furnace internal fan timer)
  0,
  0,
  {0},
  200         // set to 20 deg differential cooling limit
};

eeMem::eeMem()
{
  EEPROM.begin(1024);

  uint8_t data[sizeof(eeSet)];
  uint16_t *pwTemp = (uint16_t *)data;

  int addr = 0;
  for(int i = 0; i < sizeof(eeSet); i++, addr++)
  {
    data[i] = EEPROM.read( addr );
  }

  if(pwTemp[0] != sizeof(eeSet)) return; // revert to defaults if struct size changes
  uint16_t sum = pwTemp[1];
  pwTemp[1] = 0;
  pwTemp[1] = Fletcher16(data, sizeof(eeSet) );
  if(pwTemp[1] != sum) return; // revert to defaults if sum fails
  memcpy(&ee, data, sizeof(eeSet) );
}

void eeMem::update() // write the settings if changed
{
  if(check() == false)
    return; // Nothing has changed?

  uint16_t addr = 0;
  uint8_t *pData = (uint8_t *)&ee;
  for(int i = 0; i < sizeof(eeSet); i++, addr++)
  {
    EEPROM.write(addr, pData[i] );
  }
  EEPROM.commit();
}

bool eeMem::check()
{
  uint16_t old_sum = ee.sum;
  ee.sum = 0;
  ee.sum = Fletcher16((uint8_t*)&ee, sizeof(eeSet));

  return (old_sum == ee.sum) ? false:true;
}

uint16_t eeMem::Fletcher16( uint8_t* data, int count)
{
   uint16_t sum1 = 0;
   uint16_t sum2 = 0;

   for( int index = 0; index < count; ++index )
   {
      sum1 = (sum1 + data[index]) % 255;
      sum2 = (sum2 + sum1) % 255;
   }

   return (sum2 << 8) | sum1;
}
