import re
import unittest
from pathlib import Path

README = Path(__file__).resolve().parents[1] / "README.md"


def github_slug(heading: str) -> str:
    """GitHub's heading anchor: lowercase, drop punctuation, spaces to hyphens."""
    text = re.sub(r"<[^>]+>", "", heading).strip().lower()
    text = re.sub(r"[^\w\- ]", "", text)
    return text.replace(" ", "-")


class ReadmeAnchorTests(unittest.TestCase):
    def test_in_page_links_point_at_headings(self) -> None:
        text = README.read_text()
        slugs = set()
        for line in text.splitlines():
            match = re.match(r"^#{1,6}\s+(.*)$", line)
            if match:
                base = github_slug(match.group(1))
                slug, n = base, 1
                while slug in slugs:
                    slug, n = f"{base}-{n}", n + 1
                slugs.add(slug)
        links = set(re.findall(r"\]\(#([^)]+)\)", text))
        self.assertTrue(links)
        self.assertEqual(sorted(links - slugs), [])

    def test_slug_matches_github_examples(self) -> None:
        self.assertEqual(github_slug("Leaderboard — odometry, RPE [drift %/100 m], lower is better"),
                         "leaderboard--odometry-rpe-drift-100-m-lower-is-better")
        self.assertEqual(github_slug("IMU Motion & Health SDK"), "imu-motion--health-sdk")


if __name__ == "__main__":
    unittest.main()
