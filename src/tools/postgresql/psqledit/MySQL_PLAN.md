# MySQL対応 実行プラン

## 1. プロジェクト概要

### 現状
- **psqledit**: PostgreSQL用SQLエディタ（MFC/C++ Win32アプリケーション）
- **psqlgrid**: PostgreSQL用グリッド編集ツール
- **pglib**: PostgreSQL接続ライブラリ（libpq.dllを動的ロード）
- 開発環境: Visual Studio 2022, C++/MFC, Unicode対応, 64bit対応済み

### 目標
PostgreSQLと同等の機能を持つMySQL版ツール（**msqledit** / **msqlgrid**）を作成する。

---

## 2. アーキテクチャ分析

### 現在のレイヤー構成
```
┌─────────────────────────────────────┐
│  psqledit (SQLエディタ UI)           │  ← PostgreSQL固有のUI/ロジック
│  psqlgrid (グリッド編集 UI)          │
├─────────────────────────────────────┤
│  commonsrc (ログイン/接続情報)        │  ← PostgreSQL接続パラメータ依存
├─────────────────────────────────────┤
│  dbcommon (検索/フィルタダイアログ)    │  ← DB非依存（共有可能）
│  common (汎用ユーティリティ)          │  ← DB非依存（共有可能）
│  common_editor (エディタ共通)         │  ← DB非依存（共有可能）
├─────────────────────────────────────┤
│  pglib (PostgreSQL接続ライブラリ)     │  ← libpq.dll依存
├─────────────────────────────────────┤
│  libs/ (ostrutil, octrllib, etc.)    │  ← DB非依存（共有可能）
└─────────────────────────────────────┘
```

### PostgreSQL固有の依存箇所

| カテゴリ | ファイル | 依存内容 |
|---------|---------|---------|
| DB接続ライブラリ | `pglib/` 全体 | libpq.dll API（PQconnect, PQexec等） |
| データ型定義 | `pglib/pgtype.h` | PostgreSQL OID定数（INT4OID, TEXTOID等） |
| セッション管理 | `pglib/localdef.h` | PGconn, PGresultを使った構造体 |
| DLL動的ロード | `pglib/winpg.c` | libpq.dllの関数ポインタ取得 |
| 接続ダイアログ | `commonsrc/LoginDlg.*` | host/port/dbname/user/passwd接続文字列 |
| 接続情報管理 | `commonsrc/ConnectInfoDlg.*`, `ConnectList.*` | 接続先リスト管理 |
| オブジェクトリスト | `psqledit/ObjectBar.*` | pg_class, pg_tablesなどのシステムカタログ |
| オブジェクト詳細 | `psqledit/ObjectDetailBar.*` | pg_description, カラム情報取得 |
| ソース取得 | `psqledit/getsrc.cpp`, `getsrc2.cpp` | pg_proc, pg_views等からDDL再構成 |
| SQL実行 | `psqledit/localsql.*` | PostgreSQL固有SQL（search_path, pg_tables等） |
| グリッドデータ | `psqledit/pggriddata.*` | pg_oid型によるデータ型判定 |
| SQL解析 | `psqledit/sqlparse.*` | ドル引用符(`$$`)などPostgreSQL構文 |
| SQL入力補完 | `psqledit/sqlstrtoken.*` | pg_class relkindによるオブジェクト種別判定 |
| SQL補完設定 | `psqledit/completion_util.cpp` | relkind値（'r','v','i','S'）によるフィルタ |
| キーワード | `data/keywords.txt` | PostgreSQLキーワード |
| グリッド編集 | `psqlgrid/EditablePgGridData.*` | pkey取得、UPDATE/INSERT/DELETE SQL生成 |
| psql互換 | `psqledit/psql_util.*` | `\set`コマンド等 |
| SQL整形 | `psqledit/lintsql.*` | SQL整形（概ね共通だがMySQL方言確認必要） |

---

## 3. MySQL版で必要な対応

