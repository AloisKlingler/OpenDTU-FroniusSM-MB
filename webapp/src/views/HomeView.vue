<template>
    <BasePage
        :title="$t('home.LiveData')"
        :isLoading="dataLoading"
        :isWideScreen="true"
        :showWebSocket="true"
        :isWebsocketConnected="isWebsocketConnected"
        @reload="reloadData"
    >
        <HintView :hints="liveData.hints" />
        <InverterTotalInfo :totalData="liveData.total" /><br />
        <div class="row gy-3">
            <div class="col-sm-3 col-md-2" :style="[inverterData.length == 1 ? { display: 'none' } : {}]">
                <div
                    class="nav nav-pills row-cols-sm-1 gap-3"
                    id="v-pills-tab"
                    role="tablist"
                    aria-orientation="vertical"
                >
                    <button
                        v-for="inverter in inverterData"
                        :key="inverter.serial"
                        class="nav-link border border-primary text-break"
                        :id="'v-pills-' + inverter.serial + '-tab'"
                        data-bs-toggle="pill"
                        :data-bs-target="'#v-pills-' + inverter.serial"
                        type="button"
                        role="tab"
                        :aria-controls="'v-pills-' + inverter.serial"
                        aria-selected="true"
                    >
                        <div class="d-flex align-items-center">
                            <div class="me-2">
                                <span
                                    v-if="inverter.AC"
                                    class="badge"
                                    :class="{
                                        'text-bg-secondary': !inverter.poll_enabled,
                                        'text-bg-danger': inverter.poll_enabled && !inverter.reachable,
                                        'text-bg-warning':
                                            inverter.poll_enabled && inverter.reachable && !inverter.producing,
                                        'text-bg-success':
                                            inverter.poll_enabled && inverter.reachable && inverter.producing,
                                    }"
                                >
                                    {{ $n(inverter.AC[0]?.Power?.v || 0, 'decimalNoDigits') }}
                                    {{ inverter.AC[0]?.Power?.u }}
                                </span>
                                <span v-else class="badge text-bg-light">-</span>
                            </div>
                            <div class="ms-auto me-auto">
                                {{ inverter.name }}
                            </div>
                        </div>
                    </button>
                </div>
            </div>

            <div
                class="tab-content"
                id="v-pills-tabContent"
                :class="{
                    'col-sm-9 col-md-10': inverterData.length > 1,
                    'col-sm-12 col-md-12': inverterData.length == 1,
                }"
            >
                <div
                    v-for="inverter in inverterData"
                    :key="inverter.serial"
                    class="tab-pane fade show"
                    :id="'v-pills-' + inverter.serial"
                    role="tabpanel"
                    :aria-labelledby="'v-pills-' + inverter.serial + '-tab'"
                    tabindex="0"
                >
                    <div class="card">
                        <div
                            class="card-header d-flex justify-content-between align-items-center"
                            :class="{
                                'text-bg-tertiary': !inverter.poll_enabled,
                                'text-bg-danger': inverter.poll_enabled && !inverter.reachable,
                                'text-bg-warning': inverter.poll_enabled && inverter.reachable && !inverter.producing,
                                'text-bg-success': inverter.poll_enabled && inverter.reachable && inverter.producing,
                            }"
                        >
                            <div class="p-1 flex-grow-1">
                                <div class="d-flex flex-wrap">
                                    <div style="padding-right: 2em">
                                        {{ inverter.name }}
                                    </div>
                                    <div style="padding-right: 2em">
                                        {{ $t('home.SerialNumber') }}{{ inverter.serial }}
                                    </div>
                                    <div style="padding-right: 2em">
                                        {{ $t('home.CurrentLimit') }}:
                                        <template v-if="inverter.limit_absolute > -1">
                                            {{ $n(inverter.limit_absolute, 'decimalNoDigits') }} W | </template
                                        >{{ $n(inverter.limit_relative / 100, 'percentOneDigit') }}
                                    </div>
                                    <div style="padding-right: 2em">
                                        <DataAgeDisplay :data-age-ms="inverter.data_age_ms" />
                                    </div>
                                </div>
                            </div>
                            <div class="btn-toolbar p-2" role="toolbar">
                                <div class="btn-group me-2" role="group">
                                    <button
                                        :disabled="!isLogged"
                                        type="button"
                                        class="btn btn-sm btn-danger"
                                        @click="onShowLimitSettings(inverter.serial)"
                                        v-tooltip
                                        :title="$t('home.ShowSetInverterLimit')"
                                    >
                                        <BIconSpeedometer style="font-size: 24px" />
                                    </button>
                                </div>

                                <div class="btn-group me-2" role="group">
                                    <button
                                        :disabled="!isLogged"
                                        type="button"
                                        class="btn btn-sm btn-danger"
                                        @click="onShowReactiveSettings(inverter.serial)"
                                        v-tooltip
                                        :title="$t('home.ShowSetInverterReactivePower')"
                                    >
                                        <BIconLightningCharge style="font-size: 24px" />
                                    </button>
                                </div>

                                <div class="btn-group me-2" role="group">
                                    <button
                                        :disabled="!isLogged"
                                        type="button"
                                        class="btn btn-sm btn-danger"
                                        @click="onShowPowerFactorSettings(inverter.serial)"
                                        v-tooltip
                                        :title="$t('home.ShowSetInverterPowerFactor')"
                                    >
                                        <BIconSliders style="font-size: 24px" />
                                    </button>
                                </div>

                                <div class="btn-group me-2" role="group">
                                    <button
                                        :disabled="!isLogged"
                                        type="button"
                                        class="btn btn-sm btn-danger"
                                        @click="onShowPowerSettings(inverter.serial)"
                                        v-tooltip
                                        :title="$t('home.TurnOnOff')"
                                    >
                                        <BIconPower style="font-size: 24px" />
                                    </button>
                                </div>

                                <div class="btn-group me-2" role="group">
                                    <button
                                        type="button"
                                        class="btn btn-sm btn-info"
                                        @click="onShowDevInfo(inverter.serial)"
                                        v-tooltip
                                        :title="$t('home.ShowInverterInfo')"
                                    >
                                        <BIconCpu style="font-size: 24px" />
                                    </button>
                                </div>

                                <div class="btn-group me-2" role="group">
                                    <button
                                        type="button"
                                        class="btn btn-sm btn-info"
                                        @click="onShowGridProfile(inverter.serial)"
                                        v-tooltip
                                        :title="$t('home.ShowGridProfile')"
                                    >
                                        <BIconOutlet style="font-size: 24px" />
                                    </button>
                                </div>

                                <div class="btn-group me-2" role="group">
                                    <button
                                        :disabled="!isLogged"
                                        type="button"
                                        class="btn btn-sm btn-warning"
                                        @click="onShowYieldTotalSettings(inverter)"
                                        v-tooltip
                                        :title="$t('home.ShowSetYieldTotal')"
                                    >
                                        <BIconBattery style="font-size: 24px" />
                                    </button>
                                </div>

                                <div class="btn-group" role="group">
                                    <button
                                        v-if="inverter.events >= 0"
                                        type="button"
                                        class="btn btn-sm btn-secondary position-relative"
                                        @click="onShowEventlog(inverter.serial)"
                                        v-tooltip
                                        :title="$t('home.ShowEventlog')"
                                    >
                                        <BIconJournalText style="font-size: 24px" />
                                        <span
                                            class="position-absolute top-0 start-100 translate-middle badge rounded-pill text-bg-danger"
                                        >
                                            {{ inverter.events }}
                                            <span class="visually-hidden">{{ $t('home.UnreadMessages') }}</span>
                                        </span>
                                    </button>
                                </div>
                            </div>
                        </div>
                        <div class="card-body">
                            <div class="row flex-row-reverse flex-wrap-reverse g-3">
                                <template
                                    v-for="chanType in [
                                        { obj: inverter.INV, name: 'INV' },
                                        { obj: inverter.AC, name: 'AC' },
                                        { obj: inverter.DC, name: 'DC' },
                                    ].reverse()"
                                >
                                    <template v-if="chanType.obj != null">
                                        <template
                                            v-for="channel in Object.keys(chanType.obj)
                                                .sort()
                                                .reverse()
                                                .map((x) => +x)"
                                            :key="channel"
                                        >
                                            <template
                                                v-if="
                                                    chanType.name != 'DC' ||
                                                    (chanType.name == 'DC' && getSumIrridiation(inverter) == 0) ||
                                                    (chanType.name == 'DC' &&
                                                        getSumIrridiation(inverter) > 0 &&
                                                        chanType.obj[channel]?.Irradiation?.max) ||
                                                    0 > 0
                                                "
                                            >
                                                <div class="col" v-if="chanType.obj[channel]">
                                                    <InverterChannelInfo
                                                        :channelData="chanType.obj[channel]"
                                                        :channelType="chanType.name"
                                                        :channelNumber="channel"
                                                    />
                                                </div>
                                            </template>
                                        </template>
                                    </template>
                                </template>
                            </div>

                            <BootstrapAlert class="m-3" :show="!inverter.hasOwnProperty('INV')">
                                <div class="d-flex justify-content-center align-items-center">
                                    <div class="spinner-border m-1" role="status">
                                        <span class="visually-hidden">{{ $t('home.LoadingInverter') }}</span>
                                    </div>
                                    <span>{{ $t('home.LoadingInverter') }}</span>
                                </div>
                            </BootstrapAlert>

                            <div class="accordion mt-5" id="accordionRadioStats">
                                <div class="accordion-item accordion-table">
                                    <h2 class="accordion-header">
                                        <button
                                            class="accordion-button collapsed"
                                            type="button"
                                            data-bs-toggle="collapse"
                                            data-bs-target="#collapseStats"
                                            aria-expanded="true"
                                            aria-controls="collapseStats"
                                        >
                                            <BIconBroadcast />&nbsp;{{ $t('home.RadioStats') }}
                                        </button>
                                    </h2>
                                    <div
                                        id="collapseStats"
                                        class="accordion-collapse collapse"
                                        data-bs-parent="#accordionRadioStats"
                                    >
                                        <div class="accordion-body">
                                            <table class="table table-striped table-hover">
                                                <tbody>
                                                    <tr>
                                                        <td>{{ $t('home.TxRequest') }}</td>
                                                        <td>{{ $n(inverter.radio_stats.tx_request) }}</td>
                                                        <td></td>
                                                    </tr>
                                                    <tr>
                                                        <td>{{ $t('home.RxSuccess') }}</td>
                                                        <td>{{ $n(inverter.radio_stats.rx_success) }}</td>
                                                        <td>
                                                            {{
                                                                ratio(
                                                                    inverter.radio_stats.rx_success,
                                                                    inverter.radio_stats.tx_request
                                                                )
                                                            }}
                                                        </td>
                                                    </tr>
                                                    <tr>
                                                        <td>{{ $t('home.RxFailNothing') }}</td>
                                                        <td>{{ $n(inverter.radio_stats.rx_fail_nothing) }}</td>
                                                        <td>
                                                            {{
                                                                ratio(
                                                                    inverter.radio_stats.rx_fail_nothing,
                                                                    inverter.radio_stats.tx_request
                                                                )
                                                            }}
                                                        </td>
                                                    </tr>
                                                    <tr>
                                                        <td>{{ $t('home.RxFailPartial') }}</td>
                                                        <td>{{ $n(inverter.radio_stats.rx_fail_partial) }}</td>
                                                        <td>
                                                            {{
                                                                ratio(
                                                                    inverter.radio_stats.rx_fail_partial,
                                                                    inverter.radio_stats.tx_request
                                                                )
                                                            }}
                                                        </td>
                                                    </tr>
                                                    <tr>
                                                        <td>{{ $t('home.RxFailCorrupt') }}</td>
                                                        <td>{{ $n(inverter.radio_stats.rx_fail_corrupt) }}</td>
                                                        <td>
                                                            {{
                                                                ratio(
                                                                    inverter.radio_stats.rx_fail_corrupt,
                                                                    inverter.radio_stats.tx_request
                                                                )
                                                            }}
                                                        </td>
                                                    </tr>
                                                    <tr>
                                                        <td>{{ $t('home.TxReRequest') }}</td>
                                                        <td>{{ $n(inverter.radio_stats.tx_re_request) }}</td>
                                                        <td></td>
                                                    </tr>
                                                    <tr>
                                                        <td>
                                                            {{ $t('home.Rssi') }}
                                                            <BIconInfoCircle v-tooltip :title="$t('home.RssiHint')" />
                                                        </td>
                                                        <td>
                                                            {{ $t('home.dBm', { dbm: $n(inverter.radio_stats.rssi) }) }}
                                                        </td>
                                                        <td></td>
                                                    </tr>
                                                </tbody>
                                            </table>
                                            <div class="d-flex">
                                                <button
                                                    :disabled="!isLogged || performRadioStatsReset"
                                                    type="button"
                                                    class="btn btn-danger ms-auto me-3 mt-3"
                                                    @click="onResetRadioStats(inverter.serial)"
                                                >
                                                    <template v-if="!performRadioStatsReset">
                                                        <BIconArrowCounterclockwise />&nbsp;{{ $t('home.StatsReset') }}
                                                    </template>
                                                    <template v-else>
                                                        <span
                                                            class="spinner-border spinner-border-sm"
                                                            aria-hidden="true"
                                                        ></span>
                                                        <span role="status">&nbsp;{{ $t('home.StatsResetting') }}</span>
                                                    </template>
                                                </button>
                                            </div>
                                        </div>
                                    </div>
                                </div>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </BasePage>

    <ModalDialog
        modalId="eventView"
        :title="$t('home.EventLog')"
        :loading="eventLogLoading"
        :show-reload="true"
        @reload="onShowEventlog(eventLogSerial)"
    >
        <EventLog :eventLogList="eventLogList" />
    </ModalDialog>

    <ModalDialog
        modalId="devInfoView"
        :title="$t('home.InverterInfo')"
        :loading="devInfoLoading"
        :show-reload="true"
        @reload="onShowDevInfo(devInfoList.serial)"
    >
        <DevInfo :devInfoList="devInfoList" />
    </ModalDialog>

    <ModalDialog
        modalId="gridProfileView"
        :title="$t('home.GridProfile')"
        :loading="gridProfileLoading"
        :show-reload="true"
        @reload="onReloadGridProfile(gridProfileSerial)"
    >
        <GridProfile
            :gridProfileList="gridProfileList"
            :gridProfileRawList="gridProfileRawList"
            :inverterSerial="gridProfileSerial"
            @refresh="loadGridProfile(gridProfileSerial, false)"
        />
    </ModalDialog>

    <ModalDialog modalId="limitSettingView" :title="$t('home.LimitSettings')" :loading="limitSettingLoading">
        <BootstrapAlert v-model="showAlertLimit" :variant="alertTypeLimit">
            {{ alertMessageLimit }}
        </BootstrapAlert>

        <div class="row mb-3">
            <label for="inputCurrentLimit" class="col-sm-3 col-form-label">{{ $t('home.CurrentLimit') }} </label>
            <div class="col-sm-4">
                <div class="input-group">
                    <input
                        type="text"
                        class="form-control"
                        id="inputCurrentLimit"
                        aria-describedby="currentLimitType"
                        v-model="currentLimitRelative"
                        disabled
                    />
                    <span class="input-group-text" id="currentLimitType">%</span>
                </div>
            </div>

            <div class="col-sm-4" v-if="currentLimitList.max_power > 0">
                <div class="input-group">
                    <input
                        type="text"
                        class="form-control"
                        id="inputCurrentLimitAbsolute"
                        aria-describedby="currentLimitTypeAbsolute"
                        v-model="currentLimitAbsolute"
                        disabled
                    />
                    <span class="input-group-text" id="currentLimitTypeAbsolute">W</span>
                </div>
            </div>
        </div>

        <div class="row mb-3 align-items-center">
            <label for="inputLastLimitSet" class="col-sm-3 col-form-label">
                {{ $t('home.LastLimitSetStatus') }}
            </label>
            <div class="col-sm-9">
                <span
                    class="badge"
                    :class="{
                        'text-bg-danger': currentLimitList.limit_set_status == 'Failure',
                        'text-bg-warning': currentLimitList.limit_set_status == 'Pending',
                        'text-bg-success': currentLimitList.limit_set_status == 'Ok',
                        'text-bg-secondary': currentLimitList.limit_set_status == 'Unknown',
                    }"
                >
                    {{ $t('home.' + currentLimitList.limit_set_status) }}
                </span>
            </div>
        </div>

        <div class="row mb-3">
            <label for="inputTargetLimit" class="col-sm-3 col-form-label">{{ $t('home.SetLimit') }}</label>
            <div class="col-sm-9">
                <div class="input-group">
                    <input
                        type="number"
                        name="inputTargetLimit"
                        class="form-control"
                        id="inputTargetLimit"
                        :min="targetLimitMin"
                        :max="targetLimitMax"
                        v-model="targetLimitList.limit_value"
                    />
                    <button
                        class="btn btn-primary dropdown-toggle"
                        type="button"
                        data-bs-toggle="dropdown"
                        aria-expanded="false"
                    >
                        {{ targetLimitTypeText }}
                    </button>
                    <ul class="dropdown-menu dropdown-menu-end">
                        <li>
                            <a class="dropdown-item" @click="onSelectType(true)" href="#">{{ $t('home.Relative') }}</a>
                        </li>
                        <li>
                            <a class="dropdown-item" @click="onSelectType(false)" href="#">{{ $t('home.Absolute') }}</a>
                        </li>
                    </ul>
                </div>
                <div
                    v-if="!targetLimitRelative"
                    class="alert alert-secondary mt-3"
                    role="alert"
                    v-html="$t('home.LimitHint')"
                ></div>
            </div>
        </div>

        <template #footer>
            <button type="button" class="btn btn-danger" @click="onSetLimitSettings(true)">
                {{ $t('home.SetPersistent') }}
            </button>

            <button type="button" class="btn btn-danger" @click="onSetLimitSettings(false)">
                {{ $t('home.SetNonPersistent') }}
            </button>
        </template>
    </ModalDialog>

    <ModalDialog
        modalId="reactivePowerSettingView"
        :title="$t('home.ReactivePowerSettings')"
        :loading="reactiveSettingLoading"
    >
        <BootstrapAlert v-model="showAlertReactive" :variant="alertTypeReactive">
            {{ alertMessageReactive }}
        </BootstrapAlert>

        <div class="row mb-3">
            <label for="inputCurrentReactive" class="col-sm-3 col-form-label">
                {{ $t('home.CurrentReactivePower') }}
            </label>
            <div class="col-sm-4">
                <div class="input-group">
                    <input
                        type="text"
                        class="form-control"
                        id="inputCurrentReactive"
                        aria-describedby="currentReactiveType"
                        v-model="currentReactiveRelative"
                        disabled
                    />
                    <span class="input-group-text" id="currentReactiveType">%</span>
                </div>
            </div>

            <div class="col-sm-4" v-if="currentReactiveList.max_power > 0">
                <div class="input-group">
                    <input
                        type="text"
                        class="form-control"
                        id="inputCurrentReactiveAbsolute"
                        aria-describedby="currentReactiveTypeAbsolute"
                        v-model="currentReactiveAbsolute"
                        disabled
                    />
                    <span class="input-group-text" id="currentReactiveTypeAbsolute">var</span>
                </div>
            </div>
        </div>

        <div class="row mb-3 align-items-center">
            <label for="inputLastReactiveSet" class="col-sm-3 col-form-label">
                {{ $t('home.LastReactivePowerSetStatus') }}
            </label>
            <div class="col-sm-9">
                <span
                    class="badge"
                    :class="{
                        'text-bg-danger': currentReactiveList.reactive_set_status == 'Failure',
                        'text-bg-warning': currentReactiveList.reactive_set_status == 'Pending',
                        'text-bg-success': currentReactiveList.reactive_set_status == 'Ok',
                        'text-bg-secondary': currentReactiveList.reactive_set_status == 'Unknown',
                    }"
                >
                    {{ $t('home.' + currentReactiveList.reactive_set_status) }}
                </span>
            </div>
        </div>

        <div class="row mb-3">
            <label for="inputTargetReactive" class="col-sm-3 col-form-label">
                {{ $t('home.SetReactivePower') }}
            </label>
            <div class="col-sm-9">
                <div class="input-group">
                    <input
                        type="number"
                        name="inputTargetReactive"
                        class="form-control"
                        id="inputTargetReactive"
                        :min="targetReactiveMin"
                        :max="targetReactiveMax"
                        v-model="targetReactiveList.reactive_value"
                    />
                    <button
                        class="btn btn-primary dropdown-toggle"
                        type="button"
                        data-bs-toggle="dropdown"
                        aria-expanded="false"
                    >
                        {{ targetReactiveTypeText }}
                    </button>
                    <ul class="dropdown-menu dropdown-menu-end">
                        <li>
                            <a class="dropdown-item" @click="onSelectReactiveType(true)" href="#">
                                {{ $t('home.Relative') }}
                            </a>
                        </li>
                        <li>
                            <a class="dropdown-item" @click="onSelectReactiveType(false)" href="#">
                                {{ $t('home.Absolute') }}
                            </a>
                        </li>
                    </ul>
                </div>
                <div class="alert alert-secondary mt-3" role="alert" v-html="$t('home.ReactivePowerHint')"></div>
            </div>
        </div>

        <template #footer>
            <button type="button" class="btn btn-danger" @click="onSetReactiveSettings(true)">
                {{ $t('home.SetPersistent') }}
            </button>

            <button type="button" class="btn btn-danger" @click="onSetReactiveSettings(false)">
                {{ $t('home.SetNonPersistent') }}
            </button>
        </template>
    </ModalDialog>

    <ModalDialog
        modalId="powerFactorSettingView"
        :title="$t('home.PowerFactorSettings')"
        :loading="powerFactorSettingLoading"
    >
        <BootstrapAlert v-model="showAlertPowerFactor" :variant="alertTypePowerFactor">
            {{ alertMessagePowerFactor }}
        </BootstrapAlert>

        <div class="row mb-3">
            <label for="inputCurrentPowerFactor" class="col-sm-3 col-form-label">
                {{ $t('home.CurrentPowerFactor') }}
            </label>
            <div class="col-sm-9">
                <input
                    type="text"
                    class="form-control"
                    id="inputCurrentPowerFactor"
                    v-model="currentPowerFactorValue"
                    disabled
                />
            </div>
        </div>

        <div class="row mb-3 align-items-center">
            <label for="inputLastPowerFactorSet" class="col-sm-3 col-form-label">
                {{ $t('home.LastPowerFactorSetStatus') }}
            </label>
            <div class="col-sm-9">
                <span
                    class="badge"
                    :class="{
                        'text-bg-danger': currentPowerFactorList.power_factor_set_status == 'Failure',
                        'text-bg-warning': currentPowerFactorList.power_factor_set_status == 'Pending',
                        'text-bg-success': currentPowerFactorList.power_factor_set_status == 'Ok',
                        'text-bg-secondary': currentPowerFactorList.power_factor_set_status == 'Unknown',
                    }"
                >
                    {{ $t('home.' + currentPowerFactorList.power_factor_set_status) }}
                </span>
            </div>
        </div>

        <div class="row mb-3">
            <label for="inputTargetPowerFactor" class="col-sm-3 col-form-label">
                {{ $t('home.SetPowerFactor') }}
            </label>
            <div class="col-sm-9">
                <input
                    type="number"
                    name="inputTargetPowerFactor"
                    class="form-control"
                    id="inputTargetPowerFactor"
                    min="0"
                    max="1"
                    step="0.001"
                    v-model="targetPowerFactorList.power_factor"
                />
                <div class="alert alert-secondary mt-3" role="alert" v-html="$t('home.PowerFactorHint')"></div>
            </div>
        </div>

        <template #footer>
            <button type="button" class="btn btn-danger" @click="onSetPowerFactorSettings(true)">
                {{ $t('home.SetPersistent') }}
            </button>

            <button type="button" class="btn btn-danger" @click="onSetPowerFactorSettings(false)">
                {{ $t('home.SetNonPersistent') }}
            </button>
        </template>
    </ModalDialog>

    <ModalDialog modalId="powerSettingView" :title="$t('home.PowerSettings')" :loading="powerSettingLoading">
        <BootstrapAlert v-model="showAlertPower" :variant="alertTypePower">
            {{ alertMessagePower }}
        </BootstrapAlert>

        <div class="row mb-3 align-items-center">
            <label for="inputLastPowerSet" class="col col-form-label">{{ $t('home.LastPowerSetStatus') }}</label>
            <div class="col">
                <span
                    class="badge"
                    :class="{
                        'text-bg-danger': successCommandPower == 'Failure',
                        'text-bg-warning': successCommandPower == 'Pending',
                        'text-bg-success': successCommandPower == 'Ok',
                        'text-bg-secondary': successCommandPower == 'Unknown',
                    }"
                >
                    {{ $t('home.' + successCommandPower) }}
                </span>
            </div>
        </div>

        <div class="d-grid gap-2 col-6 mx-auto">
            <button type="button" class="btn btn-success" @click="onSetPowerSettings(true)">
                <BIconToggleOn class="fs-4" />&nbsp;{{ $t('home.TurnOn') }}
            </button>
            <button type="button" class="btn btn-danger" @click="onSetPowerSettings(false)">
                <BIconToggleOff class="fs-4" />&nbsp;{{ $t('home.TurnOff') }}
            </button>
            <button type="button" class="btn btn-warning" @click="onSetPowerSettings(true, true)">
                <BIconArrowCounterclockwise class="fs-4" />&nbsp;{{ $t('home.Restart') }}
            </button>
        </div>
    </ModalDialog>

    <ModalDialog
        modalId="yieldTotalSettingView"
        :title="$t('home.YieldTotalSettings')"
        :loading="yieldTotalSettingLoading"
    >
        <BootstrapAlert v-model="showAlertYieldTotal" :variant="alertTypeYieldTotal">
            {{ alertMessageYieldTotal }}
        </BootstrapAlert>

        <div class="row mb-3 align-items-center">
            <label class="col-sm-4 col-form-label">{{ $t('home.LastYieldTotalSetStatus') }}</label>
            <div class="col-sm-8">
                <span
                    class="badge"
                    :class="{
                        'text-bg-danger': currentYieldTotalStatus == 'Failure',
                        'text-bg-warning': currentYieldTotalStatus == 'Pending',
                        'text-bg-success': currentYieldTotalStatus == 'Ok',
                        'text-bg-secondary': currentYieldTotalStatus == 'Unknown',
                    }"
                >
                    {{ $t('home.' + currentYieldTotalStatus) }}
                </span>
            </div>
        </div>

        <div class="row mb-3" v-for="(_, idx) in targetYieldTotalKwh" :key="idx">
            <label :for="'inputYieldString' + idx" class="col-sm-4 col-form-label">
                {{ $t('home.YieldTotalString', { n: idx + 1 }) }}
            </label>
            <div class="col-sm-8">
                <div class="input-group">
                    <input
                        type="number"
                        min="0"
                        step="1"
                        class="form-control"
                        :id="'inputYieldString' + idx"
                        v-model.number="targetYieldTotalKwh[idx]"
                    />
                    <span class="input-group-text">kWh</span>
                </div>
            </div>
        </div>

        <div class="alert alert-secondary" role="alert" v-html="$t('home.YieldTotalHint')"></div>

        <template #footer>
            <button type="button" class="btn btn-warning" @click="onSetYieldTotalSettings()">
                {{ $t('home.SetYieldTotal') }}
            </button>
        </template>
    </ModalDialog>
