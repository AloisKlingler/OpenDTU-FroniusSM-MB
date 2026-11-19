// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 Thomas Basler and others
 */

/*
Reactive power write.
*/
#include "ReactivePowerControlCommand.h"
#include "inverters/InverterAbstract.h"

#define CRC_SIZE 6

ReactivePowerControlCommand::ReactivePowerControlCommand(InverterAbstract* inv, const uint64_t router_address)
    : DevControlCommand(inv, router_address)
{
    _payload[10] = 0x0c;
    _payload[11] = 0x00;
    _payload[12] = 0x00;
    _payload[13] = 0x00;
    _payload[14] = 0x00;
    _payload[15] = 0x00;

    udpateCRC(CRC_SIZE);

    _payload_size = 18;

    setTimeout(2000);
}

String ReactivePowerControlCommand::getCommandName() const
{
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "ReactivePowerControl (%02X)", getType());
    return buffer;
}

bool ReactivePowerControlCommand::areSameParameter(CommandAbstract* other)
{
    return CommandAbstract::areSameParameter(other)
        && this->getType() == static_cast<ReactivePowerControlCommand*>(other)->getType();
}

void ReactivePowerControlCommand::setReactivePowerLimit(const float limit, const PowerLimitControlType type)
{
    const int16_t s = static_cast<int16_t>(limit * 10);
    const uint16_t l = static_cast<uint16_t>(s);

    _payload[12] = (l >> 8) & 0xff;
    _payload[13] = (l) & 0xff;

    const uint16_t type_value = getControlTypeValue(type);
    _payload[14] = (type_value >> 8) & 0xff;
    _payload[15] = (type_value) & 0xff;

    udpateCRC(CRC_SIZE);
}

bool ReactivePowerControlCommand::handleResponse(const fragment_t fragment[], const uint8_t max_fragment_id)
{
    if (!DevControlCommand::handleResponse(fragment, max_fragment_id)) {
        return false;
    }

    _inv->SystemConfigPara()->setLastReactivePowerUpdateCommand(millis());
    std::shared_ptr<ReactivePowerControlCommand> cmd(std::shared_ptr<ReactivePowerControlCommand>(), this);
    if (_inv->getRadio()->countSimilarCommands(cmd) == 1) {
        _inv->SystemConfigPara()->setLastReactivePowerCommandSuccess(CMD_OK);
    }
    return true;
}

float ReactivePowerControlCommand::getLimit() const
{
    const int16_t l = static_cast<int16_t>((static_cast<uint16_t>(_payload[12]) << 8) | _payload[13]);
    return l / 10.0f;
}

PowerLimitControlType ReactivePowerControlCommand::getType() const
{
    const uint16_t type_val = (static_cast<uint16_t>(_payload[14]) << 8) | _payload[15];
    for (uint8_t i = 0; i < PowerLimitControl_Max; i++) {
        if (type_val == getControlTypeValue(static_cast<PowerLimitControlType>(i))) {
            return static_cast<PowerLimitControlType>(i);
        }
    }
    return PowerLimitControlType::RelativNonPersistent;
}

uint16_t ReactivePowerControlCommand::getControlTypeValue(PowerLimitControlType type)
{
    // From ActiveDTU writeReactivePower(): typeHi (byte 14) = persistent, typeLo (byte 15) = relative.
    switch (type) {
    case AbsolutNonPersistent: return 0x0000;
    case RelativNonPersistent: return 0x0001;
    case AbsolutPersistent:    return 0x0100;
    case RelativPersistent:    return 0x0101;
    default:                   return 0x0000;
    }
}

void ReactivePowerControlCommand::gotTimeout()
{
    _inv->SystemConfigPara()->setLastReactivePowerCommandSuccess(CMD_NOK);
}
