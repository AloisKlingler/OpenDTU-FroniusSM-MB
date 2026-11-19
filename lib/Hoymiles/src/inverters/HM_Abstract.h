// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include "InverterAbstract.h"

class HM_Abstract : public InverterAbstract {
public:
    explicit HM_Abstract(HoymilesRadio* radio, const uint64_t serial);
    bool sendStatsRequest();
    bool sendAlarmLogRequest(const bool force = false);
    bool sendDevInfoRequest();
    bool sendSystemConfigParaRequest();
    bool sendActivePowerControlRequest(float limit, const PowerLimitControlType type);
    bool resendActivePowerControlRequest();
    bool sendReactivePowerControlRequest(float limit, const PowerLimitControlType type) override;
    bool sendPowerFactorControlRequest(float pf, const PowerLimitControlType type) override;
    bool sendPowerControlRequest(const bool turnOn);
    bool sendRestartControlRequest();
    bool resendPowerControlRequest();
    bool sendGridOnProFileParaRequest(const bool viaCommand = false) override;
    bool sendRfInfoRequest() override;
    bool supportsPowerDistributionLogic() override;

    bool sendGridProfileWriteRequest(const std::vector<uint8_t>& profile) override;
    bool getGridProfileWriteRunning() const override;
    void abortGridProfileWriteRequest() override;
    void onGridProfileWriteCompleted(const bool success) override;

    bool sendYieldTotalSetRequest(const uint32_t valuesWh[4], const uint8_t valueCount) override;
    bool getYieldTotalSetRunning() const override;
    LastCommandSuccess getLastYieldTotalSetSuccess() const override;
    void onYieldTotalSetCompleted(const bool success) override;

protected:
    float _activePowerControlLimit = 0;
    PowerLimitControlType _activePowerControlType = PowerLimitControlType::AbsolutNonPersistent;

private:
    uint8_t _lastAlarmLogCnt = 0;
    uint8_t _powerState = 1;

    bool _gridProfileWriteRunning = false;

    bool _yieldTotalSetRunning = false;
    LastCommandSuccess _lastYieldTotalSetSuccess = CMD_NOK;
};
