"""T-L17 (Rule 15, step d): the on-demand reference downloader accepts matching files and refuses anything else."""
import hashlib, json, pathlib, subprocess, sys, threading, functools, http.server

TOOL = pathlib.Path(__file__).resolve().parent.parent / "assets/listening/references/fetch_refs.py"


def serve(directory):
    handler = functools.partial(http.server.SimpleHTTPRequestHandler, directory=str(directory))
    handler.log_message = lambda *a, **k: None
    srv = http.server.ThreadingHTTPServer(("127.0.0.1", 0), handler)
    threading.Thread(target=srv.serve_forever, daemon=True).start()
    return srv


def run(manifest, out, base):
    return subprocess.run([sys.executable, "-I", str(TOOL), str(manifest), str(out), "--base-url", base], capture_output=True, text=True)


def test_T_L17_downloads_verify_and_every_failure_is_refused(tmp_path):
    web = tmp_path / "web"; web.mkdir()
    blobs = {f"f{i}.wav": bytes([i]) * 50 + b"data" for i in range(3)}
    for n, b in blobs.items(): (web / n).write_bytes(b)
    man = {"references": [{"file": n, "sha256": hashlib.sha256(b).hexdigest()} for n, b in blobs.items()]}
    mp = tmp_path / "m.json"; mp.write_text(json.dumps(man))
    srv = serve(web); base = f"http://127.0.0.1:{srv.server_address[1]}"
    try:
        p = run(mp, tmp_path / "ok", base)
        assert p.returncode == 0 and "3/3 verified" in p.stdout, p.stdout + p.stderr
        assert all((tmp_path / "ok" / n).read_bytes() == b for n, b in blobs.items())
        # a changed source file must be refused
        (web / "f1.wav").write_bytes(b"changed upstream")
        q = run(mp, tmp_path / "bad", base)
        assert q.returncode == 1 and "MISMATCH f1.wav" in q.stdout, q.stdout
        # a file that is already on disk is re-checked, not trusted
        (tmp_path / "ok" / "f0.wav").write_bytes(b"tampered locally")
        r = run(mp, tmp_path / "ok", base)
        assert r.returncode == 1 and "MISMATCH f0.wav" in r.stdout, r.stdout
        # a missing upstream file is a failure, not a skip
        (web / "f2.wav").unlink()
        s = run(mp, tmp_path / "gone", base)
        assert s.returncode == 1 and "FAIL f2.wav" in s.stdout, s.stdout
    finally:
        srv.shutdown()
    # a manifest that cannot be read exits 2
    assert subprocess.run([sys.executable, "-I", str(TOOL), str(tmp_path / "nope.json"), str(tmp_path / "x")], capture_output=True, text=True).returncode == 2
