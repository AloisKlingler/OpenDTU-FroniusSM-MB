// SPDX-License-Identifier: GPL-2.0-or-later
//
// Parser and helpers for the community grid profile list in the OpenDTU wiki
// (https://github.com/tbnobody/OpenDTU/wiki/Grid-Profile-Parser).
//
// This module is intentionally free of any Vue / browser dependencies so it can
// be unit tested and used by the snapshot generator under Node.
//
// The wiki is community edited and therefore UNTRUSTED input. Nothing in here
// is ever rendered as HTML and every blob is verified (CRC16) before it is
// used. The device additionally re-validates every profile before writing it.

export const WIKI_RAW_URL = 'https://raw.githubusercontent.com/wiki/tbnobody/OpenDTU/Grid-Profile-Parser.md';

/** One usable row of the wiki table. The hex string never contains the CRC16. */
export interface WikiRow {
    country: string;
    /** Normalized model, e.g. "HMT-2250-6T". */
    model: string;
    /** Free text profile name from the wiki (may be empty). */
    name: string;
    /** Inverter firmware version from the wiki (may be empty). */
    fw: string;
    /** Profile bytes as lower case hex separated by spaces, without CRC16. */
    hex: string;
}

export interface ParseResult {
    rows: WikiRow[];
    /** Table rows that were skipped because they were unusable. */
    rejected: number;
}

export type MatchLevel = 'exact' | 'family' | 'other';

export interface WikiSource {
    country: string;
    model: string;
    name: string;
    fw: string;
    level: MatchLevel;
}

/** All wiki rows that carry the same profile bytes, ranked for one inverter. */
export interface ProfileCandidate {
    hex: string;
    /** Number of profile bytes (without CRC16). */
    length: number;
    /** First two bytes, identify the profile (country / standard). */
    header: string;
    /** CRC16 of the profile as 4 hex digits. Tells apart variants with the same name. */
    id: string;
    level: MatchLevel;
    /** Rows that confirm this profile for exactly this model. */
    exactCount: number;
    /** Rows that confirm this profile for a sister model of the same family. */
    familyCount: number;
    label: string;
    sources: WikiSource[];
}

export interface DiffItem {
    section: string;
    name: string;
    unit: string;
    from: number | undefined;
    to: number | undefined;
}

interface SectionLike {
    name: string;
    items: Array<{ n: string; u: string; v: number }>;
}

// ---------------------------------------------------------------- CRC16 ----

/** CRC16 Modbus (init 0xFFFF, poly 0xA001) - same as crc16() in the firmware. */
export function crc16Modbus(bytes: ArrayLike<number>): number {
    let crc = 0xffff;
    for (let i = 0; i < bytes.length; i++) {
        crc ^= (bytes[i] ?? 0) & 0xff;
        for (let bit = 0; bit < 8; bit++) {
            const lsb = crc & 1;
            crc >>= 1;
            if (lsb) {
                crc ^= 0xa001;
            }
        }
    }
    return crc & 0xffff;
}

/** True if the last two bytes are a valid big endian CRC16 of the bytes before. */
export function hasValidCrc(bytes: ArrayLike<number>): boolean {
    if (bytes.length < 7) {
        return false;
    }
    const body = Array.prototype.slice.call(bytes, 0, bytes.length - 2) as number[];
    const stored = (((bytes[bytes.length - 2] ?? 0) & 0xff) << 8) | ((bytes[bytes.length - 1] ?? 0) & 0xff);
    return crc16Modbus(body) === stored;
}

/** Removes the trailing CRC16 if (and only if) it is valid. */
export function stripCrc(bytes: number[]): number[] {
    return hasValidCrc(bytes) ? bytes.slice(0, bytes.length - 2) : bytes;
}

export function bytesToHex(bytes: ArrayLike<number>): string {
    const out: string[] = [];
    for (let i = 0; i < bytes.length; i++) {
        out.push((((bytes[i] ?? 0) & 0xff) + 0x100).toString(16).slice(1));
    }
    return out.join(' ');
}

