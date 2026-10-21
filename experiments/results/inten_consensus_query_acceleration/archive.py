from pathlib import Path
import gzip,hashlib,json,shutil
P={"archival":{"uncompressed_shard_bytes":32*1024*1024}}
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def write(p,d):p.write_text(json.dumps(d,indent=2)+"\n")

def archive(source, target):
    """Losslessly retain every result file; scanners live outside source."""
    target.mkdir(parents=True, exist_ok=False)
    manifest = {}
    for path in sorted(source.rglob('*')):
        if not path.is_file():
            continue
        relative = path.relative_to(source)
        entry = dict(original_path=str(path), bytes=path.stat().st_size, sha256=sha(path))
        if path.suffix == '.jsonl' or path.name == 'native.stdout.log':
            pieces = target / (relative.as_posix() + '.parts')
            pieces.mkdir(parents=True)
            parts = []
            recovered = hashlib.sha256()
            with path.open('rb') as stream:
                while True:
                    raw = stream.read(P['archival']['uncompressed_shard_bytes'])
                    if not raw:
                        break
                    part = pieces / ('part-%05d.gz' % len(parts))
                    part.write_bytes(gzip.compress(raw, compresslevel=6, mtime=0))
                    decoded = gzip.decompress(part.read_bytes())
                    if decoded != raw:
                        raise RuntimeError('Archive shard does not round-trip')
                    recovered.update(decoded)
                    parts.append(dict(path=part.relative_to(target).as_posix(),
                                      gzip_sha256=sha(part), gzip_bytes=part.stat().st_size,
                                      raw_sha256=hashlib.sha256(raw).hexdigest(), raw_bytes=len(raw)))
            if recovered.hexdigest() != entry['sha256'] or sha(path) != entry['sha256']:
                raise RuntimeError('Archive stream differs from original')
            entry.update(storage='ordered_gzip_shards', parts=parts)
        else:
            destination = target / relative
            destination.parent.mkdir(parents=True, exist_ok=True)
            if path.suffix == '.log':
                destination = destination.with_suffix('.log.gz')
                destination.write_bytes(gzip.compress(path.read_bytes(), mtime=0))
                restored = gzip.decompress(destination.read_bytes())
            else:
                shutil.copyfile(path, destination)
                restored = destination.read_bytes()
            if hashlib.sha256(restored).hexdigest() != entry['sha256']:
                raise RuntimeError('Archive file differs from original')
            entry.update(storage='gzip' if path.suffix == '.log' else 'verbatim',
                         path=destination.relative_to(target).as_posix(), stored_sha256=sha(destination))
        manifest[relative.as_posix()] = entry
    write(target / 'archive.json', dict(files=manifest, byte_roundtrip_verified=True, exit_code=0))
