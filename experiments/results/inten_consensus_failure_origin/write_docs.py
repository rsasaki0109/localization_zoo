"""Interpret only the complete committed stage report, without changing scores."""
from pathlib import Path
import json
import subprocess

E = Path(__file__).resolve().parent; W = E.parents[2]


def committed(path):
    if subprocess.check_output(['git', 'show', 'HEAD:'+str(path.relative_to(W))], cwd=W) != path.read_bytes():
        raise ValueError('Commit numerical evidence first: '+str(path))


def main():
    for name in ('summary.json', 'entries.json', 'introduced_events.json'): committed(E/'report'/name)
    summary = json.loads((E/'report/summary.json').read_text())
    entries = json.loads((E/'report/entries.json').read_text())
    if summary['exit_code'] != 0 or summary['sequence_count'] != 11: raise ValueError('Incomplete study')
    numerical = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=W, text=True).strip()
    counts = []
    for seq, modes in summary['sequences'].items():
        m = modes['candidate']; a = m['transitions']['length']; b = m['transitions']['vector']
        seed = a.get('joint_good_seed_bad_accepted_bad', 0); refinement = a.get('joint_good_seed_good_accepted_bad', 0)
        damage_vector = b.get('joint_good_seed_bad_accepted_bad', 0)+b.get('joint_good_seed_good_accepted_bad', 0)
        counts.append(f"| {seq} | {m['moving_applied']} | {seed} | {refinement} | {damage_vector} | {a.get('joint_bad_to_accepted_good', 0)} | {len(m['new_final_bad_length_frames'])} |")
    table = ('| Sequence | Moving applied | Length crossing already at corrected seed | Length crossing after good corrected seed | Vector crossings | Joint bad to accepted good (length) | New final bad length vs reference |\n'
             '|---|---:|---:|---:|---:|---:|---:|\n'+'\n'.join(counts)+'\n')
    critical = [x for x in entries if x['mode'] == 'candidate' and x['label'] == 'length' and x['entry']['crossing_length'].startswith('joint_good_seed_')]
    def f(value): return '-' if value is None else f'{value:.6f}'
    details = []
    for item in critical:
        r = item['entry']
        details.append(f"| {r['sequence']} | {item['first_frame']}–{item['last_frame']} | {f(r['predicted_length_ratio'])} | {f(r['joint_length_ratio'])} | {f(r['corrected_seed_length_ratio'])} | {f(r['accepted_length_ratio'])} | {f(r['final_length_ratio'])} | {f(r['chosen_shift_m'])} | {r['chosen_votes']} / {r['zero_votes']} / {r['valid_clusters']} | {r['refinement_stop']} |")
    detail_table = ('| Sequence | Final bad-length interval | Prediction / GT | Joint / GT | Corrected seed / GT | Accepted / GT | Final / GT | Chosen shift [m] | Chosen / zero / valid clusters | Refinement stop |\n'
                    '|---|---|---:|---:|---:|---:|---:|---:|---|---|\n'+'\n'.join(details)+'\n')
    note = '''These are **observed stage crossings on the candidate's own realized history**.
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
'''
    text = '# Where recursive consensus introduces short steps\n\n'+note
    text += f'\nComplete numerical snapshot `{numerical}` precedes this interpretation.\n\n'
    text += '## Every full sequence\n\n'+table
    text += '\nThe first two crossing columns partition moving, applied frames where the original joint length was good and the accepted length became bad. Rescues and all remaining joint-bad cases are retained in the complete report. Vector crossings are classified separately. New final bad frames are the exact previously committed reference/candidate set, not a new metric.\n'
    text += '\n## Every bad-length interval entered with a good joint estimate\n\n'+detail_table
    text += '\nAll baseline/candidate length and vector interval entries, their previous frame, every step, all damage events and all rescues remain in `report/`. The chosen and zero-shift votes use the actual admission bits, verified independently against every archived decision. A majority is a property of these cluster votes; it does not establish static-scene truth. Converged stops and finite pair counts do not establish correct motion.\n'
    text += '''
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
'''
    (E/'README.md').write_text(text)
    method = W/'papers/inten_loam/README.md'; old = method.read_text()
    marker = '<!-- INTEN_CONSENSUS_FAILURE_ORIGIN -->'
    if marker in old: raise ValueError('Do not overwrite an existing interpretation')
    method.write_text(old+'\n\n'+marker+'\n## Where recursive consensus introduces short steps\n\n'+note+'\n'+table+'\n'+detail_table+'\nCommands, every adverse/neutral/rescue event, complete stage traces and limits: [full diagnosis](../../experiments/results/inten_consensus_failure_origin/README.md). Numerical `'+numerical[:8]+'` precedes this interpretation. The option remains default off; this diagnosis does not remedy full-sequence accuracy.\n')
    roadmap = W/'docs/roadmap.md'; old = roadmap.read_text(); needle='13. **CT-ICP'
    if old.count(needle) != 1: raise ValueError('Unexpected roadmap insertion')
    paragraph=('    The [archived consensus stage diagnosis](../experiments/results/inten_consensus_failure_origin/README.md)\n'
               '    checks every frame of both fixed full00–10 arms. It separates the actual\n'
               '    corrected seed from the original joint output and accepted refinement,\n'
               '    retaining all same-frame damage, rescues and remaining bad cases.\n'
               '    Some new collapse entries already follow a good joint estimate, so\n'
               '    changed mapping history alone is insufficient to explain them.\n'
               '    No safeguard is adopted or new accuracy score claimed; native\n'
               '    same-history rescue and newly adverse controls must precede another\n'
               '    recursive trial. GT remains offline diagnostic only.\n'
               f'    Numerical `{numerical[:8]}` precedes interpretation.\n\n')
    roadmap.write_text(old.replace(needle, paragraph+needle))


if __name__ == '__main__': main()
