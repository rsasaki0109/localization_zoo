"""Losslessly encode the compiler's intentional final blank line; no native rerun."""
import gzip
from common import E, read, write, sha


def main():
    folder = E/'controls'
    manifest = read(folder/'archive.json')
    name = 'compiler.stdout'
    entry = manifest['files'][name]
    original = folder/entry['path']
    assert entry['storage'] == 'verbatim' and sha(original) == entry['sha256']
    raw = original.read_bytes()
    assert raw.endswith(b'\n\n')
    encoded = folder/(name+'.gz')
    with encoded.open('xb') as out: out.write(gzip.compress(raw, mtime=0))
    assert gzip.decompress(encoded.read_bytes()) == raw
    entry.update(storage='gzip', path=encoded.name, stored_sha256=sha(encoded))
    write(folder/'archive.json', manifest)
    original.unlink()  # The original stage file and every scientific byte remain.
    write(E/'archival_encoding.json', dict(exit_code=0, command='python experiments/results/inten_cluster_motion/archive_encoding.py',
          initial_check=dict(command=['git', 'diff', '--cached', '--check'], exit_code=2,
                             stdout='experiments/results/inten_cluster_motion/controls/compiler.stdout:5: new blank line at EOF.\n'),
          file=name, decoded_sha256=entry['sha256'], encoded_sha256=entry['stored_sha256'],
          original_stage_file_unchanged=True, numerical_output_unchanged=True, native_rerun=False,
          reason='Compiler --version intentionally emits a final blank line. Preserve it in gzip rather than editing the recorded output.'))


if __name__ == '__main__':
    main()
