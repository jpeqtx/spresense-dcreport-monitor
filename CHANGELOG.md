# Changelog

## v1.1 - 2026-11-10

- `status` コマンドを追加（衛星・位置ブロックを、要求した時に1回だけ表示）
- `satellites period` コマンドを追加（周期ブロックの間隔を表示・変更。off で周期表示を停止）
- モードごとに表示間隔を持たせた（recommended と disaster は600秒、gnss と full は60秒）。おすすめ設定の衛星表示間隔を60秒から600秒へ変更

## v1.0 - 2026-11-10

初公開版。SPRESENSE メインボード（内蔵アンテナ、窓際）での実機動作を確認。

- MT43（災危通報）の全12カテゴリを平文表示（QZQSM ライブラリ）
- MT44（DCX）のデコードを追加し、Jアラート、Lアラート、自治体送信情報、海外組織情報を表示（QZSSDCX ライブラリ）
- 重複抑制（MT43 はデコード済み履歴20件、MT44 は内容バイト照合）
- DCX 空メッセージの表示抑制と件数表示
- 衛星状態と位置情報の60秒周期表示
- 表示項目の全面的なスイッチ化（27項目＋書式2項目、カテゴリ選択、通報区分選択、DCX 分類選択）
- 用途別モード7種（recommended、disaster、gnss、jalert、eew、full、min）
- コマンド体系を引数方式に統一（set、categories、priority、dcxclass、mode、show、help）
