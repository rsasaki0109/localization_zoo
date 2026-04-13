# Implementation Notes

This document summarizes the implementation quality and paper fidelity of each method family integrated into `pcd_dogfooding`.

> **27 methods** integrated | **38/38 tests passed** | Generated: 2026-04-13

| Method | Source | LOC | Tests | Fidelity | Key Deviations |
|--------|--------|-----|-------|----------|----------------|
| aloam | scan_registration.cpp, laser_odometry.cpp, laser_mapping.cpp | 1358 | 1 test(s) | High | Uses Ceres compat header |
| balm2 | balm2.cpp | 754 | 1 test(s) | High | Uses Ceres compat header |
| clins | clins.cpp | 203 | 1 test(s) | Medium | None noted |
| ct_icp | ct_icp_registration.cpp | 682 | 1 test(s) | High | Uses Ceres compat header |
| ct_lio | ct_lio_registration.cpp | 1204 | 1 test(s) | High | Uses Ceres compat header |
| dlio | dlio.cpp | 379 | 1 test(s) | Medium | None noted |
| dlo | dlo.cpp | 265 | 1 test(s) | Medium | None noted |
| fast_lio2 | fast_lio2.cpp | 641 | 1 test(s) | High | Uses Ceres compat header |
| fast_lio_slam | fast_lio_slam.cpp | 473 | 1 test(s) | Medium | Uses Ceres compat header |
| floam | floam.cpp | 214 | 1 test(s) | Medium | None noted |
| gicp | gicp_registration.cpp | 292 | 1 test(s) | Medium | None noted |
| hdl_graph_slam | hdl_graph_slam.cpp | 684 | 1 test(s) | High | Uses Ceres compat header |
| isc_loam | isc_loam.cpp | 735 | 1 test(s) | High | Uses Ceres compat header |
| kiss_icp | kiss_icp.cpp | 388 | 1 test(s) | Medium | None noted |
| lego_loam | lego_loam.cpp | 421 | 1 test(s) | Medium | None noted |
| lins | lins.cpp | 389 | 1 test(s) | Medium | None noted |
| lio_sam | lio_sam.cpp | 558 | 1 test(s) | High | Uses Ceres compat header |
| litamin2 | litamin2_registration.cpp | 522 | 1 test(s) | High | None noted |
| loam_livox | loam_livox.cpp | 324 | 1 test(s) | Medium | None noted |
| mulls | mulls.cpp | 513 | 1 test(s) | High | Uses Ceres compat header |
| ndt | ndt_registration.cpp | 424 | 1 test(s) | Medium | None noted |
| point_lio | point_lio.cpp | 392 | 1 test(s) | Medium | None noted |
| small_gicp | small_gicp_registration.cpp | 359 | 1 test(s) | Medium | None noted |
| suma | suma.cpp | 536 | 1 test(s) | High | Uses Ceres compat header |
| vgicp_slam | vgicp_slam.cpp | 673 | 1 test(s) | High | Uses Ceres compat header |
| voxel_gicp | voxel_gicp_registration.cpp | 358 | 1 test(s) | Medium | None noted |
| xicp | xicp_registration.cpp | 437 | 1 test(s) | Medium | None noted |
