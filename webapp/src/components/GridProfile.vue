<template>
    <BootstrapAlert :show="!hasValidData">
        <h4 class="alert-heading"><BIconInfoSquare class="fs-2" />&nbsp;{{ $t('gridprofile.NoInfo') }}</h4>
        {{ $t('gridprofile.NoInfoLong') }}
    </BootstrapAlert>

    <template v-if="hasValidData">
        <BootstrapAlert :show="isModified" variant="warning">
            {{ $t('gridprofile.ModifiedWarning') }}
        </BootstrapAlert>

        <table class="table table-hover">
            <tbody>
                <tr>
                    <td>{{ $t('gridprofile.Name') }}</td>
                    <td>
                        {{ gridProfileList.name }}
                        <span v-if="isModified" class="badge bg-warning text-dark ms-1">
                            {{ $t('gridprofile.Modified') }}
                        </span>
                    </td>
                </tr>
                <tr>
                    <td>{{ $t('gridprofile.Version') }}</td>
                    <td>{{ gridProfileList.version }}</td>
                </tr>
            </tbody>
        </table>

        <!-- Write / edit control panel -->
        <div class="card mb-3" v-if="inverterSerial">
            <div class="card-header d-flex justify-content-between align-items-center">
                <span>{{ $t('gridprofile.WriteTitle') }}</span>
                <span v-if="writeStatus.running || writeStatus.state === 'Pending'" class="badge bg-warning text-dark">
                    {{ $t('gridprofile.WriteStatePending') }}
                </span>
                <span v-else-if="writeStatus.state === 'Ok'" class="badge bg-success">
                    {{ $t('gridprofile.WriteStateOk') }}
                </span>
                <span v-else-if="writeStatus.state === 'Failure'" class="badge bg-danger">
                    {{ $t('gridprofile.WriteStateFailure') }}
                </span>
            </div>
            <div class="card-body">
                <BootstrapAlert :show="writeAlertShow" :variant="writeAlertVariant">
                    {{ writeAlertMessage }}
                </BootstrapAlert>

                <BootstrapAlert :show="true" variant="warning">
                    <strong>{{ $t('gridprofile.WriteWarningTitle') }}</strong>
                    <div v-html="$t('gridprofile.WriteWarningBody')"></div>
                </BootstrapAlert>

                <div class="row g-2 align-items-end" v-if="presets.length > 0">
                    <div class="col-md-8">
                        <label class="form-label">{{ $t('gridprofile.WritePresetLabel') }}</label>
                        <select v-model="selectedPresetId" class="form-select" :disabled="writeBusy">
                            <option :value="-1" disabled>{{ $t('gridprofile.WritePresetChoose') }}</option>
                            <option v-for="p in presets" :key="p.id" :value="p.id">
                                {{ p.label }} ({{ p.size }} B)
                            </option>
                        </select>
                    </div>
                    <div class="col-md-4">
                        <button
                            type="button"
                            class="btn btn-primary w-100"
                            :disabled="writeBusy || selectedPresetId < 0"
                            @click="applyPreset"
                        >
                            {{ $t('gridprofile.WriteApplyPreset') }}
                        </button>
                    </div>
                </div>

                <div class="row g-2 mt-2">
                    <div class="col-md-8">
                        <div class="form-check form-switch">
                            <input
                                class="form-check-input"
                                type="checkbox"
                                id="editModeSwitch"
                                v-model="editMode"
                                :disabled="writeBusy"
                            />
                            <label class="form-check-label" for="editModeSwitch">
                                {{ $t('gridprofile.WriteEditModeToggle') }}
                            </label>
                        </div>
                    </div>
                    <div class="col-md-4">
                        <button
                            type="button"
                            class="btn btn-warning w-100"
                            :disabled="!editMode || writeBusy || !editedProfile"
                            @click="applyEdited"
                        >
                            {{ $t('gridprofile.WriteApplyEdits') }}
                        </button>
                    </div>
                </div>

                <div class="mt-2" v-if="writeStatus.running || writeStatus.queue_has_write">
                    <button type="button" class="btn btn-outline-danger btn-sm" @click="abortWrite">
                        {{ $t('gridprofile.WriteAbort') }}
                    </button>
                </div>
            </div>
        </div>

        <div class="accordion" id="accordionProfile">
            <div class="accordion-item accordion-table" v-for="(section, index) in displaySections" :key="index">
                <h2 class="accordion-header">
                    <button
                        class="accordion-button collapsed"
                        type="button"
                        data-bs-toggle="collapse"
                        :data-bs-target="`#collapse${index}`"
                        aria-expanded="true"
                        :aria-controls="`collapse${index}`"
                    >
                        {{ section.name }}
                    </button>
                </h2>
                <div :id="`collapse${index}`" class="accordion-collapse collapse" data-bs-parent="#accordionProfile">
                    <div class="accordion-body">
                        <table class="table table-hover">
                            <tbody>
                                <tr v-for="(value, vIdx) in section.items" :key="value.n + '_' + vIdx">
                                    <th>{{ value.n }}</th>
                                    <td>
                                        <template v-if="editMode && value.u != 'bool'">
                                            <div class="input-group input-group-sm">
                                                <input
                                                    type="number"
                                                    class="form-control"
                                                    step="any"
                                                    :min="capMin(value.n)"
                                                    :max="capMax(value.n)"
                                                    :value="value.v"
                                                    @input="
                                                        onEditValue(
                                                            index,
                                                            vIdx,
                                                            ($event.target as HTMLInputElement).value
                                                        )
                                                    "
                                                />
                                                <span class="input-group-text">{{ value.u }}</span>
                                                <span
                                                    v-if="directionHint(value)"
                                                    class="input-group-text"
                                                    :title="$t('gridprofile.PfHint')"
                                                    >{{ directionHint(value) }}</span
                                                >
                                            </div>
                                        </template>
                                        <template v-else-if="editMode && value.u == 'bool'">
                                            <select
                                                class="form-select form-select-sm"
                                                :value="value.v"
                                                @change="
                                                    onEditValue(index, vIdx, ($event.target as HTMLSelectElement).value)
                                                "
                                            >
                                                <option :value="1">{{ $t('gridprofile.Enabled') }}</option>
                                                <option :value="0">{{ $t('gridprofile.Disabled') }}</option>
                                            </select>
                                        </template>
                                        <template v-else-if="value.u != 'bool'">
                                            {{ $n(value.v, 'decimal') }} {{ value.u }}
                                            <span
                                                v-if="directionHint(value)"
                                                class="badge bg-secondary ms-1"
                                                :title="$t('gridprofile.PfHint')"
                                                >{{ directionHint(value) }}</span
                                            >
                                        </template>
                                        <template v-else>
                                            <StatusBadge
                                                :status="value.v == 1"
                                                true_text="gridprofile.Enabled"
                                                false_text="gridprofile.Disabled"
                                            />
                                        </template>
                                    </td>
                                </tr>
                            </tbody>
                        </table>
                    </div>
                </div>
            </div>
        </div>

        <br />

        <div class="accordion" id="accordionDev">
            <div class="accordion-item">
                <h2 class="accordion-header">
                    <button
                        class="accordion-button collapsed"
                        type="button"
                        data-bs-toggle="collapse"
                        data-bs-target="#collapseDev"
                        aria-expanded="true"
                        aria-controls="collapseDev"
                    >
                        {{ $t('gridprofile.GridprofileSupport') }}
                    </button>
                </h2>
                <div id="collapseDev" class="accordion-collapse collapse" data-bs-parent="#accordionDev">
                    <div class="accordion-body">
                        <BootstrapAlert :show="true" variant="danger">
                            <h4 class="info-heading">
                                <BIconInfoSquare class="fs-2" />&nbsp;{{ $t('gridprofile.GridprofileSupport') }}
                            </h4>
                            <div v-html="$t('gridprofile.GridprofileSupportLong')"></div>
                        </BootstrapAlert>
                        <samp>
                            {{ rawContent() }}
                        </samp>
                    </div>
                </div>
            </div>
        </div>
    </template>
