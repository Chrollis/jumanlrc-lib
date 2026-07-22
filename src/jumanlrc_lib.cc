//
// jumanpp_lib.cc — C++ wrapper implementation
//

#include "jumanlrc_lib.h"

#include "core/analysis/analyzer_impl.h"
#include "core/core.h"
#include "jpp_jumandic_cg.h"
#include "util/logging.hpp"

namespace jumanlrc
{
  using namespace jumanpp;

  namespace lib
  {

    JumanppLib::JumanppLib() = default;
    JumanppLib::~JumanppLib() = default;

    bool JumanppLib::loadModel(const std::string &modelPath, int beamSize,
                               int globalBeam, int rightBeam, int rightCheck)
    {
      Status s = env_.loadModel(modelPath);
      if (!s)
      {
        error_ = "Failed to load model: " +
                 std::string(s.message().begin(), s.message().end());
        return false;
      }

      env_.setBeamSize(static_cast<u32>(beamSize));
      env_.setGlobalBeam(globalBeam, rightCheck, rightBeam);

      jumanpp_generated::JumandicStatic features;
      s = env_.initFeatures(&features);
      if (!s)
      {
        error_ = "Failed to init features: " +
                 std::string(s.message().begin(), s.message().end());
        return false;
      }

      s = env_.makeAnalyzer(&analyzer_);
      if (!s)
      {
        error_ = "Failed to create analyzer: " +
                 std::string(s.message().begin(), s.message().end());
        return false;
      }

      const auto &outMgr = analyzer_.output();
      s = outMgr.stringField("surface", &surfaceField_);
      if (!s)
      {
        error_ = "Failed to get 'surface' field: " +
                 std::string(s.message().begin(), s.message().end());
        return false;
      }
      s = outMgr.stringField("reading", &readingField_);
      if (!s)
      {
        error_ = "Failed to get 'reading' field: " +
                 std::string(s.message().begin(), s.message().end());
        return false;
      }

      initialized_ = true;
      return true;
    }

    bool JumanppLib::analyze(const std::string &sentence)
    {
      if (!initialized_)
      {
        error_ = "Not initialized. Call loadModel() first.";
        return false;
      }

      result_.clear();

      Status s = analyzer_.analyze(sentence);
      if (!s)
      {
        error_ = "Analysis failed: " +
                 std::string(s.message().begin(), s.message().end());
        return false;
      }

      s = analysisResult_.reset(analyzer_);
      if (!s)
      {
        error_ = "Failed to reset analysis result: " +
                 std::string(s.message().begin(), s.message().end());
        return false;
      }

      s = analysisResult_.fillTop1(&top1_);
      if (!s)
      {
        error_ = "Failed to fill top1 path: " +
                 std::string(s.message().begin(), s.message().end());
        return false;
      }

      const auto &outMgr = analyzer_.output();

      // Cursor-based dedup: walk through the sentence and only accept
      // words that match at the current cursor position.
      size_t cursor = 0;

      while (top1_.nextBoundary())
      {
        if (top1_.remainingNodesInChunk() <= 0)
          continue;

        core::analysis::ConnectionPtr connPtr;
        if (!top1_.nextNode(&connPtr))
          continue;

        // Collect all node surfaces from this chunk
        std::vector<WordResult> chunkWords;

        // First node
        {
          core::analysis::LatticeNodePtr nodePtr{connPtr.boundary, connPtr.right};
          if (outMgr.locate(nodePtr, &walker_))
          {
            while (walker_.next())
            {
              WordResult wr;
              auto surf = surfaceField_[walker_];
              auto read = readingField_[walker_];
              wr.surface.assign(surf.begin(), surf.end());
              wr.reading.assign(read.begin(), read.end());
              chunkWords.push_back(std::move(wr));
            }
          }
        }

        // Additional compound nodes
        while (top1_.nextNode(&connPtr))
        {
          core::analysis::LatticeNodePtr nodePtr{connPtr.boundary, connPtr.right};
          if (outMgr.locate(nodePtr, &walker_))
          {
            while (walker_.next())
            {
              WordResult wr;
              auto surf = surfaceField_[walker_];
              auto read = readingField_[walker_];
              wr.surface.assign(surf.begin(), surf.end());
              wr.reading.assign(read.begin(), read.end());
              chunkWords.push_back(std::move(wr));
            }
          }
        }

        // Try each word from this chunk against the cursor position;
        // accept the first one that matches.
        for (auto &w : chunkWords)
        {
          if (cursor + w.surface.size() <= sentence.size() &&
              sentence.compare(cursor, w.surface.size(), w.surface) == 0)
          {
            cursor += w.surface.size();
            result_.push_back(std::move(w));
            break;
          }
        }
      }

      // Validate: we must have consumed the entire sentence
      if (cursor != sentence.size())
      {
        error_ = "Analysis mismatch: consumed " +
                 std::to_string(cursor) + " of " +
                 std::to_string(sentence.size()) + " bytes";
        result_.clear();
        return false;
      }

      return true;
    }

