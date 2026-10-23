"""Exercise the full-trace decision checker on the exact cached native controls."""
import json
from common import E,R,write
from check_consensus_trace import check_frame

def main():
    total=0
    for threads in (1,4):
        with (R/'parity_stage_corrected'/('native_'+str(threads)+'.stdout.jsonl')).open() as source:
            for line in source:
                row=json.loads(line)
                if row['type']=='terminal':check_frame(row['trace'],row['mode']==1);total+=1
    write(E/'checks/cached_parity/decision_verification.json',dict(exit_code=0,terminal_calls_checked=total,both_thread_counts=True,no_gt=True))

if __name__=='__main__':main()