### 3.1 MySQL接続ライブラリ (`mylib/`) の新規作成

pglib相当のMySQL接続ライブラリを新規作成する。MySQL C API (`libmysql.dll`) を使用。

#### 新規ファイル一覧
| ファイル | 内容 |
|---------|------|
| `mylib/mylib.h` | 公開ヘッダ（HMySession, HMyDataset型定義） |
| `mylib/myapi.h` | API関数宣言（pgapi.hのMySQL版） |
| `mylib/mytype.h` | MySQLフィールド型定数（MYSQL_TYPE_LONG等） |
| `mylib/localdef.h` | 内部構造体定義（MYSQL*, MYSQL_RES*使用） |
| `mylib/myutil.c` | 接続/切断/バージョン取得/文字コード変換 |
| `mylib/mysql.c` | SQL実行（mysql_query, mysql_store_result等） |
| `mylib/myerr.c` | エラーメッセージ取得 |
| `mylib/mydataset.c` | 結果セット→独自データセット変換 |
| `mylib/dsetutil.c` | CSV/TSV出力 |
| `mylib/winmy.c` | libmysql.dllの動的ロード |
| `mylib/mymsg.h` | メッセージ定数 |
| `mylib/mylib.vcxproj` | Visual Studioプロジェクトファイル |

#### API対応表（pglib → mylib）

| pglib関数 | mylib関数 | MySQL C API |
|-----------|-----------|-------------|
| `pg_login()` | `my_login()` | `mysql_init()` + `mysql_real_connect()` |
| `pg_logout()` | `my_logout()` | `mysql_close()` |
| `pg_exec_sql()` | `my_exec_sql()` | `mysql_query()` |
| `pg_create_dataset()` | `my_create_dataset()` | `mysql_store_result()` + データ取得 |
| `pg_free_dataset()` | `my_free_dataset()` | `mysql_free_result()` |
| `pg_dataset_row_cnt()` | `my_dataset_row_cnt()` | 内部カウント |
| `pg_dataset_col_cnt()` | `my_dataset_col_cnt()` | `mysql_num_fields()` |
| `pg_dataset_data()` | `my_dataset_data()` | 内部バッファ |
| `pg_dataset_get_colname()` | `my_dataset_get_colname()` | `MYSQL_FIELD.name` |
| `pg_dataset_get_coltype()` | `my_dataset_get_coltype()` | `MYSQL_FIELD.type` |
| `pg_dataset_is_null()` | `my_dataset_is_null()` | 内部NULLフラグ |
| `pg_explain_plan()` | `my_explain_plan()` | `EXPLAIN <sql>` 実行 |
| `pg_get_remote_version()` | `my_get_remote_version()` | `mysql_get_server_info()` |
| `pg_user()` | `my_user()` | 接続時のユーザー名保持 |
| `pg_host()` | `my_host()` | 接続時のホスト名保持 |
| `pg_db()` | `my_db()` | 接続時のDB名保持 |
| `pg_port()` | `my_port()` | 接続時のポート保持 |
| `pg_init_library()` | `my_init_library()` | `libmysql.dll` の動的ロード |
| `pg_free_library()` | `my_free_library()` | DLL解放 |
| `pg_is_ssl_mode()` | `my_is_ssl_mode()` | `mysql_get_ssl_cipher()` |
| `pg_auto_commit_off()` | `my_auto_commit_off()` | `mysql_autocommit(conn, 0)` |
| `pg_commit()` | `my_commit()` | `mysql_commit()` |
| `pg_rollback()` | `my_rollback()` | `mysql_rollback()` |
| `pg_notice()` | `my_notice()` | `mysql_warning_count()` + `SHOW WARNINGS` |
| `pg_parameter_status()` | `my_parameter_status()` | `SHOW VARIABLES LIKE ...` |

#### 接続文字列の違い

