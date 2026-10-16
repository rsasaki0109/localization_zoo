# Sensor setup assistant validation

The implementation source is `37416fd536766bdf9a3ce514da212ec5a4252431`.
Execution evidence was committed before publishing the numbers in the guide.
This is diagnostic-tool validation, not a new odometry accuracy result.

- `reports/kitti00.json`: the complete report for KITTI 00 frames 000000,
  000001 and 000002, with command, input hashes and source commit.
- `kitti_download.json`, `fetch_commands.json`, `fetch_kitti_samples.py`:
  official archive entries, hashes and the executed selective-fetch script.
  It reuses `fetch_kitti_odometry_sequences.py` one entry per Range request.
- `checks/commands.json` and logs: actual configure/build, CTest, Python and
  document-check exits, plus the assistant command. CTest initially could not
  write a pre-existing root-owned log directory; no tests started. The failed
  start is preserved and the rerun uses the prior container default user.
  Final per-check exits are zero: CTest 122 tests and Python 280 tests.
- `environment.json`: interpreter/dependency versions and the capture command.
- `historical_aggregate_sha256.json`: the historical benchmark aggregate hashes;
  the assistant does not alter them or estimator defaults.
- `protocol.json`: fixed tool configuration and development scope.
- `nclt_network_status.json`: the actual connection denial and observed policy.
  Host addition is approved but not applied. Real NCLT validation is pending.
- `fetch_nclt_samples.py`: prepared bounded retrieval, not executed while the
  official host is blocked. It requests only an initial compressed prefix and
  stops after the requested scan entries; it never fetches the full archive.

Raw KITTI/NCLT scans are not committed. Paths in captured commands and copied
one-off helpers are the original execution paths; adjust the workspace/output
prefix when replaying them. The public assistant is invoked from the repo root:

```bash
python3 evaluation/scripts/check_scan_frame.py scan_0.bin scan_1.bin scan_2.bin --json-output setup.json
```

The KITTI nominal FOV is supplied by the model candidate; the identity pose is
an assumption, not a measured calibration. Observed elevation extents are
reported separately. Synthetic HDL-32E tests exercise an inverted/translated
mount, but do not complete the missing NCLT real-data check. See
[the guide](../../../docs/sensor_setup_assistant.md) for the supported inference
limits and candidate-flag interpretation.
