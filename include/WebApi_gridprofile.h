// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <ESPAsyncWebServer.h>
#include <TaskSchedulerDeclarations.h>

class WebApiGridProfileClass {
public:
    void init(AsyncWebServer& server, Scheduler& scheduler);

private:
    void onGridProfileStatus(AsyncWebServerRequest* request);
    void onGridProfileRawdata(AsyncWebServerRequest* request);
    void onGridProfileKnownPresets(AsyncWebServerRequest* request);
    void onGridProfileWriteStatus(AsyncWebServerRequest* request);
    void onGridProfileWritePost(AsyncWebServerRequest* request);
    void onGridProfileAbortPost(AsyncWebServerRequest* request);
    void onGridProfileRefreshPost(AsyncWebServerRequest* request);
};
