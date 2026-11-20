// SPDX-License-Identifier: GPL-2.0-or-later
//
// Where the community grid profile list comes from: the live wiki page, a
// 24h browser cache of it, or the snapshot that is bundled with the firmware.
// The wiki is only ever contacted by the browser of the user, never by the DTU.

import snapshotFile from '@/data/gridprofile-wiki-snapshot.json';
import { parseWikiMarkdown, WIKI_RAW_URL, type WikiRow } from './gridProfileWiki';

export type WikiDataKind = 'live' | 'cache' | 'snapshot';

export interface WikiData {
    kind: WikiDataKind;
    rows: WikiRow[];
    /** When the data was fetched (live / cache) or generated (snapshot). */
    date: string;
    /** Set if the live data could not be used and a fallback was taken. */
    error?: string;
}

interface SnapshotFile {
    generatedAt: string;
    blobs: string[];
    rows: Array<[string, string, string, string, number]>;
}

const CACHE_KEY = 'gridprofile.wiki.cache.v1';
const LIVE_KEY = 'gridprofile.wiki.live';
const CACHE_TTL_MS = 24 * 60 * 60 * 1000;
const FETCH_TIMEOUT_MS = 8000;
/** A wiki page yielding less than this many rows is treated as broken. */
const MIN_PLAUSIBLE_ROWS = 20;

const HEX_PATTERN = /^([0-9a-f]{2} ){6,198}[0-9a-f]{2}$/;

const snapshot = snapshotFile as unknown as SnapshotFile;

function sanitizeRows(input: unknown): WikiRow[] {
    if (!Array.isArray(input)) {
        return [];
    }
    const out: WikiRow[] = [];
    for (const r of input) {
        if (
            r &&
            typeof r === 'object' &&
            typeof r.country === 'string' &&
            typeof r.model === 'string' &&
            typeof r.name === 'string' &&
            typeof r.fw === 'string' &&
            typeof r.hex === 'string' &&
            HEX_PATTERN.test(r.hex)
        ) {
            out.push({
                country: r.country.slice(0, 3),
                model: r.model.slice(0, 30),
                name: r.name.slice(0, 80),
                fw: r.fw.slice(0, 20),
                hex: r.hex,
            });
        }
    }
    return out;
}

export function snapshotRows(): WikiRow[] {
    const out: WikiRow[] = [];
    for (const [country, model, name, fw, idx] of snapshot.rows) {
        const hex = snapshot.blobs[idx];
        if (hex) {
            out.push({ country, model, name, fw, hex });
        }
    }
    return out;
}

export function snapshotDate(): string {
    return snapshot.generatedAt;
}

/** Profiles contained in the bundled snapshot, used to flag live-only entries. */
export function snapshotHexSet(): Set<string> {
    return new Set(snapshot.blobs);
}

export function isLiveEnabled(): boolean {
    try {
        return localStorage.getItem(LIVE_KEY) !== '0';
    } catch {
        return true;
    }
}

export function setLiveEnabled(enabled: boolean) {
    try {
        localStorage.setItem(LIVE_KEY, enabled ? '1' : '0');
    } catch {
        /* storage not available - the choice only lasts for this page view */
    }
}

function readCache(): { rows: WikiRow[]; fetchedAt: number } | undefined {
    try {
        const parsed = JSON.parse(localStorage.getItem(CACHE_KEY) || 'null');
        const rows = sanitizeRows(parsed?.rows);
        if (rows.length >= MIN_PLAUSIBLE_ROWS && typeof parsed?.fetchedAt === 'number') {
            return { rows, fetchedAt: parsed.fetchedAt };
        }
    } catch {
        /* ignore a corrupt cache */
    }
    return undefined;
}

function writeCache(rows: WikiRow[]) {
    try {
        localStorage.setItem(CACHE_KEY, JSON.stringify({ fetchedAt: Date.now(), rows }));
    } catch {
        /* quota exceeded or storage disabled - caching is optional */
    }
}

function fromSnapshot(error?: string): WikiData {
    return { kind: 'snapshot', rows: snapshotRows(), date: snapshotDate(), error };
}

async function fetchLive(): Promise<WikiRow[]> {
    const controller = new AbortController();
    const timer = window.setTimeout(() => controller.abort(), FETCH_TIMEOUT_MS);
    try {
        const response = await fetch(WIKI_RAW_URL, {
            signal: controller.signal,
            // Never send the address of the DTU as referrer and no credentials.
            referrerPolicy: 'no-referrer',
            credentials: 'omit',
        });
        if (!response.ok) {
            throw new Error(`HTTP ${response.status}`);
        }
        const { rows } = parseWikiMarkdown(await response.text());
        if (rows.length < MIN_PLAUSIBLE_ROWS) {
            throw new Error('unexpected wiki format');
        }
        return rows;
    } finally {
        window.clearTimeout(timer);
    }
}

/**
 * Loads the profile list. Order: fresh browser cache, live wiki, stale cache,
 * bundled snapshot. With live access switched off only the snapshot is used.
 */
export async function loadWikiData(forceReload = false): Promise<WikiData> {
    if (!isLiveEnabled()) {
        return fromSnapshot();
    }

    const cache = readCache();
    if (cache && !forceReload && Date.now() - cache.fetchedAt < CACHE_TTL_MS) {
        return { kind: 'cache', rows: cache.rows, date: new Date(cache.fetchedAt).toISOString() };
    }

    try {
        const rows = await fetchLive();
        writeCache(rows);
        return { kind: 'live', rows, date: new Date().toISOString() };
    } catch (e) {
        const reason = e instanceof Error ? e.message : String(e);
        if (cache) {
            return { kind: 'cache', rows: cache.rows, date: new Date(cache.fetchedAt).toISOString(), error: reason };
        }
        return fromSnapshot(reason);
    }
}
