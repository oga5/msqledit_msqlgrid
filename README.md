# msqledit / msqlgrid

## 概要

このリポジトリには、2つのプロジェクトが含まれています：

1. **msqledit** - MySQL用SQLエディタ  
   - SQLの編集や実行に特化したエディタです。

2. **msqlgrid** - MySQL用グリッドビューア  
   - データベースのテーブルデータをグリッド形式で表示・編集できるツールです。

これらはPostgreSQL用ツール（psqledit / psqlgrid）をベースにMySQL対応したものです。

## ライセンス

このプロジェクトはGNU General Public License v2.0 or later の下でライセンスされています。詳細は `LICENSE` をご覧ください。

元になったPostgreSQL用ツール（psqledit / psqlgrid）のコードはBSD 2-Clause Licenseの下でライセンスされています。詳細は `LICENSE_BSD.txt` をご覧ください。

MySQL対応部分（`src/libs/mylib/`、`src/tools/mysql/`）の各ソースファイルには、GPL v2.0 or later と元のBSD License の両方のライセンス表記が記載されています。

## ダウンロード

ビルド済みの実行ファイルは以下からダウンロードできます。  
[https://code73.mydns.jp/a_ogawa/](https://code73.mydns.jp/a_ogawa/)

## ビルド方法

### 必要環境

- Visual Studio 2026

### ビルド手順

#### msqleditのビルド

1. Visual Studioで `src\tools\mysql\msqledit\msqledit.sln` を開きます
2. ビルドを実行します

#### msqlgridのビルド

1. Visual Studioで `src\tools\mysql\msqlgrid\msqlgrid.sln` を開きます
2. ビルドを実行します

ビルドが成功すると、リポジトリのルートに以下のフォルダが作成されます：
- `exe` - 実行ファイルが格納されます
- `lib` - ライブラリファイルが格納されます
- `obj` - 中間オブジェクトファイルが格納されます

実行ファイルは `exe` フォルダ内の対応するビルド構成とプラットフォームのサブディレクトリに出力されます。

### 実行方法

実行するにはlibmysql.dllが必要です。
exeファイルと同じ場所においてください。

## ロードマップ

本プロジェクトは「シンプルな機能を維持しながら、高速化・操作性の改善・デザインの改善」を目指しています。  
今後の主な開発方針は以下の通りです。

- シンプルな機能の維持
- 不要な機能の削除
- 起動・動作速度の高速化
- 操作性（UI/UX）の改善
- デザイン（見た目）の改善
- テストコードの充実
- ドキュメントの充実

ご意見やご要望があれば、Issueでお知らせください。