</template>

<script lang="ts">
import BasePage from '@/components/BasePage.vue';
import BootstrapAlert from '@/components/BootstrapAlert.vue';
import DataAgeDisplay from '@/components/DataAgeDisplay.vue';
import DevInfo from '@/components/DevInfo.vue';
import EventLog from '@/components/EventLog.vue';
import GridProfile from '@/components/GridProfile.vue';
import HintView from '@/components/HintView.vue';
import InverterChannelInfo from '@/components/InverterChannelInfo.vue';
import InverterTotalInfo from '@/components/InverterTotalInfo.vue';
import { LimitType } from '@/types/LimitConfig';
import ModalDialog from '@/components/ModalDialog.vue';
import type { DevInfoStatus } from '@/types/DevInfoStatus';
import type { EventlogItems } from '@/types/EventlogStatus';
import type { GridProfileRawdata } from '@/types/GridProfileRawdata';
import type { GridProfileStatus } from '@/types/GridProfileStatus';
import type { LimitConfig } from '@/types/LimitConfig';
import type { LimitStatus } from '@/types/LimitStatus';
import type { Inverter, LiveData } from '@/types/LiveDataStatus';
import type { PowerFactorConfig, PowerFactorStatus } from '@/types/PowerFactorConfig';
import type { ReactivePowerConfig, ReactivePowerStatus } from '@/types/ReactivePowerConfig';
import type { YieldTotalStatus } from '@/types/YieldTotalConfig';
import { authHeader, authUrl, handleResponse, isLoggedIn } from '@/utils/authentication';
import * as bootstrap from 'bootstrap';
import {
    BIconArrowCounterclockwise,
    BIconBattery,
    BIconBroadcast,
    BIconCpu,
    BIconInfoCircle,
    BIconJournalText,
    BIconLightningCharge,
    BIconOutlet,
    BIconPower,
    BIconSliders,
    BIconSpeedometer,
    BIconToggleOff,
    BIconToggleOn,
} from 'bootstrap-icons-vue';
import { defineComponent } from 'vue';
import WebSocketService from '@/utils/websocketService';

