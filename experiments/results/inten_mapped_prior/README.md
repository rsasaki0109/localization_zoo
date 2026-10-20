# Recursive mapped-motion S2S seed experiment

Status: implementation and fixed protocol; full results pending.

This opt-in repository extension is not in the paper. It seeds normal S2S
from the previous final mapped relative pose; the accepted S2S cache, deskew
and low-count fallback are unchanged. Mapping-off and forced diagnostics
keep the original behavior. `s2s_mapped_prior` defaults to false.

The inherited profile was selected on KITTI07, with00 informing diagnosis.
Known01/02/10 failures motivated this candidate, so there is no independent
held-out claim. See [protocol.json](protocol.json) and the committed manifests.
All sequences use the same paired settings without retuning.