| 項目 | PostgreSQL (pglib) | MySQL (mylib) |
|------|-------------------|---------------|
| 接続方法 | `PQconnectdb(conninfo)` 文字列 | `mysql_real_connect()` 個別パラメータ |
| デフォルトポート | 5432 | 3306 |
| 文字コード | `client_encoding`パラメータ | `mysql_set_character_set("utf8mb4")` |
| SSL | PQgetssl() | `mysql_ssl_set()` / `MYSQL_OPT_SSL_MODE` |
| 非同期通知 | `LISTEN/NOTIFY` | なし（MySQL未対応、削除） |

#### 非同期SQL実行

pglibは`PQsendQuery()` + `PQgetResult()`で非同期実行しているが、MySQL C APIには直接対応がない。
- **方式**: 別スレッドで`mysql_query()` + `mysql_store_result()`を実行
- 既存のキャンセルダイアログのスレッド構造は流用可能
- `mysql_kill()`または新しい接続からの`KILL QUERY <id>`でキャンセル実装

### 3.2 ログイン/接続 (`commonsrc/` のMySQL版)

#### 新規ディレクトリ: `src/tools/mysql/commonsrc/`

| ファイル | 変更内容 |
|---------|---------|
| `LoginDlg.h/.cpp` | `HPgSession` → `HMySession`, ポートデフォルト3306, `option`欄削除/charset欄追加 |
| `ConnectInfoDlg.h/.cpp` | 接続情報フィールドをMySQL仕様に変更 |
| `ConnectList.h/.cpp` | レジストリ保存のキー名変更 |
| `common.h/.cpp` | バージョン情報表示のMySQL対応 |

#### ログインダイアログの変更点
- **削除**: PostgreSQL固有の`option`欄
- **追加**: `charset`選択（utf8mb4等）
- **変更**: デフォルトポートを`3306`に
- **変更**: 接続文字列形式（個別パラメータに分解）

### 3.3 SQLエディタ (`msqledit/`)

`src/tools/postgresql/psqledit/` を `src/tools/mysql/msqledit/` にコピーして改修。

#### 3.3.1 オブジェクトリスト (`ObjectBar`)

**PostgreSQL版のオブジェクト種別**:
TABLE, FOREIGN TABLE, INDEX, VIEW, MATERIALIZED VIEW, SEQUENCE, FUNCTION, PROCEDURE, TRIGGER, TYPE

**MySQL版のオブジェクト種別**:
TABLE, VIEW, INDEX, FUNCTION, PROCEDURE, TRIGGER, EVENT

| 対象 | PostgreSQLクエリ | MySQLクエリ |
|------|-----------------|-------------|
| ユーザー一覧 | `SELECT usename FROM pg_user` | `SELECT user FROM mysql.user` または `SELECT DISTINCT table_schema FROM information_schema.tables` |
| テーブル一覧 | `pg_tables` | `SHOW TABLES` または `information_schema.tables WHERE table_type='BASE TABLE'` |
| ビュー一覧 | `pg_views` | `information_schema.views` |
| インデックス一覧 | `pg_indexes` | `SHOW INDEX FROM <table>` または `information_schema.statistics` |
| 関数一覧 | `pg_proc WHERE prokind='f'` | `information_schema.routines WHERE routine_type='FUNCTION'` |
| プロシージャ一覧 | `pg_proc WHERE prokind='p'` | `information_schema.routines WHERE routine_type='PROCEDURE'` |
| トリガー一覧 | `pg_trigger` | `information_schema.triggers` |
| シーケンス一覧 | `pg_class WHERE relkind='S'` | なし（MySQL 8.0にはシーケンスなし、MariaDBのみ） |
| イベント一覧 | なし | `information_schema.events`（MySQL固有追加） |
| カラム一覧 | `pg_attribute + pg_type` | `SHOW COLUMNS FROM <table>` または `information_schema.columns` |
| コメント | `pg_description` | `information_schema.tables.TABLE_COMMENT`, `information_schema.columns.COLUMN_COMMENT` |