export default defineComponent({
    components: {
        BasePage,
        BootstrapAlert,
        DataAgeDisplay,
        DevInfo,
        EventLog,
        GridProfile,
        HintView,
        InverterChannelInfo,
        InverterTotalInfo,
        ModalDialog,
        BIconArrowCounterclockwise,
        BIconBattery,
        BIconBroadcast,
        BIconCpu,
        BIconInfoCircle,
        BIconJournalText,
        BIconLightningCharge,
        BIconOutlet,
        BIconPower,
        BIconSliders,
        BIconSpeedometer,
        BIconToggleOff,
        BIconToggleOn,
    },
    data() {
        return {
            isLogged: isLoggedIn(),

            socket: {} as WebSocketService,
            heartInterval: 0,
            dataAgeTimers: {} as Record<string, number>,
            dataLoading: true,
            liveData: {} as LiveData,
            isFirstFetchAfterConnect: true,
            eventLogView: {} as bootstrap.Modal,
            eventLogSerial: '',
            eventLogList: {} as EventlogItems,
            eventLogLoading: true,
            devInfoView: {} as bootstrap.Modal,
            devInfoList: {} as DevInfoStatus,
            devInfoLoading: true,
            gridProfileView: {} as bootstrap.Modal,
            gridProfileSerial: '',
            gridProfileList: {} as GridProfileStatus,
            gridProfileRawList: {} as GridProfileRawdata,
            gridProfileLoading: true,

            limitSettingView: {} as bootstrap.Modal,
            limitSettingLoading: true,

            currentLimitList: {} as LimitStatus,
            targetLimitList: {} as LimitConfig,

            targetLimitMin: 0,
            targetLimitMax: 100,
            targetLimitTypeText: this.$t('home.Relative'),
            targetLimitRelative: true,

            alertMessageLimit: '',
            alertTypeLimit: 'info',
            showAlertLimit: false,
            performRadioStatsReset: false,

            reactiveSettingView: {} as bootstrap.Modal,
            reactiveSettingLoading: true,

            currentReactiveList: {} as ReactivePowerStatus,
            targetReactiveList: {} as ReactivePowerConfig,

            targetReactiveMin: -100,
            targetReactiveMax: 100,
            targetReactiveTypeText: this.$t('home.Relative'),
            targetReactiveRelative: true,

            alertMessageReactive: '',
            alertTypeReactive: 'info',
            showAlertReactive: false,

            powerFactorSettingView: {} as bootstrap.Modal,
            powerFactorSettingLoading: true,

            currentPowerFactorList: {} as PowerFactorStatus,
            targetPowerFactorList: {} as PowerFactorConfig,

            alertMessagePowerFactor: '',
            alertTypePowerFactor: 'info',
            showAlertPowerFactor: false,

            powerSettingView: {} as bootstrap.Modal,
            powerSettingSerial: '',
            powerSettingLoading: true,
            alertMessagePower: '',
            alertTypePower: 'info',
            showAlertPower: false,
            successCommandPower: '',

            yieldTotalSettingView: {} as bootstrap.Modal,
            yieldTotalSettingLoading: true,
            yieldTotalSerial: '',
            currentYieldTotalStatus: 'Unknown' as string,
            targetYieldTotalKwh: [] as number[],
            alertMessageYieldTotal: '',
            alertTypeYieldTotal: 'info',
            showAlertYieldTotal: false,

            isWebsocketConnected: false,
        };
    },
    created() {
        this.getInitialData();
        this.initSocket();
        this.$emitter.on('logged-in', () => {
            this.isLogged = this.isLoggedIn();
        });
        this.$emitter.on('logged-out', () => {
            this.isLogged = this.isLoggedIn();
        });
    },
    mounted() {
        this.eventLogView = new bootstrap.Modal('#eventView');
        this.devInfoView = new bootstrap.Modal('#devInfoView');
        this.gridProfileView = new bootstrap.Modal('#gridProfileView');
        this.limitSettingView = new bootstrap.Modal('#limitSettingView');
        this.reactiveSettingView = new bootstrap.Modal('#reactivePowerSettingView');
        this.powerFactorSettingView = new bootstrap.Modal('#powerFactorSettingView');
        this.powerSettingView = new bootstrap.Modal('#powerSettingView');
        this.yieldTotalSettingView = new bootstrap.Modal('#yieldTotalSettingView');
    },
    unmounted() {
        this.socket?.close();
    },
    updated() {
        console.log('Updated');
        // Select first tab
        if (this.isFirstFetchAfterConnect) {
            console.log('isFirstFetchAfterConnect');

            this.$nextTick(() => {
                console.log('nextTick');
                const firstTabEl = document.querySelector('#v-pills-tab:first-child button');
                if (firstTabEl != null) {
                    this.isFirstFetchAfterConnect = false;
                    console.log('Show');
                    const firstTab = new bootstrap.Tab(firstTabEl);
                    firstTab.show();
                }
            });
        }
    },
    computed: {
        currentLimitAbsolute(): string {
            if (this.currentLimitList.max_power > 0) {
                return this.$n(
                    (this.currentLimitList.limit_relative * this.currentLimitList.max_power) / 100,
                    'decimalNoDigits'
                );
            }
            return '0';
        },
        currentLimitRelative(): string {
            return this.$n(this.currentLimitList.limit_relative, 'decimalOneDigit');
        },
        currentReactiveAbsolute(): string {
            if (this.currentReactiveList.max_power > 0) {
                return this.$n(
                    (this.currentReactiveList.reactive_relative * this.currentReactiveList.max_power) / 100,
                    'decimalNoDigits'
                );
            }
            return '0';
        },
        currentReactiveRelative(): string {
            return this.$n(this.currentReactiveList.reactive_relative, 'decimalOneDigit');
        },
        currentPowerFactorValue(): string {
            return this.$n(this.currentPowerFactorList.power_factor, 'decimalThreeDigits');
        },
        inverterData(): Inverter[] {
            return this.liveData.inverters.slice().sort((a: Inverter, b: Inverter) => {
                return a.order - b.order;
            });
        },
    },
    methods: {
        isLoggedIn,
        getInitialData(triggerLoading: boolean = true) {
            if (triggerLoading) {
                this.dataLoading = true;
            }
            fetch('/api/livedata/status', { headers: authHeader() })
                .then((response) => handleResponse(response, this.$emitter, this.$router))
                .then((data) => {
                    this.liveData = data;
                    if (triggerLoading) {
                        this.dataLoading = false;
                    }
                });
        },
        reloadData() {
            this.socket?.close();

            this.getInitialData(false);
            this.initSocket();
        },
        handleMessage(event: MessageEvent) {
            if (!event.data || event.data === '{}') {
                this.socket?.close(); // force reconnect
                this.initSocket();
                return;
            }

            const newData = JSON.parse(event.data);

            Object.assign(this.liveData.total, newData.total);
            Object.assign(this.liveData.hints, newData.hints);

            const idx = this.liveData.inverters.findIndex((i) => i.serial === newData.inverters[0].serial);

            if (idx == -1) {
                Object.assign(this.liveData.inverters, newData.inverters);
                this.liveData.inverters.forEach((inv) => this.resetDataAging(inv));
            } else if (this.liveData.inverters[idx]) {
                Object.assign(this.liveData.inverters[idx], newData.inverters[0]);
                this.resetDataAging(this.liveData.inverters[idx]);
            }
        },
        initSocket() {
            console.log('Starting connection to WebSocket Server');

            const { protocol, host } = location;
            const authString = authUrl();
            const webSocketUrl = `${protocol === 'https:' ? 'wss' : 'ws'}://${authString}${host}/livedata`;

            this.socket = new WebSocketService(webSocketUrl, {
                onMessage: this.handleMessage,
                onOpen: () => {
                    console.log('WebSocket connected');
                    this.isWebsocketConnected = true;
                },
                onClose: () => {
                    console.log('WebSocket closed');
                    this.isWebsocketConnected = false;
                },
            });

            // Listen to window events , When the window closes , Take the initiative to disconnect websocket Connect
            window.onbeforeunload = () => {
                this.socket?.close();
            };

            this.socket?.connect();
        },
        resetDataAging(inv: Inverter) {
            if (this.dataAgeTimers[inv.serial] !== undefined) {
                clearTimeout(this.dataAgeTimers[inv.serial]);
            }

            const nextMs = 1000 - (inv.data_age_ms % 1000);
            this.dataAgeTimers[inv.serial] = setTimeout(() => {
                this.doDataAging(inv.serial);
            }, nextMs);
        },
        doDataAging(serial: string) {
            const inv = this.liveData?.inverters?.find((inv) => inv.serial === serial);
            if (inv === undefined) {
                return;
            }

            inv.data_age_ms += 1000;

            this.dataAgeTimers[serial] = setTimeout(() => {
                this.doDataAging(serial);
            }, 1000);
        },
        onShowEventlog(serial: string) {
            this.eventLogLoading = true;
            this.eventLogSerial = serial;
            fetch('/api/eventlog/status?inv=' + serial + '&locale=' + this.$i18n.locale, {
                headers: authHeader(),
            })
                .then((response) => handleResponse(response, this.$emitter, this.$router))
                .then((data) => {
                    this.eventLogList = data;
                    this.eventLogLoading = false;
                });

            this.eventLogView.show();
        },
        onShowDevInfo(serial: string) {
            this.devInfoLoading = true;
            fetch('/api/devinfo/status?inv=' + serial, { headers: authHeader() })
                .then((response) => handleResponse(response, this.$emitter, this.$router))
                .then((data) => {
                    this.devInfoList = data;
                    this.devInfoList.serial = serial;
                    this.devInfoLoading = false;
                });

            this.devInfoView.show();
        },
        onShowGridProfile(serial: string) {
            this.loadGridProfile(serial);
            this.gridProfileView.show();
        },
        onReloadGridProfile(serial: string) {
            // Queue a fresh read from the inverter; the GridProfile component
            // reloads the view once the answer has arrived. The refresh endpoint
            // needs full credentials, read-only viewers just reload the cache.
            if (!this.isLogged) {
                this.loadGridProfile(serial);
                return;
            }
            const formData = new FormData();
            formData.append('data', JSON.stringify({ serial: serial }));
            fetch('/api/gridprofile/refresh', {
                method: 'POST',
                headers: authHeader(),
                body: formData,
            })
                .then((response) => handleResponse(response, this.$emitter, this.$router, true))
                .catch(() => {
                    /* still show the cached profile below */
                })
                .finally(() => this.loadGridProfile(serial));
        },
        // showSpinner=false updates the data in place without unmounting the
        // GridProfile component (keeps its write status / edit state).
        loadGridProfile(serial: string, showSpinner = true) {
            this.gridProfileSerial = serial;
            if (showSpinner) {
                this.gridProfileLoading = true;
            }
            fetch('/api/gridprofile/status?inv=' + serial, { headers: authHeader() })
                .then((response) => handleResponse(response, this.$emitter, this.$router))
                .then((data) => {
                    this.gridProfileList = data;

                    fetch('/api/gridprofile/rawdata?inv=' + serial, { headers: authHeader() })
                        .then((response) => handleResponse(response, this.$emitter, this.$router))
                        .then((data) => {
                            this.gridProfileRawList = data;
                            this.gridProfileLoading = false;
                        });
                });
        },
        onShowLimitSettings(serial: string) {
            this.showAlertLimit = false;
            this.targetLimitList.serial = '';
            this.targetLimitList.limit_value = 0;
            this.onSelectType(true);

            this.limitSettingLoading = true;
            fetch('/api/limit/status', { headers: authHeader() })
                .then((response) => handleResponse(response, this.$emitter, this.$router))
                .then((data) => {
                    this.currentLimitList = data[serial];
                    this.targetLimitList.serial = serial;
                    this.limitSettingLoading = false;
                });

            this.limitSettingView.show();
        },
        onResetRadioStats(serial: string) {
            this.performRadioStatsReset = true;
            fetch('/api/inverter/stats_reset?inv=' + serial, { headers: authHeader() })
                .then((response) => handleResponse(response, this.$emitter, this.$router))
                .then(() => {
                    this.performRadioStatsReset = false;
                });
        },
        onSetLimitSettings(setPersistent: boolean) {
            if (setPersistent) {
                if (this.targetLimitRelative) {
                    this.targetLimitList.limit_type = LimitType.RelativPersistent;
                } else {
                    this.targetLimitList.limit_type = LimitType.AbsolutPersistent;
                }
            } else {
                if (this.targetLimitRelative) {
                    this.targetLimitList.limit_type = LimitType.RelativNonPersistent;
                } else {
                    this.targetLimitList.limit_type = LimitType.AbsolutNonPersistent;
                }
            }
            const formData = new FormData();
            formData.append('data', JSON.stringify(this.targetLimitList));

            console.log(this.targetLimitList);

            fetch('/api/limit/config', {
                method: 'POST',
                headers: authHeader(),
                body: formData,
            })
                .then((response) => handleResponse(response, this.$emitter, this.$router))
                .then((response) => {
                    if (response.type == 'success') {
                        this.limitSettingView.hide();
                    } else {
                        this.alertMessageLimit = this.$t('apiresponse.' + response.code, response.param);
                        this.alertTypeLimit = response.type;
                        this.showAlertLimit = true;
                    }
                });
        },
        onSelectType(isRelative: boolean) {
            if (isRelative) {
                this.targetLimitTypeText = this.$t('home.Relative');
                this.targetLimitMin = 0;
                this.targetLimitMax = 100;
            } else {
                this.targetLimitTypeText = this.$t('home.Absolute');
                this.targetLimitMin = 0;
                this.targetLimitMax = this.currentLimitList.max_power > 0 ? this.currentLimitList.max_power : 2250;
            }
            this.targetLimitRelative = isRelative;
        },

        onShowReactiveSettings(serial: string) {
            this.showAlertReactive = false;
            this.targetReactiveList.serial = '';
            this.targetReactiveList.reactive_value = 0;
            this.onSelectReactiveType(true);

            this.reactiveSettingLoading = true;
            fetch('/api/reactivepower/status', { headers: authHeader() })
                .then((response) => handleResponse(response, this.$emitter, this.$router))
                .then((data) => {
                    this.currentReactiveList = data[serial];
                    this.targetReactiveList.serial = serial;
                    this.reactiveSettingLoading = false;
                });

            this.reactiveSettingView.show();
        },
        onSetReactiveSettings(setPersistent: boolean) {
            if (setPersistent) {
                if (this.targetReactiveRelative) {
                    this.targetReactiveList.reactive_type = LimitType.RelativPersistent;
                } else {
                    this.targetReactiveList.reactive_type = LimitType.AbsolutPersistent;
                }
            } else {
                if (this.targetReactiveRelative) {
                    this.targetReactiveList.reactive_type = LimitType.RelativNonPersistent;
                } else {
                    this.targetReactiveList.reactive_type = LimitType.AbsolutNonPersistent;
                }
            }
            const formData = new FormData();
            formData.append('data', JSON.stringify(this.targetReactiveList));

            fetch('/api/reactivepower/config', {
                method: 'POST',
                headers: authHeader(),
                body: formData,
            })
                .then((response) => handleResponse(response, this.$emitter, this.$router))
                .then((response) => {
                    if (response.type == 'success') {
                        this.reactiveSettingView.hide();
                    } else {
                        this.alertMessageReactive = this.$t('apiresponse.' + response.code, response.param);
                        this.alertTypeReactive = response.type;
                        this.showAlertReactive = true;
                    }
                });
        },
        onSelectReactiveType(isRelative: boolean) {
            if (isRelative) {
                this.targetReactiveTypeText = this.$t('home.Relative');
                this.targetReactiveMin = -100;
                this.targetReactiveMax = 100;
            } else {
                this.targetReactiveTypeText = this.$t('home.Absolute');
                const maxPow = this.currentReactiveList.max_power > 0 ? this.currentReactiveList.max_power : 2250;
                this.targetReactiveMin = -maxPow;
                this.targetReactiveMax = maxPow;
            }
            this.targetReactiveRelative = isRelative;
        },

        onShowPowerFactorSettings(serial: string) {
            this.showAlertPowerFactor = false;
            this.targetPowerFactorList.serial = '';
            this.targetPowerFactorList.power_factor = 0;

            this.powerFactorSettingLoading = true;
            fetch('/api/powerfactor/status', { headers: authHeader() })
                .then((response) => handleResponse(response, this.$emitter, this.$router))
                .then((data) => {
                    this.currentPowerFactorList = data[serial];
                    this.targetPowerFactorList.serial = serial;
                    this.powerFactorSettingLoading = false;
                });

            this.powerFactorSettingView.show();
        },
        onSetPowerFactorSettings(setPersistent: boolean) {
            this.targetPowerFactorList.power_factor_type = setPersistent
                ? LimitType.RelativPersistent
                : LimitType.RelativNonPersistent;

            const formData = new FormData();
            formData.append('data', JSON.stringify(this.targetPowerFactorList));

            fetch('/api/powerfactor/config', {
                method: 'POST',
                headers: authHeader(),
                body: formData,
            })
                .then((response) => handleResponse(response, this.$emitter, this.$router))
                .then((response) => {
                    if (response.type == 'success') {
                        this.powerFactorSettingView.hide();
                    } else {
                        this.alertMessagePowerFactor = this.$t('apiresponse.' + response.code, response.param);
                        this.alertTypePowerFactor = response.type;
                        this.showAlertPowerFactor = true;
                    }
                });
        },

        onShowPowerSettings(serial: string) {
            this.showAlertPower = false;
            this.powerSettingSerial = '';
            this.powerSettingLoading = true;
            fetch('/api/power/status', { headers: authHeader() })
                .then((response) => handleResponse(response, this.$emitter, this.$router))
                .then((data) => {
                    this.successCommandPower = data[serial].power_set_status;
                    this.powerSettingSerial = serial;
                    this.powerSettingLoading = false;
                });
            this.powerSettingView.show();
        },

        onSetPowerSettings(turnOn: boolean, restart = false) {
            const data = restart
                ? {
                      serial: this.powerSettingSerial,
                      restart: true,
                  }
                : {
                      serial: this.powerSettingSerial,
                      power: turnOn,
                  };

            const formData = new FormData();
            formData.append('data', JSON.stringify(data));

            console.log(data);

            fetch('/api/power/config', {
                method: 'POST',
                headers: authHeader(),
                body: formData,
            })
                .then((response) => handleResponse(response, this.$emitter, this.$router))
                .then((response) => {
                    if (response.type == 'success') {
                        this.powerSettingView.hide();
                    } else {
                        this.alertMessagePower = this.$t('apiresponse.' + response.code, response.param);
                        this.alertTypePower = response.type;
                        this.showAlertPower = true;
                    }
                });
        },
        onShowYieldTotalSettings(inverter: Inverter) {
            this.showAlertYieldTotal = false;
            this.yieldTotalSerial = inverter.serial;
            this.currentYieldTotalStatus = 'Unknown';

            const channelCount = inverter.DC ? Object.keys(inverter.DC).length : 1;
            // Real inverters only ship in 1/2/4-string variants; a 3-string reading is treated as 4.
            const slots = channelCount <= 1 ? 1 : channelCount <= 2 ? 2 : 4;
            this.targetYieldTotalKwh = Array.from({ length: slots }, () => 0);

            for (let i = 0; i < slots; i++) {
                const wh = inverter.DC?.[i]?.YieldTotal?.v;
                if (typeof wh === 'number') {
                    // YieldTotal in live-data is reported in kWh with fractional digits;
                    // pre-fill the input rounded to the nearest whole kWh.
                    this.targetYieldTotalKwh[i] = Math.round(wh);
                }
            }

            this.yieldTotalSettingLoading = true;
            fetch('/api/yieldtotal/status', { headers: authHeader() })
                .then((response) => handleResponse(response, this.$emitter, this.$router))
                .then((data: Record<string, YieldTotalStatus>) => {
                    this.currentYieldTotalStatus = data[inverter.serial]?.yield_total_set_status || 'Unknown';
                    this.yieldTotalSettingLoading = false;
                });

            this.yieldTotalSettingView.show();
        },
        onSetYieldTotalSettings() {
            const payload = {
                serial: this.yieldTotalSerial,
                values: this.targetYieldTotalKwh.map((v) => Math.max(0, Math.floor(v * 1000))),
            };

            const formData = new FormData();
            formData.append('data', JSON.stringify(payload));

            fetch('/api/yieldtotal/config', {
                method: 'POST',
                headers: authHeader(),
                body: formData,
            })
                .then((response) => handleResponse(response, this.$emitter, this.$router))
                .then((response) => {
                    if (response.type == 'success') {
                        this.yieldTotalSettingView.hide();
                    } else {
                        this.alertMessageYieldTotal = this.$t('apiresponse.' + response.code, response.param);
                        this.alertTypeYieldTotal = response.type;
                        this.showAlertYieldTotal = true;
                    }
                });
        },
        getSumIrridiation(inv: Inverter): number {
            let total = 0;
            Object.keys(inv.DC).forEach((key) => {
                total += inv.DC[key as unknown as number]?.Irradiation?.max || 0;
            });
            return total;
        },
        ratio(val_small: number, val_large: number): string {
            if (val_large == 0) {
                return '-';
            }
            return this.$n(val_small / val_large, 'percent');
        },
    },
});
</script>

<style scoped>
.btn-group {
    border-radius: var(--bs-border-radius);
    margin-top: 0.25rem;
}
</style>
