// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2026 Thomas Basler and others
 */
#include "HM_Abstract.h"
#include "Hoymiles.h"
#include "HoymilesRadio.h"
#include "commands/ActivePowerControlCommand.h"
#include "commands/AlarmDataCommand.h"
#include "commands/DevInfoAllCommand.h"
#include "commands/DevInfoSimpleCommand.h"
#include "commands/GridOnProFilePara.h"
#include "commands/GridProfileWriteCommand.h"
#include "commands/PowerControlCommand.h"
#include "commands/PowerFactorControlCommand.h"
#include "commands/ReactivePowerControlCommand.h"
#include "commands/RealTimeRunDataCommand.h"
#include "commands/RfInfoCommand.h"
#include "commands/SystemConfigParaCommand.h"
#include "commands/YieldTotalSetCommand.h"
#include <esp_log.h>

#undef TAG
static const char* TAG = "hoymiles";

HM_Abstract::HM_Abstract(HoymilesRadio* radio, const uint64_t serial)
    : InverterAbstract(radio, serial)
{
}

bool HM_Abstract::sendStatsRequest()
{
    if (!getEnablePolling()) {
        return false;
    }

    time_t now;
    time(&now);

    auto cmd = _radio->prepareCommand<RealTimeRunDataCommand>(this);
    cmd->setTime(now);
    _radio->enqueCommand(cmd);

    return true;
}

bool HM_Abstract::sendAlarmLogRequest(const bool force)
{
    if (!getEnablePolling()) {
        return false;
    }

    if (!force) {
        if (Statistics()->hasChannelFieldValue(TYPE_INV, CH0, FLD_EVT_LOG)) {
            if (static_cast<uint8_t>(Statistics()->getChannelFieldValue(TYPE_INV, CH0, FLD_EVT_LOG)) == _lastAlarmLogCnt) {
                return false;
            }
        }
    }

    _lastAlarmLogCnt = static_cast<uint8_t>(Statistics()->getChannelFieldValue(TYPE_INV, CH0, FLD_EVT_LOG));

    time_t now;
    time(&now);

    auto cmd = _radio->prepareCommand<AlarmDataCommand>(this);
    cmd->setTime(now);
    EventLog()->setLastAlarmRequestSuccess(CMD_PENDING);
    _radio->enqueCommand(cmd);

    return true;
}

bool HM_Abstract::sendDevInfoRequest()
{
    if (!getEnablePolling()) {
        return false;
    }

    time_t now;
    time(&now);

    auto cmdAll = _radio->prepareCommand<DevInfoAllCommand>(this);
    cmdAll->setTime(now);
    _radio->enqueCommand(cmdAll);

    auto cmdSimple = _radio->prepareCommand<DevInfoSimpleCommand>(this);
    cmdSimple->setTime(now);
    _radio->enqueCommand(cmdSimple);

    return true;
}

bool HM_Abstract::sendSystemConfigParaRequest()
{
    if (!getEnablePolling()) {
        return false;
    }

    time_t now;
    time(&now);

    auto cmd = _radio->prepareCommand<SystemConfigParaCommand>(this);
    cmd->setTime(now);
    SystemConfigPara()->setLastLimitRequestSuccess(CMD_PENDING);
    _radio->enqueCommand(cmd);

    return true;
}

bool HM_Abstract::sendActivePowerControlRequest(float limit, const PowerLimitControlType type)
{
    if (!getEnableCommands()) {
        return false;
    }

    if (type == PowerLimitControlType::RelativNonPersistent || type == PowerLimitControlType::RelativPersistent) {
        limit = min<float>(100, limit);
    }

    _activePowerControlLimit = limit;
    _activePowerControlType = type;

    auto cmd = _radio->prepareCommand<ActivePowerControlCommand>(this);
    cmd->setActivePowerLimit(limit, type);
    SystemConfigPara()->setLastLimitCommandSuccess(CMD_PENDING);
    _radio->enqueCommand(cmd);

    return true;
}

bool HM_Abstract::resendActivePowerControlRequest()
{
    return sendActivePowerControlRequest(_activePowerControlLimit, _activePowerControlType);
}

