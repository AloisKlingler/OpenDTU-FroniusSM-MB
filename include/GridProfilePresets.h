// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <cstddef>
#include <cstdint>

/*
 * Curated catalogue of known-good grid profile blobs.
 *
 */

struct GridProfilePreset {
    uint16_t id;
    const char* label;
    const uint8_t* data;
    size_t dataLen;
};

extern const GridProfilePreset* const kGridProfilePresets;
extern const size_t kGridProfilePresetsCount;
