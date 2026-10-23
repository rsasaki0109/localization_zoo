#!/usr/bin/env python3
"""Bind the support study to its archived production sources, not today's code.

The frozen verifier and protocols remain unchanged. Only its source root is
temporarily redirected: production files come from the recorded build archive,
while every experimental source and result still points at the current checkout
and must pass the original hashes and numerical checks. This does not validate
the current estimator; its native tests and default-off parity are separate.
"""
from contextlib import contextmanager
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
def historical_sources(study):
    study = Path(study)
    repository = study.parents[2]
    protocol = json.loads((study / 'protocol.json').read_text())
    build = study / 'checks/build'
    manifest = json.loads((build / 'source_manifest.json').read_text())
    with tempfile.TemporaryDirectory(prefix='inten_historical_sources_') as folder:
        root = Path(folder)
        (root / 'experiments').symlink_to(repository / 'experiments', target_is_directory=True)
        for name, expected in protocol['source_sha256'].items():
            relative = relative_path(name)
            if relative.parts[0] != 'experiments':
                if relative.parts[0] not in ('papers', 'evaluation'):
                    raise ValueError('Unexpected production source: ' + name)
                entry = manifest[name]
                if entry['sha256'] != expected:
                    raise ValueError('Archived source manifest differs: ' + name)
                archive = build / relative_path(entry['path'])
                raw = gzip.decompress(archive.read_bytes())
                if digest(raw) != expected:
                    raise ValueError('Archived production source changed: ' + name)
                target = root / relative
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_bytes(raw)
            if digest((root / relative).read_bytes()) != expected:
                raise ValueError('Frozen source changed: ' + name)
        yield root


def validate_in(verifier, folder):
    """Run every frozen group/global check; restore module state even on error."""
    with historical_sources(verifier.E) as root:
        previous = verifier.checker.W
        verifier.checker.W = root
        try:
            return verifier.validate_in(Path(folder))
        finally:
            verifier.checker.W = previous
