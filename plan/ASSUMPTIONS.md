# Assumptions (A-###)

Intake. The protocol allows one intake batch; none could be held while planning, so every ambiguity below took the stated default
and is listed in the hand-over message for the owner to override at the first opportunity (an override re-runs only the steps named).

| ID | Assumption (default taken) | Overridden by / audited in |
|---|---|---|
| A-001 | Target: `qualitycoding/oildrumkit`, branch `impl/listening-validation` (exists at 21972a2, base `main` e31e8d0; `main` is now 1900ac8). The plan resumes that branch, never recreates it. | `premortem/ASSUMPTION_AUDIT.md` |
| A-002 | "Current code" is `main` as of planning (1900ac8). Its `Source/`, `Tools/`, `CMakeLists.txt` and `Renders/` are identical to commit 35fbfea, the commit the modal-engine rewrite was audited at (spike S1). | S-000 bootstrap re-checks at run time |
| A-003 | The listener is the repository owner, on a phone with their own headphones, replying in chat or by committing `GATE-G-LISTEN.RESPONSE.md`. | unverifiable; gate allows `stop` |
| A-004 | The execution environment is `ephemeral`: a working copy was lost to a sandbox reset on 2026-10-09 (lesson L-20261004T102007Z-sandbox-reset-lost-working-tree). Remote branches are the only durable surface. | S-000 probe |
| A-005 | The implementer can push to `qualitycoding/oildrumkit` (branches `impl/listening-validation`, `evidence/G-LISTEN-round-N`) once the session attaches the repository with push access; on 2026-11-05 the API reported push permission after attachment. It cannot push workflow files, and no `.github/` directory exists on `main`. | S-000 `git push --dry-run` |
| A-006 | The repository is public (GitHub API: private false, Apache-2.0, 2026-11-05). Evidence on an orphan branch is therefore world-readable, which is why reference licences must permit redistribution (D-014). | S-000 |
| A-007 | No real-recording references exist yet (round 1 is `blocked`, prerequisite P-4), and none can be downloaded from the implementer's shell: freesound.org answers 403 to the shell; bigsoundbank.com first refused and later answered 200 (spike S5); five reference pages were read with WebFetch and none was a usable reference (spike S4). Zenodo, PyPI, raw.githubusercontent.com and, after attachment, api.github.com are reachable. | G-REF |
| A-008 | The owner can supply references by Route A (recording their own, D-005) or by downloading leads from a browser (Route B). | unverifiable; Route C (demos only) is the fallback |
| A-009 | The pass rule is the one frozen by D-981 and the skill: at most 8 of 12 correct and no unresolved qualitative finding. A pass supports only the narrow claim in D-015. | none |
| A-010 | Allowed reference licences: CC0-1.0, CC-BY-4.0, CC-BY-3.0, and owner-attested CC0-1.0 or CC-BY-4.0. Non-commercial (NC) and no-derivatives (ND) licences are refused. | D-014 |
| A-011 | Python 3.13 with numpy 2.5.3, scipy 1.18.1, soundfile 0.14.0, pytest 9.2.0 and g++ 13.3 are available or installable from PyPI. | S-000 |
| A-012 | Lessons go to `qualitycoding/Lessons` and knowledge items, including the source register, to `qualitycoding/knowledge`, at gate G-003 (protocol 3.7.2). Both stores exist and have the Appendix C layout and schema script. | S-070 |
