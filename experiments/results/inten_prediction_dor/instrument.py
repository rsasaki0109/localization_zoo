"""Generate only the declared passive instrumentation; never edit production files."""
from pathlib import Path
import hashlib,json

def transformed(source):
 s='#include "diagnostic_hooks.h"\n'+source
 needle='  std::vector<int> total(dynamic.size(), 0), flagged(dynamic.size(), 0);'
 assert s.count(needle)==1
 s=s.replace(needle,'''  const int diagnostic_map_pixels = std::count_if(DM.begin(), DM.end(), [](float r) { return r > 0; });
'''+needle+'''
  std::vector<int> covered(dynamic.size(), 0);''')
 # Map-only coverage must be sampled before the previous scan is rendered.
 declaration='  const int diagnostic_map_pixels = std::count_if(DM.begin(), DM.end(), [](float r) { return r > 0; });\n'
 s=s.replace(declaration,'')
 needle='  for (const auto& f : prev_->all.pts) render('
 assert s.count(needle)==1
 s=s.replace(needle,declaration+needle)
 needle='    const double dp = f.p.norm();'
 assert s.count(needle)==1
 s=s.replace(needle,'    ++covered[static_cast<size_t>(f.cluster)];\n'+needle)
 needle='  return dynamic;'
 assert s.count(needle)==1
 s=s.replace(needle,'''  diagnostic::mask(frame_, params_, curr, T_w, DM, total, covered, flagged, dynamic, diagnostic_map_pixels);
'''+needle)
 needle='  Eigen::Matrix4d rel = params_.s2s_mapped_prior && params_.use_mapping ? last_mapped_rel_ : last_rel_;'
 assert s.count(needle)==1
 s=s.replace(needle,needle+'''
  const Eigen::Matrix4d diagnostic_prediction = drift_ * (odom_ * rel);
  if (!forced) {
    diagnostic::stage = 0;
    dynamicMask(frame, diagnostic_prediction);
    diagnostic::stage = 2;
  }''')
 needle='      dynamic = dynamicMask(frame, pose_);'
 assert s.count(needle)==1
 s=s.replace(needle,'''      diagnostic::stage = 1;
      dynamicMask(frame, diagnostic_prediction);
      diagnostic::stage = 2;
'''+needle)
 start=s.index('int InTenLoamPaper::solveS2S(');end=s.index('int InTenLoamPaper::solveS2M(',start)
 a=s[start:end];needle='      auto plane = [&](const PaperFeature&, '
 assert a.count(needle)==1
 a=a.replace(needle,'      auto plane = [&](const PaperFeature& source, ',1)
 needle='        return true;\n      };\n      accumulateFeatures(src'
 assert a.count(needle)==1
 a=a.replace(needle,'''        if (cls == 1 && it == 0)
          diagnostic::facade(frame_, source, tree.pts, *indices, *distances, same, adj, x, nrm, *residual);
'''+needle)
 return s[:start]+a+s[end:]

if __name__=='__main__':
 E=Path(__file__).resolve().parent;W=E.parents[2];source=(W/'papers/inten_loam/src/inten_loam_paper.cpp').read_text();result=transformed(source)
 (E/'diagnostic_native.cpp').write_text(result)
 p=dict(schema_version=1,parent_commit='b603cd20af5a9d9786886f9d3d1a68372802708e',native_reference_commit='9a01528b05128794df962b52ace59e727e834417',sequence='01',scope='Full continuous01 passive DOR detection, not an accuracy candidate. No GT or hand ROI enters native producer. Retain every native frame, every first-iteration accepted facade query, and every non-ground cluster decision under all declared renders.',selected_endpoints=[190,200,202,203,204,205,247,433],detailed_frames=sorted({i+j for i in [190,200,202,203,204,205,247,433] for j in [-1,0]}),profile='Unchanged PR159 candidate / PR161 reference: mapped seed, translation prior0.03, ground/facade PCA, acceleration ON; S2M spread0 and row completion OFF; max range120m, elevation0.17deg, S2M plane check0.05m. Selection07,00 diagnosis, inherited DiLO01 prior tuning, all currentKITTI development.',stages=['prediction/prior-local: available pre-S2S support using actual previous local map','prediction/current-local: after current S2M local-map rebuild, same prediction pose; retrospective diagnostic, not deployable before S2S','final/current-local: original unchanged native DOR/map insertion'],controls='Exact every full reference frame field except validated scan-path convention against archived facade-PCA candidate. All original DOR masks/insertion/history unchanged; extra calls are const. Require exact original source->instrumented transformation and first facade counts against native trace. Prediction uses drift_*(odom_*initial_relative), before S2S. Distinguish pose changes from local-map rebuild; never attribute first-vs-final solely to pose.',analysis='For every moving step retain GT-relative motion errors offline, initial facade gradients through full damped native H, current prediction masks, previous final masks for all original neighbour supports, coverage/range discrepancies and native cluster decisions. Aggregate all full01 moving/bad/not-bad steps under inherited0.1m/20percent labels, not just favorable204. Detailed original range-image samples for every non-ground point at all inherited endpoints and previous frames. No GT in an automatic decision and no semantic-object truth inferred from a cluster.',thresholds='Existing native dor_relative_range0.1 and dor_object_ratio0.5 unchanged. No sweep, fitted threshold, hand-box rule, or default change. A new mechanism must be separately frozen; any accuracy setting must run all full00-10 before ratios.',paper_scope='Repository documents Sec3.3 DOR as post-S2M map insertion only and warns past pre-extraction filtering removed valid registration input. This passive study does not claim a paper-compliance defect or authorize changing defaults.',data='Official fetch full01 only; verify all existing committed scan hashes. Preserve lossless archives, commands, sources/binaries and failures; byte-roundtrip, commit and normal push before owned raw deletion.',native_source_sha256=hashlib.sha256(source.encode()).hexdigest(),diagnostic_source_sha256=hashlib.sha256(result.encode()).hexdigest())
 (E/'protocol.json').write_text(json.dumps(p,indent=2)+'\n')
