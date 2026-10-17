"""Compare pre-extraction and shared filters through real runner consumers.

This is a loader/odometry regression on the committed short MCD fixture, not
NCLT validation or a full-sequence accuracy claim.
"""
from support import *

verify_binaries()
folder = E / 'checks/fixture_parity'
cases = {
    'disabled': [],
    'explicit_zero': ['--input-min-range-m', '0'],
    'five_m': ['--input-min-range-m', '5'],
    'sensor_frame_five_m': ['--input-sensor-pose', '1', '2', '-0.957', '180', '0', '0',
                          '--input-min-range-m', '5'],
    'corrected_five_m': ['--input-sensor-pose', '1', '2', '-0.957', '180', '0', '0',
                         '--input-min-range-m', '5', '--input-vertical-angle-correction-deg', '0.205'],
}
checks = []
for name, flags in cases.items():
    results = {}
    for binary in ('baseline', 'candidate'):
        out = folder / name / binary
        summary = out / 'summary.json'
        command = docker(frozen=binary == 'baseline') + [str(WRAPPER),
            str(REPO / 'evaluation/fixtures/mcd_kth_smoke'),
            str(REPO / 'evaluation/fixtures/mcd_kth_smoke_gt.csv'), '3',
            '--methods', 'dilo,elo,inten_loam', '--no-gt-seed',
            '--dilo-paper-odometry', '--elo-dense-profile', '--inten-loam-paper-odometry',
            '--summary-json', str(summary), *flags]
        run(binary, command, out)
        results[binary] = json.loads(summary.read_text())
    for method in ('DiLO', 'ELO', 'InTEn-LOAM'):
        a, b = [next(x for x in results[k]['methods'] if x['name'] == method)
                for k in ('baseline', 'candidate')]
        metrics = ('status', 'frames', 'ate_m', 'rpe_trans_pct',
                   'kitti_rte_trans_pct', 'kitti_rte_rot_deg_per_100m')
        matching = {key: a.get(key) == b.get(key) for key in metrics}
        pose_name = method.replace('-', '_') + '.txt'
        poses = [native_output(folder / name / k / 'summary.json') / pose_name
                 for k in ('baseline', 'candidate')]
        poses_match = poses[0].read_bytes() == poses[1].read_bytes()
        checks.append(dict(case=name, method=method, arguments=flags,
                           metrics_match=matching, saved_poses_equal=poses_match,
                           baseline_metrics={key: a.get(key) for key in metrics},
                           candidate_metrics={key: b.get(key) for key in metrics}))
        assert a['status'] == b['status'] == 'ok' and a['frames'] == b['frames'] == 3
        assert all(matching.values()) and poses_match
write(folder / 'comparison.json', dict(source_commit=SOURCE, checks=checks, exit_code=0,
      scope='Three real runner consumers on the committed short MCD fixture; not NCLT.'))
