"""Interpret only the already committed complete conditional factorial."""
from pathlib import Path
import json,subprocess
E=Path(__file__).resolve().parent;W=E.parents[2]
def main():
    post=json.loads((E/'postprocessing_protocol.json').read_text())
    evidence=post['numerical_commit']
    assert subprocess.check_output(['git','show',evidence+':'+str((E/'analysis/report.json').relative_to(W))],cwd=W)==(E/'analysis/report.json').read_bytes()
    report=json.loads((E/'analysis/report.json').read_text());cases=report['cases'];lookup={c['frame']:c for c in cases}
    def metric(frame,arm,key):return lookup[frame]['arms'][arm][key]
    f=lambda x:f'{x:.6f}'
    titles=['original joint','preceding combined joint','fixed R original','fixed R seed only','fixed R mask only','fixed R seed + mask']
    text='''# Rotation-conditioned translation feasibility

The joint consensus correction could recover translation while increasing
rotation error. This prospective diagnostic asks whether keeping the original
unmasked **estimated** rotation and actually re-solving translation can retain
that recovery. It re-associates every iteration; it does not splice a translation
from the old joint solution. No GT rotation, hand region, parameter sweep,
per-endpoint winner or production change is used.

'''
    text+=f'Complete numerical snapshot `{evidence}` precedes this interpretation.\n'
    text+=f"All{report['native_calls']} actual native calls at every inherited KITTI01 endpoint\nare retained, including every adverse control. Source/rule/protocol were committed\nbefore measurement. This is fixed-original-history sensitivity, not a recursive\ntrajectory, official RTE, paper ratio or evidence of full01 recovery.\n\n"
    text+='''## Fixed intervention and controls

The unchanged preceding consensus admits multiple shifts for ambiguous clusters,
uses equal cluster votes, and requires a strict majority plus an interior grid
optimum. It changes neither profile costs nor thresholds. Corrected translation
is also the native adaptive-prior center. Source facade masking can change data
and adaptive-prior information; other classes still re-associate.

Original joint and preceding combined joint arms reproduce their committed
output, every first class system and the entire solver trace exactly. Four
translation-only arms form a complete seed-correction × facade-mask factorial.
They all use the original unmasked joint estimate's rotation. Only the conditional
damped Htt translation block is solved, with zero angular increment and additive
translation update. This is not a marginalized Schur translation solve or the
translation part of the joint update. Native first-iteration Schur-based prior
information remains unchanged in definition, and is frozen through the solve.
Pair gates, iteration/convergence limits, Huber scale and associations remain native.

The predeclared candidate uses the combined translation-only arm when consensus
is usable, otherwise the original joint result exactly. All raw factorial arms
are still run at fallback endpoints. No error metric influences this choice.
The inherited progression condition and a stricter all-endpoint translation
nonregression requirement were fixed before output; neither was relaxed.

'''
    text+=f"**Necessary conditional gate: {'PASS' if report['candidate_gate_passed'] else 'FAIL'}.** "
    counts=report['metric_counts_vs_baseline']['candidate']
    text+=f"Candidate length and vector errors each improve at{counts['vector_error']['lower']} endpoints,\ntie at{counts['vector_error']['equal']}, and worsen at{counts['vector_error']['higher']}. "
    text+='Rotation is bit-for-bit original at every translation-only iteration and output.\n'
    text+='This passes a known-development feasibility condition, not a generalization\nor production-adoption condition. Original rotation errors remain.\n\n'
    text+='## Every endpoint and arm\n\nLength ratio and vector error are normalized by GT translation length;\nrotation is offline geodesic error in mrad. No value below is official KITTI RTE.\n\n'
    text+='| Endpoint | Arm | Length/GT | Vector error/GT | Rotation mrad | Candidate uses this arm |\n|---:|---|---:|---:|---:|---|\n'
    for c in cases:
        for arm in c['arms']:
            text+=f"| {c['frame']} | {titles[arm['mode']]} | {f(arm['length_ratio'])} | {f(arm['vector_error'])} | {f(arm['rotation_error_mrad'])} | {'yes' if c['candidate_mode']==arm['mode'] else ''} |\n"
    text+='\n![Every actual arm and declared candidate](figures/all_endpoints.png)\n\n'
    text+='## Result and retained negatives\n\n'
    text+=f"At204, candidate length/GT changes{f(metric(204,0,'length_ratio'))}→{f(metric(204,5,'length_ratio'))},\nvector error{f(metric(204,0,'vector_error'))}→{f(metric(204,5,'vector_error'))}, while rotation stays\n{f(metric(204,0,'rotation_error_mrad'))}mrad instead of the preceding combined joint\n{f(metric(204,1,'rotation_error_mrad'))}mrad. Thus the local translation recovery does not\nrequire the preceding joint solution's adverse rotation change.\n\n"
    text+=f"At205, seed-only fixed-rotation length/GT is{f(metric(205,3,'length_ratio'))},\nmask-only{f(metric(205,4,'length_ratio'))}, combined{f(metric(205,5,'length_ratio'))}, from\n{f(metric(205,0,'length_ratio'))}. A changed seed/prior center remains important. But holding\noriginal rotation forfeits the preceding combined improvement from\n{f(metric(205,0,'rotation_error_mrad'))} to{f(metric(205,1,'rotation_error_mrad'))}mrad, and\nvector error is slightly worse than that preceding combined solution\n({f(metric(205,1,'vector_error'))}→{f(metric(205,5,'vector_error'))}). This mechanism does not\ndominate the preceding joint arm on every metric or endpoint.\n\n"
    text+=f"At247 the raw translation-only combined arm worsens length/GT\n{f(metric(247,0,'length_ratio'))}→{f(metric(247,5,'length_ratio'))} and vector error\n{f(metric(247,0,'vector_error'))}→{f(metric(247,5,'vector_error'))}. The already fixed majority\nfallback returns the original, avoiding this extra harm but leaving the original\nfailure. At433 the fixed boundary fallback also returns the original, forfeiting\nthe raw slight length improvement{f(metric(433,0,'length_ratio'))}→{f(metric(433,5,'length_ratio'))}.\n"
    raw=report['metric_counts_vs_baseline']
    text+=f"The uncorrected translation-only control worsens length and vector errors at\n{raw['2']['vector_error']['higher']} endpoints; corrected-seed-only worsens them at{raw['3']['vector_error']['higher']}.\nUnconditionally fixing rotation and re-solving is therefore not itself safe.\nEvery control, adverse result and fallback is retained.\n\n"
    text+='''## Provenance, limits and next decision

All observations here are development: original profile calibration07,
diagnosis00, inherited DiLO01 translation prior0.03, and every previous KITTI
observation. Consensus grid, trimming, admission scale and fallback were inherited
without retuning. Cluster votes are not physical-object votes or static truth;
fragmentation, moving majorities, aperture, repeated geometry and search direction
can bias them. Keeping estimated rotation also keeps its biases. GT is read only
after native execution for the offline errors above.

The factorial establishes a local computational alternative to the observed
rotation/support tradeoff. It neither repairs the original247 failure nor explains
sequence10 drift. No production switch or accuracy profile is added here. Next,
freeze the complete recursive estimated-rotation/consensus mechanism and evaluate
the same setting across every full00-10 sequence before any accuracy/paper-ratio
claim. That experiment must include original-history feedback and all adverse
sequences, and retain the original247 failure as an unresolved condition.

## Reproduction and checks

Run from the isolated worktree, with the pinned host Python and existing native
container. The producer requires a clean, frozen source tree; use fresh stage,
input, object and binary paths rather than overwriting historical runs.

```bash
/workspace/localization-env/host-venv/bin/python experiments/results/inten_fixed_rotation/execute.py
/workspace/localization-env/host-venv/bin/python experiments/results/inten_fixed_rotation/complete.py
```

The complete actual commands, cwd, source commit, stdin hash, environments,
compiler/linker flags, binary/static-library hashes and actual exits are in
`controls/`, `collection_execution/`, `analysis_execution/` and `completion/`.
Cached-feature/profile inputs are losslessly restored from committed archives,
hash-checked and pinned; no raw scans were acquired or deleted.

Every actual first accepted query independently reconstructs class H/g/count.
The full first prior H/g are reconstructed; every finite joint or conditional
translation update is backward-checked, with additive translation and unchanged
rotation verified at every projected step. Masked first facade queries are ordered
subsets and nonfacade systems match within each seed. The portable regression
recomputes the data-derived decisions, equations, errors, fallback and fixed gate.
Old production/defaults, all historical results and publications remain unchanged.
'''
    maxima=report['maxima']
    text+=f"\nMaximum first-data H relative discrepancy{maxima['first_H_relative']:.3e}, g\n{maxima['first_g_relative']:.3e}; full first-prior H{maxima['first_prior_H_relative']:.3e},\ng{maxima['first_prior_g_relative']:.3e}; damped-step backward error{maxima['step_backward']:.3e}.\nActual steps checked: joint{report['steps']['joint']}, translation-only{report['steps']['translation_only']}.\n"
    (E/'README.md').write_text(text)
    method=W/'papers/inten_loam/README.md'
    with method.open('a') as out:
        out.write('\n\n## Fixed estimated rotation: conditional translation factorial\n\n')
        out.write(f"Numerical snapshot `{evidence[:8]}` precedes interpretation. The\n[complete study](../../experiments/results/inten_fixed_rotation/README.md) retains all\n{report['native_calls']} terminal calls and first-query equations at every inherited01 endpoint.\nOutside-paper diagnostic only: no production/default/profile change or full-sequence\naccuracy claim. GT is used only for offline errors; original estimated rotation is\nheld fixed while translation actually re-associates and solves. Seed also changes\nthe adaptive-prior center. Original07/00/DiLO01 provenance is inherited, and every\nprevious KITTI observation remains development.\n\n")
        out.write('| Endpoint | Original length/GT | Previous joint length/GT | Candidate length/GT | Original vector/GT | Candidate vector/GT | Original / previous / candidate rotation mrad | Candidate arm |\n|---:|---:|---:|---:|---:|---:|---|---|\n')
        for c in cases:
            a,b,d=c['arms'][0],c['arms'][1],c['candidate']
            out.write(f"| {c['frame']} | {f(a['length_ratio'])} | {f(b['length_ratio'])} | {f(d['length_ratio'])} | {f(a['vector_error'])} | {f(d['vector_error'])} | {f(a['rotation_error_mrad'])} / {f(b['rotation_error_mrad'])} / {f(d['rotation_error_mrad'])} | {'combined fixed rotation' if c['candidate_mode']==5 else 'original fallback'} |\n")
        out.write('\nThe fixed necessary local gate passes: translation improves at6 endpoints, ties\nat2, with original rotation bit-for-bit. At204 length/GT0.607064→1.002040\nwithout the prior joint rotation worsening1.716533→2.958909mrad. At205\nseed-only0.927375, mask-only0.387723 and combined0.954865 retain seed/prior-center\nrescue, but forfeit the previous joint rotation gain2.419936→1.798922mrad\nand slightly worsen its vector error0.045419→0.045511. This does not dominate\nthe previous joint solution.\n\n')
        out.write('Negative controls remain material: uncorrected translation-only worsens length\nand vector errors at4 endpoints; seed-only at2. At247 the raw combined arm\nworsens length/GT0.522727→0.420339 and vector0.477580→0.579843. The inherited\nmajority fallback avoids this extra harm but leaves the original failure. At433\nboundary fallback forfeits the raw slight length improvement0.926498→0.926734.\nAll six arms and errors are in the study table. Cluster consensus is not static\nobject truth; fixed estimated rotation retains its original bias. Full01 recovery\nand10 drift remain open. Next is a separately frozen recursive all00-10 experiment\nwith one setting; no RTE or paper-ratio claim follows from these local controls.\n')
    roadmap=W/'docs/roadmap.md';old=roadmap.read_text();anchor='\n13. **CT-ICP'
    assert old.count(anchor)==1
    paragraph=f'''
    The [fixed estimated-rotation factorial](../experiments/results/inten_fixed_rotation/README.md)
    retains all48 calls, including exact original/preceding joint controls and a
    complete translation-only seed×mask factorial. At204 length/GT improves
    0.607064→1.002040 with original1.716533mrad rotation;205 reaches0.954865.
    The fixed local gate passes:6 translation improvements,2 fallback ties,
    rotation bit-for-bit original. Negatives remain: raw247 worsens0.522727→0.420339,
    and holding original205 rotation forfeits the prior joint1.798922mrad gain.
    No full trajectory, production option or accuracy claim. Next freeze recursive
    estimated-rotation/consensus behavior and run one setting across all full00-10;
    retain unresolved247 and10 drift. Numerical `{evidence[:8]}` precedes prose;
    every prior KITTI observation remains development.
'''
    roadmap.write_text(old.replace(anchor,'\n'+paragraph+anchor))
    print('Committed numerical evidence interpreted with all adverse controls retained')

if __name__=='__main__':main()
