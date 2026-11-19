// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 Thomas Basler and others
 */
#include "WebApi_powerfactor.h"
#include "WebApi.h"
#include "WebApi_errors.h"
#include "helper.h"
#include <AsyncJson.h>
#include <Hoymiles.h>
#include <cmath>

void WebApiPowerFactorClass::init(AsyncWebServer& server, Scheduler& /*scheduler*/)
{
    using std::placeholders::_1;

    server.on("/api/powerfactor/status", HTTP_GET, static_cast<ArRequestHandlerFunction>(std::bind(&WebApiPowerFactorClass::onPowerFactorStatus, this, _1)));
    server.on("/api/powerfactor/config", HTTP_POST, static_cast<ArRequestHandlerFunction>(std::bind(&WebApiPowerFactorClass::onPowerFactorPost, this, _1)));
}

void WebApiPowerFactorClass::onPowerFactorStatus(AsyncWebServerRequest* request)
{
    if (!WebApi.checkCredentialsReadonly(request)) {
        return;
    }

    AsyncJsonResponse* response = new AsyncJsonResponse();
    auto& root = response->getRoot();

    for (uint8_t i = 0; i < Hoymiles.getNumInverters(); i++) {
        auto inv = Hoymiles.getInverterByPos(i);

        String serial = inv->serialString();

        root[serial]["power_factor"] = inv->SystemConfigPara()->getPowerFactor();

        LastCommandSuccess status = inv->SystemConfigPara()->getLastPowerFactorCommandSuccess();
        String pfStatus = "Unknown";
        if (status == LastCommandSuccess::CMD_OK) {
            pfStatus = "Ok";
        } else if (status == LastCommandSuccess::CMD_NOK) {
            pfStatus = "Failure";
        } else if (status == LastCommandSuccess::CMD_PENDING) {
            pfStatus = "Pending";
        }
        root[serial]["power_factor_set_status"] = pfStatus;
    }

    WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
}

void WebApiPowerFactorClass::onPowerFactorPost(AsyncWebServerRequest* request)
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
            && root["power_factor"].is<float>()
            && root["power_factor_type"].is<uint16_t>())) {
        retMsg["message"] = "Values are missing!";
        retMsg["code"] = WebApiError::GenericValueMissing;
        WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
        return;
    }

    const uint64_t serial = strtoll(root["serial"].as<String>().c_str(), NULL, 16);

    if (serial == 0) {
        retMsg["message"] = "Serial must be a number > 0!";
        retMsg["code"] = WebApiError::PowerFactorSerialZero;
        WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
        return;
    }

    const float powerFactor = root["power_factor"].as<float>();
    const float powerFactorMag = std::abs(powerFactor);
    if (powerFactorMag < 0.8f || powerFactorMag > 1.0f) {
        retMsg["message"] = "Power factor out of range!";
        retMsg["code"] = WebApiError::PowerFactorInvalidLimit;
        retMsg["param"]["min"] = 0.8;
        retMsg["param"]["max"] = 1;
        WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
        return;
    }

    if (!(root["power_factor_type"].as<uint16_t>() < PowerLimitControlType::PowerLimitControl_Max)) {
        retMsg["message"] = "Invalid type specified!";
        retMsg["code"] = WebApiError::PowerFactorInvalidType;
        WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
        return;
    }

    PowerLimitControlType type = root["power_factor_type"].as<PowerLimitControlType>();

    auto inv = Hoymiles.getInverterBySerial(serial);
    if (inv == nullptr) {
        retMsg["message"] = "Invalid inverter specified!";
        retMsg["code"] = WebApiError::PowerFactorInvalidInverter;
        WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
        return;
    }

    inv->sendPowerFactorControlRequest(powerFactor, type);

    retMsg["type"] = "success";
    retMsg["message"] = "Settings saved!";
    retMsg["code"] = WebApiError::GenericSuccess;

    WebApi.sendJsonResponse(request, response, __FUNCTION__, __LINE__);
}