#### 3.3.2 オブジェクト詳細バー (`ObjectDetailBar`)

| 詳細情報 | 変更内容 |
|---------|---------|
| カラムリスト | `information_schema.columns` を使用 |
| インデックスリスト | `SHOW INDEX FROM <table>` を使用 |
| トリガーリスト | `information_schema.triggers` を使用 |
| シーケンス | 削除（MySQL非対応） |

#### 3.3.3 ソース取得機能 (`getsrc.cpp`, `getsrc2.cpp`) → 全面書き換え

| オブジェクト | PostgreSQL方式 | MySQL方式 |
|------------|---------------|-----------|
| TABLE | `pg_attribute`, `pg_constraint`等から再構成 | `SHOW CREATE TABLE <name>` |
| VIEW | `pg_views.definition` | `SHOW CREATE VIEW <name>` |
| FUNCTION | `pg_proc.prosrc` + 引数組立 | `SHOW CREATE FUNCTION <name>` |
| PROCEDURE | `pg_proc.prosrc` + 引数組立 | `SHOW CREATE PROCEDURE <name>` |
| TRIGGER | `pg_trigger`の各フィールドから再構成 | `SHOW CREATE TRIGGER <name>` |
| INDEX | `pg_indexes.indexdef` | `SHOW CREATE TABLE <name>` からINDEX部分抽出、または `SHOW INDEX` |
| TYPE | `pg_type`から再構成 | なし（削除） |
| EVENT | なし | `SHOW CREATE EVENT <name>`（追加） |

MySQLでは`SHOW CREATE ...`で簡単にソース取得できるため、getsrc.cpp/getsrc2.cppは**大幅に簡略化**される。

#### 3.3.4 SQL実行関連 (`localsql.cpp`)

| 関数 | 変更内容 |
|------|---------|
| `show_search_path()` | 削除（MySQL非対応）→ `SELECT DATABASE()` でカレントDB取得 |
| `set_search_path()` | 削除 → `USE <database>` に置換 |
| `get_table_schema_name()` | `information_schema.tables`で検索 |
| `get_object_list()` | `information_schema`系クエリに全面書き換え |
| `get_user_list()` | データベース一覧（`SHOW DATABASES`）に変更 |
| `get_column_list()` | `SHOW COLUMNS FROM <table>` に変更 |
| `get_index_list_by_table()` | `SHOW INDEX FROM <table>` に変更 |
| `get_trigger_list_by_table()` | `information_schema.triggers` に変更 |
| `get_object_properties()` | 各オブジェクト型に対応するMySQL情報スキーマクエリ |
| `explain_plan()` | `EXPLAIN <sql>` をそのまま実行（形式は類似） |
| `download()` | データセット保存（pglib APIをmylib APIに置換するのみ） |

#### 3.3.5 グリッドデータ (`pggriddata.*` → `mygriddata.*`)

| 変更点 | 内容 |
|-------|------|
| 型判定 | `pg_oid` → `enum_field_types`（MYSQL_TYPE_LONG等）による数値型判定 |
| NULLデータ | `pg_dataset_is_null()` → `my_dataset_is_null()` |
| データ取得 | API名を`pg_*` → `my_*`に置換 |

#### 3.3.6 SQL解析 (`sqlparse.*`)

| 変更点 | 内容 |
|-------|------|
| ドル引用符 `$$` | 削除（MySQL非対応） |
| バッククォート `` ` `` | 追加（MySQL識別子引用符） |
| デリミタ `DELIMITER` | 追加（MySQL固有のデリミタ変更構文への対応） |
| `#` 行コメント | 追加（MySQLでは`#`も行コメント） |

#### 3.3.7 SQL入力支援 (`sqlstrtoken.*`)

| 変更点 | 内容 |
|-------|------|
| キーワード補完用データ | `information_schema.tables`, `information_schema.columns`からテーブル名・カラム名取得 |
| オブジェクト種別フィルタ | `relkind`値 → `TABLE_TYPE`値に変更 |
| スキーマ補完 | `schema.table` → `database.table` に対応 |

