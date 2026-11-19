// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include "CommandAbstract.h"
#include <cstddef>
#include <vector>

/*
Command used to write a grid profile blob to a Hoymiles inverter.

*/

class GridProfileWriteCommand : public CommandAbstract {
public:
    explicit GridProfileWriteCommand(InverterAbstract* inv, const uint64_t router_address = 0);

    virtual String getCommandName() const;

    void setPacketNumber(const uint8_t packetNumber, const bool isLast);

    void setPayload(const uint8_t* data, const uint8_t len);

    void setFullProfile(const uint8_t* profile, const size_t profileLen);

    virtual bool handleResponse(const fragment_t fragment[], const uint8_t max_fragment_id);
    virtual void gotTimeout();

    virtual uint8_t getMaxResendCount() const;
    virtual uint8_t getMaxRetransmitCount() const;
    bool expectsResponse() const override { return _isLast; }
    virtual QueueInsertType getQueueInsertType() const { return QueueInsertType::AllowMultiple; }

    bool isGridProfileWriteCommand() const override { return true; }

    bool isLastFragment() const { return _isLast; }
    uint8_t getPacketNumber() const { return _packetNumber; }

private:
    void appendCrc16IfLast();

    uint8_t _packetNumber = 0;
    bool _isLast = false;
    uint8_t _chunkLen = 0;

    std::vector<uint8_t> _fullProfile;
    bool _crc16Applied = false;
};
