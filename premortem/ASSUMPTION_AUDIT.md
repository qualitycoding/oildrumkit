# Assumption audit

| A-ID | Verified how | Result |
|---|---|---|
| A-001 | `git ls-remote` | branch exists at 21972a2 |
| A-002 | spike S3 diff against 35fbfea | unchanged |
| A-003 | cannot verify | open (Q3) |
| A-004 | sandbox reset observed 2026-10-09 | holds |
| A-005 | cannot verify; this session has no push access | open, S-000 checks |
| A-006 | anonymous clone in S3 | holds |
| A-007 | HTTP probes (S5) | holds |
| A-008 | cannot verify | open (Q1) |
| A-009, A-010 | frozen by D-981 and skill | holds |
| A-011 | tests ran with these versions | holds |
