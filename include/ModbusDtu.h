#pragma once

#include <cstdint>
#include "Configuration.h"
#include <ModbusIP_ESP8266.h>
#include <Hoymiles.h>

class ModbusDtuClass {
public:
    void init();
    void loop();
    bool isrunning();

private:
    uint32_t _lastPublish;
    bool _isstarted = false;
};

extern ModbusDtuClass ModbusDtu;