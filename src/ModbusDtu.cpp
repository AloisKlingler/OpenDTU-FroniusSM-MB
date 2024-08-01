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
    _modbusTask.enable();
}

void ModbusDtuClass::modbus()
{
    if (_isstarted) mb.task();
}

void ModbusDtuClass::setup()
{
    if ((Configuration.get().Dtu.Serial) < 0x100000000000 || (Configuration.get().Dtu.Serial) > 0x199999999999) {
        MessageOutput.printf("Fronius SM Simulation: need a DTU Serial between 100000000000 and 199999999999 (currently configured: %llx)\r\n", Configuration.get().Dtu.Serial);
        _isstarted = false;
        return;
    }
    mb.server();
    mb.addHreg(0x9c40, 21365); //40000
    mb.addHreg(0x9c41, 28243);
    mb.addHreg(0x9c42, 1);
    mb.addHreg(0x9c43, 65);
    mb.addHreg(0x9c44, 18034); //40004 Manufacturer start 4672 == Fr
    mb.addHreg(0x9c45, 28526); //                         6f6e == on
    mb.addHreg(0x9c46, 26997); //                         6975 == iu
    mb.addHreg(0x9c47, 29440); //                         7300 == s
    mb.addHreg(0x9c48, 0, 12); //40019 Manufacturer end
    mb.addHreg(0x9c54, 21357); //40020 Device Model start 536d == Sm
    mb.addHreg(0x9c55, 24946); //                         6172 == ar
    mb.addHreg(0x9c56, 29728); //                         7420 == t
    mb.addHreg(0x9c57, 19813); //                         4d65 == Me
    mb.addHreg(0x9c58, 29797); //                         7465 == te
    mb.addHreg(0x9c59, 29216); //                         7220 == r
    mb.addHreg(0x9c5a, 21587); //                         5453 == TS
    mb.addHreg(0x9c5b, 8246);  //                         2036 ==  6
    mb.addHreg(0x9c5c, 13633); //                         3541 == 5A
    mb.addHreg(0x9c5d, 11571); //                         2d33 == -3
    mb.addHreg(0x9c5e, 0, 6); //40035 Device Model end
    mb.addHreg(0x9c64, 15472); //40036 Options start 3c70 == <p
    mb.addHreg(0x9c65, 29289); //                    7269 == ri
    mb.addHreg(0x9c66, 28001); //                    6d61 == ma
    mb.addHreg(0x9c67, 29305); //                    7279 == ry
    mb.addHreg(0x9c68, 15872); //                    3E00 == >
    mb.addHreg(0x9c69, 0, 3); //40043 Options end
    mb.addHreg(0x9c6c, 12590); //40044 Software Version start 312e == 1.
    mb.addHreg(0x9c6d, 13056); //                             3300 == 3
    mb.addHreg(0x9c6e, 0, 6); //40051 Software Version N/A end
    char buff[24];
    uint16_t *hexbytes = reinterpret_cast<uint16_t *>(buff);
    snprintf(buff,sizeof(buff),"%llx",(Configuration.get().Dtu.Serial));
    MessageOutput.printf("Fronius SM Simulation: init uses DTU Serial: %llx\r\n", Configuration.get().Dtu.Serial);
    MessageOutput.printf("Fronius SM Simulation: writing to init modbus registers %d %d %d %d %d %d\r\n", ntohs(hexbytes[0]), ntohs(hexbytes[1]), ntohs(hexbytes[2]), ntohs(hexbytes[3]), ntohs(hexbytes[4]), ntohs(hexbytes[5]));
    mb.addHreg(0x9c74, ntohs(hexbytes[0])); //40052 Serial Number start
    mb.addHreg(0x9c75, ntohs(hexbytes[1]));
    mb.addHreg(0x9c76, ntohs(hexbytes[2]));
    mb.addHreg(0x9c77, ntohs(hexbytes[3]));
    mb.addHreg(0x9c78, ntohs(hexbytes[4]));
    mb.addHreg(0x9c79, ntohs(hexbytes[5]));
    mb.addHreg(0x9c7a, 0, 10); //40067 Serial Number end
    mb.addHreg(0x9c84, 202); //40068 Modbus TCP Address: 202
    mb.addHreg(0x9c85, 213); //40069
    mb.addHreg(0x9c86, 124); //40070
    mb.addHreg(40071, 0, 123); //40071 - 40194 smartmeter data
    mb.addHreg(0x9d03, 65535); //40195 end block identifier
    mb.addHreg(0x9d04, 0); //40196
    _isstarted = true;
}