    // ─── Ruby helper: mora (拍) splitting ─────────────────────────

    // 3-byte UTF-8 sequence → UTF-32 codepoint
    static uint32_t utf8ToCodepoint(const std::string &s, size_t pos)
    {
      auto c0 = static_cast<unsigned char>(s[pos]);
      if (c0 < 0xE0)
        return c0;
      return ((c0 & 0x0F) << 12) |
             ((s[pos + 1] & 0x3F) << 6) |
             (s[pos + 2] & 0x3F);
    }

    // Check if codepoint is a CJK kanji
    static bool isKanjiCp(uint32_t cp)
    {
      return (cp >= 0x4E00 && cp <= 0x9FFF) ||
             (cp >= 0x3400 && cp <= 0x4DBF);
    }

    // Check if a 3-byte char at pos is small ゃ/ゅ/ょ (or ャ/ュ/ョ)
    static bool isSmallYoon(const std::string &s, size_t pos)
    {
      if (pos + 2 >= s.size())
        return false;
      auto cp = utf8ToCodepoint(s, pos);
      // ゃ U+3083, ゅ U+3085, ょ U+3087
      // ャ U+30E3, ュ U+30E5, ョ U+30E7
      return cp == 0x3083 || cp == 0x3085 || cp == 0x3087 ||
             cp == 0x30E3 || cp == 0x30E5 || cp == 0x30E7;
    }

    // Check if a 3-byte char at pos is small っ/ッ (sokuon)
    static bool isSokuonCp(const std::string &s, size_t pos)
    {
      if (pos + 2 >= s.size())
        return false;
      auto cp = utf8ToCodepoint(s, pos);
      return cp == 0x3063 || cp == 0x30C3; // っ or ッ
    }

    // Check if a 3-byte char at pos is ー (long vowel mark)
    static bool isChoonCp(const std::string &s, size_t pos)
    {
      if (pos + 2 >= s.size())
        return false;
      return utf8ToCodepoint(s, pos) == 0x30FC; // ー
    }

    // Split kana string into mora (拍) units.
    // Rules:
    //   1. 拗音 (small ゃ/ゅ/ょ) → merge with previous char (one mora)
    //   2. 長音 ー → merge with previous char
    //   3. 促音 っ/ッ → standalone mora (its own beat)
    //   4. Everything else is its own mora
    static void splitIntoMora(const std::string &kana,
                              std::vector<std::string> &out)
    {
      out.clear();
      if (kana.empty())
        return;
      size_t i = 0;
      while (i < kana.size())
      {
        unsigned char c = static_cast<unsigned char>(kana[i]);
        if (c < 0xE0)
        {
          if (c < 0xC0)
          {
            out.emplace_back(1, kana[i]);
            i += 1;
          }
          else
          {
            out.push_back(kana.substr(i, 2));
            i += 2;
          }
          continue;
        }

        // Check if this is small ゃ/ゅ/ょ → merge with previous mora
        if (isSmallYoon(kana, i) && !out.empty())
        {
          out.back() += kana.substr(i, 3);
          i += 3;
          continue;
        }

        // Normal start of a mora (sokuon っ/ッ is its own beat)
        std::string mora;
        mora += kana.substr(i, 3);
        i += 3;

        out.push_back(mora);
      }
    }

