# DcReportMonitorAll

SPRESENSE メインボードで、準天頂衛星みちびき（QZSS）の L1S 信号による災害・危機管理通報（災危通報）を全て受信し、平文で表示する Arduino スケッチです。

- MT43（災危通報）：気象庁防災情報の全12カテゴリを QZQSM ライブラリでデコード
- MT44（災危通報（拡張）／DCX）：Jアラート、Lアラート、自治体送信情報、海外組織情報を QZSSDCX ライブラリでデコード
- 受信と重複記録は常時全対象。表示は項目ごとに細かく切り替えでき、用途別のおすすめモードも用意しています

English version: [README.en.md](README.en.md)

## 特徴

- 全情報受信が原則。間引きは受信側では一切行わず、絞る判断は表示（および将来の下流）で行います
- 同じ通報の繰り返し配信は自動で抑制します（MT43はデコード済み履歴20件と照合、MT44は内容バイトで照合）
- DCX の空メッセージ（警報が無い時の定常メッセージ）は、表示せず件数だけ数えるのがデフォルトです
- 衛星状態（可視衛星、システム別内訳、使用衛星系、衛星ごとの仰角・方位角・C/N）と位置情報（フィックス状態、緯度経度、高度、速度、方位、DOP）を、モードで決まる間隔（おすすめ設定では600秒）ごとに表示できます。`status` コマンドで、その場で1回だけ表示することもできます
- 表示項目は27項目（オンオフ25項目＋書式2項目）に加え、カテゴリ、通報区分、DCX分類の3つの選択機能があり、シリアルモニタからコマンドで切り替えます
- 起動時のデフォルトはおすすめモード（recommended）です

## 動作環境

### ハードウェア

| 項目 | 内容 |
|---|---|
| ボード | SPRESENSE メインボード（CXD5602PWBMAIN1） |
| アンテナ | ボード内蔵のチップアンテナで受信可能（窓際で実証済み）。外部アンテナは不要 |
| ケーブル | データ通信対応の MicroUSB ケーブル |
| PC | Arduino IDE が動く PC（開発は Windows で実施） |

対象はメインボード内蔵 GNSS です。GNSS アドオンボード用ではありません。

### ソフトウェア

| 項目 | 内容 |
|---|---|
| ドライバ | Silicon Labs CP210x USB to UART Bridge ドライバ（Windows） |
| Arduino IDE | 2.x で確認 |
| ボードパッケージ | SPRESENSE 3.4.7（ボードマネージャから導入。初回のみブートローダーの書き込みが必要） |
| ボード設定 | ツール → ボード → Spresense Boards → Spresense、ポートはボードの COM ポート |
| シリアル | 115200 bps |

### ライブラリ

ライブラリマネージャから導入します。

| ライブラリ | 作者 | ライセンス | 用途 |
|---|---|---|---|
| QZQSM | baggio63446333 | BSD 3-Clause | MT43 のデコード |
| QZSSDCX | Switch Science | MIT | MT44（DCX）のデコード |

GNSS、GNSSPositionData、RTC はボードパッケージに同梱されています。

注意：QZSSDCX はライブラリ情報上 esp32 向けと表示され、コンパイル時にアーキテクチャの警告が出ますが、SPRESENSE でも動作します（実機確認済み）。

## セットアップ

1. CP210x ドライバをインストールし、ボードが COM ポートとして認識されることを確認する
2. Arduino IDE の追加のボードマネージャ URL に Spresense の URL を登録し、ボードマネージャから SPRESENSE をインストールする
3. 初回のみ、ツール → 書き込み装置 → Spresense Firmware Updater でブートローダーを書き込む
4. ライブラリマネージャから QZQSM と QZSSDCX をインストールする
5. このリポジトリの `DcReportMonitorAll/DcReportMonitorAll.ino` を開く
6. ボードとポートを選んで書き込む
7. シリアルモニタを 115200 bps で開き、ボードのリセットボタンを押して起動の頭から表示させる

## 使い方

起動すると、衛星を捕捉し、災危通報が届くたびに表示します。何も設定しなくても、おすすめモードで全カテゴリの通報が表示されます。

コマンドはシリアルモニタの送信欄に入力して送ります。改行で確定します。大文字小文字は区別しません。コマンドへの応答は `>>` で始まります。

### 現在の設定を見る

```
show
```

モード名と全項目の現在値を表示します。

```
help
```

コマンドの一覧を表示します。

### 衛星・位置ブロックの間隔とオンデマンド表示

```
status                 衛星・位置ブロックを1回だけ表示
satellites period      現在の表示間隔を表示
satellites period 300  表示間隔を300秒に変更
satellites period off  周期表示を停止（status で要求した時だけ表示）
```

