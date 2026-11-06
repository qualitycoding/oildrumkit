# Reference leads (Route B)

Status 2026-11-05: five pages were opened and read (spike S4); no audio file has been downloaded, listened to or hashed. Treat everything below as a lead to confirm.

Closest match: Lots of Sounds, "Metal Oil Tank Hit", https://www.lotsofsounds.com/sounds/metal-oil-tank-hit-fs-108264 . Described as a large metal oil drum banged with echoing resonance, 0:54 MP3, licence line "CC0: free for commercial use, no attribution required". The creator and original source are not named and no Freesound link is given, so the CC0 grant cannot be traced to an author. Confirm the origin before using it. It is one hit file (a recording of several bangs), so cutting it into 12 matched single hits is unlikely to work: more files are needed.

Other leads, not matched to the target:
- BigSoundBank "Anvil #1" (CC0, 48 kHz 24-bit): ringing steel, but an anvil. https://bigsoundbank.com/anvil-1-s3589.html . bigsoundbank.com answered from the shell later on 2026-11-05; its "oil drum" search page loads but its list was not parsed: https://bigsoundbank.com/search?q=oil+drum .
- Zenodo records 3994999 "Drum and Percussion Kits" and 12685116 "Sound database of Industrial Machine for Audio Anomaly Detection" (both listed CC-BY-4.0): titles only, not opened.
- Freesound 795432 "Metal Barrel Hit": could not be read; licence unknown.

Refused: Freesound 628491 (CC BY-NC 4.0). Not a steel reference: Freesound 410514 (a snare, CC BY 4.0).

For each file you choose, record the URL, the licence text as shown on the page, the author, and the sha256 of the downloaded file in `manifest.json`. Freesound downloads need a login. Do not add leads here as verified unless a page was read and a file was downloaded and hashed. See the register for the licence read on each page.

Free to Use Sounds (freetousesounds.com, about 15,000 recordings by its founders): the owner opened egress to it on 2026-11-05. Its home page says sounds are free for personal and commercial use but may not be resold or redistributed, which conflicts with a public evidence bundle (D-014), and no oil-drum page was found. The full licence agreement could not be read. Use only if the agreement allows redistribution inside a public repository, or ask the site for permission.

Update 2026-11-05 (D-020): the repository links to reference files instead of republishing them, so non-commercial, no-derivatives and personal-use-only licences are now acceptable (private listening only). The Free to Use Sounds licence agreement still needs reading. Provenance rules are unchanged: a named author and an original source page are required.

## Shortlist from knowledge item K-20261106T112500Z-struck-metal-real-recording-catalogue (read 2026-11-06)

The catalogue was compiled by another research run; its confidence is `single-source` and no audio was downloaded or listened to. This planner re-read only one licence: VCSL is CC0 (its README, read via raw.githubusercontent.com). Every other licence below is as the catalogue states and must be read from the sound's own page when you download it (the manifest requires it). The planner's fetch tool no longer opens these pages (not on its allowlist), and the shell cannot reach freesound.org or theremin.music.uiowa.edu, so the downloads are yours (Route B).

Fit to the kit, best first. Families come from the 12-hit need; "drum body" is the weak spot, because the catalogue found almost no real oil-drum recordings.

| Family (kit instrument index) | Candidate | Licence as catalogued | Notes and checks |
|---|---|---|---|
| Drum body (bass 0, snare 1-2, toms 3-8) | engreitz "Oil drum.wav" https://freesound.org/people/engreitz/sounds/79605/ | unverified: read it on the page | Steel oil drum hit with a thin rod at different places, 4.3 s, 16-bit 44.1 kHz. Cut 3 or 4 single hits at different positions; usable only if the licence is acceptable. |
| Drum body | DeVern "Oil Drum Soft Impact" https://freesound.org/people/DeVern/sounds/524817/ | CC0 | Filed under synths; may be processed. Listen: refuse if it is not a raw recording. |
| Drum body | klakmart "BangingOilTank.wav" (Freesound 108264, uploaded 2010-11-08), re-hosted by Lots of Sounds https://www.lotsofsounds.com/sounds/metal-oil-tank-hit-fs-108264 | CC0 (owner read the Freesound page, 2026-11-06) | Owner confirmed it is the same audio. About 54 s of several bangs. Best drum-body lead: author and licence settled. Needs the owner to download the original WAV; then record sha256 and cut single hits (see S6). |
| Cymbal (splash 9, ride 10) | jfmcrae thunder sheet https://freesound.org/people/jfmcrae/sounds/512925/ ; loganzsound plate hit 6 https://freesound.org/people/loganzsound/sounds/784157/ ; Sheyvan metal plate https://freesound.org/people/Sheyvan/sounds/476121/ | CC0 each | Steel or iron plates and sheet; 24-bit. The thunder sheet is a 26.8 s ring: cut a window after the onset. |
| Cymbal | University of Iowa MIS gongs and tam-tams https://theremin.music.uiowa.edu/MISgongtamtams.html | "without restrictions" (MIS main page) | Anechoic, 24-bit, three dynamics. Not a clear redistribution grant, so mark `redistributable: false` unless you confirm otherwise. Not steel: bronze-type. |
| Hat (closed 11, open 12) | Anthousai metal pipe hit https://freesound.org/people/Anthousai/sounds/406555/ ; Zott820 conduit https://freesound.org/people/Zott820/sounds/271170/ | CC0 each | Short clinks. The conduit file is a 14 s multi-hit recorded with a USB microphone: cut it, expect noise. |
| Bell (cowbell 13), hat, cymbal, other | VCSL https://github.com/sgossner/VCSL (Idiophones/Struck Idiophones) | CC0 (LICENSE file read) | 13 hits probed and pinned in `assets/listening/references/vcsl-candidates.manifest.json`; see S6. Fetched with `fetch_refs.py`. 16-bit stereo. |
| Rimshot (14) and other | VCSL anvil (CC0); captainvince anvil https://freesound.org/people/captainvince/sounds/223553/ (CC BY 3.0); jorickhoofd metal on metal https://freesound.org/people/jorickhoofd/sounds/176674/ (CC BY 4.0) | as shown | The anvil file is a 22 s multi-hit: cut single hits. |

Not worth using: Pixabay and Wikimedia Commons (lossy MP3 or OGG), Sonniss, BBC RemArc and Greatest Hits (restrictive terms or not dry), RealImpact (no dataset licence), the Zenodo plate (not audio), and the two entries the catalogue excludes as synthesised.

Honest limit: almost all of this is plate, pipe, gong and anvil metal, not oil drums. A pair test built mainly from it checks the engine's cymbal, hat, bell and rimshot voices against real steel, and the drum voices against two or three drum recordings at best. Say so in the gate (D-015). If the engreitz and DeVern files fail on licence or provenance, the drum-body family has no reference, and the choices are to record it yourself (Route A) or to drop the drum voices from the scored set and say so.
