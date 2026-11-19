// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2026 Thomas Basler and others
 */
#include "WebApi_gridprofile.h"
#include "GridProfilePresets.h"
#include "WebApi.h"
#include "WebApi_errors.h"
#include <AsyncJson.h>
#include <Hoymiles.h>
#include <parser/GridProfileParser.h>
#include <algorithm>
#include <cctype>

namespace {
static uint8_t hexNybble(const char c)
{
    if (c >= '0' && c <= '9') return static_cast<uint8_t>(c - '0');
    if (c >= 'a' && c <= 'f') return static_cast<uint8_t>(10 + c - 'a');
    if (c >= 'A' && c <= 'F') return static_cast<uint8_t>(10 + c - 'A');
    return 0xff;
}

static bool decodeHexString(const String& s, std::vector<uint8_t>& out)
{
    out.clear();
    out.reserve(s.length() / 2);
    uint8_t high = 0;
    bool haveHigh = false;
    for (size_t i = 0; i < s.length(); i++) {
        const char c = s[i];
        if (std::isspace(static_cast<unsigned char>(c)) || c == ':' || c == '-') {
            continue;
        }
        const uint8_t n = hexNybble(c);
        if (n == 0xff) {
            return false;
        }
        if (!haveHigh) {
            high = n;
            haveHigh = true;
        } else {
            out.push_back(static_cast<uint8_t>((high << 4) | n));
            haveHigh = false;
        }
    }
    return !haveHigh;
}

static const char* successStateToString(LastCommandSuccess s)
{
    switch (s) {
    case CMD_OK:      return "Ok";
    case CMD_NOK:     return "Failure";
    case CMD_PENDING: return "Pending";
    }
    return "Unknown";
}
} // namespace

void WebApiGridProfileClass::init(AsyncWebServer& server, Scheduler& scheduler)
{
    using std::placeholders::_1;

    server.on("/api/gridprofile/status",         HTTP_GET,  static_cast<ArRequestHandlerFunction>(std::bind(&WebApiGridProfileClass::onGridProfileStatus,       this, _1)));
    server.on("/api/gridprofile/rawdata",        HTTP_GET,  static_cast<ArRequestHandlerFunction>(std::bind(&WebApiGridProfileClass::onGridProfileRawdata,      this, _1)));
    server.on("/api/gridprofile/knownprofiles",  HTTP_GET,  static_cast<ArRequestHandlerFunction>(std::bind(&WebApiGridProfileClass::onGridProfileKnownPresets, this, _1)));
    server.on("/api/gridprofile/writestatus",    HTTP_GET,  static_cast<ArRequestHandlerFunction>(std::bind(&WebApiGridProfileClass::onGridProfileWriteStatus,  this, _1)));
    server.on("/api/gridprofile/write",          HTTP_POST, static_cast<ArRequestHandlerFunction>(std::bind(&WebApiGridProfileClass::onGridProfileWritePost,    this, _1)));
    server.on("/api/gridprofile/abort",          HTTP_POST, static_cast<ArRequestHandlerFunction>(std::bind(&WebApiGridProfileClass::onGridProfileAbortPost,    this, _1)));
    server.on("/api/gridprofile/refresh",        HTTP_POST, static_cast<ArRequestHandlerFunction>(std::bind(&WebApiGridProfileClass::onGridProfileRefreshPost,  this, _1)));
}

void WebApiGridProfileClass::onGridProfileStatus(AsyncWebServerRequest* request)
{
    if (!WebApi.checkCredentialsReadonly(request)) {
        return;
    }

    AsyncJsonResponse* response = new AsyncJsonResponse();
    auto& root = response->getRoot();
    auto serial = WebApi.parseSerialFromRequest(request);
    auto inv = Hoymiles.getInverterBySerial(serial);

    if (inv != nullptr) {
        root["name"] = inv->GridProfile()->getProfileName();
        root["version"] = inv->GridProfile()->getProfileVersion();
        root["last_update"] = inv->GridProfile()->getLastUpdate();

        int32_t matchedPresetId = -1;
        bool matchesPreset = false;
        auto raw = inv->GridProfile()->getRawData();
        if (raw.size() >= 2) {
            for (size_t i = 0; i < kGridProfilePresetsCount; i++) {
                const auto& p = kGridProfilePresets[i];
                if (p.dataLen < 2 || p.data[0] != raw[0] || p.data[1] != raw[1]) {
                    continue;
                }
                matchedPresetId = p.id;
                matchesPreset = (p.dataLen == raw.size()) && std::equal(raw.begin(), raw.end(), p.data);
                break;
            }
        }
        root["matchedPresetId"] = matchedPresetId;
        root["matchesPreset"] = matchesPreset;

        auto jsonSections = root["sections"].to<JsonArray>();
        auto profSections = inv->GridProfile()->getProfile();

        for (auto& profSection : profSections) {
            auto jsonSection = jsonSections.add<JsonObject>();
            jsonSection["name"] = profSection.SectionName;

            auto jsonItems = jsonSection["items"].to<JsonArray>();

            for (auto& profItem : profSection.items) {
                auto jsonItem = jsonItems.add<JsonObject>();

                jsonItem["n"] = profItem.Name;
                jsonItem["u"] = profItem.Unit;
                jsonItem["v"] = profItem.Value;
            }
        }
    }

    WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
}

