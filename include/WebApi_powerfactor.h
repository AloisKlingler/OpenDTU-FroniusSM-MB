// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <ESPAsyncWebServer.h>
#include <TaskSchedulerDeclarations.h>

class WebApiPowerFactorClass {
public:
    void init(AsyncWebServer& server, Scheduler& scheduler);

private:
    void onPowerFactorStatus(AsyncWebServerRequest* request);
    void onPowerFactorPost(AsyncWebServerRequest* request);
};
