#!/usr/bin/env python3
"""Inspect a few spinning-LiDAR scans and print sensor setup candidates.

Reads ASCII/binary PCD, KITTI float32 XYZI, or explicitly selected NCLT
velodyne_sync data. Suggestions never change a runner or estimator default.
Full extrinsics cannot be recovered from arbitrary scans: the automatic frame
search assumes the sensor is on the stored z axis and its vertical axis is
parallel to it. Confirm that assumption or supply --input-sensor-pose.
"""
from __future__ import annotations

import argparse
from dataclasses import dataclass
import hashlib
import json
import math
from pathlib import Path
import shlex
import subprocess
import sys

import numpy as np


@dataclass
class Scan:
    points: np.ndarray
    intensity: np.ndarray | None = None
    rings: np.ndarray | None = None
    organized_rows: int | None = None


def load_scan(path: Path, input_format: str = 'auto') -> Scan:
    """Preserve intensity/ring fields; .bin means KITTI unless explicitly NCLT."""
    mode = ('kitti' if path.suffix.lower() == '.bin' else 'pcd') if input_format == 'auto' else input_format
    raw = path.read_bytes()
    if mode == 'kitti':
        if not raw or len(raw) % 16:
            raise ValueError(f'{path}: KITTI XYZI needs nonempty 16-byte records')
        rows = np.frombuffer(raw, dtype='<f4').reshape(-1, 4).astype(float)
        return Scan(rows[:, :3], rows[:, 3])
    if mode == 'nclt':
        dtype = np.dtype([('xyz', '<u2', (3,)), ('intensity', 'u1'), ('ring', 'u1')])
        if not raw or len(raw) % dtype.itemsize:
            raise ValueError(f'{path}: NCLT needs nonempty 8-byte records')
        rows = np.frombuffer(raw, dtype=dtype)
        return Scan(rows['xyz'].astype(float) * .005 - 100, rows['intensity'].astype(float), rows['ring'].astype(int))
    if mode != 'pcd':
        raise ValueError(f'Unsupported input format {mode}')
    header = {}
    position = 0
    while position < len(raw):
        end = raw.find(b'\n', position)
        if end < 0:
            raise ValueError(f'{path}: missing PCD DATA header')
        line = raw[position:end].decode('ascii', 'strict').strip()
        position = end + 1
        if not line or line.startswith('#'):
            continue
        key, *values = line.split()
        header[key.upper()] = values
        if key.upper() == 'DATA':
            break
    try:
        fields = header['FIELDS']
        counts = list(map(int, header.get('COUNT', ['1'] * len(fields))))
        if len(counts) != len(fields) or any(c < 1 for c in counts):
            raise ValueError('invalid COUNT')
        for name in ['x', 'y', 'z']:
            if counts[fields.index(name)] != 1:
                raise ValueError('XYZ fields must be scalar')
        number = int(header.get('POINTS', [str(int(header['WIDTH'][0]) * int(header.get('HEIGHT', ['1'])[0]))])[0])
        if number < 1:
            raise ValueError('no points')
        body = raw[position:]
        if header['DATA'] == ['ascii']:
            array = np.loadtxt(body.decode().splitlines(), ndmin=2)
            if array.shape != (number, sum(counts)):
                raise ValueError('point count or field count mismatch')
            offsets = np.cumsum([0] + counts[:-1])
            values = {name: array[:, offset] for name, offset, count in zip(fields, offsets, counts) if count == 1}
        elif header['DATA'] == ['binary']:
            sizes = list(map(int, header['SIZE']))
            types = header['TYPE']
            if len(sizes) != len(fields) or len(types) != len(fields):
                raise ValueError('SIZE/TYPE mismatch')
            dtype = np.dtype([(name, {'F': 'f', 'U': 'u', 'I': 'i'}[kind] + str(size), (count,) if count > 1 else ())
                              for name, kind, size, count in zip(fields, types, sizes, counts)]).newbyteorder('<')
            if len(body) != number * dtype.itemsize:
                raise ValueError('binary payload length mismatch')
            array = np.frombuffer(body, dtype=dtype, count=number)
            values = {name: array[name].astype(float) for name, count in zip(fields, counts) if count == 1}
        else:
            raise ValueError('only ASCII and uncompressed binary PCD are supported')
        intensity = next((values[n] for n in ['intensity', 'reflectance'] if n in values), None)
        rings = next((values[n] for n in ['ring', 'laser_id', 'layer'] if n in values), None)
        height = int(header.get('HEIGHT', ['1'])[0])
        return Scan(np.column_stack([values[n] for n in ['x', 'y', 'z']]), intensity, rings, height if height > 1 else None)
    except (KeyError, IndexError, TypeError, ValueError) as error:
        raise ValueError(f'{path}: invalid PCD: {error}') from error


