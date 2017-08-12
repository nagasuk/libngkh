libngkh
=======

概要
----
C 及び C++ 向けの汎用ライブラリ．
"ngkh" に深い意味はない．
拡張子が `.c`，`.h` ならば C 向け，`.cpp`，`.hpp` ならば C++ 向けのライブラリ．

内容
----
* `print_progress`:  
  進捗を表示するクラス．（プログレスバーに近いが，少し違う）
* `subproc`:  
  サブプロセス（子プロセス）を生成する関数（C），並びにクラス（C++）．
  利用の際にライブラリのリンクが必要．（`-lngkh_subproc`）

利用方法
--------
### 個人のプロジェクトで利用
CMake の利用推奨．
利用するプロジェクトの CMakeLists.txt において `add_subdirectory` で本ディレクトリを追加するだけで OK．

### ライブラリのみをビルドする
```Bash
$ mkdir build && cd build
$ cmake ..
$ make all
```

