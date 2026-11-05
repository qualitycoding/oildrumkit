#!/usr/bin/env python3
"""Validate reference recordings against references/MANIFEST.json and canonicalise them.
  ingest_refs.py --manifest MANIFEST.json --src DIR --out REF_DIR
Rules: schema_version 1; at least 12 references; unique stems; sha256 of each source file matches; licence in the allowed set;
an owner-attested licence needs a non-empty ATTESTATION file named in the row, next to the manifest; at least 3 distinct families;
no clipped file (peak >= 0.999); at least 1.0 s after the onset. Output: REF_DIR/<stem>.wav (mono, 44.1 kHz, PCM-24) and f0.json."""
import argparse, hashlib, json, pathlib, sys
import numpy as np, soundfile as sf
from scipy.signal import resample_poly

OK_LICENCES = {"CC0-1.0", "CC-BY-4.0", "CC-BY-3.0", "owner-attested-CC0-1.0", "owner-attested-CC-BY-4.0"}
FAMILIES = {"bass", "snare", "tom", "hat", "cymbal", "bell", "other"}

def f0_estimate(x, fs):
    i0 = int(np.flatnonzero(np.abs(x) > 0.01 * np.abs(x).max())[0])
    seg = x[i0:i0 + fs // 2]
    spec = np.abs(np.fft.rfft(seg * np.hanning(len(seg))))
    freqs = np.fft.rfftfreq(len(seg), 1 / fs)
    band = (freqs >= 30) & (freqs <= 2000)
    return float(freqs[band][np.argmax(spec[band])])

def main(argv=None):
    ap = argparse.ArgumentParser()
    ap.add_argument("--manifest", required=True); ap.add_argument("--src", required=True); ap.add_argument("--out", required=True)
    a = ap.parse_args(argv)
    mp = pathlib.Path(a.manifest); m = json.loads(mp.read_text()); src = pathlib.Path(a.src)
    errs, rows = [], m.get("references", [])
    if m.get("schema_version") != 1: errs.append("schema_version must be 1")
    if len(rows) < 12: errs.append(f"{len(rows)} references, at least 12 required")
    if len({r.get("stem") for r in rows}) != len(rows): errs.append("duplicate stems")
    fams = {r.get("family") for r in rows}
    if len(fams & FAMILIES) < 3 or not fams <= FAMILIES: errs.append(f"families {sorted(map(str, fams))}: need at least 3 distinct from {sorted(FAMILIES)}")
    out = pathlib.Path(a.out); f0 = {}
    for r in rows:
        f = src / r.get("file", "?")
        if not f.is_file(): errs.append(f"{r.get('stem')}: file {f.name} missing"); continue
        if hashlib.sha256(f.read_bytes()).hexdigest() != r.get("sha256"): errs.append(f"{r['stem']}: sha256 mismatch")
        if r.get("licence") not in OK_LICENCES: errs.append(f"{r['stem']}: licence {r.get('licence')!r} not allowed")
        if str(r.get("licence", "")).startswith("owner-attested"):
            att = mp.parent / (r.get("attestation") or "")
            if not (r.get("attestation") and att.is_file() and att.read_text().strip()): errs.append(f"{r['stem']}: attestation file missing or empty")
        elif not r.get("source"): errs.append(f"{r['stem']}: source URL missing")
        x, fs = sf.read(f); x = x.mean(axis=1) if x.ndim > 1 else x
        if np.max(np.abs(x)) >= 0.999: errs.append(f"{r['stem']}: clipped"); continue
        if fs != 44100: x = resample_poly(x, 44100, fs)
        i0 = int(np.flatnonzero(np.abs(x) > 0.01 * np.abs(x).max())[0])
        if len(x) - i0 < 44100: errs.append(f"{r['stem']}: shorter than 1.0 s after the onset"); continue
        if not errs:
            out.mkdir(parents=True, exist_ok=True)
            sf.write(out / f"{r['stem']}.wav", x, 44100, subtype="PCM_24")
            f0[r["stem"]] = round(f0_estimate(x, 44100), 1)
    for e in errs: print("FAIL:", e)
    if errs:
        return 1
    (out / "f0.json").write_text(json.dumps(f0, indent=1) + "\n")
    print(f"ingested {len(rows)} references; f0 estimates in {out / 'f0.json'}")
    return 0

if __name__ == "__main__":
    sys.exit(main())
