#!/usr/bin/env python3
"""Validate a streamed InTEn trace against its native summary and exit receipt.

This is offline structural validation, not a motion-quality or cause diagnosis.
It reads one frame at a time and never supplies input to an estimator.
"""
from collections import Counter
import argparse
import hashlib
import json
import math
from pathlib import Path
import sys


class TraceError(ValueError):
    pass


def require(condition, message):
    if not condition:
        raise TraceError(message)


def integer(value, label):
    require(type(value) is int and value >= 0, label + ' must be a nonnegative integer')
    return value


def boolean(value, label):
    require(type(value) is bool, label + ' must be a boolean')
    return value


def strict_json(data):
    def pairs(items):
        obj = {}
        for key, value in items:
            require(key not in obj, 'duplicate JSON key: ' + key)
            obj[key] = value
        return obj
    def constant(value):
        raise TraceError('nonstandard JSON numeric literal: ' + value)
    try:
        return json.loads(data, object_pairs_hook=pairs, parse_constant=constant)
    except (ValueError, UnicodeError) as error:
        raise TraceError(str(error)) from error


def validate_trace(trace_path, summary_path, receipt_path):
    """Return statistics only after the end record and run binding are validated.

    Matrix dimensions, flags, counts and termination are checked. This does not
    verify SE(3) quality, binary/input hashes, residual meaning or GT alignment.
    """
    nonfinite = 0
    def number(value, label, allow_nonfinite=False):
        nonlocal nonfinite
        if type(value) in (int, float):
            try:
                finite = math.isfinite(value)
            except OverflowError:
                finite = False
            if finite:
                return
        if allow_nonfinite and value in ('NaN', '+Infinity', '-Infinity'):
            nonfinite += 1
            return
        raise TraceError(label + ' has an invalid/nonfinite value')
    def matrix(value, length, label, allow_nonfinite=False):
        require(isinstance(value, list) and len(value) == length, label + ' has wrong matrix/vector length')
        for v in value:
            number(v, label, allow_nonfinite)
    stats = {stage: dict(calls=0, iterations=0, stops=Counter()) for stage in ('s2s', 's2m')}
    def solver(s, label):
        require(isinstance(s, dict), label + ' must be an object')
        called = boolean(s['called'], label + '.called')
        returned = integer(s['returned_pairs'], label + '.returned_pairs')
        matrix(s['input'], 16, label + '.input')
        matrix(s['output'], 16, label + '.output')
        iterations = s['iterations']
        require(isinstance(iterations, list), label + '.iterations must be an array')
        stop = s['stop']
        allowed = {'not_run', 'no_iterations', 'insufficient_pairs', 'nonfinite_step', 'converged', 'iteration_limit'}
        require(stop in allowed, label + ' has unknown stop reason')
        require(called == (stop != 'not_run'), label + ' called/stop mismatch')
        if not called or stop == 'no_iterations':
            require(not iterations and returned == 0, label + ' unrun/zero-budget stage has observations')
        else:
            require(bool(iterations), label + ' called stage lacks attempted iterations')
        for index, it in enumerate(iterations):
            require(isinstance(it, dict), label + ' iteration must be an object')
            require(integer(it['index'], label + '.index') == index, label + ' iteration index is not contiguous')
            matrix(it['linearization_pose'], 16, label + '.linearization_pose')
            matrix(it['H'], 36, label + '.H', True)
            matrix(it['g'], 6, label + '.g', True)
            matrix(it['delta'], 6, label + '.delta', True)
            counts = it['class_pairs']
            require(isinstance(counts, list) and len(counts) == 4, label + ' needs four class counts')
            for v in counts:
                integer(v, label + '.class_pairs')
            require(sum(counts) == integer(it['pairs'], label + '.pairs'), label + ' class/total pair count mismatch')
            number(it['weighted_squared_residual'], label + '.weighted_squared_residual', True)
            attempted = boolean(it['step_attempted'], label + '.step_attempted')
            finite = boolean(it['step_finite'], label + '.step_finite')
            applied = boolean(it['update_applied'], label + '.update_applied')
            require(not finite or attempted, label + ' finite step was not attempted')
            if not attempted:
                require(index == len(iterations) - 1 and stop == 'insufficient_pairs', label + ' unattempted step did not terminate')
            require(not applied or (attempted and finite), label + ' applied step is not finite/attempted')
            if finite:
                require(all(type(v) in (int, float) and math.isfinite(v) for v in it['delta']), label + ' finite flag has nonfinite delta')
            elif attempted:
                require(index == len(iterations) - 1 and stop == 'nonfinite_step', label + ' failed step did not terminate')
                require(any(isinstance(v, str) for v in it['delta']), label + ' failed step lost its nonfinite delta')
        if iterations:
            last = iterations[-1]
            if stop == 'insufficient_pairs':
                require(not last['step_attempted'], label + ' insufficient-pairs stop attempted a step')
            elif stop == 'nonfinite_step':
                require(last['step_attempted'] and not last['step_finite'] and returned == 0, label + ' nonfinite stop/return mismatch')
            else:
                require(last['update_applied'] and returned == last['pairs'], label + ' completed solve/return mismatch')
        stats[label]['calls'] += int(called)
        stats[label]['iterations'] += len(iterations)
        stats[label]['stops'][stop] += 1
        return called

    summary_bytes, receipt_bytes = Path(summary_path).read_bytes(), Path(receipt_path).read_bytes()
    summary, receipt = strict_json(summary_bytes), strict_json(receipt_bytes)
    try:
        require(isinstance(summary, dict) and isinstance(receipt, dict), 'summary/receipt must be objects')
        require(integer(receipt['exit_code'], 'native exit_code') == 0, 'native receipt is not exit 0')
    except (KeyError, TypeError, TraceError) as error:
        raise TraceError('native summary/receipt: ' + str(error)) from error
    processed = skipped = inputs = records = 0
    ended = False
    header = None
    digest = hashlib.sha256()
    try:
        with Path(trace_path).open('rb') as stream:
            for line_no, raw in enumerate(stream, 1):
                digest.update(raw)
                require(not ended, 'record after end')
                item = strict_json(raw)
                require(isinstance(item, dict), 'record must be an object')
                kind = item['type']
                records += 1
                if header is None:
                    require(kind == 'header', 'first record must be header')
                    header = item
                    require(item['schema'] == 'localization_zoo.inten_trace' and type(item['version']) is int and item['version'] == 1, 'unsupported schema/version')
                    for key, value in [('matrix_layout', 'row_major_flat'), ('perturbation', 'left_translation_rotation'),
                                       ('s2s_target_frame', 'previous_sensor'), ('s2m_target_frame', 'estimator_world'),
                                       ('nonfinite_encoding', 'NaN,+Infinity,-Infinity strings'),
                                       ('residual_definition', 'sum_huber_irls_weight_times_squared_residual_at_linearization')]:
                        require(item[key] == value, 'unsupported ' + key)
                    integer(item['input_frames'], 'header.input_frames')
                    argv = item['argv']
                    require(isinstance(argv, list) and len(argv) >= 3 and all(isinstance(v, str) for v in argv), 'invalid header argv')
                    require(isinstance(item['params'], dict), 'params must be an object')
                    for v in item['params'].values():
                        if type(v) is not bool:
                            number(v, 'effective parameter', True)
                    loader = item['loader']
                    for key in ['min_range_m', 'max_range_m', 'elevation_correction_rad']:
                        number(loader[key], 'loader.' + key)
                    matrix(loader['sensor_pose'], 16, 'sensor_pose')
                    matrix(item['output_anchor'], 16, 'output_anchor')
                    if 'native_argv' in receipt:
                        require(receipt['native_argv'] == argv, 'trace/receipt argv mismatch')
                    else:
                        command = receipt['command']
                        require(isinstance(command, list) and command[-len(argv):] == argv, 'trace/receipt command suffix mismatch')
                    require(summary['pcd_dir'] == argv[1] and summary['gt_csv'] == argv[2], 'summary input paths differ from trace argv')
                elif kind == 'end':
                    require(integer(item['processed_frames'], 'end.processed_frames') == processed and
                            integer(item['skipped_frames'], 'end.skipped_frames') == skipped, 'end counters mismatch')
                    require(inputs == header['input_frames'], 'end does not cover all input frames')
                    ended = True
                elif kind in ('frame', 'skipped_frame'):
                    require(integer(item['input_index'], 'input_index') == inputs, 'input index is not contiguous')
                    require(isinstance(item['scan_path'], str), 'scan_path must be a string')
                    inputs += 1
                    if kind == 'skipped_frame':
                        require(item['reason'] == 'empty_cloud', 'unknown skipped-input reason')
                        require('s2s' not in item and 's2m' not in item, 'skipped input has solver observations')
                        skipped += 1
                        continue
                    require(integer(item['frame_index'], 'frame_index') == processed, 'estimator frame index is not contiguous')
                    initialization = boolean(item['initialization'], 'initialization')
                    require(initialization == (processed == 0), 'initialization/frame mismatch')
                    for key in ['pose', 'anchored_pose', 'accepted_relative', 's2m_prediction']:
                        matrix(item[key], 16, key)
                    for key in ['edge', 'ground', 'facade', 'reflector']:
                        integer(item['features'][key], 'features.' + key)
                    integer(item['dor_removed'], 'dor_removed')
                    s2s_called, s2m_called = solver(item['s2s'], 's2s'), solver(item['s2m'], 's2m')
                    require(s2s_called == (not initialization) and not (initialization and s2m_called), 'initialization/stage call mismatch')
                    fallback = boolean(item['s2s_fallback'], 's2s_fallback')
                    accepted = boolean(item['s2m_accepted'], 's2m_accepted')
                    require(not fallback or s2s_called, 'uncalled S2S has fallback')
                    require(not accepted or s2m_called, 'uncalled S2M is accepted')
                    processed += 1
                else:
                    raise TraceError('unexpected record type: ' + str(kind))
        require(header is not None and ended, 'missing header/end; trace is incomplete')
        require(integer(summary['num_frames'], 'summary.num_frames') == inputs, 'summary/input frame count mismatch')
        methods = summary['methods']
        require(isinstance(methods, list) and len(methods) == 1, 'summary must contain only InTEn-LOAM')
        method = methods[0]
        require(method['name'] == 'InTEn-LOAM' and method['status'] == 'ok', 'native method is not successful InTEn-LOAM')
        require(integer(method['frames'], 'method.frames') == processed, 'summary/processed frame count mismatch')
    except (KeyError, TypeError, TraceError) as error:
        raise TraceError('trace line ' + str(locals().get('line_no', 0)) + ': ' + str(error)) from error
    return dict(schema_version=1, exit_code=0, scope='Structural/run-receipt validation only; accuracy, GT indexing, binary/input hashes and solver cause are not validated.',
                trace_sha256=digest.hexdigest(), summary_sha256=hashlib.sha256(summary_bytes).hexdigest(),
                receipt_sha256=hashlib.sha256(receipt_bytes).hexdigest(), records=records,
                input_frames=inputs, processed_frames=processed, skipped_frames=skipped,
                nonfinite_values=nonfinite, stages=stats)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--trace', required=True, type=Path)
    parser.add_argument('--summary', required=True, type=Path)
    parser.add_argument('--receipt', required=True, type=Path)
    parser.add_argument('--output', type=Path, help='New report path; existing files are never overwritten.')
    args = parser.parse_args(argv)
    try:
        report = validate_trace(args.trace, args.summary, args.receipt)
        text = json.dumps(report, indent=2, allow_nan=False) + '\n'
        if args.output:
            with args.output.open('x') as stream:
                stream.write(text)
        else:
            sys.stdout.write(text)
        return 0
    except (OSError, ValueError, KeyError, TypeError) as error:
        print('invalid InTEn trace: ' + str(error), file=sys.stderr)
        return 1


if __name__ == '__main__':
    raise SystemExit(main())