</template>

<script lang="ts">
import BootstrapAlert from '@/components/BootstrapAlert.vue';
import type { GridProfileRawdata } from '@/types/GridProfileRawdata';
import type { GridProfileSection, GridProfileStatus } from '@/types/GridProfileStatus';
import type {
    GridProfileKnownList,
    GridProfilePreset,
    GridProfileWriteRequest,
    GridProfileWriteStatus,
} from '@/types/GridProfileWrite';
import { authHeader, handleResponse } from '@/utils/authentication';
import { BIconInfoSquare } from 'bootstrap-icons-vue';
import { defineComponent, type PropType } from 'vue';
import StatusBadge from './StatusBadge.vue';

interface EditedItem {
    n: string;
    u: string;
    v: number;
}
interface EditedSection {
    name: string;
    items: EditedItem[];
}

const GRID_CAPS: Record<string, [number, number]> = {
    // H/LVRT (V)
    'Low Voltage 1 (LV1)': [170, 195.5],
    'High Voltage 1 (HV1)': [253, 264],
    'Low Voltage 2 (LV2)': [140, 161],
    'High Voltage 2 (HV2)': [264, 275],
    'High Voltage 3 (HV3)': [275, 280],
    '10mins Average High Voltage (AHV)': [245, 253],
    // H/LFRT (Hz)
    'Low Frequency 1 (LF1)': [48, 49.9],
    'High Frequency 1 (HF1)': [50.1, 51],
    'Low Frequency 2 (LF2)': [47.5, 47.9],
    'High Frequency 2 (HF2)': [51.1, 52],
    // Reconnection
    'Reconnect Time (RT)': [10, 300],
    'Reconnect High Voltage (RHV)': [240, 253],
    'Reconnect Low Voltage (RLV)': [195.5, 210],
    'Reconnect High Frequency (RHF)': [50.1, 50.9],
    'Reconnect Low Frequency (RLF)': [49, 49.9],
    // Ramp rate
    'Normal Ramp up Rate(RUR_NM)': [10, 100],
    'Soft Start Ramp up Rate (RUR_SS)': [0.1, 10],
    // Frequency Watt
    'Start of Frequency Watt Droop (Fstart)': [50.2, 52],
    'FW Droop Slope (Kpower_Freq)': [16.7, 100],
    'Recovery Ramp Rate (RRR)': [0.1, 50],
    'FW Settling Time (Tr)': [0, 2],
    // Volt Var
    'Reactive Set Point Q1': [0, 50],
    'Reactive Set Point Q4': [0, 50],
    // Active Power Control
    'Power Ramp Rate (PRR)': [0.33, 0.66],
    // Specified / rated power factor (signed: sign = leading/lagging)
    'Power Factor (PF)': [-0.9, 0.9],
    'Power Factor ar Rated Power (PFRP)': [-0.9, 0.9],
    // Reactive Power Control (signed %Sn)
    'Reactive Power (VAR)': [-50, 50],
};

