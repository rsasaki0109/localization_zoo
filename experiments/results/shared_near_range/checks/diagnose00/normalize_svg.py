"""Remove SVG line-end padding while preserving every non-whitespace token."""
import gzip
import hashlib
from pathlib import Path
import re
import sys
import xml.etree.ElementTree as ET

E = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(E))
from support import write

path = E / 'diagnostics/00/trace.svg'
folder = Path(__file__).parent
archive = folder / 'original_trace.svg.gz'
assert not archive.exists(), 'Preserve previous formatting evidence.'
raw = path.read_bytes()
normalized = b'\n'.join(line.rstrip(b' \t') for line in raw.splitlines()) + b'\n'
assert re.sub(rb'\s+', b' ', raw) == re.sub(rb'\s+', b' ', normalized)
ET.fromstring(raw)
ET.fromstring(normalized)
archive.write_bytes(gzip.compress(raw, mtime=0))
assert gzip.decompress(archive.read_bytes()) == raw
path.write_bytes(normalized)
write(folder / 'svg_normalization.json', dict(command=[sys.executable, *sys.argv],
    original_sha256=hashlib.sha256(raw).hexdigest(),
    normalized_sha256=hashlib.sha256(normalized).hexdigest(),
    original_archive=str(archive), non_whitespace_tokens_identical=True,
    both_xml_documents_parse=True, exit_code=0))
