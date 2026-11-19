// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "Parser.h"

#define SYSTEM_CONFIG_PARA_SIZE 16

class SystemConfigParaParser : public Parser {
public:
    SystemConfigParaParser();
    void clearBuffer();
    void appendFragment(const uint8_t offset, const uint8_t* payload, const uint8_t len);

    float getLimitPercent() const;
    void setLimitPercent(const float value);

    void setLastLimitCommandSuccess(const LastCommandSuccess status);
    LastCommandSuccess getLastLimitCommandSuccess() const;
    uint32_t getLastUpdateCommand() const;
    void setLastUpdateCommand(const uint32_t lastUpdate);

    void setLastLimitRequestSuccess(const LastCommandSuccess status);
    LastCommandSuccess getLastLimitRequestSuccess() const;
    uint32_t getLastUpdateRequest() const;
    void setLastUpdateRequest(const uint32_t lastUpdate);

    void setLastReactivePowerCommandSuccess(const LastCommandSuccess status);
    LastCommandSuccess getLastReactivePowerCommandSuccess() const;
    uint32_t getLastReactivePowerUpdateCommand() const;
    void setLastReactivePowerUpdateCommand(const uint32_t lastUpdate);

    // Last reactive power value commanded by the DTU (relative %, no hardware readback exists)
    float getReactivePowerPercent() const;
    void setReactivePowerPercent(const float value);

    void setLastPowerFactorCommandSuccess(const LastCommandSuccess status);
    LastCommandSuccess getLastPowerFactorCommandSuccess() const;
    uint32_t getLastPowerFactorUpdateCommand() const;
    void setLastPowerFactorUpdateCommand(const uint32_t lastUpdate);

    // Last power factor value commanded by the DTU (0..1, no hardware readback exists)
    float getPowerFactor() const;
    void setPowerFactor(const float value);

    // Returns 1 based amount of expected bytes of data
    uint8_t getExpectedByteCount() const;

private:
    uint8_t _payload[SYSTEM_CONFIG_PARA_SIZE];
    uint8_t _payloadLength;

    LastCommandSuccess _lastLimitCommandSuccess = CMD_OK; // Set to OK because we have to assume nothing is done at startup
    LastCommandSuccess _lastLimitRequestSuccess = CMD_NOK; // Set to NOK to fetch at startup

    LastCommandSuccess _lastReactivePowerCommandSuccess = CMD_OK;
    uint32_t _lastReactivePowerUpdateCommand = 0;
    float _reactivePowerPercent = 0;

    LastCommandSuccess _lastPowerFactorCommandSuccess = CMD_OK;
    uint32_t _lastPowerFactorUpdateCommand = 0;
    float _powerFactor = 0;

    uint32_t _lastUpdateCommand = 0;
    uint32_t _lastUpdateRequest = 0;
};