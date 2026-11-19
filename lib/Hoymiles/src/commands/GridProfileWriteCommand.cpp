// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024-2026 Thomas Basler and others
 */
#include "GridProfileWriteCommand.h"
#include "../inverters/InverterAbstract.h"
#include "../parser/GridProfileParser.h"
#include "crc.h"
#include <algorithm>
#include <cstring>
#include <esp_log.h>

#undef TAG
static const char* TAG = "hoymiles";

#define GRID_PROFILE_CHUNK_SIZE 16

#define GRID_PROFILE_MID_TIMEOUT_MS 50
#define GRID_PROFILE_LAST_TIMEOUT_MS 3000

GridProfileWriteCommand::GridProfileWriteCommand(InverterAbstract* inv, const uint64_t router_address)
    : CommandAbstract(inv, router_address)
{
    _payload[0] = 0x0A;
    _payload[9] = 0x00;
    _payload_size = 10;
    setTimeout(GRID_PROFILE_MID_TIMEOUT_MS);
}

String GridProfileWriteCommand::getCommandName() const
{
    char buf[40];
    snprintf(buf, sizeof(buf), "GridProfileWrite (nub=%u%s)",
        static_cast<unsigned>(_packetNumber),
        _isLast ? ", LAST" : "");
    return buf;
}

void GridProfileWriteCommand::setPacketNumber(const uint8_t packetNumber, const bool isLast)
{
    _packetNumber = packetNumber;
    _isLast = isLast;
    _payload[9] = static_cast<uint8_t>(packetNumber | (isLast ? 0x80 : 0x00));
    setTimeout(isLast ? GRID_PROFILE_LAST_TIMEOUT_MS : GRID_PROFILE_MID_TIMEOUT_MS);
}

void GridProfileWriteCommand::setPayload(const uint8_t* data, const uint8_t len)
{
    const uint8_t safe = std::min<uint8_t>(len, GRID_PROFILE_CHUNK_SIZE);
    memcpy(&_payload[10], data, safe);
    _chunkLen = safe;
    _payload_size = 10 + safe;
    _crc16Applied = false;
    appendCrc16IfLast();
}

void GridProfileWriteCommand::setFullProfile(const uint8_t* profile, const size_t profileLen)
{
    _fullProfile.assign(profile, profile + profileLen);
}

void GridProfileWriteCommand::appendCrc16IfLast()
{
    if (!_isLast || _crc16Applied || _fullProfile.empty()) {
        return;
    }
    if (10 + _chunkLen + 2 + 1 > RF_LEN) {
        ESP_LOGE(TAG, "GridProfileWrite: last chunk too large for inline CRC16");
        return;
    }
    const uint16_t c = crc16(_fullProfile.data(), _fullProfile.size());
    _payload[10 + _chunkLen] = static_cast<uint8_t>(c >> 8);
    _payload[10 + _chunkLen + 1] = static_cast<uint8_t>(c & 0xff);
    _payload_size = 10 + _chunkLen + 2;
    _crc16Applied = true;
}

bool GridProfileWriteCommand::handleResponse(const fragment_t fragment[], const uint8_t max_fragment_id)
{
    // Middle frames: no reply expected, so any packet arriving is unexpected.
    if (!_isLast) {
        return false;
    }

    for (uint8_t i = 0; i < max_fragment_id; i++) {
        char hex[3 * MAX_RF_PAYLOAD_SIZE + 1];
        size_t off = 0;
        for (uint8_t j = 0; j < fragment[i].len && off + 3 < sizeof(hex); j++) {
            off += snprintf(&hex[off], sizeof(hex) - off, "%02X ", fragment[i].fragment[j]);
        }
        hex[off] = '\0';
        ESP_LOGI(TAG, "GridProfileWrite RX frag[%u] mainCmd=0x%02X len=%u: %s",
            i, fragment[i].mainCmd, fragment[i].len, hex);
    }

    for (uint8_t i = 0; i < max_fragment_id; i++) {
        if (fragment[i].mainCmd != (_payload[0] | 0x80)) {
            ESP_LOGW(TAG, "GridProfileWrite: unexpected mainCmd 0x%02X (want 0x%02X)",
                fragment[i].mainCmd, static_cast<uint8_t>(_payload[0] | 0x80));
            _inv->GridProfile()->setLastWriteCommandSuccess(CMD_NOK);
            return false;
        }
    }

    uint8_t state = 0xFF;
    if (max_fragment_id > 0 && fragment[0].len >= 1) {
        state = fragment[0].fragment[0];
    }

    if (state == 0x00) {
        ESP_LOGI(TAG, "GridProfileWrite: inverter reported success (state=0)");
        _inv->GridProfile()->setLastWriteCommandSuccess(CMD_OK);
        _inv->onGridProfileWriteCompleted(true);
        return true;
    }

    ESP_LOGW(TAG, "GridProfileWrite: inverter reported failure (state=0x%02X)", state);
    _inv->GridProfile()->setLastWriteCommandSuccess(CMD_NOK);
    _inv->onGridProfileWriteCompleted(false);
    return true;
}

void GridProfileWriteCommand::gotTimeout()
{
    if (_isLast) {
        ESP_LOGW(TAG, "GridProfileWrite: timeout waiting for ack on last frame");
        _inv->GridProfile()->setLastWriteCommandSuccess(CMD_NOK);
        _inv->onGridProfileWriteCompleted(false);
    } else {
        ESP_LOGW(TAG, "GridProfileWrite: middle frame nub=%u not delivered, aborting write",
            static_cast<unsigned>(_packetNumber));
        _inv->abortGridProfileWriteRequest();
    }
    CommandAbstract::gotTimeout();
}

uint8_t GridProfileWriteCommand::getMaxResendCount() const
{
    return _isLast ? 3 : 2;
}

uint8_t GridProfileWriteCommand::getMaxRetransmitCount() const
{
    return 0;
}
