# Human gates

| Gate | Opens after | Question | Allowed responses | Timeout |
|---|---|---|---|---|
| G-REF | S-020 | Supply references by Route A, B or C (D-005). | `route-A: <path or repo>`, `route-B: <path>`, `route-C`, `stop` | none; plan stays parked |
| G-LISTEN | S-040 (round n) | 1) Which side is the reference in each of 12 pairs, in order? 2) For each of the 9 demos: convincing metallic ring, attack, decay, velocity response, roll variation, cymbal shimmer versus hiss, snare rattle, hat choke, stereo/room character, cowbell partial structure? Give the instrument and a timestamp for every adverse finding. | `proceed`, `iterate: <notes>`, `proceed-with-rescope: <text>`, `stop` | none |
| G-003 | S-070 | Lessons and knowledge push (protocol 3.7.2): targets `qualitycoding/Lessons` and `qualitycoding/knowledge`; the source register is named on its own line. | `push-to-proposed`, `push-to: <lesson-target> [knowledge-target]`, `do-not-push` | none |

Pass rule (D-007) is applied by the implementer after the answer; the human response does not override a failed score except by
`proceed-with-rescope`, which is recorded as a shortfall, not a pass.
Gate files are written to `GATE-<id>.md`, answers to `GATE-<id>.RESPONSE.md`, and committed with the response.

Resume on `proceed`: S-RETRO, then S-KNOW and G-003 (execution plan §7). `iterate` stays at G-LISTEN and changes `Tools/voicing.json` or a general engine mechanism, with a new seed.

G-REF answered 2026-11-05T21:45Z: `route-B` (GATE-G-REF.RESPONSE.md). Files not yet received.
