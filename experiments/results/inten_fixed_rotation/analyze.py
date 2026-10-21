"""Use GT for offline evaluation only, after every native call finishes."""
import json
import numpy as np
from common import E,R,read,write
from equations import evaluate

def main():
    previous={(v['frame'],v['mode']):v for v in map(json.loads,(R/'inputs/previous.jsonl').read_text().splitlines())}
    profiles=[v for v in map(json.loads,(R/'inputs/profiles.jsonl').read_text().splitlines()) if v['type']=='profile']
    native=list(map(json.loads,(R/'stage/native.stdout.jsonl').read_text().splitlines()))
    truth={v['frame']:np.array(v['gt_relative']).reshape(4,4) for v in read(R/'inputs/report.json')['cases']}
    with (R/'stage/residuals.jsonl').open() as source:
        report=evaluate(read(E/'protocol.json'),previous,profiles,read(R/'stage/selections.json')['frames'],native,map(json.loads,source),truth)
    write(R/'analysis/report.json',report)

if __name__=='__main__':main()
