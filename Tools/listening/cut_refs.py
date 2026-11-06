#!/usr/bin/env python3
"""Cut single-hit reference files out of longer source recordings, reproducibly (planning protocol Rule 15, derived references).
  cut_refs.py --manifest references/MANIFEST.json --parents DIR --out SRC_DIR
For each manifest row with `parent_file`, read DIR/<parent_file>, check its sha256 against `parent_sha256`, write samples
[cut_start_s, cut_end_s) at the source sample rate and bit depth (PCM_24) to SRC_DIR/<file>, and check the result against the row's `sha256`.
Exit 0 only if every cut matched; exit 1 otherwise."""
import argparse, hashlib, json, pathlib, sys
import soundfile as sf


def main(argv=None):
    ap = argparse.ArgumentParser()
    ap.add_argument("--manifest", required=True); ap.add_argument("--parents", required=True); ap.add_argument("--out", required=True)
    a = ap.parse_args(argv)
    rows = [r for r in json.loads(pathlib.Path(a.manifest).read_text())["references"] if r.get("parent_file")]
    out = pathlib.Path(a.out); out.mkdir(parents=True, exist_ok=True); bad = 0
    for r in rows:
        p = pathlib.Path(a.parents) / r["parent_file"]
        if not p.is_file() or hashlib.sha256(p.read_bytes()).hexdigest() != r["parent_sha256"]:
            print(f"FAIL {r['stem']}: parent {r['parent_file']} missing or hash differs"); bad += 1; continue
        x, fs = sf.read(p, always_2d=True)
        seg = x[int(round(r["cut_start_s"] * fs)):int(round(r["cut_end_s"] * fs))]
        sf.write(out / r["file"], seg, fs, subtype="PCM_24")
        if hashlib.sha256((out / r["file"]).read_bytes()).hexdigest() != r["sha256"]:
            print(f"FAIL {r['stem']}: cut hash differs from the manifest"); bad += 1
        else:
            print(f"ok {r['stem']} {r['file']}")
    print(f"{len(rows) - bad}/{len(rows)} cuts verified")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
