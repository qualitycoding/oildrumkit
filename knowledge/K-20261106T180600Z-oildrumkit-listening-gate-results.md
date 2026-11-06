---
id: K-20261106T180600Z-oildrumkit-listening-gate-results
statement: "Blind listening gate G-LISTEN for oildrumkit, six rounds on 2026-11-06 against 13 real struck-steel and cymbal references: 11, 8, 9, 6, 8 and 12 of 12 identified; the gate failed."
status: current
supersedes: []
tags: [subject:oildrumkit-listening-phase, domain:audio, phase:execution]
applies_to_version: "oildrumkit impl/listening-validation at df96cee"
source: { url: "https://github.com/qualitycoding/oildrumkit/blob/impl/listening-validation/GATE-G-LISTEN.md", doi: "", title: "GATE-G-LISTEN.md", locator: "status and round sections", accessed: "2026-11-06", tier: 1 }
confidence: verified
derived_from: [C-01, C-02]
discovered_by: planner
run: gen-20261105T211145Z-oildrumkit-listening-phase
recorded_at: 2026-11-06T18:06:00Z
---
Round, correct of 12, one-sided p: 1: 11 (0.003); 2: 8 (0.19); 3: 9 (0.073); 4: 6 (0.61); 5: 8 (0.19); 6: 12 (0.0002). Rounds 2 to 6 pooled 43 of 60. Listener cues: timbre (1); decay, brightness, resonance (2); resonance (4); cymbal noise and drum resonance (5). Seeds 20261103 + 1000(n-1); levels -29, -30, -31, -33, -33, -33 dB RMS. Numeric targets, unchanged since round 4: partials at 50 ms 10/15 (fail), centroid rise (fail), snare-roll correlation max 0.433 to 0.485 (pass), HF/LF T60 max 0.219 with all 15 measurable (pass). The owner stopped the tests after round 6.
