// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 Thomas Basler and others
 */
#include "WebApi_reactivepower.h"
#include "WebApi.h"
#include "WebApi_errors.h"
#include "defaults.h"
#include "helper.h"
#include <AsyncJson.h>
#include <Hoymiles.h>

void WebApiReactivePowerClass::init(AsyncWebServer& server, Scheduler& /*scheduler*/)
{
    using std::placeholders::_1;

    server.on("/api/reactivepower/status", HTTP_GET, static_cast<ArRequestHandlerFunction>(std::bind(&WebApiReactivePowerClass::onReactivePowerStatus, this, _1)));
    server.on("/api/reactivepower/config", HTTP_POST, static_cast<ArRequestHandlerFunction>(std::bind(&WebApiReactivePowerClass::onReactivePowerPost, this, _1)));
}

void WebApiReactivePowerClass::onReactivePowerStatus(AsyncWebServerRequest* request)
{
    if (!WebApi.checkCredentialsReadonly(request)) {
        return;
    }

    AsyncJsonResponse* response = new AsyncJsonResponse();
    auto& root = response->getRoot();

    for (uint8_t i = 0; i < Hoymiles.getNumInverters(); i++) {
        auto inv = Hoymiles.getInverterByPos(i);

        String serial = inv->serialString();

        root[serial]["max_power"] = inv->DevInfo()->getMaxPower();
        root[serial]["reactive_relative"] = inv->SystemConfigPara()->getReactivePowerPercent();

        LastCommandSuccess status = inv->SystemConfigPara()->getLastReactivePowerCommandSuccess();
        String reactiveStatus = "Unknown";
        if (status == LastCommandSuccess::CMD_OK) {
            reactiveStatus = "Ok";
        } else if (status == LastCommandSuccess::CMD_NOK) {
            reactiveStatus = "Failure";
        } else if (status == LastCommandSuccess::CMD_PENDING) {
            reactiveStatus = "Pending";
        }
        root[serial]["reactive_set_status"] = reactiveStatus;
    }

    WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
}

void WebApiReactivePowerClass::onReactivePowerPost(AsyncWebServerRequest* request)
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

    if (!(root["serial"].is<String>()
            && root["reactive_value"].is<float>()
            && root["reactive_type"].is<uint16_t>())) {
        retMsg["message"] = "Values are missing!";
        retMsg["code"] = WebApiError::GenericValueMissing;
        WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
        return;
    }

    const uint64_t serial = strtoll(root["serial"].as<String>().c_str(), NULL, 16);

    if (serial == 0) {
        retMsg["message"] = "Serial must be a number > 0!";
        retMsg["code"] = WebApiError::ReactivePowerSerialZero;
        WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
        return;
    }

    const float reactiveValue = root["reactive_value"].as<float>();
    if (reactiveValue < -MAX_INVERTER_LIMIT || reactiveValue > MAX_INVERTER_LIMIT) {
        retMsg["message"] = "Reactive power out of range!";
        retMsg["code"] = WebApiError::ReactivePowerInvalidLimit;
        retMsg["param"]["max"] = MAX_INVERTER_LIMIT;
        WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
        return;
    }

    if (!(root["reactive_type"].as<uint16_t>() < PowerLimitControlType::PowerLimitControl_Max)) {
        retMsg["message"] = "Invalid type specified!";
        retMsg["code"] = WebApiError::ReactivePowerInvalidType;
        WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
        return;
    }

    PowerLimitControlType type = root["reactive_type"].as<PowerLimitControlType>();

    auto inv = Hoymiles.getInverterBySerial(serial);
    if (inv == nullptr) {
        retMsg["message"] = "Invalid inverter specified!";
        retMsg["code"] = WebApiError::ReactivePowerInvalidInverter;
        WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
        return;
    }

    inv->sendReactivePowerControlRequest(reactiveValue, type);

    retMsg["type"] = "success";
    retMsg["message"] = "Settings saved!";
    retMsg["code"] = WebApiError::GenericSuccess;

    WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
}
