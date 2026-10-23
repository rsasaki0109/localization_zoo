"""Check recorded reports without treating roundoff-tied witness IDs as data."""
from pathlib import Path
import copy,importlib.util,json
E=Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location('normalized_recorded_verifier',E/'verify.py')
v=importlib.util.module_from_spec(spec);spec.loader.exec_module(v)

def check(actual,details,recorded):
    aligned=copy.deepcopy(actual);witnesses=0
    v.require(actual['fields'].keys()==recorded['fields'].keys(),'Field groups changed')
    for group,fields in actual['fields'].items():
        v.require(len(fields)==len(recorded['fields'][group]),'Field count changed')
        for index,(field,oldfield) in enumerate(zip(fields,recorded['fields'][group])):
            v.require(field['field']==oldfield['field'],'Field identity changed')
            a,b=field['largest'],oldfield['largest']
            if a is None or b is None:
                v.require(a is None and b is None,'Largest witness presence changed')
                continue
            boundaries=details[group][field['field']]
            supported=[r for r in boundaries if r['both_valid']]
            maximum=max(abs(r['jump']) for r in supported)
            for witness in (a,b):
                key=tuple(witness[k] for k in ('index','axis','face'))
                matches=[r for r in supported if tuple(r[k] for k in ('index','axis','face'))==key]
                v.require(len(matches)==1,'Unknown or ambiguous largest witness')
                # Validate both reported identities, supports, positions, values,
                # gradients and coverage against independently reconstructed faces.
                v.V.equivalent(matches[0],witness)
                v.require(v.V.close(abs(witness['jump']),maximum),'Witness is not a maximum at the frozen tolerance')
            # A CPU may choose another maximum when all differences are roundoff.
            # Align only the witness identity after proving both witnesses above.
            # No metric, count, decision or numerical tolerance is changed.
            aligned['fields'][group][index]['largest']=copy.deepcopy(b)
            witnesses+=1
    v.V.equivalent(aligned,recorded)
    return dict(exit_code=0,field_witnesses_verified=witnesses,
                all_report_metrics_counts_and_decisions_verified=True,
                both_witnesses_reconstructed=True,tolerances_unchanged=True)

if __name__=='__main__':
    import argparse
    parser=argparse.ArgumentParser();parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args();actual,details=v.validate()
    result=check(actual,details,v.read(E/'report/summary.json'))
    args.output.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result))
