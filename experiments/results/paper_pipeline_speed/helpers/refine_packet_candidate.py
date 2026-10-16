from pathlib import Path
r=Path('/workspace/localization-env/paper-speed');assert (r/'3541f773_binary.json').exists()
p=Path('/workspace/localization_zoo/papers/mesh_loam/src/mesh_loam.cpp');s=p.read_text();a=s.index('  const Facet* found = nullptr;',s.index('FacetBin::nearest('));b=s.index('    for (size_t lane = 0;',a)
s=s[:a]+'''  // Scale the additional broad-filter slack for non-KITTI search-bin sizes.
  // The scalar tests below, with their original fixed slack, remain decisive.
  const float scale = std::max(1.0f, relative_f.cwiseAbs().maxCoeff() +
      lo.cwiseAbs().cwiseMax(hi.cwiseAbs()).maxCoeff() + max_rad);
  const float extra_slack = 1e-5f + 32.0f * std::numeric_limits<float>::epsilon() * scale;
  const Facet* found = nullptr;
  for (size_t first = 0; first < facets.size(); first += 4) {
    using Packet = Eigen::Array4f;
    const Eigen::Map<const Packet> nx(v + first), ny(v + stride + first),
        nz(v + 2 * stride + first);
    const Eigen::Array<bool, 4, 1> normal_match =
        (nx * normal_f.x() + ny * normal_f.y() + nz * normal_f.z()).abs() >=
        static_cast<float>(cos_gate) - 2e-5f;
    // Nearby facets usually share an orientation. Avoid loading centroids or
    // computing distance bounds when every lane fails the cheap normal gate.
    if (!normal_match.any()) continue;
    const Eigen::Map<const Packet> sx(v + 3 * stride + first),
        sy(v + 4 * stride + first), sz(v + 5 * stride + first);
    const Packet dx = relative_f.x() - sx * (1.0f / 3.0f);
    const Packet dy = relative_f.y() - sy * (1.0f / 3.0f);
    const Packet dz = relative_f.z() - sz * (1.0f / 3.0f);
    const Packet plane = nx * dx + ny * dy + nz * dz;
    // Recheck survivors with the original arithmetic and the latest bound;
    // packet rounding never decides an association or changes the tie order.
    const float broad = bound + extra_slack;
    const float broad_centroid = broad + max_rad;
    const Eigen::Array<bool, 4, 1> possible = normal_match &&
        (plane.square() <= broad * broad) &&
        (dx.square() + dy.square() + dz.square() <= broad_centroid * broad_centroid);
'''+s[b:];s=s.replace('#include <numeric>','#include <numeric>\n#include <limits>');p.write_text(s)