void WebApiGridProfileClass::onGridProfileRawdata(AsyncWebServerRequest* request)
{
    if (!WebApi.checkCredentialsReadonly(request)) {
        return;
    }

    AsyncJsonResponse* response = new AsyncJsonResponse();
    auto& root = response->getRoot();
    auto serial = WebApi.parseSerialFromRequest(request);
    auto inv = Hoymiles.getInverterBySerial(serial);

    if (inv != nullptr) {
        auto raw = root["raw"].to<JsonArray>();
        auto data = inv->GridProfile()->getRawData();

        copyArray(&data[0], data.size(), raw);
    }

    WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
}

void WebApiGridProfileClass::onGridProfileKnownPresets(AsyncWebServerRequest* request)
{
    if (!WebApi.checkCredentialsReadonly(request)) {
        return;
    }

    AsyncJsonResponse* response = new AsyncJsonResponse();
    auto& root = response->getRoot();
    auto arr = root["profiles"].to<JsonArray>();

    for (size_t i = 0; i < kGridProfilePresetsCount; i++) {
        const auto& p = kGridProfilePresets[i];
        auto o = arr.add<JsonObject>();
        o["id"] = p.id;
        o["label"] = p.label;
        o["size"] = static_cast<uint32_t>(p.dataLen);
    }

    WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
}

void WebApiGridProfileClass::onGridProfileWriteStatus(AsyncWebServerRequest* request)
{
    if (!WebApi.checkCredentialsReadonly(request)) {
        return;
    }

    AsyncJsonResponse* response = new AsyncJsonResponse();
    auto& root = response->getRoot();
    auto serial = WebApi.parseSerialFromRequest(request);
    auto inv = Hoymiles.getInverterBySerial(serial);

    if (inv != nullptr) {
        const bool running = inv->getGridProfileWriteRunning();
        const auto st = inv->GridProfile()->getLastWriteCommandSuccess();
        const uint32_t lastUpdate = inv->GridProfile()->getLastWriteUpdate();
        root["running"] = running;

        if (running && st != CMD_NOK) {
            root["state"] = "Pending";
        } else if (lastUpdate == 0) {
            root["state"] = "None";
        } else {
            root["state"] = successStateToString(st);
        }
        root["last_update"] = lastUpdate;
        root["profile_last_update"] = inv->GridProfile()->getLastUpdate();
        root["queue_has_write"] = inv->getRadio()->hasGridProfileWriteCommands(inv.get());
    } else {
        root["running"] = false;
        root["state"] = "Unknown";
    }

    WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
}

