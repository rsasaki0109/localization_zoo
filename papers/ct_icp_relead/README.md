# CT-ICP + RELEAD: Degeneracy-Aware Continuous-Time ICP

CT-ICPの連続時間レジストレーションにRELEADの退化検知を組み合わせたハイブリッド手法。
トンネルなどの退化環境でもCT-ICPのモーション補償を維持しつつ、退化方向のドリフトを防ぐ。

## アイデア

1. CT-ICPの対応関係からヘシアンを構築
2. RELEADの退化検知でローカライズ可能性を分析
3. 退化方向への更新を制約して最適化
