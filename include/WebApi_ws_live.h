// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include "Configuration.h"
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <Hoymiles.h>
#include <TaskSchedulerDeclarations.h>

class WebApiWsLiveClass {
public:
    WebApiWsLiveClass();
    void init(AsyncWebServer& server, Scheduler& scheduler);

private:
<<<<<<< HEAD
    void generateJsonResponse(JsonVariant& root);
    void generateJsonResponseTotals(JsonVariant& root);
    void addField(JsonObject& root, uint8_t idx, std::shared_ptr<InverterAbstract> inv, ChannelType_t type, ChannelNum_t channel, FieldId_t fieldId, String topic = "");
    void addTotalField(JsonObject& root, String name, float value, String unit, uint8_t digits);
=======
    static void generateInverterCommonJsonResponse(JsonObject& root, std::shared_ptr<InverterAbstract> inv);
    static void generateInverterChannelJsonResponse(JsonObject& root, std::shared_ptr<InverterAbstract> inv);
    static void generateCommonJsonResponse(JsonVariant& root);

    static void addField(JsonObject& root, std::shared_ptr<InverterAbstract> inv, const ChannelType_t type, const ChannelNum_t channel, const FieldId_t fieldId, String topic = "");
    static void addTotalField(JsonObject& root, const String& name, const float value, const String& unit, const uint8_t digits);

>>>>>>> e541a885f51dcd3b88195ff4fa01a0f413889807
    void onLivedataStatus(AsyncWebServerRequest* request);
    void onLivedataTotals(AsyncWebServerRequest* request);
    void onWebsocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len);

    AsyncWebSocket _ws;

    uint32_t _lastPublishStats[INV_MAX_COUNT] = { 0 };

    std::mutex _mutex;

    Task _wsCleanupTask;
    void wsCleanupTaskCb();

    Task _sendDataTask;
    void sendDataTaskCb();
};
