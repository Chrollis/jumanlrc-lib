//
// index.js — High-level JavaScript wrapper for jumanlrc native addon
//

const addon = require('./build/Release/jumanlrc_node.node');

/**
 * JumanLRC — Japanese morphological analyzer for lyrics ruby annotation.
 *
 * @example
 * const jumanlrc = require('jumanlrc');
 * const jpp = new jumanlrc.JumanLRC();
 *
 * if (jpp.loadModel('./jumandic.jppmdl')) {
 *   const words = jpp.analyze('私は学生です');
 *   console.log(words);
 *   // → [{ surface: '私', reading: 'わたし' }, ...]
 * }
 */
class JumanLRC {
  /**
   * Create a new JumanLRC instance.
   * Call {@link JumanLRC#loadModel} to load a model before analyzing.
   */
  constructor() {
    this._handle = new addon.JumanLRCAddon();
  }

  /**
   * Load a Juman++ model file.
   *
   * @param {string}  modelPath      - Path to the Juman++ model file (.jppmdl)
   * @param {object}  [options]      - Optional beam parameters
   * @param {number}  [options.beamSize=5]
   * @param {number}  [options.globalBeam=6]
   * @param {number}  [options.rightBeam=5]
   * @param {number}  [options.rightCheck=1]
   * @returns {boolean} true if the model was loaded successfully
   */
  loadModel(modelPath, options = {}) {
    const {
      beamSize = 5,
      globalBeam = 6,
      rightBeam = 5,
      rightCheck = 1,
    } = options;

    return this._handle.loadModel(
      modelPath,
      beamSize,
      globalBeam,
      rightBeam,
      rightCheck,
    );
  }

  /**
   * Perform word-level morphological analysis.
   *
   * @param {string} sentence - The Japanese sentence to analyze.
   * @returns {Array<{surface: string, reading: string}>|null}
   *   Array of words with surface form and reading, or null on failure.
   */
  analyze(sentence) {
    if (typeof sentence !== 'string' || sentence.length === 0) {
      throw new Error('sentence must be a non-empty string');
    }
    return this._handle.analyze(sentence);
  }

  /**
   * Toggle katakana → hiragana annotation.
   * When enabled, katakana words are annotated with their hiragana readings.
   * When disabled (default), katakana output is as-is without readings.
   *
   * @param {boolean} on - Whether to annotate katakana with readings.
   */
  setKatakanaAnnotation(on) {
    this._handle.setKatakanaAnnotation(!!on);
  }

  /**
   * Perform ruby-ready analysis.
   * Returns an array where kanji words keep their surface form with
   * mora-split readings; kana-only words have an empty readings array.
   *
   * @param {string} sentence - The Japanese sentence to analyze.
   * @returns {Array<{surface: string, readings: string[]}>|null}
   */
  analyzeRuby(sentence) {
    if (typeof sentence !== 'string' || sentence.length === 0) {
      throw new Error('sentence must be a non-empty string');
    }
    return this._handle.analyzeRuby(sentence);
  }

  /**
   * Get the last error message.
   * @returns {string}
   */
  getError() {
    return this._handle.getError();
  }

  /**
   * Destroy the native handle and release resources.
   * After calling this, the instance should no longer be used.
   */
  destroy() {
    if (this._handle) {
      this._handle.destroy();
      this._handle = null;
    }
  }
}

module.exports = { JumanLRC };
