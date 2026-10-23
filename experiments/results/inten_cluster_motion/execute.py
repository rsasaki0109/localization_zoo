"""Retain actual producer/analysis exits and all output before interpretation."""
from common import E, R, PY, run, module


def main():
    archive = module('cluster_archive', E/'archive.py').archive
    status = run('collection', [PY, str(E/'run.py')], R/'collection_record')
    if status:
        archive(R/'collection_record', E/'collection_failure')
        if (R/'stage').exists(): archive(R/'stage', E/'native_failure')
        raise RuntimeError('Collection failed; preserve evidence before any correction')
    status = run('analysis', [PY, str(E/'analyze.py')], R/'analysis_record')
    if status:
        archive(R/'analysis_record', E/'analysis_failure')
        if (R/'analysis').exists(): archive(R/'analysis', E/'partial_analysis_failure')
        archive(R/'stage', E/'controls')
        archive(R/'collection_record', E/'collection_execution')
        raise RuntimeError('Analysis failed; native outputs remain unchanged')
    archive(R/'stage', E/'controls')
    archive(R/'analysis', E/'analysis')
    archive(R/'collection_record', E/'collection_execution')
    archive(R/'analysis_record', E/'analysis_execution')
    print('All numerical evidence archived with exact byte roundtrip; commit before interpreting', flush=True)


if __name__ == '__main__':
    main()