#### 3.3.8 キーワードファイル

`data/keywords.txt` のMySQL版（`data/mysql_keywords.txt`）を作成。
MySQL 8.0の予約語・非予約語を収録。

#### 3.3.9 psql互換コマンド (`psql_util.*`)

| コマンド | 対応 |
|---------|------|
| `\set` | 変数展開機能として流用可能 |
| `show` コマンド | `SHOW VARIABLES`, `SHOW STATUS` 等に対応 |
| `@` ファイル実行 | `SOURCE` コマンドとして対応可能 |

#### 3.3.10 削除・無効化する機能

| 機能 | 理由 |
|------|------|
| 非同期通知（LISTEN/NOTIFY） | MySQL非対応 |
| search_path管理 | MySQL非対応（USE databaseで代替） |
| シーケンスリスト/プロパティ | MySQL 8.0非対応 |
| TYPE一覧 | MySQL非対応 |
| MATERIALIZED VIEW | MySQL非対応 |
| FOREIGN TABLE | MySQL非対応（FEDERATED Engineで類似だが省略） |
| ドル引用符 `$$` | MySQL非対応 |

### 3.4 グリッド編集ツール (`msqlgrid/`)

`src/tools/postgresql/psqlgrid/` を `src/tools/mysql/msqlgrid/` にコピーして改修。

#### 3.4.1 `EditablePgGridData` → `EditableMyGridData`

| 変更点 | 内容 |
|-------|------|
| 主キー取得 | `pg_index + pg_attribute` → `SHOW INDEX FROM <table> WHERE Key_name='PRIMARY'` |
| UPDATE文生成 | PostgreSQL構文 → MySQL構文（概ね同一だが、引用符を`` ` ``に） |
| INSERT文生成 | 同上 |
| DELETE文生成 | 同上 |
| データロック | `SELECT ... FOR UPDATE` は同一 |
| トランザクション | `BEGIN/COMMIT/ROLLBACK` は同一 |
| データ型判定 | `pg_oid` → `MYSQL_TYPE_*` |

#### 3.4.2 テーブルリストダイアログ (`TableListDlg`)

- テーブル一覧取得を`SHOW TABLES`または`information_schema.tables`に変更

---

## 4. ディレクトリ構成（変更後）

```
src/
├── libs/
│   ├── pglib/                  ← 既存（変更なし）
│   ├── mylib/                  ← ★新規作成（MySQL接続ライブラリ）
│   ├── common_scmlib/          ← 既存（共有）
│   ├── octrllib/               ← 既存（共有）
│   ├── ofileutil/              ← 既存（共有）
│   └── ostrutil/               ← 既存（共有）
├── tools/
│   ├── common/                 ← 既存（共有）
│   ├── common_editor/          ← 既存（共有）
│   ├── dbcommon/               ← 既存（共有）
│   ├── postgresql/             ← 既存（変更なし）
│   │   ├── commonsrc/
│   │   ├── psqledit/
│   │   └── psqlgrid/
│   └── mysql/                  ← ★新規作成
│       ├── commonsrc/          ← ★LoginDlg, ConnectInfoDlg等のMySQL版
│       ├── msqledit/           ← ★SQLエディタ（MySQL版）
│       └── msqlgrid/           ← ★グリッド編集（MySQL版）
```

---

## 5. 実装フェーズ

### フェーズ1: MySQL接続ライブラリ（mylib）の作成 [推定: 2-3週間]

1. `mylib/` ディレクトリ作成、vcxprojファイル作成
2. `winmy.c` — libmysql.dll動的ロード実装
3. `myutil.c` — 接続/切断、バージョン取得、文字コード変換
4. `mysql.c` — SQL実行（同期・スレッド経由の非同期）
5. `mydataset.c` — 結果セット→独自データセット変換
6. `myerr.c` — エラーメッセージ取得
7. `dsetutil.c` — CSV/TSV出力
8. 単体テスト（接続→クエリ実行→結果取得→切断の一連動作確認）

### フェーズ2: 接続・ログイン機能（commonsrc MySQL版）[推定: 1週間]

1. `commonsrc/`のファイルをコピーしてMySQL版に改修
2. `LoginDlg` — デフォルトポート3306、charset選択、option欄変更
3. `ConnectInfoDlg` — 接続情報フィールドのMySQL対応
4. `ConnectList` — レジストリキー名変更
5. 接続→ログイン→切断の動作確認

### フェーズ3: SQLエディタ基本機能（msqledit）[推定: 3-4週間]

1. `psqledit/`をコピーして`msqledit/`作成
2. 全ファイルの`#include "pglib.h"` → `#include "mylib.h"` 置換
3. `HPgSession` → `HMySession`, `HPgDataset` → `HMyDataset` 型名置換
4. `pg_*` API呼び出しを `my_*` に置換
5. `localsql.cpp` — オブジェクトリスト取得クエリをMySQL用に書き換え
6. `ObjectBar.cpp` — オブジェクト種別リストをMySQL用に変更
7. `ObjectDetailBar.cpp` — カラムリスト等の取得クエリ変更
8. `sqlparse.cpp` — ドル引用符削除、バッククォート追加、DELIMITER対応
9. `sqlstrtoken.cpp` — キーワード補完クエリ変更
10. `pggriddata.*` → `mygriddata.*` — データ型判定変更
11. MySQLキーワードファイル作成
12. SQL実行→結果表示の基本動作確認