    // Check if a UTF-8 string contains any CJK kanji
    static bool hasKanji(const std::string &s)
    {
      for (size_t i = 0; i + 2 < s.size();)
      {
        unsigned char c = static_cast<unsigned char>(s[i]);
        if (c >= 0xE0)
        {
          if (isKanjiCp(utf8ToCodepoint(s, i)))
            return true;
          i += 3;
        }
        else if (c >= 0xC0)
          i += 2;
        else
          i += 1;
      }
      return false;
    }

    bool JumanppLib::analyzeRuby(const std::string &sentence)
    {
      if (!analyze(sentence))
        return false;
      rubyResult_.clear();
      for (const auto &w : result_)
      {
        if (hasKanji(w.surface))
        {
          // Find okurigana boundary (trailing kana after last kanji)
          int okuriStart = (int)w.surface.size();
          for (int i = (int)w.surface.size() - 3; i >= 0;)
          {
            unsigned char c = static_cast<unsigned char>(w.surface[i]);
            if (c >= 0xE0)
            {
              if (isKanjiCp(utf8ToCodepoint(w.surface, i)))
              {
                okuriStart = i + 3;
                break;
              }
              i -= 3;
            }
            else if (c >= 0xC0)
              i -= 2;
            else
              i -= 1;
          }

          std::string kanjiPart = w.surface.substr(0, okuriStart);
          std::string okuriPart = w.surface.substr(okuriStart);

          // Split reading into mora
          std::vector<std::string> allMora;
          splitIntoMora(w.reading, allMora);

          // Estimate mora for okurigana by counting kana chars
          int okuriChars = 0;
          for (size_t i = 0; i < okuriPart.size();)
          {
            unsigned char c = static_cast<unsigned char>(okuriPart[i]);
            if (c >= 0xE0)
            {
              okuriChars++;
              i += 3;
            }
            else if (c >= 0xC0)
            {
              okuriChars++;
              i += 2;
            }
            else
            {
              okuriChars++;
              i += 1;
            }
          }

          int totalMora = (int)allMora.size();
          int nk = okuriChars < totalMora ? okuriChars : 0;
          int kanjiMoraCount = totalMora - nk;

          // Kanji unit with its portion of reading
          RubyUnit ku;
          ku.surface = kanjiPart;
          for (int i = 0; i < kanjiMoraCount && i < totalMora; i++)
            ku.readings.push_back(allMora[i]);
          rubyResult_.push_back(std::move(ku));

          // Okurigana: each mora as its own unit
          for (int i = kanjiMoraCount; i < totalMora; i++)
          {
            RubyUnit ou;
            ou.surface = allMora[i];
            rubyResult_.push_back(std::move(ou));
          }
        }
        else
        {
          // Check if word contains katakana
          bool hasKata = false;
          if (annotateKatakana_)
          {
            for (size_t i = 0; i + 2 < w.surface.size(); i += 3)
            {
              auto cp = utf8ToCodepoint(w.surface, i);
              if (cp >= 0x30A0 && cp <= 0x30FF)
              {
                hasKata = true;
                break;
              }
            }
          }

          std::vector<std::string> surfMora, readMora;
          splitIntoMora(w.surface, surfMora);
          splitIntoMora(w.reading, readMora);

          if (surfMora.size() == readMora.size() && surfMora.size() > 1)
          {
            for (size_t i = 0; i < surfMora.size(); i++)
            {
              RubyUnit u;
              if (hasKata)
              {
                // Katakana: show reading (hiragana) as the annotation
                u.surface = surfMora[i];
                if (surfMora[i] != readMora[i])
                  u.readings.push_back(readMora[i]);
              }
              else
              {
                u.surface = surfMora[i];
              }
              rubyResult_.push_back(std::move(u));
            }
          }
          else
          {
            RubyUnit u;
            u.surface = w.surface;
            if (hasKata)
            {
              splitIntoMora(w.reading, u.readings);
            }
            else if (w.surface != w.reading)
            {
              splitIntoMora(w.reading, u.readings);
            }
            rubyResult_.push_back(std::move(u));
          }
        }
      }

      return true;
    }

  } // namespace lib
} // namespace jumanlrc