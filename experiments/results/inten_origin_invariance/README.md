# InTEn-LOAM: origin dependence of a fixed solver update

The native update has a measurable dependence on the target-coordinate origin.
This equation-level experiment separates its finite retraction from its fixed
coordinate damping. It does **not** establish the cause of the KITTI failures,
test a replacement estimator, or measure an accuracy improvement.

Source/protocol `8cdac888b64f4f29722b6c4e4edcdfed006455eb` precede execution.
Raw execution evidence is committed in `80cfaf23`; stage maxima and their exact
aggregation command are committed in `c316b4df`, before this interpretation.

## Fixed protocol and inputs

The [protocol](protocol.json) includes every observed system in all **7** existing
positive conditioning archives: **231** iterations from the short real MCD
fixture and its compatibility variants, including skipped-input handling.
Some archives have identical bytes; these are compatibility tuples, not
independent statistical samples. No KITTI failure data select a parameter.

Each system receives the same predeclared shifts: zero and **±100 m / ±1,000 m**
along each target axis. Two analytic systems supply pure translation and pure
rotation controls. All **3,029** cases and their original systems are retained.
The driver checks committed input bytes and rechecks hashes after execution.
No scan, correspondence, residual or trajectory is reconstructed or rerun.

`native_fragments.h` retains the original retraction verbatim and the exact
regularized solve from native source `09e173f3`:
`delta = (H + 1e-6 I).ldlt().solve(-g)`.
The native function named `expSe3` rotates using AngleAxis but directly sets
translation to `v`; it does not apply the SE(3) exponential's translation
Jacobian. The experiment changes no production source or estimator setting.

## What is compared

For a pure target-origin translation `C=(I,c)`, with tangent order `[v,omega]`,
`B=Ad(C)=[[I,[c]x],[0,I]]`. Re-express the same equations and pose as
`H'=B^-T H B^-1`, `g'=B^-T g`, `T'=C T`.

First transport the **same** original step as `delta'=B delta`, without solving
again. Compare the updated sensor position from the native retraction
`C^-1 F(delta') C T` against `F(delta) T`. Their difference is
`(R_delta-I)c + c cross omega`. This isolates the finite-update property from
both associations and damping. An independently implemented exact SE(3)
exponential supplies a conjugacy control, not an estimator option.

Then re-solve the shifted equations with the unchanged native `1e-6 I` damping.
This prior depends on the coordinate origin. Report infinitesimal translation
in sensor axes as `R.T (v - t cross omega)`, and rotation in radians separately.
Also solve with the transported original penalty `1e-6 B^-T B^-1` as a
**numerical control**: coordinate re-expression can itself lose precision.
This control is neither a proposed solver nor a trajectory result.

## Measured results

Maxima below cover all stored tuples and both signs of every declared axis.
S2S uses a local target scan in the estimator; shifting that target by a large
distance here tests a mathematical property, not the actual S2S world origin.
S2M uses the map target. Neither table represents accumulated trajectory drift.

| Stage | Shift norm (m) | Same transported step: position difference (m) | Native re-solve plus native update: position difference (m) |
|---|---:|---:|---:|
| S2S | 100 | 0.0966418 | 0.0966268 |
| S2S | 1,000 | 0.966418 | 0.950322 |
| S2M | 100 | 0.00696058 | 0.00696090 |
| S2M | 1,000 | 0.0696058 | 0.0692961 |

| Stage | Shift norm (m) | Native re-solve: sensor translation difference (m) | Native re-solve: rotation difference (rad) |
|---|---:|---:|---:|
| S2S | 100 | 6.41111e-6 | 3.84903e-6 |
| S2S | 1,000 | 5.26911e-4 | 3.76920e-4 |
| S2M | 100 | 2.44281e-6 | 4.25696e-7 |
| S2M | 1,000 | 2.36406e-4 | 4.09184e-5 |

The transported-penalty numerical control differs by at most **7.56956e-13 m**
and **4.33051e-13 rad** over the stored fixtures. Its small errors support the
damping comparison for these systems; they are not a general stability bound.
Each table cell has its own maximizing case; columns need not share a case.
The largest same-step S2S case is `s216_c7`: skipped-input fixture, accepted
frame **1**, original input **2**, iteration **0**, shift `[-1000,0,0]` m.
The largest S2M case is `s10_c7`: legacy-default fixture, frame **1**,
iteration **0**, same shift. These are individual iterations, not converged
registration outputs. [Stage maxima and case IDs](reported_values.json) and
[every case](evidence/cases.jsonl.gz) retain the full selection.

All observed native deltas reproduce with maximum component error **0**.
Over all cases, the analytic finite-error formula agrees within
**2.01377e-13 m** and exact-exponential conjugacy within **2.15111e-13 m**.
Pure translation passes the predeclared zero-rotation control. Both solves are
finite in every case; **0** metrics are unavailable. Compilation, native
execution, the full driver and aggregation each exit **0**. These controls
validate this bounded experiment, not a full-sequence fix.

## Reproduce and interpret

The actual [compiler/build commands](build_commands.json),
[driver invocation and exit](analysis_command.json),
[native command, input hash, executable hash and exit](evidence/command.json),
and [aggregation command and exit](report_command.json) are retained.
The raw input stream, native CSV/stderr, original systems and parsed cases are
losslessly archived under [evidence/](evidence/). The aggregate summary preserves
per-tuple finite/unavailable counts and failed controls without zero-filling.

On the recorded container, from the repository root:

```bash
c++ -std=c++17 -O3 -DNDEBUG -I/usr/include/eigen3 \
  experiments/results/inten_origin_invariance/probe.cpp \
  -o /workspace/localization-env/inten-origin-study/probe
python3 experiments/results/inten_origin_invariance/run.py \
  --binary /workspace/localization-env/inten-origin-study/probe \
  --output-dir /workspace/localization-env/inten-origin-study/fresh-evidence
```

The driver launches that binary in `localization-zoo-dev` through the explicit
local Docker socket; the output directory must be fresh. The command receipts
record the exact host/container paths, user, thread settings and source commit.
Compilation inside that container uses the recorded Docker prefix.

This negative result identifies an implementation property worth checking in
the failing natural traces. Correspondences are frozen here: re-association,
nonlinear iterations, fallback, map evolution and stopping criteria are not
tested. Nothing here shows that exact exponentiation or different damping
would improve full KITTI results. No non-paper option or default is added, and
existing accuracy/paper results remain attached to their original runs.

[Repository checks](checks/commands.json) retain every command and actual exit,
with losslessly compressed logs and [preservation hashes](checks/preservation.json).
All required documentation generators and validators exit **0**. The full
Python suite runs **331** tests with **5** built-evaluator skips in this separate
checkout, exiting **0**; native coverage is left to the PR CI build. All **688**
historical aggregate hashes, Table 6, the full-variant CSV, root README,
showcase and paper-number JSON remain unchanged. Regenerated timestamp-only
changes retain prior bytes with a [disposition receipt](checks/generation_disposition.json).

Next, after the frozen whole-zoo replay releases native/input ownership, collect
continuous natural traces on the failing sequences and measure their actual
rotation steps, map origins and error-growth intervals. Only evidence linking
this effect to those failures justifies a fixed, opt-in candidate evaluated
unchanged across the full paper sequence set.
