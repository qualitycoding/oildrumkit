# S6 - VCSL single hits and the klakmart oil-tank lead (2026-11-06)

## VCSL (Versilian Community Sample Library)
- Repository: https://github.com/sgossner/VCSL , commit c1ea7bcc3c7309650ab0da9d15c9cd1fbc4a4c7e . Licence: CC0 1.0 Universal (LICENSE file and README read).
- 14 candidate single hits were fetched on demand from raw.githubusercontent.com and probed (PCM_16, 44.1 kHz, stereo, unclipped, at least 1 s after onset). 13 are kept in `assets/listening/references/vcsl-candidates.manifest.json`; `Gong1_mf` (34 s) is left out as too long to cut cleanly.
- Families covered: hat (3), cymbal (4), bell (2), other (4: two anvil hits, two brake-drum hits). The drum body (bass, snares, toms) is not covered: the VCSL membranophones are skin drums.
- Caveat: the files are 16-bit, so the independent decode check in the listening skill still applies.
- No audio is stored in this repository. `assets/listening/references/fetch_refs.py` downloads the files on demand and refuses any sha256 mismatch (test T-L17). A live run against the pinned commit verified 13/13.
- Instrument, velocity and hammer values in the manifest are the planner's starting guesses and are marked DRAFT.

## Drum body lead: klakmart "BangingOilTank.wav"
- Reported by the owner on 2026-11-06 after comparing the Lots of Sounds page (https://www.lotsofsounds.com/sounds/metal-oil-tank-hit-fs-108264) with Freesound sound 108264 (https://freesound.org/s/108264/): same audio.
- Uploader: klakmart. Licence: CC0 (as reported by the owner from the Freesound page). Uploaded 2010-11-08. Length about 54 s (per the Lots of Sounds copy).
- Provenance is now settled by the original uploader's page, which the owner read; the planner could not open it (not on the fetch allowlist) and the shell cannot download from Freesound. Treat the licence as owner-attested until the file is in hand.
- Status: lead only. No hash exists yet. The owner downloads the original WAV, and the implementer records its sha256 in the manifest as `owner-attested-CC0-1.0` (or `CC0-1.0` if the licence is confirmed by a page the tooling can read), then cuts single hits, marking each cut as from the same source.
- The earlier objection (no creator named) no longer applies.

## klakmart file received (2026-11-06 11:39)
- The owner supplied the original WAV. sha256 `14a9c4bbc17445cba49fc41d91eda49f57cfbe6b26f09d8643d1b03fb12edaaf`; PCM_24, 48 kHz, mono, 53.59 s, peak 0.38, no clipping.
- Energy-onset scan finds 23 onsets. Six are isolated by at least 1.3 s: 13.34, 18.3, 23.81, 27.05, 32.69 and 49.76 s. These are draft cut points only; the implementer chooses and listens.
- Recorded in `assets/listening/references/klakmart-oiltank.manifest.json` (licence `owner-attested-CC0-1.0`). The audio is not committed (D-020).
- Open: this single source must supply bass, snares and toms, so those voices share one resonance. The pairs for them will test pitch-shifted cuts, not separate drums; say so in the round notes.

## Knowledge item K-20261106T120000Z (oil drum hit candidates), read 2026-11-06 12:05
- Adds betchkal (CC BY 4.0, 100 lb and 20 lb propane tanks, 192 kHz 24-bit) and neirbonad1 (CC0 by snippet, gas tank). The item itself dropped its BangingOilTank note, which is covered here. Its listening notes are the owner's.
- Effect on the limitation above: betchkal gives two real tank sizes, so bass/tom voices need not all come from one drum. Plan: betchkal 100 lb for bass and low toms, betchkal 20 lb and klakmart for snares and high toms, with the family assignment checked by ear in S-030.
- Not hashed: no file has been supplied. The owner must download them from Freesound (the shell cannot), after which sha256 and cuts are recorded as for klakmart. Licence CC-BY-4.0 is in the redistributable set but obliges credit.
- The paid A1 Studio "Industrial Steel Sample Pack" (real 55-gallon barrels) is noted by the item but its licence was not read; not used.
