//
// jumanpp_lib.h — Internal C++ wrapper for Juman++ (surface + reading only)
//

#ifndef JUMANLRC_LIB_H
#define JUMANLRC_LIB_H

#include <string>
#include <vector>

#include "core/analysis/analysis_result.h"
#include "core/analysis/analyzer.h"
#include "core/analysis/output.h"
#include "core/env.h"

namespace jumanlrc
{
  using namespace jumanpp;

  namespace lib
  {

    struct WordResult
    {
      std::string surface;
      std::string reading;
    };

    // Ruby-ready unit: surface with reading split into mora (拍)
    struct RubyUnit
    {
      std::string surface;
      std::vector<std::string> readings; // empty if surface==reading (pure kana)
    };

    class JumanppLib
    {
    public:
      JumanppLib();
      ~JumanppLib();

      bool loadModel(const std::string &modelPath, int beamSize = 5,
                     int globalBeam = 6, int rightBeam = 5, int rightCheck = 1);

      // Word-level analysis (original Juman++ output)
      bool analyze(const std::string &sentence);
      // Ruby-ready: kanji words keep surface with mora-split readings array;
      //             okurigana and pure-kana words are split into mora units
      bool analyzeRuby(const std::string &sentence);

      // Control katakana annotation:
      //   false (default): katakana output as-is without readings
      //   true:            katakana annotated with hiragana readings
      void setKatakanaAnnotation(bool on) { annotateKatakana_ = on; }

      const std::vector<WordResult> &lastResult() const { return result_; }
      const std::vector<RubyUnit> &lastRubyResult() const { return rubyResult_; }
      const std::string &error() const { return error_; }

    private:
      core::JumanppEnv env_;
      core::analysis::Analyzer analyzer_;
      core::analysis::AnalysisResult analysisResult_;
      core::analysis::AnalysisPath top1_;
      core::analysis::NodeWalker walker_;
      core::analysis::StringField surfaceField_;
      core::analysis::StringField readingField_;
      std::vector<WordResult> result_;
      std::vector<RubyUnit> rubyResult_;
      bool annotateKatakana_ = false;
      std::string error_;
      bool initialized_ = false;
    };

  } // namespace lib
} // namespace jumanlrc

#endif // JUMANLRC_LIB_H
