# Prior knowledge loaded (R0)

## Prevention-rule checklist

1. Verify blind references against their sources by an independent decode before a round is issued (Critical lesson L-20261106T010000Z-verify-blind-references-against-sources). Applied: T-L02 (D-004).
2. Keep the listening plan's evidence branch consistent with the adapter contract (L-20261104T013500Z-plan-evidence-branch-contract-mismatch). Applied: D-009 (orphan branch `evidence/G-LISTEN-round-N`, no workflow).
3. Never edit frozen tests or regenerate the manifest after freezing. Applied: tests/FROZEN_MANIFEST.sha256, S-010 red check.
4. A gate must be able to fail. Applied: negative test of check_plan.py; T-L02 and T-L08 fail cases.
5. Commit gate records in the same commit as the response (L-20261101T042000Z-commit-gate-records-with-the-response). Applied: S-050.
6. Do not claim behaviour that was not listened to or rendered (L-20261009T090805Z-generative-audio-claims-made-without-listening-or-rendering). Applied: D-015, C-06.
7. Treat the sandbox as ephemeral and push early. Applied: A-004, bootstrap idempotence.
8. Never store or use a pasted credential; tell the owner to revoke it. Applied: HANDOFF.md.
9. Use British English in prose; cite British sources for language rules. Applied: the prose of this bundle.

## Items loaded and how each is applied

The lessons above were read from the Lessons store. From the knowledge store, the sandbox egress item of 2026-10-09 (K-20261009T193501Z-sandbox-egress-policy) was compared with today's probes: it recorded 403 for zenodo.org and api.github.com, and both now answer 200, so it is complemented by K-20261105T223300Z-sandbox-reachability-2026-11-05. No register from an earlier run existed (R0 step 5); this is the first run to produce one.
