/**
 * JumanLRC — Japanese morphological analyzer for lyrics ruby annotation.
 *
 * Node.js native addon wrapping JumanLRC-Lib.
 *
 * @example
 * ```ts
 * import { JumanLRC } from 'jumanlrc';
 *
 * const jpp = new JumanLRC();
 * if (jpp.loadModel('./jumandic.jppmdl')) {
 *   const words = jpp.analyze('私は学生です');
 *   // words: Array<{ surface: string, reading: string }>
 * }
 * ```
 */

export interface WordResult {
  /** Surface form (the original word as written). */
  surface: string;
  /** Reading in hiragana. */
  reading: string;
}

export interface RubyUnit {
  /** Surface form (the original character(s) as written). */
  surface: string;
  /**
   * Mora-split readings.
   * Empty array when the surface is pure kana (surface === reading).
   */
  readings: string[];
}

export interface LoadModelOptions {
  /** Beam size (default: 5). */
  beamSize?: number;
  /** Global beam (default: 6). */
  globalBeam?: number;
  /** Right beam (default: 5). */
  rightBeam?: number;
  /** Right check (default: 1). */
  rightCheck?: number;
}

/**
 * JumanLRC wrapper class.
 */
export class JumanLRC {
  constructor();

  /**
   * Load a Juman++ model file.
   * @param modelPath - Path to the model file (.jppmdl)
   * @param options   - Optional beam parameters
   * @returns `true` on success, `false` on failure
   */
  loadModel(modelPath: string, options?: LoadModelOptions): boolean;

  /**
   * Perform word-level morphological analysis.
   * @param sentence - Japanese sentence to analyze
   * @returns Array of word results, or `null` on failure
   */
  analyze(sentence: string): WordResult[] | null;

  /**
   * Toggle katakana → hiragana annotation.
   * When enabled, katakana words are annotated with their hiragana readings.
   * When disabled (default), katakana output is as-is without readings.
   */
  setKatakanaAnnotation(on: boolean): void;

  /**
   * Perform ruby-ready analysis with mora-split readings.
   * @param sentence - Japanese sentence to analyze
   * @returns Array of ruby units, or `null` on failure
   */
  analyzeRuby(sentence: string): RubyUnit[] | null;

  /**
   * Get the last error message.
   */
  getError(): string;

  /**
   * Destroy the native handle and release resources.
   */
  destroy(): void;
}
