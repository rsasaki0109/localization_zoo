# Runtime recreation: exact executable restoration and fresh05 restart

The managed environment was recreated from an earlier snapshot while the
recovered05 audit was comparing frozen controls. Its workers, owned05 scans,
uncommitted05 outputs and both pinned executables were absent afterward.
The initial snapshot executable did not match either frozen executable.
The [interruption receipt](interruption.json) retains unknown process exits as
null. No unavailable05 output contributes to the cleaned checkpoint.

The committed [04/00/01/02/03 checkpoint](../checkpoints/03/summary.json) remains
authoritative. Its original results and the historical aggregates are retained.
The previous recovery required retained scans and an interrupted-attempt
archive; neither survived this recreation. It was not invoked again.

## Exact reconstruction, before replay

The existing Docker image and original root build path were used, with the
recorded GNU compiler and Release configuration. Each executable was rebuilt
at its original source commit and accepted only after SHA256 matched the
existing committed spec. The specs were not rewritten.

| Executable | Source | Rebuilt SHA256 | Match |
|---|---|---|---|
| Baseline | `40278ee01ede562a490e4d09d256e435a1b562d6` | `ede46d492a31f6dfea95b111bb8941974ad63a903c68e9ee860f7e2f60ca2450` | exact |
| Candidate | `0b079dbbcb7c6f58c1d8367ade881c15f003e525` | `e9ed1f650d42b73d1156e8b3eda3a145ebed4ad382323863f0d89d886d5d166d` | exact |

The [baseline receipt](baseline_commands.json), [hash check](baseline_hash.json)
and [build log](baseline_build.log) were committed at `f98f6b7c`.
The [candidate receipt](candidate_commands.json), [hash check](candidate_hash.json)
and [build log](candidate_build.log) were committed at `5722b805`, before this prose.
Both configure/build commands and both hash checks exited **0**.
The recorded build command for each source was:

```bash
env -u DOCKER_HOST -u DOCKER_CONTEXT -u DOCKER_TLS -u DOCKER_TLS_VERIFY -u DOCKER_CERT_PATH \
  docker --host=unix:///var/run/docker.sock exec --user 0:0 \
  -e OMP_NUM_THREADS=1 -e OPENBLAS_NUM_THREADS=1 \
  -w /workspace/localization_zoo localization-zoo-dev \
  cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
env -u DOCKER_HOST -u DOCKER_CONTEXT -u DOCKER_TLS -u DOCKER_TLS_VERIFY -u DOCKER_CERT_PATH \
  docker --host=unix:///var/run/docker.sock exec --user 0:0 \
  -e OMP_NUM_THREADS=1 -e OPENBLAS_NUM_THREADS=1 \
  -w /workspace/localization_zoo localization-zoo-dev \
  cmake --build build -j1 --target pcd_dogfooding
```

The initial checkout encountered snapshot directory ownership that prevented
host writes. The [repair receipts](baseline_checkout_success.json) distinguish
that failed checkout from the later successful source switch. Only ownership
metadata and the agent's partial checkout were repaired; stored result bytes
were preserved.

## Fresh boundary and remote checkpoint backup

The [protocol](restart_protocol.json) and [launched bootstrap](launched_bootstrap.py)
were committed at `c2d1d9ba` before starting native work. The
[preflight](resume_preflight.json) verifies the exact binaries, recursively
checks **527** original native paths, checks sealed execution files, requires
the committed cleaned prefix and absent remaining-sequence evidence, and
rejects an existing controller/native worker or backup branch.

The original controller starts at the fresh fetch boundary:

```bash
/workspace/localization-env/host-venv/bin/python \
  /workspace/localization_zoo/experiments/results/shared_near_range/run_all.py \
  --resume-from 05
```

Its source, cell selection, arguments, native wrappers and thread counts are
unchanged. Official scans are fetched one owned sequence at a time and checked
against committed official input hashes; the original controller rehashes
before deletion and commits cleanup before moving on. This audits historical
per-cell settings, not a new fixed-setting accuracy study.

One supervisor starts that controller and normally pushes its completed
sequence commits to `codex/shared-range-replay295`. It verifies ancestry,
restricts changes to remaining-sequence evidence and requires committed,
hash-verified cleanup before pushing. It never force-pushes. A failed push
retains its real exit and remains pending. A reset can still lose the current
uncommitted sequence; the backup protects successfully pushed checkpoints.

The [initial backup receipt](backup_commands.jsonl) has exit **0**.
The [launch receipt](resume_launch.json) and [launch-time status](status_at_launch.json),
committed at `8fa9bf8b`, record a pending05 controller, not completed05 evidence.
No completion worker was started; finalization requires full controller success.

The publication guards reject estimator changes, edits to the closed prefix,
unverified cleanup and retained inputs. [Guard checks](bootstrap_guard_checks.json)
pass **4 tests** with exit **0** in normal and optimized Python. The
[initial failed optimized check](bootstrap_guard_checks_initial.json) is retained;
it exposed assertion-based guards, which were replaced with explicit runtime
checks before launch. No native work ran during those guard tests.

The [current bootstrap](launch_and_back_up.py) additionally recognizes both
renamed frozen executable basenames when checking for orphaned workers. The
archived launched source and the active supervisor remain unchanged; this
startup guard hardening does not restart or alter the ongoing audit.

## Integration checks

[Check receipts and complete logs](checks/), committed at `0d794ff3`, retain
actual commands and exits. The full local Python suite runs **327 tests** with
exit **0** and **5** skips for guards requiring an evaluator built in this
separate documentation worktree. Native compilation is not repeated in the
active replay checkout.

All three required generators and the showcase, leaderboard and paper-ready
checks exit **0**. Regenerated content differs only in generation timestamps;
the [disposition receipt](checks/generation_disposition.json) records both
hashes and retention of the prior bytes. The [preservation check](checks/preservation.json)
verifies all **688** historical aggregate hashes, published Table 6/full-variant
tables, paper numbers, root README and HTML, native/evaluation sources, method
READMEs and prior shared-component studies.

The first whitespace check exited **2** for a compiler log's blank final line.
Its exact output is now retained in [JSON](baseline_compiler.json), including
the original trailing newlines and SHA256. The [follow-up check](checks/whitespace_followup.json)
exits **0**. These checks establish restoration and preservation; they do not
complete the remaining KITTI audit or add a new accuracy/timing measurement.

## Integration with saved04 diagnosis

Integration source `d621fe2a` retains the reviewed saved04 CT/Mesh diagnosis
and the runtime restoration together. The roadmap conflict was resolved by
retaining both the diagnosis link and the newer recreation/restart status.
[Resolution and input/source preservation](merge144/source/) and
[complete integration check receipts](merge144/checks/), committed at
`51214662`, retain actual commands and exits. The local Python suite runs
**331 tests** with exit **0** and the same **5** missing-built-evaluator skips.
The normal/optimized bootstrap guard checks, required generators, publication
checks and full-branch whitespace check all pass. The historical aggregates,
paper values, native code, prior diagnostic evidence and both pinned runtimes
remain unchanged. The active05 controller is kept in its original checkout.
