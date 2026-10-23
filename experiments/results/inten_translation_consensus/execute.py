"""Retain all actual producer and analysis outcomes before interpretation."""
from common import E, R, PY, run, module


def main():
    archive = module('consensus_archive', E/'archive.py').archive
    code = run('collection', [PY, str(E/'run.py')], R/'collection_record')
    if code:
        archive(R/'collection_record', E/'collection_failure')
        if (R/'stage').exists(): archive(R/'stage', E/'native_failure')
        raise RuntimeError('Collection failed; preserve failure before correction')
    code = run('analysis', [PY, str(E/'analyze.py')], R/'analysis_record')
    if code:
        archive(R/'analysis_record', E/'analysis_failure')
        if (R/'analysis').exists(): archive(R/'analysis', E/'partial_analysis_failure')
        archive(R/'stage', E/'controls')
        archive(R/'collection_record', E/'collection_execution')
        raise RuntimeError('Analysis failed; native outputs retained unchanged')
    for source, target in ((R/'stage', E/'controls'), (R/'analysis', E/'analysis'),
                            (R/'collection_record', E/'collection_execution'), (R/'analysis_record', E/'analysis_execution')):
        archive(source, target)
    print('Complete numerical factorial evidence archived; commit before interpreting', flush=True)


if __name__ == '__main__': main()
