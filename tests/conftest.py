# FROZEN - DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
import os, pathlib, pytest

@pytest.fixture(scope="session")
def repo():
    p = os.environ.get("OILDRUM_REPO", "")
    assert p, "set OILDRUM_REPO to the implementation checkout of qualitycoding/oildrumkit"
    p = pathlib.Path(p).resolve()
    assert (p / "Source" / "DrumEngine.h").is_file(), f"{p} is not an oildrumkit checkout"
    return p
