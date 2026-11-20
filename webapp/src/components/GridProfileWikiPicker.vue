<template>
    <div class="border rounded p-3 mt-3">
        <div class="d-flex justify-content-between align-items-center flex-wrap gap-2 mb-2">
            <h6 class="mb-0">{{ $t('gridprofile.WikiTitle') }}</h6>
            <div class="d-flex align-items-center gap-2">
                <span v-if="wiki" class="badge" :class="sourceBadgeClass">{{ sourceText }}</span>
                <button
                    type="button"
                    class="btn btn-outline-secondary btn-sm"
                    :disabled="loading || !liveEnabled"
                    @click="load(true)"
                >
                    {{ $t('gridprofile.WikiReload') }}
                </button>
            </div>
        </div>

        <div class="form-check form-switch">
            <input
                class="form-check-input"
                type="checkbox"
                id="wikiLiveSwitch"
                :checked="liveEnabled"
                @change="onToggleLive(($event.target as HTMLInputElement).checked)"
            />
            <label class="form-check-label" for="wikiLiveSwitch">{{ $t('gridprofile.WikiLiveSwitch') }}</label>
        </div>
        <div class="form-text mb-2">{{ $t('gridprofile.WikiPrivacy') }}</div>

        <div v-if="wiki && wiki.error" class="alert alert-warning py-2">
            {{ $t('gridprofile.WikiFallback', { reason: wiki.error }) }}
        </div>
        <div v-if="!modelKnown" class="alert alert-info py-2">
            {{
                inverterType
                    ? $t('gridprofile.WikiModelAmbiguous', { model: inverterType })
                    : $t('gridprofile.WikiNoModel')
            }}
        </div>

        <template v-else>
            <div v-if="loading" class="text-muted">{{ $t('gridprofile.WikiLoading') }}</div>

            <template v-else>
                <p v-if="candidates.length === 0" class="text-muted mb-0">
                    {{ $t('gridprofile.WikiNoCandidates', { model: inverterType }) }}
                </p>

                <template v-else>
                    <label class="form-label" for="wikiProfileSelect">
                        {{ $t('gridprofile.WikiSelectLabel', { model: inverterType }) }}
                    </label>
                    <select id="wikiProfileSelect" v-model="selectedHex" class="form-select" :disabled="busy">
                        <option value="" disabled>{{ $t('gridprofile.WikiChoose') }}</option>
                        <optgroup
                            v-if="exactCandidates.length > 0"
                            :label="$t('gridprofile.WikiGroupExact', { model: inverterType })"
                        >
                            <option v-for="c in exactCandidates" :key="c.hex" :value="c.hex">
                                {{ optionText(c) }}
                            </option>
                        </optgroup>
                        <optgroup
                            v-if="showFamily && familyCandidates.length > 0"
                            :label="$t('gridprofile.WikiGroupFamily', { family: family })"
                        >
                            <option v-for="c in familyCandidates" :key="c.hex" :value="c.hex">
                                {{ optionText(c) }}
                            </option>
                        </optgroup>
                    </select>

                    <div v-if="familyCandidates.length > 0" class="form-check form-switch mt-2">
                        <input
                            class="form-check-input"
                            type="checkbox"
                            id="wikiShowFamily"
                            v-model="showFamily"
                            :disabled="busy"
                        />
                        <label class="form-check-label" for="wikiShowFamily">
                            {{ $t('gridprofile.WikiShowFamily', { family: family }) }}
                        </label>
                    </div>
                </template>

                <div v-if="selected" class="mt-3">
                    <div class="d-flex flex-wrap gap-2 mb-2">
                        <span v-if="selected.level === 'exact'" class="badge bg-success">
                            {{ $t('gridprofile.WikiVerified', { count: selected.exactCount }) }}
                        </span>
                        <span v-else class="badge bg-warning text-dark">{{ $t('gridprofile.WikiSisterModel') }}</span>
                        <span v-if="isLiveOnly(selected)" class="badge bg-info text-dark">
                            {{ $t('gridprofile.WikiLiveOnly') }}
                        </span>
                        <span v-if="isActive(selected)" class="badge bg-secondary">
                            {{ $t('gridprofile.WikiIsActive') }}
                        </span>
                    </div>

                    <div v-if="decodeState === 'loading'" class="text-muted small">
                        {{ $t('gridprofile.WikiDecoding') }}
                    </div>
                    <div v-else-if="decodeState === 'invalid'" class="alert alert-danger py-2">
                        {{ $t('gridprofile.WikiNotDecodable') }}
                    </div>
                    <div v-else-if="decodeState === 'error'" class="alert alert-danger py-2">{{ decodeError }}</div>

                    <template v-else-if="decodeState === 'valid' && decoded">
                        <p class="small mb-1">
                            {{
                                $t('gridprofile.WikiDecodedAs', {
                                    name: decoded.name,
                                    version: decoded.version,
                                    length: decoded.length,
                                })
                            }}
                        </p>
                        <p v-if="diff.length === 0" class="small text-success mb-2">
                            {{ $t('gridprofile.WikiNoDiff') }}
                        </p>
                        <details v-else class="mb-2">
                            <summary class="small">
                                {{ $t('gridprofile.WikiDiffCount', { count: diff.length }) }}
                            </summary>
                            <div class="table-responsive">
                                <table class="table table-sm table-hover small mb-0">
                                    <thead>
                                        <tr>
                                            <th>{{ $t('gridprofile.WikiColValue') }}</th>
                                            <th class="text-end">{{ $t('gridprofile.WikiColCurrent') }}</th>
                                            <th class="text-end">{{ $t('gridprofile.WikiColNew') }}</th>
                                        </tr>
                                    </thead>
                                    <tbody>
                                        <tr v-for="(d, i) in diff" :key="i">
                                            <td>
                                                {{ d.name }}
                                                <span class="text-muted">({{ d.section }})</span>
                                            </td>
                                            <td class="text-end">{{ formatValue(d.from, d.unit) }}</td>
                                            <td class="text-end fw-bold">{{ formatValue(d.to, d.unit) }}</td>
                                        </tr>
                                    </tbody>
                                </table>
                            </div>
                        </details>
                    </template>

                    <details class="mb-2">
                        <summary class="small">
                            {{ $t('gridprofile.WikiSources', { count: selected.sources.length }) }}
                        </summary>
                        <ul class="small mb-0">
                            <li v-for="(s, i) in selected.sources" :key="i">
                                {{ s.country }} · {{ s.model }} · FW {{ s.fw || '?' }} · {{ s.name || '-' }}
                            </li>
                        </ul>
                    </details>

                    <button
                        type="button"
                        class="btn btn-primary"
                        :disabled="busy || decodeState !== 'valid' || isActive(selected)"
                        @click="apply"
                    >
                        {{ $t('gridprofile.WikiApply') }}
                    </button>
                </div>
            </template>
        </template>

        <details v-if="exportRow" class="mt-3">
            <summary class="small">{{ $t('gridprofile.WikiExportTitle') }}</summary>
            <p class="small mt-2 mb-1">{{ $t('gridprofile.WikiExportHelp') }}</p>
            <textarea
                class="form-control font-monospace small"
                rows="4"
                readonly
                :value="exportRow"
                @focus="($event.target as HTMLTextAreaElement).select()"
            ></textarea>
            <button type="button" class="btn btn-outline-secondary btn-sm mt-1" @click="copyExport">
                {{ copied ? $t('gridprofile.WikiCopied') : $t('gridprofile.WikiCopy') }}
            </button>
        </details>
    </div>
