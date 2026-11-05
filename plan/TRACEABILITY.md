# Traceability

| Requirement | Step | Test |
|---|---|---|
| Plan reads the protocol and starts from the listening phase | S-000, S-010..S-050 | `check_plan.py` |
| Initialise itself with the current code | S-000 | bootstrap exit codes (spike S3) |
| Vendored pair tools unmodified | S-010 | T-L01, T-L09 |
| Level fits percussion | S-035 | T-L05 |
| Blind integrity: seed, key, format | S-035 | T-L12, T-L04, T-L03 |
| Reference decoded independently | S-035 | T-L02 |
| Reproducible rounds | S-035 | T-L15 |
| Reference manifest rules | S-020 | T-L08, T-L14 |
| Personal-use references linked, not republished (D-020) | S-020, S-040 | T-L08d, T-L16 |
| Candidate rendering | S-030 | T-L06, T-L07, T-L13 |
| Record and secrets scan | S-045, S-050 | T-L10, T-L10b, T-L11 |
