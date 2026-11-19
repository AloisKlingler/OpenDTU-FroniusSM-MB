export interface GridProfilePreset {
    id: number;
    label: string;
    size: number;
}

export interface GridProfileKnownList {
    profiles: Array<GridProfilePreset>;
}

export interface GridProfileWriteStatus {
    running: boolean;
    state: 'Ok' | 'Failure' | 'Pending' | 'Unknown';
    last_update: number;
    profile_last_update?: number;
    queue_has_write: boolean;
}

export interface GridProfileWriteRequest {
    serial: string;
    preset_id?: number;
    profile_hex?: string;
    sections?: Array<{
        name?: string;
        items: Array<{ n?: string; u?: string; v: number }>;
    }>;
}