export function hexToBytes(hex: string): number[] {
    return hex
        .toLowerCase()
        .split(/\s+/)
        .filter((x) => /^[0-9a-f]{2}$/.test(x))
        .map((x) => parseInt(x, 16));
}

// ------------------------------------------------------------ model names ----

/**
 * Brings the free text model column of the wiki to a canonical form
 * ("hms-800 2t" -> "HMS-800-2T", "HM-600." -> "HM-600").
 * Returns an empty string if it does not look like a Hoymiles model.
 */
export function normalizeModel(raw: string): string {
    let s = (raw.split('/')[0] ?? '').trim().toUpperCase();
    s = s
        .replace(/\.+$/, '')
        .replace(/[\s_]+/g, '-')
        .replace(/-+/g, '-');
    return /^(HMS|HMT|HM)-\d{3,4}(-[A-Z0-9]+)*$/.test(s) ? s : '';
}

/** "HMT-2250-6T" -> "HMT-2250" (drops the MPPT / variant suffix). */
export function modelFamily(model: string): string {
    return model.replace(/(-(\d+T|T\d+|NA|V\d+))+$/, '');
}

// ----------------------------------------------------------------- parser ----

/**
 * Parses the markdown of the wiki page. Rows without a verifiable profile
 * (no / broken hex, CRC16 mismatch, unknown model) are skipped and counted.
 */
export function parseWikiMarkdown(markdown: string): ParseResult {
    const rows: WikiRow[] = [];
    let rejected = 0;

    for (const line of markdown.split('\n')) {
        if (!line.startsWith('|') || !line.includes('<pre>')) {
            continue;
        }
        const pre = /<pre>([^|]*)/.exec(line);
        if (!pre) {
            rejected++;
            continue;
        }
        const withCrc = hexToBytes((pre[1] ?? '').replace(/<[^>]*>/g, ' '));
        // Cells: | Country | Model | <pre>..</pre> | Name | FW | FW date | Comment | Contributor |
        const cells = line
            .replace(/<pre>[^|]*/, '§')
            .split('|')
            .map((c) => c.trim());
        const model = normalizeModel(cells[2] ?? '');

        if (!model || !hasValidCrc(withCrc)) {
            rejected++;
            continue;
        }
        rows.push({
            country: (cells[1] ?? '').toUpperCase().slice(0, 3),
            model,
            name: (cells[4] ?? '').replace(/[<>]/g, '').slice(0, 80),
            fw: (cells[5] ?? '').replace(/[<>]/g, '').slice(0, 20),
            hex: bytesToHex(withCrc.slice(0, withCrc.length - 2)),
        });
    }

    return { rows, rejected };
}

// ---------------------------------------------------------- classification ----

function levelFor(inverterModel: string, rowModel: string): MatchLevel {
    if (!inverterModel) {
        return 'other';
    }
    if (rowModel === inverterModel) {
        return 'exact';
    }
    if (modelFamily(rowModel) === modelFamily(inverterModel)) {
        return 'family';
    }
    return 'other';
}

function pickLabel(sources: WikiSource[], header: string): string {
    // The most frequent meaningful name wins; "unknown" style names are ignored.
    const counts = new Map<string, number>();
    for (const s of sources) {
        const n = s.name.trim();
        if (n && !/^(unknown|unbekannt|\?|n\/?a)/i.test(n)) {
            counts.set(n, (counts.get(n) ?? 0) + 1);
        }
    }
    let best = '';
    let bestCount = 0;
    for (const [n, c] of counts) {
        if (c > bestCount) {
            best = n;
            bestCount = c;
        }
    }
    return best || `Profile ${header}`;
}

/**
 * Groups the wiki rows by profile bytes and ranks them for one inverter.
 * Only profiles that appear for this model (exact) or a sister model of the
 * same family are returned; profiles of unrelated models are never offered.
 */
