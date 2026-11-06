# Recording your own references (Route A)

You need at least 12 single hits, from at least 3 families (drum/barrel body, snare-like, cymbal-like or sheet steel, bell-like), each of a
real struck steel oil drum, barrel or similar sheet steel.

1. Record each hit alone, with 1 s or more of ring after it, at least 1 s of audio after the onset, no clipping. WAV, 24-bit if possible, 44.1 or 48 kHz.
2. Use the same stick type (wood is the default in the engine) and note which instrument in the kit it stands for.
3. Name files `<stem>.wav`; the stem must be one of the engine stems (the plan's `make_events.py` lists them) so each reference has a matched candidate.
4. Write `manifest.json` with, per file: stem, sha256, licence (`owner-attested-CC0-1.0` or `owner-attested-CC-BY-4.0`, with `redistributable` true), attribution, and `owner_attested: true` plus an `ATTESTATION.txt`
   saying you recorded them and grant that licence. Pitch: your best estimate in Hz, or leave blank and the tool estimates it.
5. Do not include anyone else's recordings or music. Do not include secrets.
The pairs are published on a public orphan branch only when every reference is redistributable (D-020); otherwise only links and hashes are published and the pairs are handed to the owner privately.
