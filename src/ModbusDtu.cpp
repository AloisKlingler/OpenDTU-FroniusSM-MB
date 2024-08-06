#include "ModbusDtu.h"
#include "Datastore.h"
#include "MessageOutput.h"

ModbusIP mb;

ModbusDtuClass ModbusDtu;

ModbusDtuClass::ModbusDtuClass()
    : _loopTask(Configuration.get().Dtu.PollInterval * TASK_SECOND, TASK_FOREVER, std::bind(&ModbusDtuClass::loop, this))
    , _modbusTask(TASK_IMMEDIATE, TASK_FOREVER, std::bind(&ModbusDtuClass::modbus, this))
{
}

void ModbusDtuClass::init(Scheduler& scheduler)
{
    scheduler.addTask(_loopTask);
    _loopTask.enable();
    scheduler.addTask(_modbusTask);
}

void ModbusDtuClass::modbus()
{
    mb.task();
}

void ModbusDtuClass::setup()
{
    if ((Configuration.get().Dtu.Serial) < 0x100000000000 || (Configuration.get().Dtu.Serial) > 0x199999999999) {
        MessageOutput.printf("Modbus: need a DTU Serial between 100000000000 and 199999999999 (currently configured: %llx)\r\n", Configuration.get().Dtu.Serial);
        _isstarted = false;
        return;
    }
    mb.server();
    mb.addHreg(0x9c40, 21365); //40000 Sunspec Id start
    mb.addHreg(0x9c41, 28243); //40001 Sunspec Id end
    mb.addHreg(0x9c42, 1);     //40002 SunSpec_DID
    mb.addHreg(0x9c43, 65);    //40003 SunSpec_Length
    const char *mfrname = Configuration.get().modbus.mfrname; //Manufacturer Name max. 32 chars
    for (uint8_t i = 0; i < 32; i += 2) {
        uint16_t value = 0;
        if (strlen(mfrname) > i) value = (mfrname[i] << 8) | (i + 1 < strlen(mfrname) ? mfrname[i + 1] : 0);
        mb.addHreg(0x9c44 + (i / 2), value); //40004 - 40019 Manufacturer name
        // MessageOutput.printf("Vendor: write %d to register %d\r\n", value, (0x9c44 + (i / 2)));
    }
    const char *modelname = Configuration.get().modbus.modelname;
    for (uint8_t i = 0; i < 32; i += 2) {
        uint16_t value = 0;
        if (strlen(modelname) > i) value = (modelname[i] << 8) | (i + 1 < strlen(modelname) ? modelname[i + 1] : 0);
        mb.addHreg(0x9c54 + (i / 2), value); //40020 - 40035 Device Model Name
        // MessageOutput.printf("Modelname: write %d to register %d\r\n", value, (0x9c54 + (i / 2)));
    }
    const char *options = Configuration.get().modbus.options;
    for (uint8_t i = 0; i < 16; i += 2) {
        uint16_t value = 0;
        if (strlen(options) > i) value = (options[i] << 8) | (i + 1 < strlen(options) ? options[i + 1] : 0);
        mb.addHreg(0x9c64 + (i / 2), value); //40036 - 40043 Options
        // MessageOutput.printf("Options: write %d to register %d\r\n", value, (0x9c64 + (i / 2)));
    }
    const char *version = Configuration.get().modbus.version;
    for (uint8_t i = 0; i < 16; i += 2) {
        uint16_t value = 0;
        if (strlen(version) > i) value = (version[i] << 8) | (i + 1 < strlen(version) ? version[i + 1] : 0);
        mb.addHreg(0x9c6c + (i / 2), value); //40044 - 40051 Version
        // MessageOutput.printf("Version: write %d to register %d\r\n", value, (0x9c6c + (i / 2)));
    }
    const char *serialconfig = Configuration.get().modbus.serial;
    if (!strlen(serialconfig)) {
        char serial[24];
        uint16_t *hexbytes = reinterpret_cast<uint16_t *>(serial);
        snprintf(serial,sizeof(serial),"%llx",(Configuration.get().Dtu.Serial));
        MessageOutput.printf("Modbus: init uses DTU Serial: %llx\r\n", Configuration.get().Dtu.Serial);
        MessageOutput.printf("Modbus: writing to init modbus registers %d %d %d %d %d %d\r\n", ntohs(hexbytes[0]), ntohs(hexbytes[1]), ntohs(hexbytes[2]), ntohs(hexbytes[3]), ntohs(hexbytes[4]), ntohs(hexbytes[5]));
        for (uint8_t i = 0; i < 6; i++) {
            mb.addHreg(0x9c74 + i, ntohs(hexbytes[i])); //40052 Serial Number start
        }
        mb.addHreg(0x9c7a, 0, 10); //40067 Serial Number end
    } else {
        for (uint8_t i = 0; i < 32; i += 2) {
            uint16_t value = 0;
            if (strlen(serialconfig) > i) value = (serialconfig[i] << 8) | (i + 1 < strlen(serialconfig) ? serialconfig[i + 1] : 0);
            mb.addHreg(0x9c74 + (i / 2), value); //40052 - 40067 Serial Number
            // MessageOutput.printf("Modbus: write %d to register %d\r\n", value, (0x9c74 + (i / 2)));
        }
    }
    mb.addHreg(0x9c84, 202);   //40068 DeviceAddress Modbus TCP Address: 202
    mb.addHreg(0x9c85, 213);   //40069 SunSpec_DID
    mb.addHreg(0x9c86, 124);   //40070 SunSpec_Length
    mb.addHreg(0x9c87, 0, 123);//40071 - 40194 smartmeter data
    mb.addHreg(0x9d03, 65535); //40195 end block identifier
    mb.addHreg(0x9d04, 0);     //40196
    _isstarted = true;
}