</template>

<script lang="ts">
import type { GridProfileSection } from '@/types/GridProfileStatus';
import type { GridProfileDecodeResponse } from '@/types/GridProfileWrite';
import { authHeader, handleResponse } from '@/utils/authentication';
import {
    buildCandidates,
    buildWikiRow,
    bytesToHex,
    diffSections,
    modelFamily,
    normalizeInverterType,
    stripCrc,
    type DiffItem,
    type ProfileCandidate,
} from '@/utils/gridProfileWiki';
import {
    isLiveEnabled,
    loadWikiData,
    setLiveEnabled,
    snapshotHexSet,
    type WikiData,
} from '@/utils/gridProfileWikiSource';
import { defineComponent, type PropType } from 'vue';

type DecodeState = 'idle' | 'loading' | 'valid' | 'invalid' | 'error';

export default defineComponent({
    props: {
        /** Model reported by the DTU, e.g. "HMT-2250-6T". */
        inverterType: { type: String, default: '' },
        /** Name of the active profile as decoded by the DTU. */
        currentName: { type: String, default: '' },
        /** Raw bytes of the active profile (may end with the inverter's CRC16). */
        currentRaw: { type: Array as PropType<number[]>, default: () => [] },
        currentSections: { type: Array as PropType<GridProfileSection[]>, default: () => [] },
        busy: { type: Boolean, default: false },
    },
    emits: ['apply'],
    data() {
        return {
            wiki: null as WikiData | null,
            loading: true,
            liveEnabled: isLiveEnabled(),
            showFamily: false,
            selectedHex: '',
            decodeState: 'idle' as DecodeState,
            decoded: null as GridProfileDecodeResponse | null,
            decodeError: '',
            copied: false,
            snapshotHexes: snapshotHexSet(),
        };
    },
    computed: {
        modelKnown(): boolean {
            return normalizeInverterType(this.inverterType) !== '';
        },
        family(): string {
            return modelFamily(normalizeInverterType(this.inverterType));
        },
        candidates(): ProfileCandidate[] {
            return this.wiki ? buildCandidates(this.wiki.rows, this.inverterType) : [];
        },
        exactCandidates(): ProfileCandidate[] {
            return this.candidates.filter((c) => c.level === 'exact');
        },
        familyCandidates(): ProfileCandidate[] {
            return this.candidates.filter((c) => c.level === 'family');
        },
        selected(): ProfileCandidate | undefined {
            return this.candidates.find((c) => c.hex === this.selectedHex);
        },
        activeHex(): string {
            return this.currentRaw.length > 0 ? bytesToHex(stripCrc(this.currentRaw.slice())) : '';
        },
        diff(): DiffItem[] {
            if (this.decodeState !== 'valid' || !this.decoded) {
                return [];
            }
            return diffSections(this.currentSections, this.decoded.sections);
        },
        sourceText(): string {
            if (!this.wiki) {
                return '';
            }
            const date = this.wiki.date.slice(0, 10);
            switch (this.wiki.kind) {
                case 'live':
                    return this.$t('gridprofile.WikiSourceLive');
                case 'cache':
                    return this.$t('gridprofile.WikiSourceCache', { date });
                default:
                    return this.$t('gridprofile.WikiSourceSnapshot', { date });
            }
        },
        sourceBadgeClass(): string {
            return this.wiki?.kind === 'live' ? 'bg-success' : 'bg-secondary';
        },
        exportRow(): string {
            if (this.currentRaw.length < 7) {
                return '';
            }
            // The profile name starts with the country, e.g. "DE - DE_VDE4105_2018".
            const country = /^([A-Z]{2})\s-\s/.exec(this.currentName)?.[1] ?? 'XX';
            return buildWikiRow({
                country,
                model: normalizeInverterType(this.inverterType) || this.inverterType,
                profile: this.currentRaw,
                name: this.currentName,
                fw: '',
            });
        },
    },
    watch: {
        inverterType() {
            this.selectedHex = '';
        },
        selectedHex(hex: string) {
            this.decodeSelected(hex);
        },
    },
    mounted() {
        this.load(false);
    },
    methods: {
        async load(force: boolean) {
            this.loading = true;
            this.wiki = await loadWikiData(force);
            this.loading = false;
        },
        onToggleLive(enabled: boolean) {
            this.liveEnabled = enabled;
            setLiveEnabled(enabled);
            this.selectedHex = '';
            this.load(false);
        },
        isActive(c: ProfileCandidate): boolean {
            return this.activeHex !== '' && c.hex === this.activeHex;
        },
        isLiveOnly(c: ProfileCandidate): boolean {
            return this.wiki?.kind !== 'snapshot' && !this.snapshotHexes.has(c.hex);
        },
        optionText(c: ProfileCandidate): string {
            const count = c.level === 'exact' ? c.exactCount : c.familyCount;
            const active = this.isActive(c) ? ` - ${this.$t('gridprofile.WikiActiveMark')}` : '';
            return `${c.label} [${c.id}] - ${c.length} B - ${count}x${active}`;
        },
        formatValue(value: number | undefined, unit: string): string {
            return value === undefined ? '-' : `${this.$n(value, 'decimal')} ${unit}`.trim();
        },
        async decodeSelected(hex: string) {
            this.decoded = null;
            this.decodeError = '';
            if (!hex) {
                this.decodeState = 'idle';
                return;
            }
            this.decodeState = 'loading';

            // Let the firmware parser decode the candidate. This validates it
            // with the very code that is used when writing, and provides the
            // values for the comparison with the active profile.
            const formData = new FormData();
            formData.append('data', JSON.stringify({ profile_hex: hex }));
            try {
                const response = await fetch('/api/gridprofile/decode', {
                    method: 'POST',
                    headers: authHeader(),
                    body: formData,
                });
                const data = (await handleResponse(response, this.$emitter, this.$router)) as
                    (GridProfileDecodeResponse & { message?: string }) | undefined;
                if (this.selectedHex !== hex) {
                    return; // the selection changed while waiting
                }
                if (data && data.type === 'success') {
                    this.decoded = data;
                    this.decodeState = data.valid ? 'valid' : 'invalid';
                } else {
                    this.decodeState = 'error';
                    this.decodeError = data?.message || 'Error';
                }
            } catch {
                if (this.selectedHex === hex) {
                    this.decodeState = 'error';
                    this.decodeError = 'Network error';
                }
            }
        },
        apply() {
            const c = this.selected;
            if (!c || this.decodeState !== 'valid') {
                return;
            }
            let message = this.$t('gridprofile.WikiApplyConfirm', {
                label: c.label,
                id: c.id,
                model: this.inverterType,
                count: this.diff.length,
            });
            if (c.level !== 'exact') {
                message += this.$t('gridprofile.WikiApplySisterNote');
            }
            if (window.confirm(message)) {
                this.$emit('apply', c.hex);
            }
        },
        async copyExport() {
            try {
                await navigator.clipboard.writeText(this.exportRow);
                this.copied = true;
                window.setTimeout(() => (this.copied = false), 2000);
            } catch {
                /* clipboard is not available on plain http - the text can be selected manually */
            }
        },
    },
});
</script>
