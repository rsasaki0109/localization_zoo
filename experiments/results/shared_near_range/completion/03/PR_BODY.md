## Summary

Share the existing optional sensor-frame near-range filter without changing defaults. All 177 selected full KITTI replay cells are now complete. 35 differ from historical metrics; all differing cells match frozen pre-extraction controls and remain documented. Original nonzero historical audit exits are retained. Fresh NCLT validation remains open.

## Type

- [ ] New paper reimplementation
- [ ] Bug fix
- [x] Benchmark / evaluation
- [x] Docs / tooling

## Checklist

- [x] Builds clean and ctest passes (frozen native source, preflight evidence)
- [ ] (New paper) ≥3 mechanism unit tests: N/A, no new paper
- [ ] (New paper) KITTI full seq00 / seq07: N/A, historical replay audit
- [ ] (New paper) Results row + docs/methods.json: N/A, no promotion
- [ ] (New paper) Module README: N/A, updated existing method READMEs
- [x] Results reported honestly, including historical differences and nonzero audit exits

## Results / notes

Native source `0b079dbbcb7c6f58c1d8367ade881c15f003e525`; full committed report at `28406fe81748bb4aae7f67d35a587d5b44ec59a0:experiments/results/shared_near_range/completion/03/report/summary.json`. See the study and method READMEs for all numeric outcomes, exact commands, cleanup and limits. Final required generators, Python/doc checks and preservation receipts are under `experiments/results/shared_near_range/completion/03/checks`. All stored aggregates and published paper values are preserved. Defaults remain off. This body is prepared for the connected GitHub tool; confirm CI at the pushed head before marking PR137 ready. No merge is performed.
