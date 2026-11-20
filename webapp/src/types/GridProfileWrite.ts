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

/** Result of decoding a profile blob with the parser of the device (no write). */
export interface GridProfileDecodeResponse {
    type?: string;
    /** True only if the device fully understands the profile. */
    valid: boolean;
    length: number;
    name: string;
    version: string;
    sections: Array<{
        name: string;
        items: Array<{ n: string; u: string; v: number }>;
    }>;
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
