# Activation and retention measurement

Localization Zoo treats a successful benchmark report as activation. Page
views, GitHub stars, and repository clones are supporting signals, not product
success on their own.

## Funnel

| Stage | Definition | Primary measure |
|---|---|---|
| Intent | A visitor reaches the quick-start section | `quickstart_viewed` |
| Start | A visitor copies the Docker command | `quickstart_command_copied` |
| Activation | The demo emits `localization_zoo_demo: OK` and writes a valid `manifest.json` | Successful manifests / command copies |
| Second run | An activated user runs another profile or dataset | A second valid manifest with a different run configuration |
| Retention | An activated user produces a valid manifest in a later week | Weekly returning successful runners / activated cohort |

The website cannot prove that a copied command finished. Treat the copy event
as intent and the validated manifest as the activation source of truth.

## Browser-side events

`docs/index.html` dispatches `localization-zoo:activation` custom events for:

- `demo_cta_clicked`
- `method_explorer_clicked`
- `quickstart_viewed`
- `quickstart_platform_selected`
- `quickstart_command_copied`

The default implementation makes no network requests. It stores only event
counts plus the first and last event day in the browser's `localStorage` under
`localization-zoo-activation-v1`. Search terms, copied commands, referrers,
user-agent strings, and IP addresses are not stored. The quick-start panel lets
the visitor delete this site-specific record.

The custom event is the integration boundary for a future privacy-reviewed
analytics adapter. Adding an adapter must not silently change the no-network
promise shown on the page.

## Experiment scorecard

Record a baseline before changing another onboarding variable. For each release,
compare:

1. Quick-start views to command copies.
2. Command copies to shared valid manifests, where sharing is explicit.
3. Median time from demo start to the success marker in controlled smoke runs.
4. Quick-profile activations to broad-profile or own-data second runs.
5. Activated cohorts returning in a later calendar week.

Segment results by selected terminal only when the cohort is large enough to
avoid exposing an individual. Do not interpret stars or page views as retained
usage.
