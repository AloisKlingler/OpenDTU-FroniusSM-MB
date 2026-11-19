// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include "CommandAbstract.h"
#include <cstdint>

/*
Command used to write the per-string YieldTotal (total produced energy, Wh)
to a Hoymiles inverter.
*/

class YieldTotalSetCommand : public CommandAbstract {
public:
    explicit YieldTotalSetCommand(InverterAbstract* inv, const uint64_t router_address = 0);

    virtual String getCommandName() const;

    void setValues(const uint32_t valuesWh[4], const uint8_t frameIndex);

    virtual bool handleResponse(const fragment_t fragment[], const uint8_t max_fragment_id);
    virtual void gotTimeout();

    virtual uint8_t getMaxResendCount() const;
    virtual uint8_t getMaxRetransmitCount() const;
    virtual QueueInsertType getQueueInsertType() const { return QueueInsertType::AllowMultiple; }

private:
    bool _isLast = false;
};
