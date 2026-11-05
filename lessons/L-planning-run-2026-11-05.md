# Lessons from planning the listening phase (drafted; not pushed)

1. A fixed loudness target in a vendored gate tool can be impossible for the material (percussive hits at -23 dB RMS clip). Measure crest factor first and compute the level per round; record why.
2. A bootstrap that uses `git checkout -B` is not idempotent and can discard unpushed work. Resume, fast-forward only, and refuse on divergence.
3. A test that passes against a stub proves nothing; run every frozen test against stubs and require the expected failures before implementing.
4. Probe which hosts the shell can reach before writing a plan that depends on downloads; name the human action instead of assuming it.
5. A leads list that was never opened is not evidence; label unreachable sources as unverified.