### フェーズ4: ソース取得・高度機能 [推定: 2週間]

1. `getsrc.cpp` — `SHOW CREATE TABLE/VIEW/FUNCTION/PROCEDURE/TRIGGER`による実装
2. `getsrc2.cpp` — テーブルDDL取得を`SHOW CREATE TABLE`に簡略化
3. `lintsql.cpp` — SQL整形のMySQL方言対応確認
4. `completion_util.cpp` — オブジェクト種別フィルタ変更
5. 実行計画取得（`EXPLAIN`）の動作確認
6. `psql_util.cpp` — `\set`コマンド等の動作確認

### フェーズ5: グリッド編集ツール（msqlgrid）[推定: 2-3週間]

1. `psqlgrid/`をコピーして`msqlgrid/`作成
2. API名・型名の全置換
3. `EditableMyGridData` — 主キー取得、SQL生成のMySQL対応
4. `TableListDlg` — テーブルリスト取得変更
5. データ編集（INSERT/UPDATE/DELETE）の動作確認
6. トランザクション管理の動作確認

### フェーズ6: テスト・調整 [推定: 2週間]

1. MySQL 8.0/8.4での動作確認
2. 日本語データの文字コード確認（utf8mb4）
3. 大量データでの動作確認
4. エッジケーステスト（NULL、長い文字列、特殊文字等）
5. MariaDB互換性の簡易確認（余裕があれば）

---

## 6. 技術的な注意点

### 6.1 libmysql.dll の動的ロード

pglib同様、`LoadLibrary` + `GetProcAddress`方式で実装する。

主要な関数ポインタ:
```c
// 接続
mysql_init, mysql_real_connect, mysql_close, mysql_options,
mysql_set_character_set, mysql_ssl_set

// クエリ実行
mysql_query, mysql_real_query, mysql_store_result, mysql_free_result,
mysql_use_result

// 結果取得
mysql_num_rows, mysql_num_fields, mysql_fetch_row, mysql_fetch_lengths,
mysql_fetch_field, mysql_fetch_fields, mysql_field_count

// エラー
mysql_error, mysql_errno

// トランザクション
mysql_autocommit, mysql_commit, mysql_rollback

// 情報取得
mysql_get_server_info, mysql_get_client_info, mysql_thread_id,
mysql_get_ssl_cipher, mysql_warning_count, mysql_info

// キャンセル
mysql_kill (または別接続から KILL QUERY <id>)
```

