# Passive InTEn-LOAM CLI export

The runner exports the existing passive solver API when
`--inten-loam-paper-trace-jsonl NEW.jsonl` is supplied. It is default off,
outside the paper, and supplies no pose or solver gate. This branch depends
on [PR #140](https://github.com/rsasaki0109/localization_zoo/pull/140).

[Usage and record semantics](../../../docs/inten_loam_passive_trace.md#streaming-export-from-the-evaluation-runner)
explain coordinate frames, incomplete traces, skipped inputs and explicit
nonfinite values. The [protocol](protocol.json) pins the source and settings.

The committed [serializer checks](evidence/serializer_check/summary.json)
pass all 3 cases with actual compile/test exit codes zero. They cover failed
step values, matrix order and precision, escaped paths, effective settings,
existing-file protection and the completion record. Evidence was committed
at `7e74cb76` before this prose. The exact [commands](evidence/serializer_check/commands.json)
and losslessly compressed logs are retained.

The full CMake build and short-fixture runner comparison are still pending.
No full KITTI failure is explained and no speed or accuracy improvement is
claimed. The original whole-zoo replay continues in a separate frozen tree;
this study fetches no scans and changes no stored result.