表示間隔はモードごとに決まっています（recommended と disaster は600秒、gnss と full は60秒）。モードを適用すると、そのモードの間隔に戻ります。手で間隔を変更すると、モード表示は custom になります。

### モード（おすすめ設定の一括切り替え）

```
mode <モード名>
```

| モード名 | 短縮名 | 用途 | 内容 |
|---|---|---|---|
| recommended | rec | 通常の監視（デフォルト） | MT43 全文、DCX 要約。衛星ブロックあり（位置は出さない）。生データなし |
| disaster | dis | 災害時 | MT43 全文、DCX 要約＋対象地域。衛星は基数だけ |
| gnss | gnss | 受信状態の診断 | 衛星と位置の全項目、生データも表示 |
| jalert | jal | Jアラートと Lアラートだけ見たい時 | MT43 を出さず、DCX 要約＋対象地域。分類は lalert と jalert のみ |
| eew | eew | 緊急地震速報だけ見たい時 | MT43 のカテゴリ1のみ全文。DCX なし。周期ブロックなし |
| full | full | JPEQ 連携や記録用 | 全項目オン。両形式とも全表示 |
| min | min | 最小表示 | MT43 は1行目のみ、DCX 要約。見出しのみで他は全部オフ |

モードを適用した後に個別の項目を変更すると、モード表示は custom になります。

### 項目のオンオフ

```
set <項目名> on
set <項目名> off
```

項目名だけを送ると、その項目の現在値を表示します（例：`satellites.list`）。

#### 共通

| 項目名 | 内容 |
|---|---|
| header | 通報の見出し行（日時、MT、衛星数。MT43 では RC と DC も併記） |
| raw | 受信した生データの16進表示 |

#### MT43（災危通報）

| 項目名 | 内容 | 取りうる値 |
|---|---|---|
| report.format | 表示書式 | full＝全文、first＝1行目のみ、off＝表示しない |

カテゴリと通報区分の選択は、後述の `categories` と `priority` で行います。

#### MT44（DCX）

| 項目名 | 内容 | 取りうる値 |
|---|---|---|
| dcx.format | 表示書式 | summary＝要約、full＝全項目（ライブラリの全項目表示）、off＝表示しない |
| dcx.empty | 空メッセージそのものを表示するか | on / off |
| dcx.emptycount | 空メッセージの受信件数を衛星ブロックに表示するか | on / off |
| dcx.library | ガイダンスライブラリ（A9）、避難方向（A10）、ガイダンス（A11） | on / off |
| dcx.area | 対象範囲（A12〜A16）と追加対象範囲（Ex3〜Ex7） | on / off |
| dcx.target | 対象地域コード（Ex1）と対象地域（Ex8、Ex9） | on / off |
| dcx.settings | 使用設定（A17、A18）と対象地域コード使用設定（Ex2） | on / off |
| dcx.meta | 分類、衛星識別SD、バージョン、A1〜A8 の生コード | on / off |

分類の選択は、後述の `dcxclass` で行います。dcx.library 以降の詳細項目は、dcx.format が summary の時に、要約の後ろへ追加表示されます。full の時は全項目表示が優先されます。

#### 衛星（周期ブロック）

| 項目名 | 内容 |
|---|---|
| satellites.count | 可視衛星数と測位使用数 |
| satellites.systems | システム別内訳と使用衛星系 |
| satellites.list | 衛星ごとの明細行を出すか |
| satellites.elevation | 明細行の仰角列 |
| satellites.azimuth | 明細行の方位角列 |
| satellites.signal | 明細行の C/N 列（単位 dBHz） |

衛星ブロックは、satellites の項目が1つでもオンの時に、設定された間隔ごとに出ます。空メッセージ件数（dcx.emptycount）もこのブロックの末尾に出ます。

#### 位置（周期ブロック）

| 項目名 | 内容 |
|---|---|
| position.time | 時刻（JST） |
| position.fix | 測位モード（無効、2D、3D）と測位種別 |
| position.used | 測位使用衛星数 |
| position.latitude | 緯度（度、小数6桁） |
| position.longitude | 経度（度、小数6桁） |
| position.altitude | 標高（メートル） |
| position.velocity | 速度（m/s） |
| position.direction | 進行方位（度） |
| position.dop | DOP（PDOP、HDOP、VDOP、TDOP） |
| position.data | 測位データの有無 |

位置ブロックは、position の項目が1つでもオンの時に、衛星ブロックと同じ間隔で出ます。フィックスが取れていない時は、時刻とデータ有無の後ろに「No fix」と出ます。

