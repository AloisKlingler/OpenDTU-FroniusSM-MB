// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024-2026 Thomas Basler and others
 */
#include "WebApi_yieldtotal.h"
#include "WebApi.h"
#include "WebApi_errors.h"
#include <AsyncJson.h>
#include <Hoymiles.h>

void WebApiYieldTotalClass::init(AsyncWebServer& server, Scheduler& scheduler)
{
    using std::placeholders::_1;

    server.on("/api/yieldtotal/status", HTTP_GET, static_cast<ArRequestHandlerFunction>(std::bind(&WebApiYieldTotalClass::onYieldTotalStatus, this, _1)));
    server.on("/api/yieldtotal/config", HTTP_POST, static_cast<ArRequestHandlerFunction>(std::bind(&WebApiYieldTotalClass::onYieldTotalPost, this, _1)));
}

void WebApiYieldTotalClass::onYieldTotalStatus(AsyncWebServerRequest* request)
{
    if (!WebApi.checkCredentialsReadonly(request)) {
        return;
    }

    AsyncJsonResponse* response = new AsyncJsonResponse();
    auto& root = response->getRoot();

    for (uint8_t i = 0; i < Hoymiles.getNumInverters(); i++) {
        auto inv = Hoymiles.getInverterByPos(i);

        String serial = inv->serialString();
        String state = "Unknown";

        if (inv->getYieldTotalSetRunning()) {
            state = "Pending";
        } else {
            const LastCommandSuccess status = inv->getLastYieldTotalSetSuccess();
            if (status == LastCommandSuccess::CMD_OK) {
                state = "Ok";
            } else if (status == LastCommandSuccess::CMD_NOK) {
                state = "Failure";
            } else {
                state = "Pending";
            }
        }
        root[serial]["yield_total_set_status"] = state;
    }

    WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
}

void WebApiYieldTotalClass::onYieldTotalPost(AsyncWebServerRequest* request)
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

    if (!(root["serial"].is<String>() && root["values"].is<JsonArray>())) {
        retMsg["message"] = "Values are missing!";
        retMsg["code"] = WebApiError::GenericValueMissing;
        WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
        return;
    }

    const uint64_t serial = strtoll(root["serial"].as<String>().c_str(), NULL, 16);
    if (serial == 0) {
        retMsg["message"] = "Serial must be a number > 0!";
        retMsg["code"] = WebApiError::YieldTotalSerialZero;
        WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
        return;
    }

    JsonArray values = root["values"].as<JsonArray>();
    // 1-in-1 sends one value, 2-in-1 sends two, 4-in-1 sends all four.
    if (values.size() != 1 && values.size() != 2 && values.size() != 4) {
        retMsg["message"] = "Values must contain 1, 2 or 4 entries!";
        retMsg["code"] = WebApiError::YieldTotalInvalidValueCount;
        WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
        return;
    }

    auto inv = Hoymiles.getInverterBySerial(serial);
    if (inv == nullptr) {
        retMsg["message"] = "Invalid inverter specified!";
        retMsg["code"] = WebApiError::YieldTotalInvalidInverter;
        WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
        return;
    }

    uint32_t valuesWh[4] = { 0, 0, 0, 0 };
    uint8_t idx = 0;
    for (JsonVariant v : values) {
        valuesWh[idx++] = v.as<uint32_t>();
    }

    if (!inv->sendYieldTotalSetRequest(valuesWh, static_cast<uint8_t>(values.size()))) {
        retMsg["message"] = "Failed to enqueue yield total write. Is \"Commands enabled\" set for this inverter?";
        retMsg["code"] = WebApiError::YieldTotalWriteInProgress;
        WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
        return;
    }

    retMsg["type"] = "success";
    retMsg["message"] = "Yield total write queued!";
    retMsg["code"] = WebApiError::YieldTotalWriteQueued;

    WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
}
