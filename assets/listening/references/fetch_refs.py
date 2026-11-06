#!/usr/bin/env python3
"""Download reference files on demand and check them against a manifest (planning-protocol Rule 15, step d).

  python3 fetch_refs.py MANIFEST.json OUT_DIR [--base-url URL]

For each reference with a `raw_url`, fetch it (or BASE_URL/<file> when --base-url is given, used by the tests) into OUT_DIR/<file> and compare its
sha256 with the manifest. Exit 0 only if every file arrived and matched; exit 1 on any mismatch or download failure (nothing is accepted silently);
exit 2 on a bad manifest. OUT_DIR should be outside the repository working tree. Existing files are re-checked, not trusted.
"""
import hashlib, json, pathlib, sys, urllib.request


def main(argv):
    args = [a for a in argv if not a.startswith("--")]
    base = None
    if "--base-url" in argv:
        base = argv[argv.index("--base-url") + 1].rstrip("/")
        args.remove(base) if base in args else None
    if len(args) != 2:
        print(__doc__); return 2
    try:
        refs = json.loads(pathlib.Path(args[0]).read_text())["references"]
    except Exception as e:
        print(f"bad manifest: {e}"); return 2
    out = pathlib.Path(args[1]); out.mkdir(parents=True, exist_ok=True)
    bad = 0
    for r in refs:
        dst = out / r["file"]
        url = f"{base}/{r['file']}" if base else r.get("raw_url")
        try:
            if not dst.is_file():
                if not url: raise RuntimeError("no raw_url")
                with urllib.request.urlopen(url, timeout=60) as resp: dst.write_bytes(resp.read())
            got = hashlib.sha256(dst.read_bytes()).hexdigest()
        except Exception as e:
            print(f"FAIL {r['file']}: {e}"); bad += 1; continue
        if got != r["sha256"]:
            print(f"MISMATCH {r['file']}: manifest {r['sha256'][:12]}..., got {got[:12]}..."); bad += 1
        else:
            print(f"ok {r['file']}")
    print(f"{len(refs) - bad}/{len(refs)} verified")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