export default defineComponent({
    components: {
        BootstrapAlert,
        BIconInfoSquare,
        StatusBadge,
    },
    emits: ['refresh'],
    props: {
        gridProfileList: { type: Object as PropType<GridProfileStatus>, required: true },
        gridProfileRawList: { type: Object as PropType<GridProfileRawdata>, required: true },
        inverterSerial: { type: String, default: '' },
    },
    data() {
        return {
            presets: [] as GridProfilePreset[],
            selectedPresetId: -1 as number,
            editMode: false,
            editedSections: null as EditedSection[] | null,
            writeStatus: {
                running: false,
                state: 'Unknown',
                last_update: 0,
                queue_has_write: false,
            } as GridProfileWriteStatus,
            writeBusy: false,
            writeAlertShow: false,
            writeAlertVariant: 'info' as 'info' | 'success' | 'danger' | 'warning',
            writeAlertMessage: '',
            statusTimer: null as number | null,
            // profile_last_update value a reload was already requested for
            // (prevents re-emitting on every poll while the reload is in flight).
            refreshRequestedFor: null as number | null,
        };
    },
    computed: {
        rawContent() {
            return () => {
                return this.gridProfileRawList.raw
                    .map(function (x) {
                        let y = x.toString(16);
                        y = ('00' + y).substr(-2);
                        return y;
                    })
                    .join(' ');
            };
        },
        hasValidData(): boolean {
            return this.gridProfileRawList.raw.reduce((sum, x) => sum + x, 0) > 0;
        },
        displaySections(): GridProfileSection[] | EditedSection[] {
            if (this.editMode && this.editedSections) {
                return this.editedSections;
            }
            return this.gridProfileList.sections || [];
        },
        editedProfile(): boolean {
            return !!this.editedSections;
        },
        isModified(): boolean {
            return this.gridProfileList.matchedPresetId >= 0 && !this.gridProfileList.matchesPreset;
        },
    },
    watch: {
        editMode(newVal: boolean) {
            if (newVal) {
                this.editedSections = (this.gridProfileList.sections || []).map((s) => ({
                    name: s.name,
                    items: s.items.map((i) => ({ n: i.n, u: i.u, v: i.v })),
                }));
            } else {
                this.editedSections = null;
            }
        },
        inverterSerial: {
            immediate: true,
            handler(newVal: string) {
                this.refreshRequestedFor = null;
                if (newVal) {
                    this.fetchPresets();
                    this.fetchWriteStatus();
                    this.startPolling();
                }
            },
        },
    },
    beforeUnmount() {
        this.stopPolling();
    },
    methods: {
        onEditValue(sectionIdx: number, itemIdx: number, raw: string) {
            if (!this.editedSections) return;
            const section = this.editedSections[sectionIdx];
            if (!section) return;
            const item = section.items[itemIdx];
            if (!item) return;
            let n = Number(raw);
            if (Number.isFinite(n)) {
                const cap = GRID_CAPS[item.n];
                if (cap) {
                    if (n < cap[0]) n = cap[0];
                    else if (n > cap[1]) n = cap[1];
                }
                item.v = n;
            }
        },
        capMin(name: string): number | undefined {
            const cap = GRID_CAPS[name];
            return cap ? cap[0] : undefined;
        },
        capMax(name: string): number | undefined {
            const cap = GRID_CAPS[name];
            return cap ? cap[1] : undefined;
        },
        directionHint(value: { n: string; u: string; v: number }): string {
            if (value.u === '' && value.n.includes('Power Factor')) {
                if (value.v > 0) return this.$t('gridprofile.PfLeading');
                if (value.v < 0) return this.$t('gridprofile.PfLagging');
                return this.$t('gridprofile.PfUnity');
            }
            if (value.n.includes('Reactive Power')) {
                if (value.v > 0) return this.$t('gridprofile.PfLeading');
                if (value.v < 0) return this.$t('gridprofile.PfLagging');
            }
            return '';
        },
        fetchPresets() {
            fetch('/api/gridprofile/knownprofiles', { headers: authHeader() })
                .then((r) => (r.ok ? r.json() : Promise.reject(r.statusText)))
                .then((data: GridProfileKnownList) => {
                    this.presets = data.profiles || [];
                })
                .catch(() => {
                    this.presets = [];
                });
        },
        fetchWriteStatus() {
            if (!this.inverterSerial) return;
            fetch(`/api/gridprofile/writestatus?inv=${this.inverterSerial}`, { headers: authHeader() })
                .then((r) => (r.ok ? r.json() : Promise.reject(r.statusText)))
                .then((data: GridProfileWriteStatus) => {
                    this.writeStatus = data;
                    this.checkProfileUpdated(data.profile_last_update);
                })
                .catch(() => {
                    /* transient - keep last known state */
                });
        },
        startPolling() {
            this.stopPolling();
            this.statusTimer = window.setInterval(() => this.fetchWriteStatus(), 1500);
        },
        stopPolling() {
            if (this.statusTimer !== null) {
                window.clearInterval(this.statusTimer);
                this.statusTimer = null;
            }
        },
        postWrite(payload: GridProfileWriteRequest) {
            this.writeBusy = true;
            this.writeAlertShow = false;

            const formData = new FormData();
            formData.append('data', JSON.stringify(payload));

            fetch('/api/gridprofile/write', {
                method: 'POST',
                headers: authHeader(),
                body: formData,
            })
                .then((response) => handleResponse(response, this.$emitter, this.$router))
                .then((response) => {
                    this.writeBusy = false;
                    this.writeAlertShow = true;
                    if (response && response.type === 'success') {
                        this.writeAlertVariant = 'success';
                        this.writeAlertMessage = this.$t('gridprofile.WriteQueued');
                        this.fetchWriteStatus();
                    } else {
                        this.writeAlertVariant = 'danger';
                        this.writeAlertMessage = (response && response.message) || 'Error';
                    }
                })
                .catch(() => {
                    this.writeBusy = false;
                    this.writeAlertShow = true;
                    this.writeAlertVariant = 'danger';
                    this.writeAlertMessage = 'Network error';
                });
        },
        applyPreset() {
            if (this.selectedPresetId < 0 || !this.inverterSerial) return;
            const ok = window.confirm(this.$t('gridprofile.WriteConfirm'));
            if (!ok) return;
            this.postWrite({
                serial: this.inverterSerial,
                preset_id: this.selectedPresetId,
            });
        },
        applyEdited() {
            if (!this.editedSections || !this.inverterSerial) return;
            const ok = window.confirm(this.$t('gridprofile.WriteConfirm'));
            if (!ok) return;
            this.postWrite({
                serial: this.inverterSerial,
                sections: this.editedSections.map((s) => ({
                    name: s.name,
                    items: s.items.map((i) => ({ n: i.n, u: i.u, v: i.v })),
                })),
            });
        },
        abortWrite() {
            if (!this.inverterSerial) return;
            const formData = new FormData();
            formData.append('data', JSON.stringify({ serial: this.inverterSerial }));
            fetch('/api/gridprofile/abort', {
                method: 'POST',
                headers: authHeader(),
                body: formData,
            })
                .then((response) => handleResponse(response, this.$emitter, this.$router))
                .then(() => this.fetchWriteStatus());
        },
        checkProfileUpdated(lastUpdate: number | undefined) {
            const shown = this.gridProfileList.last_update;
            if (lastUpdate === undefined || shown === undefined) return;
            // The DTU holds a newer profile read than the one displayed (after a
            // write, or after the reload button queued a read): reload the view.
            if (lastUpdate !== shown && lastUpdate !== this.refreshRequestedFor) {
                this.refreshRequestedFor = lastUpdate;
                this.$emit('refresh');
            }
        },
    },
});
</script>
