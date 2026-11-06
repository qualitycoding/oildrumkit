# Route B: downloading references in a browser

You chose Route B (G-REF, 2026-11-05). The planner's shell cannot download from the sound sites, so the files have to come through you.

## What to collect
At least 12 single hits of struck steel (oil drum, barrel, sheet steel, steel bin or tank). At least 3 families from: `bass` (deep drum, kick-like), `snare` (sharp, buzzy), `tom` (pitched drum body), `cymbal` (sheet or lid ringing), `hat` (short clink), `bell` (bell or cowbell-like), `other`. Each file needs 1 s or more of audio after the hit, no clipping, and a hit that rings without music or speech over it.

## Licence rule (D-020)
The reference files are not republished; the repository links to them (`REFERENCE_LINKS.md`: page URL, author, licence, sha256). So the licence only has to let you download and listen. Accepted, read from the sound's own page: CC0, CC BY 3.0 or 4.0, CC BY-SA 4.0, CC BY-NC 4.0, CC BY-NC-SA 4.0, CC BY-ND 4.0, CC BY-NC-ND 4.0, or a site's own terms that allow personal use (give the terms URL as `terms_url`; licence `site-terms-personal-use`). Refused: licence unknown, "all rights reserved", no named author or no original source page, and files where the site bars even personal use.
Set `"redistributable": true` only for CC0, CC BY and CC BY-SA files (the tool refuses it for the others). If every file is redistributable, the listening pairs may also be published; if any is not, the pairs stay private and reach you as files in this chat.

## Where to look
- Lots of Sounds, "Metal Oil Tank Hit": https://www.lotsofsounds.com/sounds/metal-oil-tank-hit-fs-108264 . Closest match, labelled CC0, but no creator is named and it is one 54-second file of several bangs. Use it only if you can trace the original author; if you cut single hits from it, mark each cut in the manifest as from the same source.
- BigSoundBank (CC0 per sound page), search "oil drum", "barrel", "bidon" and "metal drum": https://bigsoundbank.com/search?q=oil+drum . Its "Anvil #1" is real ringing steel and CC0, but an anvil: use it as an `other` or `bell` row only if you accept that it is not a drum.
- Freesound (needs a login to download), filter licence to "Creative Commons 0" or "Attribution". Attribution NonCommercial is acceptable for private listening under D-020 (mark it personal-use-only in the manifest); prefer CC0 or Attribution.
- Zenodo records 3994999 "Drum and Percussion Kits" and 12685116 (CC-BY-4.0 per the API; not opened by the planner).
- Free to Use Sounds (freetousesounds.com): its home page allows personal and commercial use but bars resale and redistribution, which D-020 permits for private listening. I could not read its licence agreement page, so check it yourself and give its URL as `terms_url`. No oil-drum page was found on its home page.
- Freesound 628491 (CC BY-NC 4.0) is now allowed on licence, but it is a processed multisample of a metal bar, not a raw recording: skip it on content grounds.
If after a fair search you have fewer than 12 usable files, say so; the choices are then Route A for the rest (recording your own hits), or Route C.

## What to hand back
1. The audio files, unchanged as downloaded (WAV or FLAC preferred; not MP3 if a WAV exists).
2. `MANIFEST.json` made from `assets/listening/MANIFEST.template.json`, one row per file: a short `stem` (s01, s02 ...), the `file` name, the `sha256` (command `sha256sum file`), `instrument` (the kit index the hit should be compared with: 0 Bass, 1 Snare1, 2 Snare2, 3 to 8 the six toms, 9 Splash, 10 Ride, 11 HatClosed, 12 HatOpen, 13 Cowbell, 14 Rimshot), `family`, `licence` (one of the names above, written exactly), `redistributable` (true or false), the page URL in `source`, the author's name in `author`, and `terms_url` for site-terms files.
3. Upload them in this chat, or tell me where they are. The implementing run will check them with `ingest_refs.py`, which refuses missing hashes, wrong licences, clipping and short files.
