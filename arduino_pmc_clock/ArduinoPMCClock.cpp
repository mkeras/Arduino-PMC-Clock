#include "ArduinoPMCClock.h"
#include <Arduino_MachineControl.h>

// Use the machinecontrol namespace
using namespace machinecontrol;

ArduinoClock::ArduinoClock(UDP& udp)
    : m_NTP_Client(NTPClient(udp)),
      m_NTPSyncRate(14400000), // sync with NTP Server every 4 hours
      m_RTCSyncRate(600000)   // sync millis() function with RTC every 10 minutes
{
}


ArduinoClock::ArduinoClock(UDP& udp, const char* timeServer)
    : m_NTP_Client(NTPClient(udp, timeServer)),
      m_NTPSyncRate(14400000), // sync with NTP Server every 4 hours
      m_RTCSyncRate(600000)   // sync millis() function with RTC every 10 minutes
{
}


ArduinoClock::ArduinoClock(UDP& udp, unsigned long ntpSyncRate, unsigned long rtcSyncRate)
    : m_NTP_Client(NTPClient(udp)),
      m_NTPSyncRate(ntpSyncRate), // sync with NTP Server according to parameter
      m_RTCSyncRate(rtcSyncRate)   // sync millis() function with RTC according to parameter
{
}


ArduinoClock::ArduinoClock(UDP& udp, const char* timeServer, unsigned long ntpSyncRate, unsigned long rtcSyncRate)
    : m_NTP_Client(NTPClient(udp, timeServer)),
      m_NTPSyncRate(ntpSyncRate), // sync with NTP Server according to parameter
      m_RTCSyncRate(rtcSyncRate)   // sync millis() function with RTC according to parameter
{
}

void ArduinoClock::ntpClientBegin() {
  rtc_controller.begin();
  m_NTP_Client.begin();
  m_udp_active = true;
}

void ArduinoClock::ntpClientEnd() {
  m_NTP_Client.end();
  m_udp_active = false;
}


bool ArduinoClock::m_millisRolledOver() {
  return (millis() < m_millisAtRTCSync);
}


uint64_t ArduinoClock::getEpochMillis() {
  if (m_millisRolledOver()) {
    // If millis() has rolled over, re sync with the RTC before returning
    syncRTC();
  }
  return m_epochMillisAtZero + millis();
}


bool ArduinoClock::syncRTC() {
  //Serial.println("SYNCING RTC");
  m_millisAtRTCSync = millis();
  m_lastRTCSync = uint64_t(rtc_controller.getEpoch()) * 1000;
  if (m_lastRTCSync < m_millisAtRTCSync) {
    m_epochMillisAtZero = uint64_t(0);
    return false;
  }
  m_epochMillisAtZero = m_lastRTCSync - m_millisAtRTCSync;
  m_initialRTCSyncDone = true;
  m_lastRTCSync = getEpochMillis();
  return true;
}


bool ArduinoClock::syncNTP() {
  //Serial.println("SYNCING NTP SERVER");
  if (!m_udp_active) {
    //Serial.println("ABORTING SYNC, UDP INACTIVE");
    return false;
  }
  if (!m_NTP_Client.forceUpdate()) {
    return false;
  }
  rtc_controller.setEpoch(m_NTP_Client.getEpochTime());
  syncRTC();
  m_initialNTPSyncDone = true;
  m_lastNTPSync = getEpochMillis();
  return true;
}


void ArduinoClock::tick() {
    if (!m_initialNTPSyncDone) {
      if (!syncNTP()) {
        return;
      }
    }

    if (!m_initialRTCSyncDone) {
      if (!syncRTC()) {
        return;  
      };
    }

    uint64_t currentTimestamp = getEpochMillis();

    if (currentTimestamp > m_lastNTPSync + m_NTPSyncRate) {
      // Sync NTP
      if (!syncNTP()) {
        m_lastNTPSync += 60000; // Retry in 1 minute
      }
      return;
    }
    
    if (currentTimestamp > m_lastRTCSync + m_RTCSyncRate) {
      if (!syncRTC()) {
        m_lastRTCSync += 60000; // Retry in 1 minute
      }
    }
}

bool ArduinoClock::forceSyncNTP() {
  return syncNTP();
}


bool ArduinoClock::forceSyncRTC() {
  return syncRTC();
}

