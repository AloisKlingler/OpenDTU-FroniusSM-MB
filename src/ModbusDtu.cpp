#include "ModbusDtu.h"
#include "Datastore.h"

ModbusIP mb;

ModbusDtuClass ModbusDtu;

void ModbusDtuClass::init()
{
    mb.server();	
    const CONFIG_T& config = Configuration.get();
    // mb.addHreg(0x2000, (config.Dtu_Serial >> 32) & 0xFFFF);
    // mb.addHreg(0x2001, (config.Dtu_Serial >> 16) & 0xFFFF);
    // mb.addHreg(0x2002, (config.Dtu_Serial) & 0xFFFF);
    // mb.addHreg(0x2003, 0);
    // mb.addHreg(0x2004, 0);
    // mb.addHreg(0x2005, 0);
    mb.addHreg(0x9c40, 21365); //40000 or 40001
    mb.addHreg(0x9c41, 28243); 
    mb.addHreg(0x9c42, 1);
    mb.addHreg(0x9c43, 65);
    mb.addHreg(0x9c44, 18034); //40005 Manufacturer start 4672 == Fr
    mb.addHreg(0x9c45, 28526); //                         6f6e == on
    mb.addHreg(0x9c46, 26997); //                         6975 == iu
    mb.addHreg(0x9c47, 29440); //                         7300 == s
    mb.addHreg(0x9c48, 0); 
    mb.addHreg(0x9c49, 0); 
    mb.addHreg(0x9c4a, 0); 
    mb.addHreg(0x9c4b, 0); 
    mb.addHreg(0x9c4c, 0); 
    mb.addHreg(0x9c4d, 0); 
    mb.addHreg(0x9c4e, 0); 
    mb.addHreg(0x9c4f, 0); 
    mb.addHreg(0x9c50, 0); 
    mb.addHreg(0x9c51, 0); 
    mb.addHreg(0x9c52, 0);  
    mb.addHreg(0x9c53, 0); //40020 Manufacturer end
    mb.addHreg(0x9c54, 21357); //40021 Device Model start 536d == Sm
    mb.addHreg(0x9c55, 24946); //                         6172 == ar
    mb.addHreg(0x9c56, 29728); //                         7420 == t 
    mb.addHreg(0x9c57, 19813); //                         4d65 == Me
    mb.addHreg(0x9c58, 29797); //                         7465 == te
    mb.addHreg(0x9c59, 29216); //                         7220 == r 
    mb.addHreg(0x9c5a, 21587); //                         5453 == TS
    mb.addHreg(0x9c5b, 8246);  //                         2036 ==  6
    mb.addHreg(0x9c5c, 13633); //                         3541 == 5A
    mb.addHreg(0x9c5d, 11571); //                         2d33 == -3
    mb.addHreg(0x9c5e, 0); 
    mb.addHreg(0x9c5f, 0); 
    mb.addHreg(0x9c60, 0); 
    mb.addHreg(0x9c61, 0); 
    mb.addHreg(0x9c62, 0);  
    mb.addHreg(0x9c63, 0); //40036 Device Model end
    mb.addHreg(0x9c64, 15472); //40037 Options start 3c70 == <p
    mb.addHreg(0x9c65, 29289); //                    7269 == ri
    mb.addHreg(0x9c66, 28001); //                    6d61 == ma
    mb.addHreg(0x9c67, 29305); //                    7279 == ry
    mb.addHreg(0x9c68, 15872); //                    3E00 == >
    mb.addHreg(0x9c69, 0); 
    mb.addHreg(0x9c6a, 0); 
    mb.addHreg(0x9c6b, 0); //40044 Options end
    mb.addHreg(0x9c6c, 12590); //40045 Software Version start 312e == 1.
    mb.addHreg(0x9c6d, 13056); //                             3300 == 3
    mb.addHreg(0x9c6e, 0); 
    mb.addHreg(0x9c6f, 0); 
    mb.addHreg(0x9c70, 0); 
    mb.addHreg(0x9c71, 0); 
    mb.addHreg(0x9c72, 0);  
    mb.addHreg(0x9c73, 0); //40052 Software Version N/A end
    mb.addHreg(0x9c74, 12850); //40053 Serial Number start 3232 == 22
    mb.addHreg(0x9c75, 13362); //                          3432 == 42
    mb.addHreg(0x9c76, 13111); //                          3337 == 37
    mb.addHreg(0x9c77, 12851); //                          3233 == 23
    mb.addHreg(0x9c78, 13360); //                          3430 == 40
    mb.addHreg(0x9c79, 0); 
    mb.addHreg(0x9c7a, 0); 
    mb.addHreg(0x9c7b, 0); 
    mb.addHreg(0x9c7c, 0); 
    mb.addHreg(0x9c7d, 0); 
    mb.addHreg(0x9c7e, 0); 
    mb.addHreg(0x9c7f, 0); 
    mb.addHreg(0x9c80, 0); 
    mb.addHreg(0x9c81, 0); 
    mb.addHreg(0x9c82, 0);  
    mb.addHreg(0x9c83, 0); //40068 Serial Number end
    mb.addHreg(0x9c84, 202); //40069 Modbus TCP Address: 240
    mb.addHreg(0x9c85, 213); //40070
    mb.addHreg(0x9c86, 124); //40071
    mb.addHreg(0x9c87, 0); //40072 start smartmeter data
    mb.addHreg(0x9c88, 0); 
    mb.addHreg(0x9c89, 0);
    mb.addHreg(0x9c8a, 0);
    mb.addHreg(0x9c8b, 0);
    mb.addHreg(0x9c8c, 0);
    mb.addHreg(0x9c8d, 0);
    mb.addHreg(0x9c8e, 0);
    mb.addHreg(0x9c8f, 0); //40080
    mb.addHreg(0x9c90, 0); 
    mb.addHreg(0x9c91, 0); 
    mb.addHreg(0x9c92, 0);  
    mb.addHreg(0x9c93, 0); 
    mb.addHreg(0x9c94, 0); 
    mb.addHreg(0x9c95, 0); 
    mb.addHreg(0x9c96, 0); 
    mb.addHreg(0x9c97, 0); //40088 or 40089 Total Real Power
    mb.addHreg(0x9c98, 0); //40089
    mb.addHreg(0x9c99, 0); //40090
    mb.addHreg(0x9c9a, 0); 
    mb.addHreg(0x9c9b, 0); 
    mb.addHreg(0x9c9c, 0); 
    mb.addHreg(0x9c9d, 0); 
    mb.addHreg(0x9c9e, 0); 
    mb.addHreg(0x9c9f, 0); 
    mb.addHreg(0x9ca0, 0); 
    mb.addHreg(0x9ca1, 0); 
    mb.addHreg(0x9ca2, 0); 
    mb.addHreg(0x9ca3, 0); //40100
    mb.addHreg(0x9ca4, 0); 
    mb.addHreg(0x9ca5, 0); 
    mb.addHreg(0x9ca6, 0); 
    mb.addHreg(0x9ca7, 0); 
    mb.addHreg(0x9ca8, 0); 
    mb.addHreg(0x9ca9, 0); 
    mb.addHreg(0x9caa, 0); 
    mb.addHreg(0x9cab, 0); //40108 Total Exported Apparent Energy
    mb.addHreg(0x9cac, 0); //40109 Total Exported Apparent Energy
    mb.addHreg(0x9cad, 0); //40110
    mb.addHreg(0x9cae, 0); 
    mb.addHreg(0x9caf, 0); 
    mb.addHreg(0x9cb0, 0); 
    mb.addHreg(0x9cb1, 0); 
    mb.addHreg(0x9cb2, 0); 
    mb.addHreg(0x9cb3, 0); //40116 Total Imported Apparent Energy
    mb.addHreg(0x9cb4, 0); //40117 Total Imported Apparent Energy
    mb.addHreg(0x9cb5, 0); 
    mb.addHreg(0x9cb6, 0); 
    mb.addHreg(0x9cb7, 0); //40120
    mb.addHreg(0x9cb8, 0); 
    mb.addHreg(0x9cb9, 0); 
    mb.addHreg(0x9cba, 0); 
    mb.addHreg(0x9cbb, 0); 
    mb.addHreg(0x9cbc, 0); 
    mb.addHreg(0x9cbd, 0); 
    mb.addHreg(0x9cbe, 0); 
    mb.addHreg(0x9cbf, 0); 
    mb.addHreg(0x9cc0, 0); 
    mb.addHreg(0x9cc1, 0); //40130
    mb.addHreg(0x9cc2, 0); 
    mb.addHreg(0x9cc3, 0); 
    mb.addHreg(0x9cc4, 0); 
    mb.addHreg(0x9cc5, 0); 
    mb.addHreg(0x9cc6, 0); 
    mb.addHreg(0x9cc7, 0); 
    mb.addHreg(0x9cc8, 0); 
    mb.addHreg(0x9cc9, 0); 
    mb.addHreg(0x9cca, 0); 
    mb.addHreg(0x9ccb, 0); //40140
    mb.addHreg(0x9ccc, 0); 
    mb.addHreg(0x9ccd, 0); 
    mb.addHreg(0x9cce, 0); 
    mb.addHreg(0x9ccf, 0); 
    mb.addHreg(0x9cd0, 0); 
    mb.addHreg(0x9cd1, 0); 
    mb.addHreg(0x9cd2, 0); 
    mb.addHreg(0x9cd3, 0); 
    mb.addHreg(0x9cd4, 0); 
    mb.addHreg(0x9cd5, 0); //40150
    mb.addHreg(0x9cd6, 0); 
    mb.addHreg(0x9cd7, 0); 
    mb.addHreg(0x9cd8, 0); 
    mb.addHreg(0x9cd9, 0); 
    mb.addHreg(0x9cda, 0); 
    mb.addHreg(0x9cdb, 0); 
    mb.addHreg(0x9cdc, 0); 
    mb.addHreg(0x9cdd, 0); 
    mb.addHreg(0x9cde, 0); 
    mb.addHreg(0x9cdf, 0); //40160
    mb.addHreg(0x9ce0, 0); //40161 end smartmeter data
    mb.addHreg(0x9ce1, 0); //40162 trailing 0 start
    mb.addHreg(0x9ce2, 0); 
    mb.addHreg(0x9ce3, 0);
    mb.addHreg(0x9ce4, 0);
    mb.addHreg(0x9ce5, 0);
    mb.addHreg(0x9ce6, 0);
    mb.addHreg(0x9ce7, 0);
    mb.addHreg(0x9ce8, 0);
    mb.addHreg(0x9ce9, 0);
    mb.addHreg(0x9cea, 0);
    mb.addHreg(0x9ceb, 0);
    mb.addHreg(0x9cec, 0);
    mb.addHreg(0x9ced, 0);
    mb.addHreg(0x9cee, 0);
    mb.addHreg(0x9cef, 0);
    mb.addHreg(0x9cf0, 0);
    mb.addHreg(0x9cf1, 0);
    mb.addHreg(0x9cf2, 0);
    mb.addHreg(0x9cf3, 0);
    mb.addHreg(0x9cf4, 0);
    mb.addHreg(0x9cf5, 0);
    mb.addHreg(0x9cf6, 0);
    mb.addHreg(0x9cf7, 0);
    mb.addHreg(0x9cf8, 0);
    mb.addHreg(0x9cf9, 0);
    mb.addHreg(0x9cfa, 0);
    mb.addHreg(0x9cfb, 0);
    mb.addHreg(0x9cfc, 0);
    mb.addHreg(0x9cfd, 0);
    mb.addHreg(0x9cfe, 0);
    mb.addHreg(0x9cff, 0);
    mb.addHreg(0x9d00, 0);
    mb.addHreg(0x9d01, 0);
    mb.addHreg(0x9d02, 0);
    mb.addHreg(0x9d03, 65535);//40196 trailing 0 end
    mb.addHreg(0x9d04, 0); //40197

    // _channels = 0;
    // for (uint8_t i = 0; i < Hoymiles.getNumInverters(); i++) {
    //     auto inv = Hoymiles.getInverterByPos(i);
    //     if (inv == nullptr) {
    //         continue;
    //     }
    //     for (auto& t : inv->Statistics()->getChannelTypes()) {
    //         for (auto& c : inv->Statistics()->getChannelsByType(t)) {
    //             if (t == TYPE_DC) {
    //                 _channels ++;

    //     //for (auto& c : inv->Statistics()->getChannelsByType(TYPE_INV)) {
    //             }
    //         }
    //     }
    // }
    // mb.addHreg(0x200, _channels);
    
    // for (uint8_t i = 0; i <= _channels; i++) {
    //     for(uint8_t j = 0; j < 20; j++) {
    //         mb.addHreg(i* 20 + 0x1000 + j, 0);
    //     }
    //     yield();
    // }
    
}

