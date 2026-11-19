import { LimitType } from '@/types/LimitConfig';

export interface PowerFactorConfig {
    serial: string;
    power_factor: number;
    power_factor_type: LimitType;
}

export interface PowerFactorStatus {
    power_factor: number;
    power_factor_set_status: string;
}