### カテゴリの選択（MT43）

衛星は全カテゴリを一斉に流しており、受信機側で受信自体を絞る仕組みはありません。選択は表示の層で行います（受信と重複記録は常に全カテゴリです）。

```
categories            現在の選択を表示
categories all        全カテゴリ（デフォルト）
categories only 1,5   指定したカテゴリだけ
categories add 5      1つ追加
categories remove 5   1つ除外
```

| 番号 | カテゴリ | 番号 | カテゴリ |
|---|---|---|---|
| 1 | 緊急地震速報 | 8 | 火山 |
| 2 | 震源 | 9 | 降灰 |
| 3 | 震度 | 10 | 気象 |
| 4 | 南海トラフ地震 | 11 | 洪水 |
| 5 | 津波 | 12 | 台風 |
| 6 | 北西太平洋津波 | 14 | 海上 |

7 と 13 は未使用です。

### 通報区分の選択（MT43）

```
priority              現在の選択を表示
priority all          全区分（デフォルト）
priority only regular 通常だけ
priority add training 訓練・試験を追加
priority remove training 訓練・試験を除外
```

値は名前（maximum、priority、regular、training）でも数値（1、2、3、7）でも指定できます。

| 値 | 区分 |
|---|---|
| 1 | 最優先 |
| 2 | 優先 |
| 3 | 通常 |
| 7 | 訓練・試験 |

### DCX 分類の選択（MT44）

```
dcxclass              現在の選択を表示
dcxclass all          全分類（デフォルト）
dcxclass only jalert  Jアラートだけ
dcxclass only lalert,jalert  LアラートとJアラート
dcxclass add local    自治体送信情報を追加
dcxclass remove outside 海外組織情報を除外
```

| 値 | 分類 |
|---|---|
| lalert | Lアラート（避難情報など、提供は FMMC） |
| jalert | Jアラート（ミサイル発射情報など、提供は消防庁） |
| local | 自治体送信情報 |
| outside | 海外組織情報 |

空メッセージは分類の選択に関係なく、dcx.empty と dcx.emptycount で制御します。

### コマンドの例

緊急地震速報だけを全文で見たい：

```
mode eew
```

Jアラートと Lアラートを、対象地域付きの要約で見たい：

```
mode jalert
```

GNSS の状態を詳しく見たい：

```
mode gnss
```

おすすめモードを土台に、生データも追加したい：

```
mode recommended
set raw on
```

台風と海上だけ見たい：

```
categories only 12,14
```

訓練・試験の通報を止めたい：

```
priority remove training
```

## 出力例

```
DcReportMonitorAll (DC Report / DCX Receiver)
Display mode: recommended - 'help' for commands, 'show' for settings
-------- Satellite status --------
Visible: 6  Used for position: 0
GPS=5 GLONASS=0 SBAS=0 QZSS-L1CA=0 QZSS-L1S=1 BeiDou=0 Galileo=0
Used systems: none
  GPS ID=28 EL=0 AZ=0 C/N=37.5 dBHz
  QZSS-L1S ID=189 EL=0 AZ=0 C/N=21.6 dBHz
DCX empty messages received: 12
==== 2026/11/10 15:38:08 (JST) MT=43 SAT=6 QZ_L1S=1 ==== RC=3 DC=12
```

通報の本文は、MT43 は日本語の平文（発表時刻、カテゴリ、本文）、MT44 は分類見出し付きの要約（または全項目）で続きます。

## 仕様と参考資料

- IS-QZSS-DCR-017（災危通報の仕様、MT43）
- IS-QZSS-DCX-004（災危通報（拡張）の仕様、MT44 と CAMF）
- IS-QZSS-L1S-009（L1S 信号の仕様）

いずれも内閣府みちびき公式サイトの「パフォーマンススタンダード及びユーザインタフェース仕様書」ページで公開されています。

災危通報は L1S 信号（250 bps）で、4秒間隔で MT43 か MT44 のどちらか一方が配信されます。配信情報が無い時はヌルメッセージ（MT63）や DCX の空メッセージが流れます。

## ライセンス

このスケッチ自体は MIT License です（[LICENSE](LICENSE)）。

スケッチの一部は QZQSM ライブラリのサンプルスケッチ DcReportMonitor を土台にしており、その部分は BSD 3-Clause License（Copyright (c) 2019, baggio63446333）に従います。QZSSDCX ライブラリは MIT License（Copyright (c) 2024 Switch Science）で、本スケッチは API の利用のみです。詳細は [NOTICE.md](NOTICE.md) を参照してください。
