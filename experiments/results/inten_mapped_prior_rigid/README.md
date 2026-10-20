# Rigid recursive mapped-motion seed: fixed full KITTI experiment

Status: fixed revision and protocol; full results pending.

The initial unprojected trial failed on00 and remains in
[the original study](../inten_mapped_prior/README.md). This revision restores
the seed rotation using a normalized quaternion, preserving translation and
all existing gates, fallback, deskew and mapping logic. No fitted threshold.
The extension is outside the paper and default off.

Inherited settings were selected on07, with00 informing diagnosis and now
this numerical correction. Known01/02/10 failures motivated the candidate.
No independent held-out claim. All sequences restart under one fixed setting.
