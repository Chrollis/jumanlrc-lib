# jumanlrc — Node.js Native Addon

Node.js binding for [JumanLRC-Lib](../README.md) — Japanese lyrics ruby (furigana) annotation with mora-level splitting.

## Prerequisites

- **Node.js** ≥ 24
- **CMake** ≥ 3.10
- **C++14 compiler** (MSVC on Windows)
- **Model file**: `jumandic.jppmdl` from the [official Juman++ release](https://github.com/ku-nlp/jumanpp/releases)

## Build (cmake-js)

```bash
cd node/

# Install dependencies (this also builds the native addon)
npm install

# Or build manually:
npm run build
```

## Usage

```js
const { JumanLRC } = require('jumanlrc');

const jpp = new JumanLRC();

// Load model
if (!jpp.loadModel('./jumandic.jppmdl')) {
  console.error(jpp.getError());
  process.exit(1);
}

// Word-level analysis
const words = jpp.analyze('私は学生です');
// → [{ surface: '私', reading: 'わたし' },
//     { surface: 'は', reading: 'は' },
//     { surface: '学生', reading: 'がくせい' },
//     { surface: 'です', reading: 'です' }]

// Ruby-ready analysis
const ruby = jpp.analyzeRuby('明日学校へ行く');

jpp.destroy(); // cleanup
```

## API

### `new JumanLRC()`

Create a new analyzer instance.

### `loadModel(modelPath, [options])`

Load a Juman++ model file.

- `modelPath` — path to `jumandic.jppmdl`
- `options.beamSize` — beam size (default: 5)
- `options.globalBeam` — global beam (default: 6)
- `options.rightBeam` — right beam (default: 5)
- `options.rightCheck` — right check (default: 1)

Returns `true` on success, `false` on failure.

### `analyze(sentence)`

Word-level analysis. Returns an array of `{surface, reading}` objects, or `null` on error.

### `analyzeRuby(sentence)`

Ruby-ready analysis. Returns an array of `{surface, readings[]}` objects.

### `getError()`

Returns the last error message string.

### `destroy()`

Release native resources.