void ModbusDtuClass::loop()
{
    _loopTask.setInterval(Configuration.get().Dtu.PollInterval * TASK_SECOND);

    if (!(Configuration.get().modbus.Fronius_SM_Simulation_Enabled)) return;

    if (!Hoymiles.isAllRadioIdle()) {
         _loopTask.forceNextIteration();
         return;
    }

    if (!_isstarted) {
        if (Datastore.getIsAllEnabledReachable() && Datastore.getTotalAcYieldTotalEnabled() != 0) {
            ModbusDtu.setup();
        } else {
            MessageOutput.printf("Fronius SM Simulation: not initializing yet! (Total Yield = 0 or not all configured inverters reachable).\r\n");
            return;
        }
    }

    if (!(Datastore.getIsAllEnabledReachable()) || !(Datastore.getTotalAcYieldTotalEnabled() != 0) || (!_isstarted)) {
        MessageOutput.printf("Fronius SM Simulation: not updating registers! (Total Yield = 0 or not all configured inverters reachable).\r\n");
        return;
    } else {
        float value;
        uint16_t *hexbytes = reinterpret_cast<uint16_t *>(&value);
        value = (Datastore.getTotalAcPowerEnabled()*-1);
        //MessageOutput.printf("Fronius SM Simulation: modbus write %.2f to 40097 and 40098\r\n", value);
        mb.Hreg(0x9ca1, hexbytes[1]);
        mb.Hreg(0x9ca2, hexbytes[0]);
        value = (Datastore.getTotalAcYieldTotalEnabled()*1000);
        if (value > _lasttotal) {
            _lasttotal = value;
            //MessageOutput.printf("Fronius SM Simulation: modbus write %.2f to 40129 and 40130\r\n", value);
            mb.Hreg(0x9cc1, hexbytes[1]);
            mb.Hreg(0x9cc2, hexbytes[0]);
        }
        /*
        if (Hoymiles.getNumInverters() == 1) {
            auto inv = Hoymiles.getInverterByPos(0);
            if (inv != nullptr) {
                for (auto& t : inv->Statistics()->getChannelTypes()) {
                    if (t == TYPE_DC) {
                        float value;
                        uint16_t *hexbytes = reinterpret_cast<uint16_t *>(&value);
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
                        value = (inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_PAC)*-1);
                        mb.Hreg(0x9ca1, hexbytes[1]);
                        mb.Hreg(0x9ca2, hexbytes[0]);
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
                        // value = (inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_Q)); // sometimes irrealistic values
                        // mb.Hreg(0x9cb1, hexbytes[1]);
                        // mb.Hreg(0x9cb1, hexbytes[0]);
                        // mb.Hreg(0x9cb2, 0);
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
                        value = (inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_YT)*1000);
                        if (value != 0){
                            mb.Hreg(0x9cc1, hexbytes[1]);
                            mb.Hreg(0x9cc2, hexbytes[0]);
                        }
                    }
                }
            }
        } else {
            float value;
            uint16_t *hexbytes = reinterpret_cast<uint16_t *>(&value);
            value = (Datastore.getTotalAcPowerEnabled()*-1);
            mb.Hreg(0x9ca1, hexbytes[1]);
            mb.Hreg(0x9ca2, hexbytes[0]);
            value = (Datastore.getTotalAcYieldTotalEnabled()*1000);
            if (value > _lasttotal && Datastore.getIsAllEnabledReachable()) {
                _lasttotal = value;
                mb.Hreg(0x9cc1, hexbytes[1]);
                mb.Hreg(0x9cc2, hexbytes[0]);
            }
        }*/
    }
}