def load_points(path: Path) -> np.ndarray:
    """Backward-compatible XYZ-only reader."""
    return load_scan(path).points


def periodicity(elevation_deg: np.ndarray, spacing_deg: float) -> float:
    if not math.isfinite(spacing_deg) or spacing_deg <= 0:
        raise ValueError('beam spacing must be positive and finite')
    hist, _ = np.histogram(elevation_deg, bins=np.arange(-60., 60., .1))
    spectrum = np.abs(np.fft.rfft(hist - hist.mean()))
    freqs = np.fft.rfftfreq(len(hist), .1)
    k = int(np.argmin(np.abs(freqs - 1 / spacing_deg)))
    return float(spectrum[max(k - 2, 0):k + 3].max() / max(np.median(spectrum), 1e-9))


def ordered_rows(points: np.ndarray) -> int | None:
    """Count complete monotone azimuth turns in ring-major scans, not wrap events.

    Calibration jitter can cross +/-pi several times. Net unwrapped angle is
    stable; shuffled or acquisition-interleaved scans must not be called rings.
    """
    p = points[np.hypot(points[:, 0], points[:, 1]) > .5]
    if len(p) < 100:
        return None
    angle = np.unwrap(np.arctan2(p[:, 1], p[:, 0]))
    delta = np.diff(angle)
    direction = 1 if angle[-1] >= angle[0] else -1
    turns = abs(angle[-1] - angle[0]) / (2 * np.pi)
    count = int(round(turns))
    if 4 <= count <= 256 and abs(turns - count) < .15 and np.mean(direction * delta >= -.02) > .98:
        return count
    return None


def beam_peaks(elevation: np.ndarray) -> list[float]:
    hist, edges = np.histogram(elevation, bins=np.arange(-60., 60.001, .1))
    smooth = np.convolve(hist, [.25, .5, .25], mode='same')
    if not smooth.max():
        return []
    # Prominence over a surrounding half-degree window suppresses broad terrain.
    candidates = np.flatnonzero((smooth[1:-1] > smooth[:-2]) & (smooth[1:-1] >= smooth[2:])) + 1
    selected = []
    for k in sorted(candidates, key=lambda i: -smooth[i]):
        background = max(smooth[max(0, k - 4):k + 1].min(), smooth[k:min(len(smooth), k + 5)].min())
        if smooth[k] < max(8, .02 * smooth.max()) or smooth[k] - background < max(.25 * smooth[k], 3 * np.sqrt(smooth[k])):
            continue
        angle = float((edges[k] + edges[k + 1]) / 2)
        if all(abs(angle - previous) >= .25 for previous in selected):
            selected.append(angle)
    return sorted(selected)


def horizontal_surface(points: np.ndarray) -> dict | None:
    h = np.hypot(points[:, 0], points[:, 1])
    p = points[(h > 2) & (h < 15) & (np.abs(points[:, 2]) < 5)]
    if len(p) < 100:
        return None
    bins = np.floor(p[:, 2] / .1).astype(int)
    labels, counts = np.unique(bins, return_counts=True)
    center = (labels[np.argmax(counts)] + .5) * .1
    plane = p[np.abs(p[:, 2] - center) < .12]
    sectors = np.unique(np.floor((np.arctan2(plane[:, 1], plane[:, 0]) + np.pi) * 12 / (2 * np.pi))).size
    if len(plane) < .10 * len(p) or sectors < 6:
        return None
    # A broad wall's horizontal slice alone is insufficient: require a flat fit.
    design = np.column_stack([plane[:, 0], plane[:, 1], np.ones(len(plane))])
    fit = np.linalg.lstsq(design, plane[:, 2], rcond=None)[0]
    if np.linalg.norm(fit[:2]) > .12:
        return None
    return {'z_m': float(fit[2]), 'point_fraction': float(len(plane) / len(p)), 'azimuth_sectors': int(sectors)}


def transform_points(points: np.ndarray, pose: list[float]) -> np.ndarray:
    x, y, z, roll, pitch, yaw = pose
    r, p, y_angle = np.deg2rad([roll, pitch, yaw])
    rx = np.array([[1, 0, 0], [0, np.cos(r), -np.sin(r)], [0, np.sin(r), np.cos(r)]])
    ry = np.array([[np.cos(p), 0, np.sin(p)], [0, 1, 0], [-np.sin(p), 0, np.cos(p)]])
    rz = np.array([[np.cos(y_angle), -np.sin(y_angle), 0], [np.sin(y_angle), np.cos(y_angle), 0], [0, 0, 1]])
    return (points - np.array([x, y, z])) @ (rz @ ry @ rx)


