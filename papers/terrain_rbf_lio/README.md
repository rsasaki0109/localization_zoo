# Terrain-Aware RBF-LIO (from-paper reimplementation)

**Paper:** *Terrain-Aware LiDAR-Inertial Odometry for Legged-Wheel Robots Based
on Radial Basis Function Approximation*, Yizhe Liu, Han Zhang, 2025
([arXiv:2509.26222](https://arxiv.org/abs/2509.26222)). Author code not released.

## Core idea

The paper models the local **terrain as a height field** `z = f(x, y)`
approximated by a Gaussian radial-basis function (RBF) over a grid of centres,
with the weights updated recursively (RLS). The terrain manifold is then used as
a **soft constraint** in the scan-to-map optimization:

```
f(x; w) = Σ_i w_i · φ(‖x − c_i‖),   φ(r) = exp(−r² / 2σ²)
J_total = J_scan(point-to-plane) + λ_M · J_terrain
```

In the original (legged-wheel robot) the terrain residual is a **wheel-contact
constraint** — the wheel centre `ξ_L` sits exactly a wheel radius `r` above the
terrain:

```
r_M = ξ_z − r − f(ξ_x, ξ_y) ≈ 0
```

## Scope / approximations for KITTI (pure-LiDAR)

The wheel-contact constraint is robot-specific (it needs wheel position, wheel
radius and 500 Hz joint encoders). For the KITTI car platform we **reinterpret
it as a vehicle-height-above-terrain prior**: the sensor/vehicle base point sits
a slowly-varying height `h_sensor` above the local terrain surface, so

```
r_M = (t_z − h_sensor) − f(t_x, t_y) ≈ 0
∂r_M/∂t = [−∂f/∂x, −∂f/∂y, 1]
```

This keeps the paper's contribution that matters off-road or on slopes — a
smooth RBF terrain manifold maintained by RLS — and turns it into a **z-drift
regularizer** for the vehicle pose. `h_sensor` is estimated online as an EMA of
`t_z − f(t_x, t_y)`.

Other approximations:

- **Constant-velocity** prediction replaces IMU preintegration (KITTI has no
  IMU), as in KISS-ICP / CT-ICP.
- The RBF is implemented as a **normalized Gaussian kernel regression**
  (Nadaraya-Watson) over per-cell ground-height estimates updated by RLS, rather
  than solving the ridge normal equations every frame — same manifold, cheaper
  and more robust. Per-cell heights track the **ground envelope** (low points),
  not roofs/walls, via a ground band.
- `J_scan` is point-to-plane against a voxel-hash map (LOAM-style features and
  the GPU path are out of scope).

Honest note: the terrain prior is genuinely useful on undulating terrain. On
flat urban KITTI its effect is small, so `terrain_weight` defaults to `0` and is
opt-in; with weight `0` the pipeline is a plain point-to-plane odometry.

## Parameters

| Param | Meaning | Default |
|---|---|---|
| `terrain_cell_size` | RBF centre grid resolution | 2.0 m (paper 0.07 m, legged-wheel) |
| `terrain_sigma` | Gaussian bandwidth σ | 3.0 m |
| `terrain_ground_band` | ground-candidate band for RLS | 1.0 m |
| `terrain_weight` | soft-constraint weight λ_M | 0.0 (opt-in) |
| `terrain_warmup` | frames before constraint activates | 10 |
| `sensor_height_alpha` | EMA rate for h_sensor | 0.05 |

## Tests

`test_terrain_rbf_lio` covers: known-translation recovery on a symmetric cube;
RBF interpolation of a sloped ground (value + gradient); ground-tracking that is
not pulled up by roof points.