bool HM_Abstract::sendReactivePowerControlRequest(float limit, const PowerLimitControlType type)
{
    if (!getEnableCommands()) {
        return false;
    }

    if (type == PowerLimitControlType::RelativNonPersistent || type == PowerLimitControlType::RelativPersistent) {
        limit = min<float>(100, limit);
    }

    const bool isAbsolute = type == PowerLimitControlType::AbsolutNonPersistent || type == PowerLimitControlType::AbsolutPersistent;
    const float maxPower = DevInfo()->getMaxPower();
    const float percent = (isAbsolute && maxPower > 0) ? (limit / maxPower * 100) : limit;
    SystemConfigPara()->setReactivePowerPercent(percent);

    auto cmd = _radio->prepareCommand<ReactivePowerControlCommand>(this);
    cmd->setReactivePowerLimit(limit, type);
    SystemConfigPara()->setLastReactivePowerCommandSuccess(CMD_PENDING);
    _radio->enqueCommand(cmd);

    return true;
}

bool HM_Abstract::sendPowerFactorControlRequest(float pf, const PowerLimitControlType type)
{
    if (!getEnableCommands()) {
        return false;
    }

    pf = min<float>(1, max<float>(0, pf));

    // No hardware readback exists for power factor either, so remember the last commanded value.
    SystemConfigPara()->setPowerFactor(pf);

    auto cmd = _radio->prepareCommand<PowerFactorControlCommand>(this);
    cmd->setPowerFactorLimit(pf, type);
    SystemConfigPara()->setLastPowerFactorCommandSuccess(CMD_PENDING);
    _radio->enqueCommand(cmd);

    return true;
}

bool HM_Abstract::sendPowerControlRequest(const bool turnOn)
{
    if (!getEnableCommands()) {
        return false;
    }

    if (turnOn) {
        _powerState = 1;
    } else {
        _powerState = 0;
    }

    auto cmd = _radio->prepareCommand<PowerControlCommand>(this);
    cmd->setPowerOn(turnOn);
    PowerCommand()->setLastPowerCommandSuccess(CMD_PENDING);
    _radio->enqueCommand(cmd);

    return true;
}

bool HM_Abstract::sendRestartControlRequest()
{
    if (!getEnableCommands()) {
        return false;
    }

    _powerState = 2;

    auto cmd = _radio->prepareCommand<PowerControlCommand>(this);
    cmd->setRestart();
    PowerCommand()->setLastPowerCommandSuccess(CMD_PENDING);
    _radio->enqueCommand(cmd);

    return true;
}

bool HM_Abstract::resendPowerControlRequest()
{
    switch (_powerState) {
    case 0:
        return sendPowerControlRequest(false);
        break;
    case 1:
        return sendPowerControlRequest(true);
        break;
    case 2:
        return sendRestartControlRequest();
        break;

    default:
        return false;
        break;
    }
}

bool HM_Abstract::sendGridOnProFileParaRequest(const bool viaCommand)
{
    if (viaCommand ? !getEnableCommands() : !getEnablePolling()) {
        return false;
    }

    time_t now;
    time(&now);

    auto cmd = _radio->prepareCommand<GridOnProFilePara>(this);
    cmd->setTime(now);
    _radio->enqueCommand(cmd);

    return true;
}

bool HM_Abstract::sendRfInfoRequest()
{
    if (!getEnablePolling()) {
        return false;
    }

    auto cmd = _radio->prepareCommand<RfInfoCommand>(this);
    _radio->enqueCommand(cmd);

    return true;
}

bool HM_Abstract::supportsPowerDistributionLogic()
{
    return false;
}

