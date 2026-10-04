import sys, json, math, statistics
sys.path.insert(0, str(__import__('pathlib').Path(__file__).resolve().parent))
import analyze_posture_windows as A

PRE = (2.0, 1.0); THR = 50.0; FALLBACK_POST = (0.5, 1.5)

def spread_deg(samples, a, b, mean):
    worst = 0.0
    for t, v in samples:
        if a <= t <= b:
            n = math.sqrt(sum(x*x for x in v))
            if n > 0:
                worst = max(worst, A.angle([x/n for x in v], mean))
    return worst

def confirm(s, impacts, start, length, step, stab):
    end = s[-1][0]
    for t in impacts:
        u = A.mean_dir(s, t - PRE[0], t - PRE[1])
        if not u:
            continue
        a = t + start
        while a + length <= t + FALLBACK_POST[1] + 1e-9 and a + length <= end:
            v = A.mean_dir(s, a, a + length)
            if v and A.angle(u, v) >= THR and spread_deg(s, a, a + length, v) <= stab:
                return a + length
            a += step
        if t + FALLBACK_POST[1] <= end:
            v = A.mean_dir(s, t + FALLBACK_POST[0], t + FALLBACK_POST[1])
            if v and A.angle(u, v) >= THR:
                return t + FALLBACK_POST[1]
    return None

def run(m, imp, acc, idx, params):
    out = []
    for i in idx:
        e = m[i]; c = confirm(acc[i], imp[i], *params)
        lat = c - e['fall_onset_s'] if (c is not None and e['label'] == 'fall') else None
        out.append((e, c is not None, lat))
    return out

def summ(rows):
    f = [r for r in rows if r[0]['label'] == 'fall']; d = [r for r in rows if r[0]['label'] != 'fall']
    lat = [r[2] for r in f if r[2] is not None]
    return dict(sens=sum(r[1] for r in f)/len(f) if f else None, fp=sum(r[1] for r in d)/len(d) if d else None,
                lat=statistics.median(lat) if lat else None)

def load():
    m = json.load(open(A.ROOT/'normalized/manifest.json'))['recordings']
    imp, acc = [], {}
    for i, e in enumerate(m):
        ev = [json.loads(l) for l in open(A.RES/'runs'/f'{i:05d}.events.jsonl') if l.strip()]
        imp.append([x['timestamp'] for x in ev if x['phase'] == 'started' and x['type'] == 'impact'])
    for i, e in enumerate(m):
        acc[i] = A.load_accel(A.ROOT/'normalized'/e['path'])
    return m, imp, acc

if __name__ == '__main__':
    m, imp, acc = load()
    cgu = lambda sp: [i for i, e in enumerate(m) if e['dataset'] == 'cgu_bes' and e['split'] == sp]
    tv = cgu('train') + cgu('validation')
    jumps_all = [i for i in tv if 'Jump' in m[i]['activity']]
    # jumps rarely trigger the CLI impact detector, so also probe them at their peak acceleration
    best = None
    for start in (0.25, 0.5):
        for length in (0.25, 0.5):
            for stab in (5.0, 10.0, 15.0, 20.0):
                p = (start, length, 0.05, stab)
                tr = summ(run(m, imp, acc, cgu('train'), p)); va = summ(run(m, imp, acc, cgu('validation'), p))
                jump_hits = sum(confirm(acc[i], [m[i]['impact_timestamp']], *p) is not None for i in jumps_all)
                ok = tr['sens'] >= 0.95 and tr['fp'] <= 0.02 and va['fp'] <= 0.02 and jump_hits == 0
                print(p, 'train', tr, 'val', va, 'jump_hits', jump_hits, 'OK' if ok else '')
                if ok and (best is None or tr['lat'] < best[1]['lat']):
                    best = (p, tr)
    print('SELECTED', best)
