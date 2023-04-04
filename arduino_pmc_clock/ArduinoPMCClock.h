#ifndef Arduino_PMC_Clock_H
#define Arduino_PMC_Clock_H

#include <Udp.h>
#include <NTPClient.h>

class ArduinoClock {
public:
    // Constructors and destructor
    ArduinoClock(UDP& udp);
    ArduinoClock(UDP& udp, const char* timeServer);
    ArduinoClock(UDP& udp, unsigned long ntpSyncRate, unsigned long rtcSyncRate);
    ArduinoClock(UDP& udp, const char* timeServer, unsigned long ntpSyncRate, unsigned long rtcSyncRate);
    //~ArduinoClock();

    // Public methods
    uint64_t getEpochMillis();
    void tick();
    void ntpClientBegin();
    void ntpClientEnd();
    //NTPClient getNTPClient();

    //bool forceSyncNTP();
    //bool forceSyncRTC();

private:
    // Private members
    uint64_t m_lastRTCSync = uint64_t(0); // Epoch millis timestamp of last RTC Sync
    uint32_t m_millisAtRTCSync = uint32_t(0); // actual millis at RTC sync, for determining if millis have rolled over
    uint64_t m_epochMillisAtZero = uint64_t(0); // Calculated Epoch millis timestamp when millis() was at 0
    uint64_t m_lastNTPSync = uint64_t(0); // Epoch millis timestamp of last NTP Sync
    unsigned long m_NTPSyncRate; // Rate to sync RTC with NTP Server in milliseconds
    unsigned long m_RTCSyncRate; // Rate to sync millis() with RTC in milliseconds
    bool m_initialNTPSyncDone = false; // flag to determine if initial NTP sync has been done
    bool m_initialRTCSyncDone = false; // flag to determine if initial RTC sync has been done
    bool m_udp_active = false;
    //const char* m_timeServer; // server name for syncing NTP
    NTPClient m_NTP_Client; // NTP client object

    // Private methods
    bool syncNTP();
    bool syncRTC();
    bool m_millisRolledOver();
    uint64_t m_getEpochMillisRTC();
};


#endif // Arduino_PMC_Clock_H