# JumanLRC-Lib

Static library for Japanese lyrics ruby (furigana) annotation with **mora-level splitting**.
Derived from [Juman++ v2.0.0-rc4](https://github.com/ku-nlp/jumanpp).

## Build

```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

Output: `build/Release/jumanlrc_lib.lib`

Requires: CMake ≥ 3.10, C++14, model file `jumandic.jppmdl` (from the [official release](https://github.com/ku-nlp/jumanpp/releases)).

### Example program

An example program `json_example` is available under `examples/`, using [nlohmann/json](https://github.com/nlohmann/json) (header-only, bundled in `examples/json/`) to format analysis results as JSON.

```bash
mkdir build && cd build
cmake .. -DJUMANLRC_BUILD_EXAMPLES=ON
cmake --build . --config Release

./Release/json_example --model jumandic.jppmdl --text "私は学生です"
```

## C API

```c
#include "jumanpp_capi.h"

jumanpp_handle* h = jumanpp_init("jumandic.jppmdl");

char** surfaces;
char** readings;
int count = jumanpp_analyze(h, "私は学生です", &surfaces, &readings);
// surfaces[0]="私"    readings[0]="わたし"
// surfaces[1]="は"    readings[1]="は"
// surfaces[2]="学生"  readings[2]="がくせい"
// surfaces[3]="です"  readings[3]="です"

jumanpp_free_result(surfaces, readings, count);
jumanpp_destroy(h);
```

| Function                                        | Description           |
| ----------------------------------------------- | --------------------- |
| `jumanpp_init(path)`                            | Load model            |
| `jumanpp_init_ex(path,beam,global,right,check)` | Load with custom beam |
| `jumanpp_analyze(h,text,&surf,&read)`           | Analyze → word count  |
| `jumanpp_free_result(surf,read,count)`          | Free results          |
| `jumanpp_error(h)`                              | Last error            |
| `jumanpp_destroy(h)`                            | Cleanup               |

## C++ API

### Word-level

```cpp
#include "jumanpp_lib.h"

jumanpp::lib::JumanppLib jpp;
jpp.loadModel("jumandic.jppmdl");
jpp.analyze("私は学生です");
for (auto& w : jpp.lastResult())
    printf("%s -> %s\n", w.surface.c_str(), w.reading.c_str());
```

### Ruby (mora) analysis

```cpp
jpp.setKatakanaAnnotation(false);  // default: no reading for katakana
jpp.analyzeRuby("明日学校へ行く");

for (auto& u : jpp.lastRubyResult()) {
    if (u.readings.empty())
        printf("%s ", u.surface.c_str());         // kana
    else {
        printf("%s(", u.surface.c_str());          // kanji
        for (auto& r : u.readings)
            printf("%s,", r.c_str());              // mora
        printf(") ");
    }
}
// → 明日(あ,す) 学校(が,っ,こ,う) へ 行(い) く
```

| Method                        | Description                           |
| ----------------------------- | ------------------------------------- |
| `loadModel(path, ...)`        | Load model with optional beam params  |
| `analyze(text)`               | Word-level analysis                   |
| `analyzeRuby(text)`           | Mora-level ruby analysis              |
| `setKatakanaAnnotation(bool)` | Toggle katakana → hiragana annotation |
| `lastResult()`                | `vector<WordResult>`                  |
| `lastRubyResult()`            | `vector<RubyUnit>`                    |

## Node.js API

Native Node.js addon (N-API) is available under [`node/`](node/).

```bash
cd node
npm install
```

```js
const { JumanLRC } = require('./node');

const jpp = new JumanLRC();
jpp.loadModel('jumandic.jppmdl');

// Word-level
const words = jpp.analyze('私は学生です');
// → [{surface:'私', reading:'わたし'}, ...]

// Ruby-ready
const ruby = jpp.analyzeRuby('明日学校へ行く');
// → [{surface:'明日', readings:['あ','す']}, ...]

jpp.destroy();
```

| Method                        | Returns        | Description                       |
| ----------------------------- | -------------- | --------------------------------- |
| `loadModel(path, opts?)`      | `boolean`      | Load model (beam params optional) |
| `analyze(text)`               | `WordResult[]` | Word-level analysis               |
| `analyzeRuby(text)`           | `RubyUnit[]`   | Mora-level ruby analysis          |
| `setKatakanaAnnotation(bool)` | `void`         | Toggle katakana → hiragana        |
| `getError()`                  | `string`       | Last error message                |
| `destroy()`                   | `void`         | Release resources                 |

TypeScript declarations included (`index.d.ts`).

## Output examples

| Text                       | `analyzeRuby` output                                       |
| -------------------------- | ---------------------------------------------------------- |
| 私は学生です               | `私(わ,た,し) は 学生(が,く,せ,い) で す`                  |
| 東京大学教授               | `東京(と,う,きょ,う) 大学(だ,い,が,く) 教授(きょ,う,じゅ)` |
| ちょっと待って             | `ちょ っ と 待(ま) っ て`                                  |
| アーケードで遊ぶ           | `ア ー ケ ー ド で 遊(あ,そ) ぶ`                           |
| 明日学校へ行く             | `明日(あ,す) 学校(が,っ,こ,う) へ 行(い) く`               |
| ラーメンを食べる (kata-on) | `ラ(ら) ー メ(め) ン(ん) を 食(た) べ る`                  |

### Mora splitting rules

| Pattern                 | Rule                     | Example                |
| ----------------------- | ------------------------ | ---------------------- |
| 拗音 (yōon)             | 1 mora                   | `きょ`, `しゅ`, `ちょ` |
| 促音 (sokuon)           | independent              | `っ`, `ッ`             |
| 長音 (chōon)            | independent              | `ー`                   |
| 送り仮名 (okurigana)    | separated                | `行(い) く`            |
| 漢字語 (kanji compound) | whole word, mora reading | `学校(が,っ,こ,う)`    |

### Data structures

```cpp
struct WordResult { string surface; string reading; };

struct RubyUnit {
    string surface;
    vector<string> readings;  // empty if surface==reading (kana match)
};
```

## Project layout

```
├── CMakeLists.txt
├── CHANGELOG
├── LICENSE           (Apache 2.0)
├── include/          → C API header, manual config headers
├── src/              → core engine, C++ wrapper, C API impl
└── libs/             → pathie-cpp, pegtl
```
