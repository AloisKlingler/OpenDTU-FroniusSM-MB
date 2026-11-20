// SPDX-License-Identifier: GPL-2.0-or-later
//
// Generates the bundled offline snapshot of the community grid profile list.
//
//   cd webapp
//   node ../tools/gen-gridprofile-snapshot.mjs [wiki-markdown-file]
//   yarn prettier --write src/data
//
// Without an argument the current wiki page is downloaded. Behind a corporate
// proxy with its own CA run node with NODE_OPTIONS=--use-system-ca.
//
// Only rows with a verifiable profile (valid CRC16, known model) end up in the
// snapshot. The profile bytes are stored once and referenced by index.

import { readFileSync, writeFileSync, mkdirSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';

const here = dirname(fileURLToPath(import.meta.url));
const utilPath = resolve(here, '../webapp/src/utils/gridProfileWiki.ts');
const outPath = resolve(here, '../webapp/src/data/gridprofile-wiki-snapshot.json');

const { parseWikiMarkdown, WIKI_RAW_URL } = await import(pathToFileURL(utilPath).href);

let markdown;
if (process.argv[2]) {
    markdown = readFileSync(process.argv[2], 'utf8');
} else {
    const res = await fetch(WIKI_RAW_URL);
    if (!res.ok) {
        throw new Error(`Download failed: HTTP ${res.status}`);
    }
    markdown = await res.text();
}

const { rows, rejected } = parseWikiMarkdown(markdown);

const blobs = [];
const blobIndex = new Map();
const compact = rows.map((r) => {
    let idx = blobIndex.get(r.hex);
    if (idx === undefined) {
        idx = blobs.length;
        blobs.push(r.hex);
        blobIndex.set(r.hex, idx);
    }
    return [r.country, r.model, r.name, r.fw, idx];
});

const snapshot = {
    source: WIKI_RAW_URL,
    generatedAt: new Date().toISOString().slice(0, 10),
    blobs,
    rows: compact,
};

mkdirSync(dirname(outPath), { recursive: true });
writeFileSync(outPath, JSON.stringify(snapshot, null, 4) + '\n');
console.log(`${rows.length} rows, ${blobs.length} distinct profiles, ${rejected} rows rejected -> ${outPath}`);