def search_height(points: np.ndarray, spacing: float, max_offset: float) -> list[dict]:
    h = np.hypot(points[:, 0], points[:, 1])
    p = points[h > .5]; h = h[h > .5]
    stride = max(1, len(p) // 40000)
    p, h = p[::stride], h[::stride]
    def score(offset):
        return periodicity(np.rad2deg(np.arctan2(p[:, 2] - offset, h)), spacing)
    trials = [(score(float(z)), float(z)) for z in np.arange(-max_offset, max_offset + 1e-8, .05)]
    best = max(trials)[1]
    trials += [(score(float(z)), float(z)) for z in np.arange(max(-max_offset, best - .05), min(max_offset, best + .05) + 1e-8, .005)]
    return [{'score': value, 'sensor_z_m': offset} for value, offset in sorted(trials, reverse=True)[:6]]


def analyze(scans: list[Scan], *, beam_spacing_deg: float | None = None,
            input_sensor_pose: list[float] | None = None, sensor_model: str = 'auto',
            rows: int | None = None, max_offset_m: float = 2.5) -> dict:
    if not math.isfinite(max_offset_m) or not 0 <= max_offset_m <= 20:
        raise ValueError('max offset must be finite and between 0 and 20 m')
    if rows is not None and (rows < 2 or rows > 256):
        raise ValueError('rows must be between 2 and 256')
    if beam_spacing_deg is not None and (not math.isfinite(beam_spacing_deg) or beam_spacing_deg <= 0):
        raise ValueError('beam spacing must be positive and finite')
    cleaned = []
    invalid = 0
    for scan in scans:
        good = np.isfinite(scan.points).all(axis=1) & (np.linalg.norm(scan.points, axis=1) > 0)
        invalid += int(np.count_nonzero(~good))
        cleaned.append(Scan(scan.points[good], scan.intensity[good] if scan.intensity is not None else None,
                            scan.rings[good] if scan.rings is not None else None, scan.organized_rows))
    points = np.vstack([s.points for s in cleaned])
    if len(points) < 100:
        raise ValueError('at least 100 finite, nonzero points are needed')
    notes = ['No estimator settings are applied automatically. Confirm candidates before use.',
             'Scans alone do not establish x/y translation, yaw, or general roll/pitch extrinsics.']
    observed = []
    for s in cleaned:
        if s.rings is not None:
            valid = s.rings[np.isfinite(s.rings)]
            if len(valid): observed.append((int(len(np.unique(valid))), 'ring_field'))
        elif s.organized_rows:
            observed.append((s.organized_rows, 'organized_pcd'))
        else:
            count = ordered_rows(s.points)
            if count: observed.append((count, 'ordered_azimuth_turns'))
    detected_rows = observed[0][0] if observed and all(r[0] == observed[0][0] for r in observed) else None
    row_source = observed[0][1] if detected_rows else 'unresolved'
    raw_h = np.hypot(points[:, 0], points[:, 1])
    raw_elev = np.rad2deg(np.arctan2(points[:, 2], raw_h))
    raw_extent = np.percentile(raw_elev, [.2, 99.8])
    model = sensor_model
    if model == 'auto':
        if detected_rows == 64 and -28 < raw_extent[0] < -18 and 0 < raw_extent[1] < 8:
            model = 'hdl64e'
        elif detected_rows == 32 and 25 < raw_extent[1] - raw_extent[0] < 55:
            model = 'hdl32e'
        else:
            model = 'unknown'
    if model != 'unknown':
        notes.append('A sensor-model match supplies nominal beam count/FOV; it is a candidate, not sensor identification.')
    if beam_spacing_deg is None and model == 'hdl32e':
        beam_spacing_deg = 4 / 3
    frame = {'status': 'unresolved', 'pose': None, 'height_candidates': []}
    if input_sensor_pose is not None:
        if len(input_sensor_pose) != 6 or not np.isfinite(input_sensor_pose).all():
            raise ValueError('input sensor pose needs six finite values')
        frame.update(status='provided', pose=list(map(float, input_sensor_pose)))
    elif beam_spacing_deg is not None:
        trials = search_height(points, beam_spacing_deg, max_offset_m)
        frame['height_candidates'] = trials
        best = trials[0]
        surface = horizontal_surface(points)
        if best['score'] >= 5 and surface and abs(surface['z_m'] - best['sensor_z_m']) > .3:
            up = 1 if surface['z_m'] < best['sensor_z_m'] else -1
            frame.update(status='axial_estimate', pose=[0., 0., round(best['sensor_z_m'], 3), 0. if up == 1 else 180., 0., 0.])
            notes.append('Axial frame estimate uses a dominant horizontal surface as ground; ceilings or sloped ground can defeat it.')
        else:
            notes.append('Beam periodicity or ground evidence is insufficient; provide --input-sensor-pose.')
    elif model == 'hdl64e':
        frame.update(status='assumed_identity', pose=[0.] * 6)
        notes.append('Uneven HDL-64E beams do not establish the origin by one-spacing periodicity. Identity is an explicit assumption; supply extrinsics for body-frame data.')
    else:
        notes.append('No reliable frame estimate; provide --input-sensor-pose and optionally --rows or --sensor-model.')
    intensity_values = [s.intensity[np.isfinite(s.intensity)] for s in cleaned if s.intensity is not None]
    intensity = {'status': 'missing', 'valid_points': 0, 'min': None, 'max': None, 'range_hint': 'unknown'}
    if intensity_values and sum(map(len, intensity_values)):
        values = np.concatenate(intensity_values)
        low, high = float(values.min()), float(values.max())
        intensity.update(status='observed', valid_points=len(values), min=low, max=high,
                         range_hint='0..1' if 0 <= low and high <= 1 else '0..255' if 0 <= low and high <= 255 else 'unknown')
    report = {'schema_version': 1, 'point_count': len(points), 'invalid_points_removed': invalid,
              'sensor_model_candidate': model, 'sensor_frame': frame,
              'intensity': intensity, 'beam_spacing_deg': beam_spacing_deg,
              'observed_rows': detected_rows, 'rows_source': row_source,
              'rows': rows if rows is not None else detected_rows, 'fov': None, 'near_range': None, 'ground': None,
              'method_flags': {}, 'notes': notes}
    if frame['pose'] is None:
        return report
    sensor_points = transform_points(points, frame['pose'])
    distance = np.linalg.norm(sensor_points, axis=1)
    report['near_range'] = {'radius_m': 5., 'fraction': float(np.mean(distance < 5)), 'frame': 'sensor',
                            'note': 'Proximity is a diagnostic; --input-min-range-m 5 is opt-in and is not recommended from this fraction alone.'}
    h = np.hypot(sensor_points[:, 0], sensor_points[:, 1])
    elevation = np.rad2deg(np.arctan2(sensor_points[h > .5, 2], h[h > .5]))
    if len(elevation) < 100:
        notes.append('Too few returns beyond 0.5 m horizontally to estimate FOV; range-image candidates are withheld.')
        return report
    extent = np.percentile(elevation, [.2, 99.8]).tolist()
    peaks = beam_peaks(elevation)
    if rows is None:
        rows = detected_rows
        gaps = np.diff(peaks)
        if rows is None and len(gaps) >= 3 and np.std(gaps) < .15 * np.mean(gaps):
            rows = len(peaks);report['rows_source'] = 'uniform_histogram_peaks'
            notes.append('Uniform histogram peaks suggest observed rows, not proof every physical beam was visible; confirm --rows.')
        if rows is None and model in {'hdl64e', 'hdl32e'}:
            rows = 64 if model == 'hdl64e' else 32;report['rows_source'] = 'model_candidate'
    else:
        report['rows_source'] = 'provided'
    report['rows'] = rows
    if model == 'hdl64e':
        low, high, source = -24.9, 2., 'hdl64e_nominal'
        max_spacing = .5
    elif model == 'hdl32e':
        # The mount can invert the nominal HDL-32E vertical coverage.
        low, high = (-11., 31.) if sum(extent) > 0 else (-31., 11.)
        source, max_spacing = 'hdl32e_nominal_outward_rounded', 4 / 3
    else:
        low, high = math.floor(extent[0] * 10) / 10, math.ceil(extent[1] * 10) / 10
        source = 'observed_extent'
        max_spacing = max(np.diff(peaks)) if len(peaks) > 1 else beam_spacing_deg
    report['fov'] = {'observed_percentile_deg': extent, 'nominal_down_deg': low,
                     'nominal_up_deg': high, 'source': source, 'observed_histogram_peaks_deg': peaks}
    ground = horizontal_surface(sensor_points)
    if ground and ground['z_m'] < -.3:
        report['ground'] = {'height_m': -ground['z_m'], 'surface': ground,
                            'note': 'Ground height is geometric evidence; paper ground-start thresholds still need confirmation.'}
    if rows is None or rows < 2:
        notes.append('Row count unresolved; range-image CLI candidates are withheld.')
        return report
    fmt = lambda x: f'{x:g}'
    common = ['--input-sensor-pose', *map(fmt, frame['pose'])]
    report['method_flags'] = {
        'suma': common + ['--suma-paper-set', f'fov_up_deg={high:g}', '--suma-paper-set', f'fov_down_deg={abs(low):g}', '--suma-paper-set', f'height={rows}'],
        'inten_loam': common + ['--inten-loam-paper-set', f'theta_up_deg={high:g}', '--inten-loam-paper-set', f'theta_down_deg={abs(low):g}', '--inten-loam-paper-set', f'height={rows}'],
        'elo': common + ['--elo-fov', fmt(high), fmt(low), '--elo-sri-height', str(rows)],
    }
    if report['ground']:
        report['method_flags']['elo'] += ['--elo-sensor-height', f"{report['ground']['height_m']:.1f}"]
    if max_spacing is not None and -90 < low < high < 90:
        scale = 2048 / (2 * np.pi)
        gap = max(4, math.ceil(scale * np.deg2rad(max_spacing)))
        height = max(192, math.ceil((scale * (np.sin(np.deg2rad(high)) - np.sin(np.deg2rad(low))) + gap) / 16) * 16)
        report['method_flags']['dilo'] = common + ['--dilo-paper-top-elevation', fmt(high), '--dilo-paper-height', str(height), '--dilo-paper-max-fill-gap', str(gap)]
    notes.append('Method flags are sensor-geometry overrides; add the intended paper/profile flag separately and preserve its tuned parameters.')
    notes.append('The InTEn-LOAM paper runner already scales 0..255 intensity to 0..1; no intensity conversion flag is needed.')
    return report


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('scans', nargs='+', type=Path)
    parser.add_argument('--format', choices=['auto', 'pcd', 'kitti', 'nclt'], default='auto')
    parser.add_argument('--beam-spacing-deg', type=float)
    parser.add_argument('--max-offset-m', type=float, default=2.5)
    parser.add_argument('--sensor-model', choices=['auto', 'unknown', 'hdl64e', 'hdl32e'], default='auto')
    parser.add_argument('--input-sensor-pose', nargs=6, type=float, metavar=('X', 'Y', 'Z', 'ROLL', 'PITCH', 'YAW'))
    parser.add_argument('--rows', type=int)
    parser.add_argument('--json', action='store_true', help='Print the machine-readable report instead of text')
    parser.add_argument('--json-output', type=Path)
    args = parser.parse_args(argv)
    try:
        scans = [load_scan(p, args.format) for p in args.scans]
        report = analyze(scans, beam_spacing_deg=args.beam_spacing_deg, input_sensor_pose=args.input_sensor_pose,
                         sensor_model=args.sensor_model, rows=args.rows, max_offset_m=args.max_offset_m)
        report['inputs'] = [{'path': str(p), 'sha256': hashlib.sha256(p.read_bytes()).hexdigest()} for p in args.scans]
        report['command'] = shlex.join(['python3', str(Path(__file__)), *(sys.argv[1:] if argv is None else argv)])
        report['evaluator_git_commit'] = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=Path(__file__).resolve().parents[2], text=True).strip()
    except (OSError, ValueError) as error:
        parser.error(str(error))
    serialized = json.dumps(report, indent=2, allow_nan=False) + '\n'
    if args.json_output:
        args.json_output.parent.mkdir(parents=True, exist_ok=True)
        args.json_output.write_text(serialized)
    if args.json:
        print(serialized, end='')
    else:
        print('Sensor frame:', report['sensor_frame']['status'], report['sensor_frame']['pose'])
        if report['sensor_frame']['height_candidates']:
            print('Height candidates: score / sensor z [m]')
            for candidate in report['sensor_frame']['height_candidates']:
                print(f"  {candidate['score']:.1f} / {candidate['sensor_z_m']:+.3f}")
        print('Rows:', report['rows'], '(' + report['rows_source'] + ')')
        fov = report['fov']
        print('FOV:', ({'observed_deg': fov['observed_percentile_deg'],
                        'candidate_up_deg': fov['nominal_up_deg'], 'candidate_down_deg': fov['nominal_down_deg'],
                        'source': fov['source']} if fov else 'unresolved'))
        print('Intensity:', report['intensity'])
        print('Returns within 5 m:', report['near_range'])
        print('Ground:', report['ground'])
        for method, flags in report['method_flags'].items():
            print(method + ': ' + shlex.join(flags))
        for note in report['notes']:
            print('Note: ' + note)
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
