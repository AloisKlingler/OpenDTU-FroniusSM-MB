import { LimitType } from '@/types/LimitConfig';

export interface ReactivePowerConfig {
    serial: string;
    reactive_value: number;
    reactive_type: LimitType;
}

export interface ReactivePowerStatus {
    max_power: number;
    reactive_relative: number;
    reactive_set_status: string;
}
