#!/usr/bin/env python3
"""Bind the support study to its archived production sources, not today's code.

The frozen verifier and protocols remain unchanged. Only its source root is
temporarily redirected: production files come from the recorded build archive,
while every experimental source and result still points at the current checkout
and must pass the original hashes and numerical checks. This does not validate
the current estimator; its native tests and default-off parity are separate.
"""
from contextlib import contextmanager
from concurrent.futures import ThreadPoolExecutor
import gzip
import hashlib
import json
from pathlib import Path
import tempfile


def digest(data):
    return hashlib.sha256(data).hexdigest()


def relative_path(name):
    path = Path(name)
    if path.is_absolute() or '..' in path.parts or not path.parts:
        raise ValueError('Unsafe archived source path: ' + name)
    return path


@contextmanager
def historical_sources(study, *, build=None, extra_sources=None):
    study = Path(study)
    repository = study.parents[2]
    protocol = json.loads((study / 'protocol.json').read_text())
    build = Path(build) if build is not None else study / 'checks/build'
    extra_sources = extra_sources or {}
    manifest = json.loads((build / 'source_manifest.json').read_text())
    with tempfile.TemporaryDirectory(prefix='inten_historical_sources_') as folder:
        root = Path(folder)
        (root / 'experiments').symlink_to(repository / 'experiments', target_is_directory=True)
        for name, expected in protocol['source_sha256'].items():
            relative = relative_path(name)
            if relative.parts[0] != 'experiments':
                if name in extra_sources:
                    raw = Path(extra_sources[name]).read_bytes()
                elif relative.parts[0] in ('papers', 'evaluation'):
                    entry = manifest[name]
                    if entry['sha256'] != expected:
                        raise ValueError('Archived source manifest differs: ' + name)
                    archive = build / relative_path(entry['path'])
                    raw = gzip.decompress(archive.read_bytes())
                else:
                    raise ValueError('Unexpected production source: ' + name)
                if digest(raw) != expected:
                    raise ValueError('Archived production source changed: ' + name)
                target = root / relative
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_bytes(raw)
            if digest((root / relative).read_bytes()) != expected:
                raise ValueError('Frozen source changed: ' + name)
        yield root


def rigidity_sources(study):
    """The passive rigidity study inherits its parent's production build."""
    return {
        'build': study.parent / 'inten_support_information/checks/build',
        'extra_sources': {
            'tests/test_inten_local_rigidity.py': study.parent / (
                'inten_geometric_prior/historical_sources/test_inten_local_rigidity.py'),
        },
    }


@contextmanager
def bind_sources(module, **kwargs):
    with historical_sources(module.E, **kwargs) as root:
        previous = module.W
        module.W = root
        try:
            yield root
        finally:
            module.W = previous


class SerialWorkers(ThreadPoolExecutor):
    """Keep the frozen pair/group order with one live worker process at a time."""
    def __init__(self, max_workers=None, **kwargs):
        super().__init__(max_workers=1, **kwargs)


@contextmanager
def bind_support_sources(verifier):
    """Bind every nested recovery verifier, including its final source audit."""
    with historical_sources(verifier.E) as root:
        pending = [verifier]
        seen = set()
        previous = []
        while pending:
            module = pending.pop()
            if id(module) in seen:
                continue
            seen.add(id(module))
            # Preserve the scheduler's original worker cwd and commands. Only
            # its nested recovery/source audits need the archived production.
            if module is not verifier and hasattr(module, 'W'):
                previous.append((module, module.W))
            for name in ('base', 'field', 'checker'):
                child = getattr(module, name, None)
                if child is not None:
                    pending.append(child)
        try:
            for module, _ in previous:
                module.W = root
            yield root
        finally:
            for module, value in reversed(previous):
                module.W = value


def validate_in(verifier, folder):
    """Run every frozen check serially; restore module state even on error.

    Only scheduling changes: the original worker commands, arithmetic, receipts,
    group population and final global checks are executed unchanged. A pair of
    large reconstruction workers exceeds this environment's available memory.
    """
    with bind_support_sources(verifier):
        previous_executor = verifier.ThreadPoolExecutor
        verifier.ThreadPoolExecutor = SerialWorkers
        try:
            return verifier.validate_in(Path(folder))
        finally:
            verifier.ThreadPoolExecutor = previous_executor
