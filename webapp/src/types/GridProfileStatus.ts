export interface GridProfileValue {
    n: string;
    u: string;
    v: number;
}

export interface GridProfileSection {
    name: string;
    items: Array<GridProfileValue>;
}

export interface GridProfileStatus {
    name: string;
    version: string;
    last_update?: number;
    /** Model of the inverter, e.g. "HMT-2250-6T". */
    inverter_type?: string;
    matchedPresetId: number;
    matchesPreset: boolean;
    sections: Array<GridProfileSection>;
}
