# Spike S3: self-initialisation script `plan/bootstrap/bootstrap.sh` run against the live repository (2026-11-05)

First run in an empty work directory: cloned `qualitycoding/oildrumkit`, found `origin/impl/listening-validation` (21972a2),
checked it out, merged `origin/main` (1900ac8, a README-only change since the branch's base e31e8d0) locally with no conflict
(merge commit af70e63, not pushed), confirmed both vendored tool hashes, built and rendered the demo, and found the render
bit-identical to `Renders/after/oildrumkit_after.wav`. Second run in the same directory: exit 0, same base (idempotent).
`code_changed_since_audited_commit` is empty. Result file: `S3_bootstrap_result.json`.

## Amendment (same day): idempotence bug found and fixed
The first version of the script used `git checkout -B impl/listening-validation origin/impl/listening-validation`, which resets a local
branch and would discard unpushed implementation commits after an interrupted run. It now never resets: it resumes the local branch,
fast-forwards only, and exits 6 (BLOCKED) on divergence. Three consecutive runs in one directory gave exit 0 and the same base commit.
