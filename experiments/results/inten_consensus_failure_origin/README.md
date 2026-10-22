# Where recursive consensus introduces short steps

These are **observed stage crossings on the candidate's own realized history**.
The original joint solve is retained in the same frame. A good original joint
output followed by a bad accepted output localizes an immediate intervention
failure. It does not reconstruct what later mapping would have done without the
intervention. A bad corrected seed that remains bad after refinement distinguishes
an unsafe seed selection from a new crossing after an initially good corrected
seed; it does not prove which physical surface or correspondence caused it.

Local labels keep the inherited threshold: translation length/vector error above
20% for GT steps at least 0.1 m. Signed forward ratios are also retained, so a
reversed vector is not mistaken for correct motion just because its length agrees.
GT is used only offline. There is no estimator change, native rerun, new trajectory,
accuracy score, parameter sweep, per-sequence adoption or held-out claim. Original
07 calibration, 00 diagnosis, inherited DiLO 01 prior and all earlier KITTI exposure
remain development. Inspecting the previous committed 01/426 and 02/4473 records
motivated this diagnosis; coverage is every frame of both fixed arms on 00–10.

Complete numerical snapshot `f391d551482ecffd78dfb2fea910e7c4ee5886e0` precedes this interpretation.

## Every full sequence

| Sequence | Moving applied | Length crossing already at corrected seed | Length crossing after good corrected seed | Vector crossings | Joint bad to accepted good (length) | New final bad length vs reference |
|---|---:|---:|---:|---:|---:|---:|
| 00 | 4513 | 0 | 0 | 0 | 1 | 0 |
| 01 | 1094 | 3 | 0 | 3 | 11 | 69 |
| 02 | 4655 | 4 | 0 | 4 | 4 | 29 |
| 03 | 784 | 0 | 0 | 0 | 0 | 0 |
| 04 | 269 | 0 | 0 | 0 | 0 | 0 |
| 05 | 2682 | 0 | 0 | 0 | 0 | 0 |
| 06 | 1099 | 0 | 0 | 0 | 0 | 0 |
| 07 | 1009 | 0 | 0 | 0 | 0 | 0 |
| 08 | 4039 | 7 | 0 | 0 | 0 | 0 |
| 09 | 1582 | 0 | 0 | 0 | 0 | 0 |
| 10 | 1152 | 0 | 0 | 0 | 0 | 0 |

The first two crossing columns partition moving, applied frames where the original joint length was good and the accepted length became bad. Rescues and all remaining joint-bad cases are retained in the complete report. Vector crossings are classified separately. New final bad frames are the exact previously committed reference/candidate set, not a new metric.

## Every bad-length interval entered with a good joint estimate

| Sequence | Final bad-length interval | Prediction / GT | Joint / GT | Corrected seed / GT | Accepted / GT | Final / GT | Chosen shift [m] | Chosen / zero / valid clusters | Refinement stop |
|---|---|---:|---:|---:|---:|---:|---:|---|---|
| 01 | 426–457 | 1.025837 | 0.966919 | 0.095879 | 0.054142 | 0.071003 | -2.950000 | 56 / 51 / 91 | converged |
| 01 | 811–834 | 1.022060 | 0.963182 | 0.066643 | 0.044600 | 0.049303 | -2.400000 | 69 / 66 / 97 | converged |
| 01 | 838–850 | 0.999113 | 0.975336 | 0.039713 | 0.013883 | 0.129892 | -2.200000 | 75 / 73 / 111 | iteration_limit |
| 02 | 1890–1890 | 1.014227 | 0.997321 | 0.178329 | 0.165588 | 0.209606 | -1.100000 | 57 / 55 / 61 | converged |
| 02 | 1892–1896 | 1.034645 | 1.015129 | 0.125218 | 0.125855 | 0.192460 | -1.200000 | 47 / 41 / 50 | iteration_limit |
| 02 | 1899–1900 | 1.010196 | 1.016934 | 0.149338 | 0.142927 | 0.217370 | -1.150000 | 70 / 68 / 74 | converged |
| 02 | 4473–4492 | 1.061591 | 1.047970 | 0.092329 | 0.087818 | 0.084815 | -1.350000 | 65 / 60 / 67 | converged |

All baseline/candidate length and vector interval entries, their previous frame, every step, all damage events and all rescues remain in `report/`. The chosen and zero-shift votes use the actual admission bits, verified independently against every archived decision. A majority is a property of these cluster votes; it does not establish static-scene truth. Converged stops and finite pair counts do not establish correct motion.

## Consequence for the next change

The full experiment cannot be explained solely by accumulated map feedback:
some failures are already visible between the valid original joint output and
the actual accepted S2S intervention in one frame. Automatic adoption of the
preceding conditional feasibility result was insufficient. No new acceptance
rule is adopted here. A prospective safeguard must retain the old rescue cases
and be tested on these newly exposed failures with native re-association and
the same realized input history before another full recursive experiment.
Zero-shift support, signed seed displacement, mask changes and refinement steps
must be separated; rejecting iteration-limit stops alone cannot explain a
failure whose actual stop is converged. These observations do not justify a
threshold sweep or a guarantee that a simpler gate will restore full accuracy.

## Reproduction and checks

Run from the repository root with a fresh private output directory:

```bash
python3 experiments/results/inten_consensus_failure_origin/analyze.py --output-dir /tmp/fresh_consensus_failure_origin
python3 experiments/results/inten_consensus_failure_origin/verify.py --output /tmp/fresh_consensus_origin_verification.json
```

`protocol.json` pins sources and all parent inputs. Trace shards and full raw
streams are SHA-256 checked; every consensus vote/mask/projected update is
independently verified. Full frame coverage, original native exits and metrics,
GT/pose coverage and every pre-existing step label/new-frame set are checked.
The report recomputes all stage decisions and counts exactly; portable float
comparisons allow relative 1e-10 / absolute 1e-12 roundoff. Actual argv, cwd,
source/environment and subprocess exits are archived in `analysis_execution/`
and `checks/`. No test success is inferred from a log suffix. Native code,
historical results, published defaults, Table 6 and paper metadata stay unchanged.


## Portability correction after the initial CI failure

The initial Native CI passed its build, CTest and fixture checks, then the new Python diagnostic failed an internal ratio comparison at relative/absolute 1e-12. The complete failure and successful ROS2 logs remain in `checks/ci_initial_failure/`.

Forced Haswell/Sandybridge kernels reproduced 21/32 strict differences among 137424 comparisons each; maximum absolute differences were 2.42728059873798e-12/2.352229522273319e-12. Every original label, stage count and native metric remained exactly equal. These measured probes were committed before the correction.

The internal comparison now uses the same already documented portable bound as the complete-report verifier: relative 1e-10, absolute 1e-12. This is a verification tolerance, not a motion-label threshold or estimator parameter. All original scientific report bytes are SHA-256 pinned and retained unchanged; original diagnostic/test source bytes and the original protocol are preserved. `portability_source_update.json` records the explicit source amendment and measured evidence. Regression tests accept the reproduced roundoff and reject a substantially different ratio. Actual corrected kernel/full-suite command exits are retained under `checks/portability_correction/`. The original probe commands reproduce at their recorded source commit; the current diagnostic uses the bounded correction.
