// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024-2026 Thomas Basler and others
 */
#include "YieldTotalSetCommand.h"
#include "../inverters/InverterAbstract.h"
#include "crc.h"
#include <cstring>
#include <esp_log.h>

#undef TAG
static const char* TAG = "hoymiles";

#define YIELD_SET_MID_TIMEOUT_MS 50
#define YIELD_SET_LAST_TIMEOUT_MS 2000

YieldTotalSetCommand::YieldTotalSetCommand(InverterAbstract* inv, const uint64_t router_address)
    : CommandAbstract(inv, router_address)
{
    _payload[0] = 0x52;
    setTimeout(YIELD_SET_MID_TIMEOUT_MS);
}

String YieldTotalSetCommand::getCommandName() const
{
    return _isLast ? "YieldTotalSet (LAST)" : "YieldTotalSet";
}

void YieldTotalSetCommand::setValues(const uint32_t valuesWh[4], const uint8_t frameIndex)
{
    _isLast = (frameIndex == 2);

    uint8_t header[16];
    header[0] = 0x01;
    header[1] = 0x00;
    header[2] = 0x00;
    header[3] = 0x0F;
    for (uint8_t i = 0; i < 3; i++) {
        const uint32_t v = valuesWh[i];
        header[4 + i * 4 + 0] = (v >> 24) & 0xFF;
        header[4 + i * 4 + 1] = (v >> 16) & 0xFF;
        header[4 + i * 4 + 2] = (v >> 8) & 0xFF;
        header[4 + i * 4 + 3] = v & 0xFF;
    }

    if (!_isLast) {
        _payload[9] = 0x01;
        memcpy(&_payload[10], header, sizeof(header));
        _payload_size = 26;
        setTimeout(YIELD_SET_MID_TIMEOUT_MS);
        return;
    }

    _payload[9] = 0x82;
    const uint32_t v4 = valuesWh[3];
    _payload[10] = (v4 >> 24) & 0xFF;
    _payload[11] = (v4 >> 16) & 0xFF;
    _payload[12] = (v4 >> 8) & 0xFF;
    _payload[13] = v4 & 0xFF;

    uint8_t crcBuf[20];
    memcpy(&crcBuf[0], header, sizeof(header));
    memcpy(&crcBuf[16], &_payload[10], 4);
    const uint16_t c16 = crc16(crcBuf, sizeof(crcBuf));
    _payload[14] = c16 >> 8;
    _payload[15] = c16 & 0xFF;
    _payload_size = 16;
    setTimeout(YIELD_SET_LAST_TIMEOUT_MS);
}

bool YieldTotalSetCommand::handleResponse(const fragment_t fragment[], const uint8_t max_fragment_id)
{
    if (!_isLast) {
        return false;
    }

    for (uint8_t i = 0; i < max_fragment_id; i++) {
        if (fragment[i].mainCmd != (_payload[0] | 0x80)) {
            ESP_LOGW(TAG, "YieldTotalSet: unexpected mainCmd 0x%02X (want 0x%02X)",
                fragment[i].mainCmd, static_cast<uint8_t>(_payload[0] | 0x80));
            _inv->onYieldTotalSetCompleted(false);
            return false;
        }
    }

    uint8_t state = 0xFF;
    uint16_t subEcho = 0xFFFF;
    if (max_fragment_id > 0 && fragment[0].len >= 4) {
        state = fragment[0].fragment[0];
        subEcho = (static_cast<uint16_t>(fragment[0].fragment[3]) << 8) | fragment[0].fragment[2];
    }

    const bool success = (state == 0 && subEcho == 0x0001);
    if (success) {
        ESP_LOGI(TAG, "YieldTotalSet: inverter reported success");
    } else {
        ESP_LOGW(TAG, "YieldTotalSet: inverter reported failure (state=0x%02X sub=0x%04X)", state, subEcho);
    }
    _inv->onYieldTotalSetCompleted(success);
    return true;
}

void YieldTotalSetCommand::gotTimeout()
{
    if (_isLast) {
        ESP_LOGW(TAG, "YieldTotalSet: timeout waiting for ack on last frame");
        _inv->onYieldTotalSetCompleted(false);
    }
    CommandAbstract::gotTimeout();
}

uint8_t YieldTotalSetCommand::getMaxResendCount() const
{
    return _isLast ? 3 : 0;
}

uint8_t YieldTotalSetCommand::getMaxRetransmitCount() const
{
    return 0;
}
