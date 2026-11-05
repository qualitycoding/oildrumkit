# Round 1 (the only research round): listening-phase planning, 2026-11-05

Reconstructed from the session record after a context reset; the protocol asks for this section to be written as the round proceeds, and this run did not do that. Disclosed in HANDOFF.md.

Questions: where the code stands (Q1), what the pair tools need for percussion (Q2), where real references can come from (Q3), what the sandbox can reach (Q4). Answers are in research/spikes/S1 to S5 and research/claims.json.

## Sources checked

| # | Source | URL | Accessed | Outcome and reason |
|---|---|---|---|---|
| 1 | Planning protocol v3.11, qualitycoding/skills | https://github.com/qualitycoding/skills/blob/108db63/protocols/planning-protocol.md | 2026-11-05 | used: The governing protocol this plan was written to: profiles and flags, claims register, spikes, frozen tests, ga |
| 2 | Planning protocol v3.12, qualitycoding/skills | https://github.com/qualitycoding/skills/blob/7daac75/protocols/planning-protocol.md | 2026-11-05 | used: Adds step R7 (source register), the Sources checked section in round logs, kind:source-register, and register |
| 3 | Planning protocol review record, qualitycoding/skills | https://github.com/qualitycoding/skills/blob/7daac75/protocols/planning-protocol-review.md | 2026-11-05 | used: Records the owner's request behind R7 and states that R7 had not yet been run in a planning run |
| 4 | Blind-listening execution plan, qualitycoding/skills | https://github.com/qualitycoding/skills/blob/108db63/protocols/blind-listening-test-execution-plan.md | 2026-11-05 | used: Result layout and schema (section 6) and the oildrumkit gate definition (section 7): demos A-D plus extras, se |
| 5 | Skill audio-blind-listening-validation (SKILL.md), qualitycoding/skills | https://github.com/qualitycoding/skills/blob/108db63/skills/audio-blind-listening-validation/SKILL.md | 2026-11-05 | used: Procedure for blind A/B pairs, demos, evidence delivery, the gate questions, scoring and iteration limits |
| 6 | Pair builder make_blind_pairs.py, qualitycoding/skills | https://github.com/qualitycoding/skills/blob/108db63/skills/audio-blind-listening-validation/scripts/make_blind_pairs.py | 2026-11-05 | used: Cuts, levels and fades reference and candidate clips and writes concealed keys; aborts if a clip would clip at |
| 7 | Scorer score_blind.py, qualitycoding/skills | https://github.com/qualitycoding/skills/blob/108db63/skills/audio-blind-listening-validation/scripts/score_blind.py | 2026-11-05 | used: Decodes the key and gives the one-sided exact binomial p-value |
| 8 | oildrumkit main, qualitycoding/oildrumkit | https://github.com/qualitycoding/oildrumkit/tree/1900ac8 | 2026-11-05 | used: The code under test: DrumEngine API, 15 instruments, render_demo, committed renders; unchanged in Source/ and |
| 9 | oildrumkit impl/listening-validation, qualitycoding/oildrumkit | https://github.com/qualitycoding/oildrumkit/tree/impl/listening-validation | 2026-11-05 | used: Existing listening branch: GATE-G-LISTEN.md and round-001 result.json showing the round blocked on missing ref |
| 10 | numpy 2.5.3, PyPI | https://pypi.org/project/numpy/2.5.3/ | 2026-11-05 | used: Array maths for the listening tools and tests; pinned in ENVIRONMENT.md |
| 11 | scipy 1.18.1, PyPI | https://pypi.org/project/scipy/1.18.1/ | 2026-11-05 | used: Resampling and filtering when sample rates differ; pinned |
| 12 | soundfile 0.14.0, PyPI | https://pypi.org/project/soundfile/0.14.0/ | 2026-11-05 | used: WAV reading and writing used by the vendored pair builder; pinned. Missing in the sandbox and installed from P |
| 13 | pytest 9.2.0, PyPI | https://pypi.org/project/pytest/9.2.0/ | 2026-11-05 | used: Test runner for the frozen tests; pinned |
| 14 | Search: freesound CC0 oil drum hit metal barrel strike drumstick sample | WebSearch query | 2026-11-05 | used: Returned freesound and bigsoundbank pages; led to the InspectorJ snare and Sample Workshop pages |
| 15 | Search: free sound library CC0 steel drum barrel percussion one-shots struck metal Creative Commons 0 | WebSearch query | 2026-11-05 | used: Returned bigsoundbank metal pages, commercial sample packs and a freesound page; led to the Anvil #1 page |
| 16 | Search: freesound oil drum OR metal barrel OR steel drum 55 gallon hit one shot CC0 OR Attribution | WebSearch query | 2026-11-05 | used: Returned Freesound 'Metal Barrel Hit', Lots of Sounds 'Metal Oil Tank Hit' and Pixabay search pages |
| 17 | Search: bigsoundbank barrel OR oil drum OR bidon metal hit sample CC0 | WebSearch query | 2026-11-05 | not relevant: Returned bigsoundbank cookware sounds and commercial packs; no barrel or drum sound on bigsoundbank was found |
| 18 | Freesound 410514 'Snare Drum, Single Hit, A (H1)', InspectorJ | https://freesound.org/people/InspectorJ/sounds/410514/ | 2026-11-05 | rejected: A single hit on a wooden-stick snare, 1.631 s, WAV 44.1 kHz 24-bit stereo; a real drum but not steel, so not a |
| 19 | BigSoundBank 'Anvil #1', Pablo Bergel | https://bigsoundbank.com/anvil-1-s3589.html | 2026-11-05 | not relevant: A struck anvil; real ringing steel but not a drum or barrel. Listed formats: WAV, FLAC, AIFF and others |
| 20 | Lots of Sounds 'Metal Oil Tank Hit' (fs-108264) | https://www.lotsofsounds.com/sounds/metal-oil-tank-hit-fs-108264 | 2026-11-05 | used: A large metal oil drum banged with echoing resonance: the closest match found to the target. The origin is not |
| 21 | Freesound 628491 (The Sample Workshop, metal bar multisample) | https://freesound.org/people/The_Sample_Workshop/sounds/628491/ | 2026-11-05 | rejected: A processed multisample of a hit metal bar built with granular and wavetable synthesis; not a raw recording |
| 22 | Freesound 795432 'Metal Barrel Hit' (rtyrty...) | https://freesound.org/people/rtyrtyrtyrtyrtyrtyrtyrty/sounds/795432/ | 2026-11-05 | unreachable: Title suggests a metal barrel hit; the fetch failed with a client error and nothing was read |
| 23 | freesound.org from the sandbox shell (site, API and CDN) | https://freesound.org | 2026-11-05 | unreachable: The shell gets 403 from freesound.org and its apiv2 search endpoint, and no connection to cdn.freesound.org; p |
| 24 | bigsoundbank.com from the sandbox shell | https://bigsoundbank.com | 2026-11-05 | used: First probe gave no connection (000); a later probe the same evening gave 200 for the home page, a sound page |
| 25 | archive.org from the sandbox shell | https://archive.org | 2026-11-05 | unreachable: No connection (000) from the shell on both probes; no content read |
| 26 | Zenodo records API, eight dataset queries | https://zenodo.org/api/records | 2026-11-05 | not relevant: Queries for oil drum, steel drum, metal barrel, struck steel and similar returned no struck-steel dataset. Two |
| 27 | VSCO-2-CE LICENSE, sgossner/VSCO-2-CE | https://github.com/sgossner/VSCO-2-CE | 2026-11-05 | not relevant: Licence text of an orchestral sample library; the repository's contents were not opened, so whether it holds a |
| 28 | GitHub archive and codeload URLs for qualitycoding/oildrumkit | https://github.com/qualitycoding/oildrumkit/archive/refs/heads/main.zip | 2026-11-05 | used: Returned 403 until the repository was attached to the session; after attachment the main-branch zip downloaded |
| 29 | GitHub REST API, api.github.com | https://api.github.com | 2026-11-05 | used: Rate-limit and repository endpoints answered 200 after attachment; the repository record shows public visibili |
| 30 | Free to Use Sounds home page (freetousesounds.com) | https://www.freetousesounds.com | 2026-11-05 | not relevant: A library of recordings made by the two founders; categories seen were nature, rain, instruments, ASMR, horror |
| 31 | Free to Use Sounds licence agreement | https://www.freetousesounds.com/license-agreement | 2026-11-05 | unreachable: The page that holds the full terms; WebFetch refused it as not on the session allowlist, so nothing was read |
| 32 | freetousesounds.com from the sandbox shell | https://freetousesounds.com | 2026-11-05 | unreachable: After the owner added firewall egress for the host, the shell still got no connection, and the proxy log shows |