void WebApiGridProfileClass::onGridProfileWritePost(AsyncWebServerRequest* request)
{
    if (!WebApi.checkCredentials(request)) {
        return;
    }

    AsyncJsonResponse* response = new AsyncJsonResponse();
    JsonDocument root;
    if (!WebApi.parseRequestData(request, response, root)) {
        return;
    }

    auto& retMsg = response->getRoot();

    if (!root["serial"].is<String>()) {
        retMsg["message"] = "Serial missing!";
        retMsg["code"] = WebApiError::GenericValueMissing;
        WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
        return;
    }

    const uint64_t serial = strtoll(root["serial"].as<String>().c_str(), NULL, 16);
    if (serial == 0) {
        retMsg["message"] = "Serial must be a number > 0!";
        retMsg["code"] = WebApiError::GridProfileSerialZero;
        WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
        return;
    }

    auto inv = Hoymiles.getInverterBySerial(serial);
    if (inv == nullptr) {
        retMsg["message"] = "Invalid inverter!";
        retMsg["code"] = WebApiError::GridProfileInvalidInverter;
        WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
        return;
    }

    if (inv->getGridProfileWriteRunning()) {
        retMsg["message"] = "A grid profile write is already in progress for this inverter.";
        retMsg["code"] = WebApiError::GridProfileWriteInProgress;
        WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
        return;
    }

    std::vector<uint8_t> profileBytes;

    // Mode 1: known preset id.
    if (root["preset_id"].is<uint16_t>()) {
        const uint16_t pid = root["preset_id"].as<uint16_t>();
        const GridProfilePreset* found = nullptr;
        for (size_t i = 0; i < kGridProfilePresetsCount; i++) {
            if (kGridProfilePresets[i].id == pid) {
                found = &kGridProfilePresets[i];
                break;
            }
        }
        if (found == nullptr) {
            retMsg["message"] = "Unknown preset id.";
            retMsg["code"] = WebApiError::GridProfileUnknownPreset;
            WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
            return;
        }
        profileBytes.assign(found->data, found->data + found->dataLen);
    }
    // Mode 2: raw hex string.
    else if (root["profile_hex"].is<String>()) {
        if (!decodeHexString(root["profile_hex"].as<String>(), profileBytes)) {
            retMsg["message"] = "profile_hex is not a valid hex string.";
            retMsg["code"] = WebApiError::GridProfileInvalidPayload;
            WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
            return;
        }
    }
    // Mode 3: edited section list (values only).
    else if (root["sections"].is<JsonArray>()) {
        std::list<GridProfileSection_t> sections;
        for (JsonObject sec : root["sections"].as<JsonArray>()) {
            GridProfileSection_t s;
            if (sec["name"].is<String>()) {
                s.SectionName = sec["name"].as<String>();
            }
            if (sec["items"].is<JsonArray>()) {
                for (JsonObject it : sec["items"].as<JsonArray>()) {
                    GridProfileItem_t item;
                    if (it["n"].is<String>()) item.Name = it["n"].as<String>();
                    if (it["u"].is<String>()) item.Unit = it["u"].as<String>();
                    if (it["v"].is<float>())  item.Value = it["v"].as<float>();
                    s.items.push_back(item);
                }
            }
            sections.push_back(s);
        }
        profileBytes = inv->GridProfile()->encodeUpdatedValues(sections);
        if (profileBytes.empty()) {
            retMsg["message"] = "Failed to encode edited profile - shape mismatch or no current profile loaded.";
            retMsg["code"] = WebApiError::GridProfileEncodeFailed;
            WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
            return;
        }
    } else {
        retMsg["message"] = "Provide either preset_id, profile_hex, or sections.";
        retMsg["code"] = WebApiError::GenericValueMissing;
        WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
        return;
    }

    if (profileBytes.empty() || profileBytes.size() > 200) {
        retMsg["message"] = "Profile payload has an invalid size.";
        retMsg["code"] = WebApiError::GridProfileInvalidPayload;
        WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
        return;
    }

    if (!inv->sendGridProfileWriteRequest(profileBytes)) {
        retMsg["message"] = "Failed to enqueue grid profile write.";
        retMsg["code"] = WebApiError::GenericWriteFailed;
        WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
        return;
    }

    retMsg["type"] = "success";
    retMsg["message"] = "Grid profile write queued.";
    retMsg["code"] = WebApiError::GridProfileWriteQueued;
    WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
}

void WebApiGridProfileClass::onGridProfileAbortPost(AsyncWebServerRequest* request)
{
    if (!WebApi.checkCredentials(request)) {
        return;
    }

    AsyncJsonResponse* response = new AsyncJsonResponse();
    JsonDocument root;
    if (!WebApi.parseRequestData(request, response, root)) {
        return;
    }

    auto& retMsg = response->getRoot();

    if (!root["serial"].is<String>()) {
        retMsg["message"] = "Serial missing!";
        retMsg["code"] = WebApiError::GenericValueMissing;
        WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
        return;
    }

    const uint64_t serial = strtoll(root["serial"].as<String>().c_str(), NULL, 16);
    auto inv = Hoymiles.getInverterBySerial(serial);
    if (inv == nullptr) {
        retMsg["message"] = "Invalid inverter!";
        retMsg["code"] = WebApiError::GridProfileInvalidInverter;
        WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
        return;
    }

    inv->abortGridProfileWriteRequest();

    retMsg["type"] = "success";
    retMsg["message"] = "Grid profile write aborted (pending frames removed).";
    retMsg["code"] = WebApiError::GridProfileWriteAborted;
    WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
}

void WebApiGridProfileClass::onGridProfileRefreshPost(AsyncWebServerRequest* request)
{
    if (!WebApi.checkCredentials(request)) {
        return;
    }

    AsyncJsonResponse* response = new AsyncJsonResponse();
    JsonDocument root;
    if (!WebApi.parseRequestData(request, response, root)) {
        return;
    }

    auto& retMsg = response->getRoot();

    if (!root["serial"].is<String>()) {
        retMsg["message"] = "Serial missing!";
        retMsg["code"] = WebApiError::GenericValueMissing;
        WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
        return;
    }

    const uint64_t serial = strtoll(root["serial"].as<String>().c_str(), NULL, 16);
    auto inv = Hoymiles.getInverterBySerial(serial);
    if (inv == nullptr) {
        retMsg["message"] = "Invalid inverter!";
        retMsg["code"] = WebApiError::GridProfileInvalidInverter;
        WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
        return;
    }

    if (!inv->sendGridOnProFileParaRequest(true)) {
        retMsg["message"] = "Failed to enqueue grid profile refresh. Is \"Commands enabled\" set for this inverter?";
        retMsg["code"] = WebApiError::GenericWriteFailed;
        WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
        return;
    }

    retMsg["type"] = "success";
    retMsg["message"] = "Grid profile refresh queued.";
    retMsg["code"] = WebApiError::GridProfileRefreshQueued;
    WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
}