export function buildCandidates(rows: WikiRow[], inverterType: string): ProfileCandidate[] {
    const model = normalizeModel(inverterType);
    if (!model) {
        return [];
    }

    const byHex = new Map<string, WikiSource[]>();
    for (const r of rows) {
        const level = levelFor(model, r.model);
        const list = byHex.get(r.hex) ?? [];
        list.push({ country: r.country, model: r.model, name: r.name, fw: r.fw, level });
        byHex.set(r.hex, list);
    }

    const out: ProfileCandidate[] = [];
    for (const [hex, sources] of byHex) {
        const exactCount = sources.filter((s) => s.level === 'exact').length;
        const familyCount = sources.filter((s) => s.level === 'family').length;
        if (exactCount === 0 && familyCount === 0) {
            continue;
        }
        const bytes = hexToBytes(hex);
        const header = bytesToHex(bytes.slice(0, 2));
        out.push({
            hex,
            length: bytes.length,
            header,
            id: crc16Modbus(bytes).toString(16).padStart(4, '0'),
            level: exactCount > 0 ? 'exact' : 'family',
            exactCount,
            familyCount,
            label: pickLabel(
                sources.filter((s) => s.level !== 'other'),
                header
            ),
            sources: sources.filter((s) => s.level !== 'other'),
        });
    }

    const rank = (c: ProfileCandidate) => (c.level === 'exact' ? 0 : 1);
    out.sort(
        (a, b) =>
            rank(a) - rank(b) ||
            b.exactCount - a.exactCount ||
            b.familyCount - a.familyCount ||
            a.label.localeCompare(b.label)
    );
    return out;
}

// -------------------------------------------------------------------- diff ----

/** Values that differ between the active profile and a candidate profile. */
export function diffSections(current: SectionLike[], candidate: SectionLike[]): DiffItem[] {
    type Flat = { section: string; name: string; unit: string; v: number };
    const flatten = (sections: SectionLike[]): Map<string, Flat> => {
        const map = new Map<string, Flat>();
        for (const s of sections) {
            const seen = new Map<string, number>();
            for (const i of s.items) {
                const nth = seen.get(i.n) ?? 0;
                seen.set(i.n, nth + 1);
                map.set(`${s.name}\u0000${i.n}\u0000${nth}`, { section: s.name, name: i.n, unit: i.u, v: i.v });
            }
        }
        return map;
    };

    const a = flatten(current);
    const b = flatten(candidate);
    const diff: DiffItem[] = [];

    for (const [key, to] of b) {
        const from = a.get(key);
        if (!from || from.v !== to.v) {
            diff.push({ section: to.section, name: to.name, unit: to.unit, from: from?.v, to: to.v });
        }
    }
    for (const [key, from] of a) {
        if (!b.has(key)) {
            diff.push({ section: from.section, name: from.name, unit: from.unit, from: from.v, to: undefined });
        }
    }
    return diff;
}

// ------------------------------------------------------------------ export ----

/**
 * Formats the active profile as a ready-to-paste row of the wiki table. The
 * CRC16 is appended because the wiki lists the raw data incl. CRC16.
 */
export function buildWikiRow(opts: {
    country: string;
    model: string;
    profile: number[];
    name: string;
    fw: string;
    comment?: string;
}): string {
    const bytes = stripCrc(opts.profile.slice());
    const crc = crc16Modbus(bytes);
    const full = bytes.concat([(crc >> 8) & 0xff, crc & 0xff]);
    const groups: string[] = [];
    for (let i = 0; i < full.length; i += 16) {
        groups.push(bytesToHex(full.slice(i, i + 16)));
    }
    const hex = `<pre>${groups.join(' <br/>')}</pre>`;
    return `| ${opts.country} | ${opts.model} | ${hex} | ${opts.name} | ${opts.fw} |  | ${opts.comment ?? ''} |  |`;
}
