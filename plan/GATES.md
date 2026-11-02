
## G-003 — split learning stores (v3.9)

Lessons and knowledge are pushed to **separate** stores: the `Lessons`
repository and the `knowledge` repository. G-003 requests the destinations at
plan close; **do-not-push** either store until human sign-off; the documented
bundle fallback applies when sign-off is withheld. Store publication never
holds up the repository amendment's completion record.

## G-LISTEN — blind realism listening (D-981)

- **Trigger step:** S-LISTEN (after the applicable reviews, before S-RETRO).
- **Comparison:** realism — current modal-engine candidate renders vs at least
  12 matched, redistribution-compatible real struck-steel recordings; legacy
  renders are demos, never references.
- **Frozen settings:** 12 onset pairs, `--window start`, 1.0 s clips, common
  RMS −23 dBFS (pre-match levels retained), 10 ms fades, 44.1 kHz PCM-24 WAV;
  pass = ≤ 8/12 correct AND no unresolved qualitative finding; maximum three
  rounds; base seed 20261103, rounds 2/3 add 1000/2000; p-value reported
  exactly but does not replace the ≤8/12 rule; general mechanisms only, never
  per-clip constants.
- **Questions:** which side is real; metallic ring, attack, decay, velocity
  response, roll variation, cymbal shimmer vs hiss, snare rattle, hat choke,
  stereo/room character, cowbell partial structure; instrument and timestamp
  for every adverse finding.
- **Allowed responses:** `proceed` (→ S-RETRO, then S-KNOW/G-003) |
  `iterate: <notes>` (general change in Tools/voicing.json or engine
  mechanism; rerun metrics/reviews; new seed) | `proceed-with-rescope: <text>`
  (narrow claim; update README/research limitations) | `stop`.
- **Evidence bundle:** `listening/G-LISTEN/round-00N/` per the common schema
  (result.json, answers.txt, score.txt, findings.md, metrics.md,
  provenance.md, render-manifest.json, SHA256SUMS), `GATE-G-LISTEN.md`, and
  the human's `GATE-G-LISTEN.RESPONSE.md`.
- **Status:** round-001 blocked — references not secured (see
  listening/G-LISTEN/round-001/result.json).