bool HM_Abstract::sendGridProfileWriteRequest(const std::vector<uint8_t>& profile)
{
    if (!getEnableCommands()) {
        return false;
    }
    if (profile.empty() || profile.size() > 200) {
        return false;
    }
    if (_gridProfileWriteRunning) {
        return false;
    }

    constexpr size_t chunkSize = 16;
    constexpr size_t crcSize = 2;
    const size_t total = profile.size();
    const size_t dataFrameCount = (total + chunkSize - 1) / chunkSize;
    if (dataFrameCount == 0) {
        return false;
    }

    const size_t lastChunkLen = total - (dataFrameCount - 1) * chunkSize;
    const bool crcFitsInline = lastChunkLen + crcSize <= chunkSize;
    const size_t frameCount = dataFrameCount + (crcFitsInline ? 0 : 1);
    if (frameCount > 127) {
        return false;
    }

    GridProfile()->setLastWriteCommandSuccess(CMD_PENDING);
    _gridProfileWriteRunning = true;

    for (size_t i = 0; i < dataFrameCount; i++) {
        const bool isLastData = (i + 1 == dataFrameCount);
        const bool isLastFrame = isLastData && crcFitsInline;
        const size_t offset = i * chunkSize;
        const size_t chunkLen = std::min(chunkSize, total - offset);

        auto cmd = _radio->prepareCommand<GridProfileWriteCommand>(this);

        cmd->setFullProfile(profile.data(), profile.size());
        cmd->setPacketNumber(static_cast<uint8_t>(i + 1), isLastFrame);
        cmd->setPayload(&profile[offset], static_cast<uint8_t>(chunkLen));

        char hex[3 * 24 + 1];
        size_t off = 0;
        for (size_t j = 0; j < chunkLen && off + 3 < sizeof(hex); j++) {
            off += snprintf(&hex[off], sizeof(hex) - off, "%02X ", profile[offset + j]);
        }
        hex[off] = '\0';
        ESP_LOGI(TAG, "GridProfileWrite TX chunk %u/%u nub=0x%02X len=%u: %s",
            static_cast<unsigned>(i + 1), static_cast<unsigned>(frameCount),
            static_cast<uint8_t>((i + 1) | (isLastFrame ? 0x80 : 0x00)),
            static_cast<unsigned>(chunkLen), hex);

        _radio->enqueCommand(cmd);
    }

    if (!crcFitsInline) {
        auto crcCmd = _radio->prepareCommand<GridProfileWriteCommand>(this);
        crcCmd->setFullProfile(profile.data(), profile.size());
        crcCmd->setPacketNumber(static_cast<uint8_t>(dataFrameCount + 1), true);
        crcCmd->setPayload(profile.data(), 0);

        ESP_LOGI(TAG, "GridProfileWrite TX chunk %u/%u nub=0x%02X len=0 (CRC-only frame)",
            static_cast<unsigned>(dataFrameCount + 1), static_cast<unsigned>(frameCount),
            static_cast<uint8_t>((dataFrameCount + 1) | 0x80));

        _radio->enqueCommand(crcCmd);
    }

    return true;
}

bool HM_Abstract::getGridProfileWriteRunning() const
{
    return _gridProfileWriteRunning;
}

void HM_Abstract::abortGridProfileWriteRequest()
{
    if (_radio != nullptr) {
        _radio->removePendingGridProfileWriteCommands(this);
    }
    _gridProfileWriteRunning = false;
    GridProfile()->setLastWriteCommandSuccess(CMD_NOK);
}

void HM_Abstract::onGridProfileWriteCompleted(const bool /*success*/)
{
    _gridProfileWriteRunning = false;

    // Read the profile back from the inverter so the cached copy (and the web
    // view) reflects what is actually active. Done on failure too: a timeout on
    // the last frame does not mean the inverter rejected the profile.
    sendGridOnProFileParaRequest(true);
}

bool HM_Abstract::sendYieldTotalSetRequest(const uint32_t valuesWh[4], const uint8_t valueCount)
{
    if (!getEnableCommands()) {
        return false;
    }
    if (valueCount == 0 || valueCount > 4) {
        return false;
    }
    if (_yieldTotalSetRunning) {
        return false;
    }

    uint32_t values[4] = { 0, 0, 0, 0 };
    for (uint8_t i = 0; i < valueCount; i++) {
        values[i] = valuesWh[i];
    }

    _lastYieldTotalSetSuccess = CMD_PENDING;
    _yieldTotalSetRunning = true;

    auto cmd1 = _radio->prepareCommand<YieldTotalSetCommand>(this);
    cmd1->setValues(values, 1);
    _radio->enqueCommand(cmd1);

    auto cmd2 = _radio->prepareCommand<YieldTotalSetCommand>(this);
    cmd2->setValues(values, 2);
    _radio->enqueCommand(cmd2);

    return true;
}

bool HM_Abstract::getYieldTotalSetRunning() const
{
    return _yieldTotalSetRunning;
}

LastCommandSuccess HM_Abstract::getLastYieldTotalSetSuccess() const
{
    return _lastYieldTotalSetSuccess;
}

void HM_Abstract::onYieldTotalSetCompleted(const bool success)
{
    _yieldTotalSetRunning = false;
    _lastYieldTotalSetSuccess = success ? CMD_OK : CMD_NOK;
}
