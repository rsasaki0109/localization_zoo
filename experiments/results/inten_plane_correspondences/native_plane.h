// Verbatim S2S plane matcher from the pinned native source.
      auto plane = [&](const PaperFeature&, const Eigen::Vector3d& x, Eigen::Vector3d* a,
                       double* residual, std::vector<int>* indices, std::vector<float>* distances) {
        const int n = tree.knn(x, 10, indices, distances);
        const auto& ids = *indices;
        const auto& ds = *distances;
        if (n < 3 || ds[0] > gate) return false;
        const PaperFeature& qj = tree.pts[static_cast<size_t>(ids[0])];
        int same = -1, adj = -1;
        for (int k = 1; k < n && ds[static_cast<size_t>(k)] <= gate; ++k) {
          const int dr = std::abs(tree.pts[static_cast<size_t>(ids[static_cast<size_t>(k)])].row - qj.row);
          if (dr == 0 && same < 0) same = ids[static_cast<size_t>(k)];
          if (dr >= 1 && dr <= 2 && adj < 0) adj = ids[static_cast<size_t>(k)];
        }
        if (same < 0 || adj < 0) return false;
        Eigen::Vector3d nrm = (qj.p - tree.pts[static_cast<size_t>(adj)].p)
                    .cross(qj.p - tree.pts[static_cast<size_t>(same)].p);
        if (nrm.norm() < 1e-6) return false;
        nrm.normalize();
        *a = nrm;
        *residual = nrm.dot(x - qj.p);
        return true;
      };
