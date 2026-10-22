# Official KITTI ground-truth input audit

Numerical evidence `4c3d972b` and portable checks `6802c1e0` precede this
interpretation. The suspected08 frame-conversion problem is **not supported**.
Official pose/calibration metadata were fetched with inherited proxy/CA trust,
bounded to16MiB per archive; no sensor scans were fetched or changed.

The unchanged converter, applying `T_world_lidar = T_world_cam * Tr`, was run
on every official00–10 sequence: all11 conversion commands exit0. All11
committed GT CSVs have exactly the same numeric fields as regeneration;
maximum translation and reconstructed-rotation differences are0 in every
sequence. Nine are byte-identical;06/09 differ only in LF versus CRLF line endings.
The full official metadata ZIPs, CRC/member manifest, SHA256 hashes, actual
commands, complete logs and source commit are retained here. They contain GT,
calibration and timing metadata, **no raw scans**.

The08/38–46 relative XYZ translations are in `semantic_report.json`. Their
large vertical component already exists in the official metadata after proper
LiDAR-frame conversion; it is not evidence of swapped camera/LiDAR axes in
this pipeline. No claim of erroneous official GT follows, and no GT component
is removed. Earlier motion metrics, thresholds and rejected candidate gates
remain unchanged. Official-GT error and geometric registration cost measure
different things; a lower geometric cost does not certify motion accuracy.

Byte-level follow-up `formatting_report.json` corrects the initial signed-zero
explanation: all text is identical after only CRLF-to-LF normalization. The
original regeneration/semantic measurements stay unchanged; numeric agreement
alone had not justified naming the formatting cause.

Actual historical invocations are retained in `commands/*/*_command.json`;
the copied measurement scripts preserve their original private working paths.
Portable offline recheck from the repository root:

```bash
python3 experiments/results/inten_gt_input_audit/verify.py --output /tmp/inten_gt_input_audit.json
python3 -m unittest discover -s tests -p 'test_inten_gt_input_audit.py' -v
```

Both actual commands exit0 in `commands/verify_stage/` and `commands/unit_stage/`;
four tests check full regeneration and reject changed translation, rotation or
frame IDs. The portable guard allows one printed decimal quantum plus eight
floating-point ULPs, declared from the converter's10-decimal serialization.
This changes no motion-error gate, sensor configuration or numerical tolerance
in any historical study. All KITTI observations remain development evidence.
