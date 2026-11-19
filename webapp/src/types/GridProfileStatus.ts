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
    matchedPresetId: number;
    matchesPreset: boolean;
    sections: Array<GridProfileSection>;
}
