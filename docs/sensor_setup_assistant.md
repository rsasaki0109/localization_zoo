# Inspect scans before choosing sensor settings

`evaluation/scripts/check_scan_frame.py` reads a few scans and reports frame
candidates, vertical coverage, row count, intensity range, returns near the
sensor, and sensor flags for the paper pipelines. The search, model matching and image-sizing rules are repository tooling
heuristics. This separate, explicitly invoked tool does not run an estimator,
change defaults, tune registration parameters, or apply a near-range cut.

```bash
python3 evaluation/scripts/check_scan_frame.py scan_0.bin scan_1.bin scan_2.bin \
  --json-output sensor_setup.json
```

PCD supports ASCII and uncompressed binary data, including reordered fields and
integer intensity. A `.bin` input defaults to KITTI float32 XYZI. NCLT's packed
Velodyne format is different and must be selected explicitly:

```bash
python3 evaluation/scripts/check_scan_frame.py --format nclt scan_0.bin scan_1.bin scan_2.bin \
  --sensor-model hdl32e --json-output sensor_setup.json
```

Use `--json` for machine-readable stdout. Every JSON report records input hashes,
the command and source commit. Missing intensity is reported as missing, not as
zero intensity. The raw intensity range is retained; the InTEn-LOAM paper runner
already normalizes its supported byte-scale input.

## Frame evidence and assumptions

The output distinguishes:

- `provided`: `--input-sensor-pose X Y Z ROLL PITCH YAW` supplies calibrated
  extrinsics, using the runner's sensor-to-stored-frame convention.
- `axial_estimate`: a beam-periodicity height search and a dominant horizontal
  surface suggest the sensor z and gravity-up sign. This assumes aligned
  vertical axes and a sensor on the stored z axis. A ceiling can be mistaken
  for ground; inspect the candidates and confirm the mount.
- `assumed_identity`: an uneven-beam model candidate is compatible with a
  sensor-centred frame, but the origin was not measured. For KITTI's uneven
  beam blocks, no single-spacing periodicity claim is made.
- `unresolved`: the evidence does not determine a usable frame. Statistics that
  require the sensor origin and method candidates are withheld; supply known
  extrinsics.

Point clouds alone do not identify arbitrary translation and orientation. In
particular, x/y offsets and yaw are not estimated by the axial search. Specify
known extrinsics rather than treating zero values in a candidate as measurements.

## Rows and field of view

Ring fields and organized PCD dimensions can supply row evidence. Ring-major
scans also expose complete monotone azimuth turns. The tool uses net unwrapped
winding, so calibration jitter crossing the azimuth boundary more than once
does not add fictitious lasers. Shuffled or acquisition-interleaved points do
not meet that ordering check.

A model match is a geometry candidate, not device identification. The report
keeps observed elevation percentiles separate from nominal model limits. This
matters on KITTI: near returns and individual laser offsets extend beyond the
nominal elevation limits. Uneven angular spacing does not prevent the tool from
reporting a model's nominal limits and the independently observed winding count.

For unknown models the elevation histogram can suggest uniformly spaced visible
rows; occluded beams remain unobservable. Supply `--rows` and/or `--sensor-model`
when metadata or a manual sensor specification is available. Lack of a credible
row count withholds range-image flags instead of inventing a physical beam count.

## Using the flag candidates

The printed lists contain only sensor overrides. Add the intended method's
paper/profile switch separately, and retain its registration parameters:

- SuMa: vertical coverage and range-image rows.
- InTEn-LOAM: vertical coverage and rows. Ground-start thresholds are not inferred
  from a ground-height observation.
- ELO: vertical coverage, rows and ground height when a horizontal surface
  supports it.
- DiLO: top elevation, image height and a beam-gap candidate for its sin-elevation
  image. Existing minimum image dimensions/fill limits are retained unless the
  sensor geometry needs a larger value.

The near-return fraction is measured about the chosen sensor origin. It does not
establish that those points cause tracking failures, so the tool does not append
`--input-min-range-m`. That non-paper processing remains opt-in; the measured
NCLT experiments are in [nclt_paper_paths.md](nclt_paper_paths.md).

## Validation and limits

The committed evidence in
[`experiments/results/sensor_setup_assistant/`](../experiments/results/sensor_setup_assistant/README.md)
records commands, input hashes, source commits, reports and check exit codes.
The KITTI check uses a few scans from sequence 00, not a new odometry run.
The synthetic tests cover uneven beams, translated/inverted uniform beams,
azimuth-boundary jitter, general supplied extrinsics, missing ground, unknown
frames, intensity preservation and file-format failures.

KITTI 00 frames 000000/000001/000002 produce 64 observed rows via
ordered azimuth turns. The frame is explicitly `assumed_identity`. Observed
elevation percentiles are -24.14 to
+3.07 deg; the HDL-64E candidate supplies
nominal limits -24.9 to +2 deg. Intensity spans 0 to
0.99; 6.35 % of usable
points are within 5 m of the assumed sensor origin.

**NCLT real-data validation is pending.** Its official archive host is blocked
by the current environment policy. The host addition has been approved but is
not yet applied. The translated/inverted HDL-32E synthetic tests pass; they are
not a substitute for running the tool on NCLT scans. The roadmap item remains
open until that check is recorded.

Both supported sensor families are development/validation examples for this
assistant. This is not held-out registration accuracy evidence, and the
historical odometry results and estimator defaults remain unchanged.
