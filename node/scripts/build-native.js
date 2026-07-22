//
// build-native.js — Build the native JumanLRC addon using cmake-js
//
// Steps:
//   1. Configure parent CMake project (generates build system with codegen)
//   2. Build the codegen binary
//   3. Run codegen to produce jpp_jumandic_cg.cc / .h
//   4. Run cmake-js compile to build the .node addon
//

const { execSync } = require('child_process');
const path = require('path');
const fs = require('fs');

const ROOT = path.resolve(__dirname, '../..');
const BUILD_DIR = path.join(ROOT, 'build');
const NODE_DIR = path.resolve(__dirname, '..');
const GEN_DIR = path.join(BUILD_DIR, 'gen');

function run(cmd, cwd) {
  console.log(`> ${cmd}`);
  execSync(cmd, { cwd, stdio: 'inherit' });
}

function main() {
  // ── Step 1: Ensure parent CMake project is configured ─────────
  if (!fs.existsSync(path.join(BUILD_DIR, 'CMakeCache.txt'))) {
    console.log('[build-native] Configuring CMake...');
    run('cmake .. -DCMAKE_BUILD_TYPE=Release', BUILD_DIR);
  }

  // ── Step 2: Build codegen binary ──────────────────────────────
  const genHdr = path.join(GEN_DIR, 'jpp_jumandic_cg.h');
  if (!fs.existsSync(genHdr)) {
    console.log('[build-native] Building codegen binary...');
    run('cmake --build . --config Release --target jumandic_cg_binary', BUILD_DIR);
    // The custom command in CMakeLists.txt runs codegen automatically
    // after building the binary, so by now the files should exist.
    if (!fs.existsSync(genHdr)) {
      // Fallback: run codegen manually
      const exe = path.join(BUILD_DIR, 'Release', 'jumandic_cg_binary.exe');
      if (fs.existsSync(exe)) {
        run(`"${exe}" jpp_jumandic_cg JumandicStatic "${GEN_DIR}"`, BUILD_DIR);
      } else {
        console.error('[build-native] ERROR: codegen binary not found!');
        process.exit(1);
      }
    }
  }

  // ── Step 3: Ensure node-addon-api is installed ────────────────
  if (!fs.existsSync(path.join(NODE_DIR, 'node_modules', 'node-addon-api'))) {
    console.log('[build-native] Installing node-addon-api...');
    run('npm install --ignore-scripts', NODE_DIR);
  }

  // ── Step 4: Build the .node addon with cmake-js ───────────────
  console.log('[build-native] Building Node.js addon with cmake-js...');
  run('npx cmake-js compile', NODE_DIR);

  console.log('[build-native] Done!');
}

main();
