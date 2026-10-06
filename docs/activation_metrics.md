# Activation and retention measurement

Localization Zoo treats a successful benchmark report as activation. Page
views, GitHub stars, and repository clones are supporting signals, not product
success on their own.

## Funnel

| Stage | Definition | Primary measure |
|---|---|---|
| Intent | A visitor reaches the project page or README quick start | Not measured |
| Start | A visitor runs the Docker command | Not measured |
| Activation | The demo emits `localization_zoo_demo: OK` and writes a valid `manifest.json` | Successful manifests |
| Second run | An activated user runs another profile or dataset | A second valid manifest with a different run configuration |
| Retention | An activated user produces a valid manifest in a later week | Weekly returning successful runners / activated cohort |

The validated manifest is the activation source of truth.

## Browser-side events

The GitHub Pages site (`docs/index.html`) records nothing: it sends no network
requests beyond loading its own files and writes nothing to `localStorage`.
Any future analytics adapter needs a privacy review and must be stated on the
page.

## Experiment scorecard

Record a baseline before changing another onboarding variable. For each release,
compare:

1. Shared valid manifests, where sharing is explicit.
2. Median time from demo start to the success marker in controlled smoke runs.
3. Quick-profile activations to broad-profile or own-data second runs.
4. Activated cohorts returning in a later calendar week.

Segment results only when the cohort is large enough to
avoid exposing an individual. Do not interpret stars or page views as retained
usage.
