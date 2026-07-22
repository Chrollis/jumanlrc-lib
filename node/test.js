//
// test.js — Basic test for the jumanlrc Node.js addon
//
// Usage:
//   node test.js --model <path-to-jumandic.jppmdl> [--text "テスト文"]
//

const { JumanLRC } = require('./index');

function parseArgs() {
  const args = {};
  for (let i = 2; i < process.argv.length; i++) {
    switch (process.argv[i]) {
      case '--model':
        args.model = process.argv[++i];
        break;
      case '--text':
        args.text = process.argv[++i];
        break;
      case '--katakana':
        args.katakana = true;
        break;
    }
  }
  return args;
}

function main() {
  const args = parseArgs();

  if (!args.model) {
    console.error(
      'Usage: node test.js --model <path-to-jumandic.jppmdl> [--text "文"]',
    );
    process.exit(1);
  }

  const jpp = new JumanLRC();
  console.log(`Loading model: ${args.model}`);
  const ok = jpp.loadModel(args.model);
  if (!ok) {
    console.error(`Failed to load model: ${jpp.getError()}`);
    process.exit(1);
  }
  if (args.katakana) {
    jpp.setKatakanaAnnotation(true);
    console.log('Katakana annotation: enabled\n');
  } else {
    console.log('Katakana annotation: disabled (use --katakana to enable)\n');
  }

  const sentences = args.text
    ? [args.text]
    : ['私は学生です', '明日学校へ行く', 'ラーメンを食べる'];

  for (const sentence of sentences) {
    console.log(`Input: ${sentence}`);

    // Word-level analysis
    const words = jpp.analyze(sentence);
    if (words) {
      console.log('  Word analysis:');
      for (const w of words) console.log(`    ${w.surface} -> ${w.reading}`);
    } else {
      console.error(`  Error: ${jpp.getError()}`);
    }

    // Ruby (mora) analysis
    const ruby = jpp.analyzeRuby(sentence);
    if (ruby) {
      console.log('  Ruby analysis:');
      const parts = ruby.map((u) => {
        if (u.readings.length) return `${u.surface}(${u.readings.join(',')})`;
        return u.surface;
      });
      console.log('    ' + parts.join(' '));
    } else {
      console.error(`  Ruby error: ${jpp.getError()}`);
    }

    console.log('');
  }

  jpp.destroy();
  console.log('Done.');
}

main();