void ModbusDtuClass::loop()
{
    _loopTask.setInterval(Configuration.get().Dtu.PollInterval * TASK_SECOND);

    if (!(Configuration.get().modbus.modbus_tcp_enabled)) return;

    if (!Hoymiles.isAllRadioIdle()) {
         _loopTask.forceNextIteration();
         return;
    }

    if (!_isstarted) {
        if (!(Configuration.get().modbus.modbus_delaystart) || (Datastore.getIsAllEnabledReachable() && Datastore.getTotalAcYieldTotalEnabled() != 0)) {
            MessageOutput.printf("Modbus: starting server ... \r\n");
            ModbusDtu.setup();
            _modbusTask.enable();
        } else {
            MessageOutput.printf("Modbus: not initializing yet! (Total Yield = 0 or not all configured inverters reachable)\r\n");
            return;
        }
    }

    if (!(Datastore.getIsAllEnabledReachable()) || !(Datastore.getTotalAcYieldTotalEnabled() != 0) || (!_isstarted) || !(Configuration.get().modbus.modbus_delaystart)) {
        MessageOutput.printf("Modbus: not updating registers! (Total Yield = 0 or not all configured inverters reachable)\r\n");
        return;
    } else {
        float value;
        uint16_t *hexbytes = reinterpret_cast<uint16_t *>(&value);
        value = (Datastore.getTotalAcPowerEnabled()*-1);
        // MessageOutput.printf("Modbus: write %.2f to 40097 and 40098\r\n", value);
        mb.Hreg(0x9ca1, hexbytes[1]);
        mb.Hreg(0x9ca2, hexbytes[0]);
        value = (Datastore.getTotalAcYieldTotalEnabled()*1000);
        if (value > _lasttotal) {
            _lasttotal = value;
            // MessageOutput.printf("Modbus: write %.2f to 40129 and 40130\r\n", value);
            mb.Hreg(0x9cc1, hexbytes[1]);
            mb.Hreg(0x9cc2, hexbytes[0]);
        }

        if (Hoymiles.getNumInverters() == 1) {
            // MessageOutput.printf("Modbus: Start additional SM Information\r\n");
            auto inv = Hoymiles.getInverterByPos(0);
            if (inv != nullptr) {
                for (auto& t : inv->Statistics()->getChannelTypes()) {
                    if (t == TYPE_DC) {
                        value = (inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_IAC));
                        mb.Hreg(0x9c87, hexbytes[1]);
                        mb.Hreg(0x9c88, hexbytes[0]);
                        value = ((inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_IAC_1) != 0 ? inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_IAC_1) : inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_IAC)));
                        mb.Hreg(0x9c89, hexbytes[1]);
                        mb.Hreg(0x9c8a, hexbytes[0]);
                        value = (inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_IAC_2));
                        mb.Hreg(0x9c8b, hexbytes[1]);
                        mb.Hreg(0x9c8c, hexbytes[0]);
                        value = (inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_IAC_3));
                        mb.Hreg(0x9c8d, hexbytes[1]);
                        mb.Hreg(0x9c8e, hexbytes[0]);
                        value = (inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_UAC_1N));
                        mb.Hreg(0x9c8f, hexbytes[1]);
                        mb.Hreg(0x9c90, hexbytes[0]);
                        value = ((inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_UAC_1N) != 0 ? inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_UAC_1N) : inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_UAC)));
                        mb.Hreg(0x9c91, hexbytes[1]);
                        mb.Hreg(0x9c92, hexbytes[0]);
                        value = (inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_UAC_2N));
                        mb.Hreg(0x9c93, hexbytes[1]);
                        mb.Hreg(0x9c94, hexbytes[0]);
                        value = (inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_UAC_3N));
                        mb.Hreg(0x9c95, hexbytes[1]);
                        mb.Hreg(0x9c96, hexbytes[0]);
                        value = (inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_UAC));
                        mb.Hreg(0x9c97, hexbytes[1]);
                        mb.Hreg(0x9c98, hexbytes[0]);
                        value = (inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_UAC_12));
                        mb.Hreg(0x9c99, hexbytes[1]);
                        mb.Hreg(0x9c9a, hexbytes[0]);
                        value = (inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_UAC_23));
                        mb.Hreg(0x9c9b, hexbytes[1]);
                        mb.Hreg(0x9c9c, hexbytes[0]);
                        value = (inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_UAC_31));
                        mb.Hreg(0x9c9d, hexbytes[1]);
                        mb.Hreg(0x9c9e, hexbytes[0]);
                        value = (inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_F));
                        mb.Hreg(0x9c9f, hexbytes[1]);
                        mb.Hreg(0x9ca0, hexbytes[0]);
                        // value = (inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_PAC)*-1); //done above already!
                        // mb.Hreg(0x9ca1, hexbytes[1]); //done above already!
                        // mb.Hreg(0x9ca2, hexbytes[0]); //done above already!
                        value = ((inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_IAC_1) != 0) ? ((inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_IAC_1)) * (inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_UAC_1N)) *-1) : ((inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_IAC)) * (inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_UAC)) *-1));
                        mb.Hreg(0x9ca3, hexbytes[1]);
                        mb.Hreg(0x9ca4, hexbytes[0]);
                        value = ((inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_IAC_2)) * (inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_UAC_2N)) *-1);
                        mb.Hreg(0x9ca5, hexbytes[1]);
                        mb.Hreg(0x9ca6, hexbytes[0]);
                        value = ((inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_IAC_3)) * (inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_UAC_3N)) *-1);
                        mb.Hreg(0x9ca7, hexbytes[1]);
                        mb.Hreg(0x9ca8, hexbytes[0]);
                        // mb.Hreg(0x9ca9, 0);
                        // mb.Hreg(0x9caa, 0);
                        // mb.Hreg(0x9cab, 0);
                        // mb.Hreg(0x9cac, 0);
                        // mb.Hreg(0x9cad, 0);
                        // mb.Hreg(0x9cae, 0);
                        // mb.Hreg(0x9caf, 0);
                        // mb.Hreg(0x9cb0, 0);
                        value = (inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_Q));
                        mb.Hreg(0x9cb1, hexbytes[1]);
                        mb.Hreg(0x9cb2, hexbytes[0]);
                        // mb.Hreg(0x9cb3, 0);
                        // mb.Hreg(0x9cb4, 0);
                        // mb.Hreg(0x9cb5, 0);
                        // mb.Hreg(0x9cb6, 0);
                        // mb.Hreg(0x9cb7, 0);
                        // mb.Hreg(0x9cb8, 0);
                        value = (inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_PF));
                        mb.Hreg(0x9cb9, hexbytes[1]);
                        mb.Hreg(0x9cba, hexbytes[0]);
                        // mb.Hreg(0x9cbb, 0);
                        // mb.Hreg(0x9cbc, 0);
                        // mb.Hreg(0x9cbd, 0);
                        // mb.Hreg(0x9cbe, 0);
                        // mb.Hreg(0x9cbf, 0);
                        // mb.Hreg(0x9cc0, 0);
                        // value = (inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_YT)*1000); //done above already!
                        // if (value > _lasttotal) {
                            // _lasttotal = value;
                            // mb.Hreg(0x9cc1, hexbytes[1]); //done above already!
                            // mb.Hreg(0x9cc2, hexbytes[0]); //done above already!
                        // }
                    }
                }
            }
            // MessageOutput.printf("Modbus: End additional SM Information\r\n");
        }
    }
}