void ModbusDtuClass::loop()
{
    
    // uint8_t chan = 0;
    // uint8_t invNumb = 0;
    if (millis() - _lastPublish > 5000 && Hoymiles.isAllRadioIdle()) {
        //mb.Hreg(0x9ca0, (uint16_t)Datastore.getTotalAcPowerEnabled());
        float CurrPWR = (Datastore.getTotalAcPowerEnabled()*-1);
        uint16_t *myhex = (uint16_t *)&CurrPWR;
        mb.Hreg(0x9ca1, myhex[1]);
        mb.Hreg(0x9ca2, myhex[0]);
        float TotYLD = (Datastore.getTotalAcYieldTotalEnabled()*1000);
        uint16_t *myhex2 = (uint16_t *)&TotYLD;
        mb.Hreg(0x9cc1, myhex2[1]);
        mb.Hreg(0x9cc2, myhex2[0]);
        // for (uint8_t i = 0; i < Hoymiles.getNumInverters(); i++) {
        //     auto inv = Hoymiles.getInverterByPos(i);
        //     if (inv == nullptr) {
        //     continue;
        //     }
        //     // Loop all channels
        //     for (auto& t : inv->Statistics()->getChannelTypes()) {
        //         for (auto& c : inv->Statistics()->getChannelsByType(t)) {
        //             if (t == TYPE_DC) {
        //                 uint64_t serialInv = inv->serial();
        //                 mb.Hreg(chan* 20 + 0x1000, 0x0C00 + ((serialInv >> 40) & 0xFF));
        //                 mb.Hreg(chan* 20 + 0x1001, (serialInv >> 24) & 0xFFFF);
        //                 mb.Hreg(chan* 20 + 0x1002, (serialInv >> 8) & 0xFFFF);
        //                 mb.Hreg(chan* 20 + 0x1003, (serialInv << 8) + c+1);
        //                 if(inv->Statistics()->getStringMaxPower(c)>0 && inv->Statistics()->getChannelFieldValue(t, c, FLD_IRR) < 500){
        //                     mb.Hreg(chan* 20 + 0x1004, (uint16_t)(inv->Statistics()->getChannelFieldValue(t, c, FLD_UDC)*10));
        //                     mb.Hreg(chan* 20 + 0x1005, (uint16_t)(inv->Statistics()->getChannelFieldValue(t, c, FLD_IDC)*100));
        //                     mb.Hreg(chan* 20 + 0x1008, (uint16_t)(inv->Statistics()->getChannelFieldValue(t, c, FLD_PDC)*10));
        //                     mb.Hreg(0x9ca0, (uint16_t)Datastore.getTotalAcPowerEnabled());
        //                     float myfloat = (Datastore.getTotalAcPowerEnabled()*-1);
        //                     uint16_t *myhex = (uint16_t *)&myfloat;
        //                     mb.Hreg(0x9ca1, myhex[1]);
        //                     mb.Hreg(0x9ca2, myhex[0]);
        //                     //mb.Hreg(0x9ca1, (uint16_t)(inv->Statistics()->getChannelFieldValue(t, c, FLD_PDC)*10)); // 0x9ca1??
        //                 } else if (inv->Statistics()->getStringMaxPower(c)==0)
        //                 {
        //                     mb.Hreg(chan* 20 + 0x1004, (uint16_t)(inv->Statistics()->getChannelFieldValue(t, c, FLD_UDC)*10));
        //                     mb.Hreg(chan* 20 + 0x1005, (uint16_t)(inv->Statistics()->getChannelFieldValue(t, c, FLD_IDC)*100));
        //                     mb.Hreg(chan* 20 + 0x1008, (uint16_t)(inv->Statistics()->getChannelFieldValue(t, c, FLD_PDC)*10));
        //                     mb.Hreg(0x9ca0, (uint16_t)Datastore.getTotalAcPowerEnabled());
        //                     float myfloat = (Datastore.getTotalAcPowerEnabled()*-1);
        //                     uint16_t *myhex = (uint16_t *)&myfloat;
        //                     mb.Hreg(0x9ca1, myhex[1]);
        //                     mb.Hreg(0x9ca2, myhex[0]);
        //                     mb.Hreg(0x9ca1, (uint16_t)(inv->Statistics()->getChannelFieldValue(t, c, FLD_PDC)*10)); // 0x9ca1??
        //                 }
                        
        //                 mb.Hreg(chan* 20 + 0x1006, (uint16_t)(inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_UAC)*10));
        //                 mb.Hreg(chan* 20 + 0x1007, (uint16_t)(inv->Statistics()->getChannelFieldValue(TYPE_AC, CH0, FLD_F)*100));
        //                 mb.Hreg(chan* 20 + 0x1009, (uint16_t)(inv->Statistics()->getChannelFieldValue(t, c, FLD_YD)));
        //                 mb.Hreg(chan* 20 + 0x100A, ((uint16_t)(((uint32_t)(inv->Statistics()->getChannelFieldValue(t, c, FLD_YT)*1000)) >> 16)) & 0xFFFF);
        //                 mb.Hreg(chan* 20 + 0x100B, (((uint16_t)(inv->Statistics()->getChannelFieldValue(t, c, FLD_YT)*1000))) & 0xFFFF);
        //                 mb.Hreg(0x9cc1, ((uint16_t)(((uint32_t)(inv->Statistics()->getChannelFieldValue(t, c, FLD_YT)*1000)) >> 16)) & 0xFFFF);
        //                 mb.Hreg(0x9cc2, (((uint16_t)(inv->Statistics()->getChannelFieldValue(t, c, FLD_YT)*1000))) & 0xFFFF); // 0x9cc1??
        //                 mb.Hreg(chan* 20 + 0x100C, (uint16_t)(inv->Statistics()->getChannelFieldValue(TYPE_INV, CH0, FLD_T)*10)); // 0x9cc1??
        //                 mb.Hreg(chan* 20 + 0x100D, 3);
        //                 mb.Hreg(chan* 20 + 0x100E, 0);
        //                 mb.Hreg(chan* 20 + 0x100F, 0);
        //                 mb.Hreg(chan* 20 + 0x1010, 0x0107);
        //                 mb.Hreg(chan* 20 + 0x1011, 0);
        //                 mb.Hreg(chan* 20 + 0x1012, 0);
        //                 mb.Hreg(chan* 20 + 0x1013, 0);
                    
        //                 chan++;
        //             }
        //         }
        //     }
        //     invNumb++;
        // }
    _lastPublish = millis();
    }
    yield();
    mb.task();
}
