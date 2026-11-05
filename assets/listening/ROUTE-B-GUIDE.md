# Route B: downloading references in a browser

You chose Route B (G-REF, 2026-11-05). The planner's shell cannot download from the sound sites, so the files have to come through you.

## What to collect
At least 12 single hits of struck steel (oil drum, barrel, sheet steel, steel bin or tank). At least 3 families from: `bass` (deep drum, kick-like), `snare` (sharp, buzzy), `tom` (pitched drum body), `cymbal` (sheet or lid ringing), `hat` (short clink), `bell` (bell or cowbell-like), `other`. Each file needs 1 s or more of audio after the hit, no clipping, and a hit that rings without music or speech over it.

## Licence rule (D-014)
The files are published on a public branch, so only these licences are accepted, read from the sound's own page: CC0 (Creative Commons Zero), Attribution 4.0 (CC BY 4.0) or Attribution 3.0 (CC BY 3.0). Refuse anything marked NonCommercial or NoDerivatives, anything that bars redistribution, and anything where the page names no author or original source.

## Where to look
- Lots of Sounds, "Metal Oil Tank Hit": https://www.lotsofsounds.com/sounds/metal-oil-tank-hit-fs-108264 . Closest match, labelled CC0, but no creator is named and it is one 54-second file of several bangs. Use it only if you can trace the original author; if you cut single hits from it, mark each cut in the manifest as from the same source.
- BigSoundBank (CC0 per sound page), search "oil drum", "barrel", "bidon" and "metal drum": https://bigsoundbank.com/search?q=oil+drum . Its "Anvil #1" is real ringing steel and CC0, but an anvil: use it as an `other` or `bell` row only if you accept that it is not a drum.
- Freesound (needs a login to download), filter licence to "Creative Commons 0" or "Attribution". Skip anything "Attribution NonCommercial".
- Zenodo records 3994999 "Drum and Percussion Kits" and 12685116 (CC-BY-4.0 per the API; not opened by the planner).
- Not usable: Free to Use Sounds (freetousesounds.com bars redistribution); Freesound 628491 (non-commercial).
If after a fair search you have fewer than 12 usable files, say so; the choices are then Route A for the rest (recording your own hits), or Route C.

## What to hand back
1. The audio files, unchanged as downloaded (WAV or FLAC preferred; not MP3 if a WAV exists).
2. `MANIFEST.json` made from `assets/listening/MANIFEST.template.json`, one row per file: a short `stem` (s01, s02 ...), the `file` name, the `sha256` (command `sha256sum file`), `instrument` (the kit index the hit should be compared with: 0 Bass, 1 Snare1, 2 Snare2, 3 to 8 the six toms, 9 Splash, 10 Ride, 11 HatClosed, 12 HatOpen, 13 Cowbell, 14 Rimshot), `family`, `licence` (exactly `CC0-1.0`, `CC-BY-4.0` or `CC-BY-3.0`), the page URL in `source`, and the author's name in `author`.
3. Upload them in this chat, or tell me where they are. The implementing run will check them with `ingest_refs.py`, which refuses missing hashes, wrong licences, clipping and short files.
