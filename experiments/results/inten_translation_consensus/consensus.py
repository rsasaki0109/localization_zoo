"""One frozen, equally weighted cluster-consensus rule; no GT or hand region."""
import math

BAND_SQUARED = .1 * .1  # Existing native Huber scale squared, not a fitted value.


def select(profiles):
    valid = []
    for row in profiles:
        costs = row['costs_m2']
        if not costs:
            continue
        if len(costs) != 121 or not all(math.isfinite(c) and c >= 0. for c in costs):
            raise ValueError('Invalid committed geometric profile')
        minimum = min(costs)
        valid.append((row['cluster'], [cost-minimum <= BAND_SQUARED for cost in costs]))
    votes = [sum(allowed[k] for _, allowed in valid) for k in range(121)]
    best = min(range(121), key=lambda k: (-votes[k], abs(k-60), k))
    usable = bool(valid and 0 < best < 120 and 2*votes[best] > len(valid))
    return dict(best_index=best, applied_index=best if usable else 60,
                shift_m=(best-60)*.05 if usable else 0., usable=usable,
                valid_clusters=len(valid), votes=votes,
                excluded_clusters=sorted(cluster for cluster, allowed in valid if usable and not allowed[best]))
