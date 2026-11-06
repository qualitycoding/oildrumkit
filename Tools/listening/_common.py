"""Shared helpers for the listening tools. Decoding for VERIFICATION uses the standard-library `wave` module only."""
import base64, json, pathlib, sys, wave
import numpy as np

REPO = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO / "scripts"))
import make_blind_pairs as mbp  # vendored builder; cut() and resample() are reused so level planning matches it exactly

SEED_BASE = 20261103
CAP_DB = -23
CORR_MIN = 0.99
SECRET = __import__("re").compile(r"github_pat_[A-Za-z0-9_]{20,}|ghp_[A-Za-z0-9]{20,}|AKIA[0-9A-Z]{16}|-----BEGIN [A-Z ]*PRIVATE KEY-----")

def seed_for(round_no):
    # The gate allows three rounds. The owner may extend it (decision D-022): OILDRUM_OWNER_EXTRA_ROUNDS=N allows N more.
    extra = int(__import__("os").environ.get("OILDRUM_OWNER_EXTRA_ROUNDS", "0") or 0)
    if not 1 <= round_no <= 3 + max(extra, 0):
        raise SystemExit(f"round {round_no} refused: the gate allows at most {3 + max(extra, 0)} rounds")
    return SEED_BASE + 1000 * (round_no - 1)

def read_pcm(path):
    with wave.open(str(path), "rb") as w:
        n, ch, sw, fs = w.getnframes(), w.getnchannels(), w.getsampwidth(), w.getframerate()
        raw = w.readframes(n)
    if sw == 2:
        x = np.frombuffer(raw, "<i2").astype(float) / 32768.0
    elif sw == 3:
        b = np.frombuffer(raw, np.uint8).reshape(-1, 3)
        v = b[:, 0].astype(np.int32) | (b[:, 1].astype(np.int32) << 8) | (b[:, 2].astype(np.int32) << 16)
        x = np.where(v >= 1 << 23, v - (1 << 24), v) / 8388608.0
    else:
        raise SystemExit(f"unsupported sample width {sw} in {path}")
    return x.reshape(-1, ch).mean(axis=1), fs, sw, ch

def crest_db(path, seconds, fs_out):
    import soundfile as sf
    x, fs = sf.read(path)
    seg = mbp.cut(mbp.resample(mbp.mono(x), fs, fs_out), fs_out, seconds, "start")
    seg = seg - seg.mean()
    return 20 * np.log10(np.max(np.abs(seg)) / max(np.sqrt(np.mean(seg ** 2)), 1e-12))

def archive_json(round_dir, name):
    return json.loads(base64.b64decode((pathlib.Path(round_dir) / "archive" / name).read_text()))