### 6.2 MySQLデータ型マッピング (`mytype.h`)

```c
// MySQL C API の enum_field_types を使用
// 数値型として判定するもの:
// MYSQL_TYPE_TINY, MYSQL_TYPE_SHORT, MYSQL_TYPE_LONG, MYSQL_TYPE_LONGLONG
// MYSQL_TYPE_FLOAT, MYSQL_TYPE_DOUBLE, MYSQL_TYPE_DECIMAL, MYSQL_TYPE_NEWDECIMAL
// MYSQL_TYPE_INT24
```

### 6.3 識別子引用符

| DB | 引用符 | 例 |
|---|-------|-----|
| PostgreSQL | `"` (ダブルクォート) | `"TableName"` |
| MySQL | `` ` `` (バッククォート) | `` `TableName` `` |

`SELECT文作成`機能や`psqlgrid起動`機能で、識別子をバッククォートで囲むように変更が必要。

### 6.4 スキーマ vs データベース

| PostgreSQL | MySQL |
|-----------|-------|
| 1接続 = 1データベース + 複数スキーマ | 1接続 = 複数データベース |
| `schema.table` | `database.table` |
| `search_path` | `USE database` |
| `pg_catalog` | `information_schema` |

オブジェクトリストの「Owner」ドロップダウンは、MySQLでは「Database」ドロップダウンとして実装する。

### 6.5 文字コード

- PostgreSQLは`client_encoding`パラメータで指定
- MySQLは`mysql_set_character_set("utf8mb4")`で指定
- `oci_str_to_win_str()` / `win_str_to_oci_str()` のUTF-8⇔TCHAR変換ロジックはそのまま流用可能

### 6.6 autocommit

- PostgreSQLはデフォルトautocommit ON、pglibで`BEGIN`を発行してOFFにする
- MySQLは`mysql_autocommit(conn, 0)`でOFFにできる
- 既存のautocommit設定オプションは概念的に流用可能

---

## 7. リソース（リソースファイル `.rc`）の変更

| 変更対象 | 内容 |
|---------|------|
| アプリケーション名 | psqledit → msqledit, psqlgrid → msqlgrid |
| ダイアログタイトル | 「PostgreSQL」→「MySQL」 |
| バージョン情報 | 製品名・著作権等の変更 |
| ログインダイアログ | option欄削除、charset欄追加 |
| アイコン | MySQL向けに変更（任意） |
| メニュー | 「psqlgridを起動」→「msqlgridを起動」 |
| オブジェクト種別コンボ | MySQL対応種別に変更 |

---

## 8. 工数見積もり概要

| フェーズ | 内容 | 推定工数 |
|---------|------|---------|
| 1 | mylib（MySQL接続ライブラリ）| 2-3週間 |
| 2 | commonsrc（接続・ログイン）| 1週間 |
| 3 | msqledit（SQLエディタ基本）| 3-4週間 |
| 4 | ソース取得・高度機能 | 2週間 |
| 5 | msqlgrid（グリッド編集）| 2-3週間 |
| 6 | テスト・調整 | 2週間 |
| **合計** | | **12-15週間** |

---

## 9. 補足：共通化の検討（将来的なリファクタリング）

現在のアーキテクチャは、PostgreSQL専用に設計されている。将来的にDB抽象化レイヤーを導入してPostgreSQL/MySQL両対応にする場合は以下を検討:

1. **DB抽象化インターフェース** (`dblib/`) — `HDbSession`, `HDbDataset` 等の共通型・共通API
2. **プラグイン方式** — DLLベースでDB固有の実装を切り替え
3. **共通UIレイヤー** — `dbedit/` として1つのSQLエディタから複数DB接続

ただし、現段階ではPostgreSQLコードベースをコピーしてMySQL固有部分を書き換える方式が最も確実で効率的である。
