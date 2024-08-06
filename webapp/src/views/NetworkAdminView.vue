<template>
    <BasePage :title="$t('networkadmin.NetworkSettings')" :isLoading="dataLoading">
        <BootstrapAlert v-model="showAlert" dismissible :variant="alertType">
            {{ alertMessage }}
        </BootstrapAlert>

        <form @submit="saveNetworkConfig">
            <CardElement :text="$t('networkadmin.WifiConfiguration')" textVariant="text-bg-primary">
                <InputElement
                    :label="$t('networkadmin.WifiSsid')"
                    v-model="networkConfigList.ssid"
                    type="text"
                    maxlength="32"
                />

                <InputElement
                    :label="$t('networkadmin.WifiPassword')"
                    v-model="networkConfigList.password"
                    type="password"
                    maxlength="64"
                />

                <InputElement
                    :label="$t('networkadmin.Hostname')"
                    v-model="networkConfigList.hostname"
                    type="text"
                    maxlength="32"
                >
                    <div class="alert alert-secondary" role="alert" v-html="$t('networkadmin.HostnameHint')"></div>
                </InputElement>

                <InputElement :label="$t('networkadmin.EnableDhcp')" v-model="networkConfigList.dhcp" type="checkbox" />
            </CardElement>

            <CardElement
                :text="$t('networkadmin.StaticIpConfiguration')"
                textVariant="text-bg-primary"
                add-space
                v-show="!networkConfigList.dhcp"
            >
                <InputElement
                    :label="$t('networkadmin.IpAddress')"
                    v-model="networkConfigList.ipaddress"
                    type="text"
                    maxlength="32"
                />

                <InputElement
                    :label="$t('networkadmin.Netmask')"
                    v-model="networkConfigList.netmask"
                    type="text"
                    maxlength="32"
                />

                <InputElement
                    :label="$t('networkadmin.DefaultGateway')"
                    v-model="networkConfigList.gateway"
                    type="text"
                    maxlength="32"
                />

                <InputElement
                    :label="$t('networkadmin.Dns', { num: 1 })"
                    v-model="networkConfigList.dns1"
                    type="text"
                    maxlength="32"
                />

                <InputElement
                    :label="$t('networkadmin.Dns', { num: 2 })"
                    v-model="networkConfigList.dns2"
                    type="text"
                    maxlength="32"
                />
            </CardElement>

            <CardElement :text="$t('networkadmin.MdnsSettings')" textVariant="text-bg-primary" add-space>
                <InputElement
                    :label="$t('networkadmin.EnableMdns')"
                    v-model="networkConfigList.mdnsenabled"
                    type="checkbox"
                />
            </CardElement>

            <CardElement :text="$t('networkadmin.AdminAp')" textVariant="text-bg-primary" add-space>
                <InputElement
                    :label="$t('networkadmin.ApTimeout')"
                    v-model="networkConfigList.aptimeout"
                    type="number"
                    min="0"
                    max="99999"
                    :postfix="$t('networkadmin.Minutes')"
                    :tooltip="$t('networkadmin.ApTimeoutHint')"
                />
            </CardElement>

            <CardElement :text="$t('networkadmin.ModbusSettings')" textVariant="text-bg-primary" add-space>
                <InputElement
                    :label="$t('networkadmin.EnableModbusTCP')"
                    :tooltip="$t('networkadmin.EnableModbusTCPHint')"
                    v-model="networkConfigList.modbus_tcp_enabled"
                    type="checkbox"
                />

                <InputElement
                    :label="$t('networkadmin.DelayModbusStart')"
                    :tooltip="$t('networkadmin.DelayModbusStartHint')"
                    v-model="networkConfigList.modbus_delaystart"
                    type="checkbox"
                />

                <InputElement
                    :label="$t('networkadmin.ModbusMfrName')"
                    :tooltip="$t('networkadmin.ModbusMfrNameHint')"
                    v-model="networkConfigList.mfrname"
                    type="text"
                    maxlength="32"
                />

                <InputElement
                    :label="$t('networkadmin.ModbusModelName')"
                    :tooltip="$t('networkadmin.ModbusModelNameHint')"
                    v-model="networkConfigList.modelname"
                    type="text"
                    maxlength="32"
                />

                <InputElement
                    :label="$t('networkadmin.ModbusOptions')"
                    :tooltip="$t('networkadmin.ModbusOptionsHint')"
                    v-model="networkConfigList.options"
                    type="text"
                    maxlength="16"
                />

                <InputElement
                    :label="$t('networkadmin.ModbusVersion')"
                    :tooltip="$t('networkadmin.ModbusVersionHint')"
                    v-model="networkConfigList.version"
                    type="text"
                    maxlength="16"
                />

                <InputElement
                    :label="$t('networkadmin.Modbusserial')"
                    :tooltip="$t('networkadmin.ModbusserialHint')"
                    v-model="networkConfigList.serial"
                    type="text"
                    maxlength="16"
                />
            </CardElement>
            <FormFooter @reload="getNetworkConfig" />
        </form>
    </BasePage>
</template>

<script lang="ts">
import BasePage from '@/components/BasePage.vue';
import BootstrapAlert from '@/components/BootstrapAlert.vue';
import CardElement from '@/components/CardElement.vue';
import FormFooter from '@/components/FormFooter.vue';
import InputElement from '@/components/InputElement.vue';
import type { NetworkConfig } from '@/types/NetworkConfig';
import { authHeader, handleResponse } from '@/utils/authentication';
import { defineComponent } from 'vue';

export default defineComponent({
    components: {
        BasePage,
        BootstrapAlert,
        CardElement,
        FormFooter,
        InputElement,
    },
    data() {
        return {
            dataLoading: true,
            networkConfigList: {} as NetworkConfig,
            alertMessage: '',
            alertType: 'info',
            showAlert: false,
        };
    },
    created() {
        this.getNetworkConfig();
    },
    methods: {
        getNetworkConfig() {
            this.dataLoading = true;
            fetch('/api/network/config', { headers: authHeader() })
                .then((response) => handleResponse(response, this.$emitter, this.$router))
                .then((data) => {
                    this.networkConfigList = data;
                    this.dataLoading = false;
                });
        },
        saveNetworkConfig(e: Event) {
            e.preventDefault();

            const formData = new FormData();
            formData.append('data', JSON.stringify(this.networkConfigList));

            fetch('/api/network/config', {
                method: 'POST',
                headers: authHeader(),
                body: formData,
            })
                .then((response) => handleResponse(response, this.$emitter, this.$router))
                .then((response) => {
                    this.alertMessage = this.$t('apiresponse.' + response.code, response.param);
                    this.alertType = response.type;
                    this.showAlert = true;
                });
        },
    },
});
</script>
