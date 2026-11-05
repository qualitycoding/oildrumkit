# Spike S5: what the shell can reach (2026-11-05)

First probe (about 21:05): bigsoundbank.com 000 (no connection); freesound.org and its apiv2 search 403; archive.org 000; raw.githubusercontent.com 200.
Second probe (about 21:22): freesound.org 403; bigsoundbank.com 000; zenodo.org 200; pypi.org 200; api.github.com 200; github.com archive and codeload URLs for oildrumkit 403 ("GitHub access to this repository is not enabled for this session"); cdn.freesound.org 000.
After `add_repo` for oildrumkit (about 22:15): github.com archive zip 200 (32 MB), codeload 200, api.github.com repository record 200 (public, Apache-2.0, push permission); bigsoundbank.com 200 for the home page, a sound page and search pages; freesound.org still 403; archive.org still 000.
Zenodo's records API answered 200 for eight dataset queries.
Consequence: reachability changes during a run. Plan steps that need a download probe first (S-000) and fall back to a human action. Anonymous access to the zip URL from a phone is untested (claim C-05).

After the owner opened firewall egress for freetousesounds.com (about 21:40): the shell still failed (no connection; the proxy log shows the gateway answering 403 to CONNECT), so the change had not reached this session. WebFetch read the home page but refused the licence-agreement page as not on its allowlist. Not retried.
