
// Generated from ../FormulaK.g4 by ANTLR 4.13.1


#include "FormulaKLexer.h"


using namespace antlr4;



using namespace antlr4;

namespace {

struct FormulaKLexerStaticData final {
  FormulaKLexerStaticData(std::vector<std::string> ruleNames,
                          std::vector<std::string> channelNames,
                          std::vector<std::string> modeNames,
                          std::vector<std::string> literalNames,
                          std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), channelNames(std::move(channelNames)),
        modeNames(std::move(modeNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  FormulaKLexerStaticData(const FormulaKLexerStaticData&) = delete;
  FormulaKLexerStaticData(FormulaKLexerStaticData&&) = delete;
  FormulaKLexerStaticData& operator=(const FormulaKLexerStaticData&) = delete;
  FormulaKLexerStaticData& operator=(FormulaKLexerStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> channelNames;
  const std::vector<std::string> modeNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag formulaklexerLexerOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
FormulaKLexerStaticData *formulaklexerLexerStaticData = nullptr;

void formulaklexerLexerInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (formulaklexerLexerStaticData != nullptr) {
    return;
  }
#else
  assert(formulaklexerLexerStaticData == nullptr);
#endif
  auto staticData = std::make_unique<FormulaKLexerStaticData>(
    std::vector<std::string>{
      "T__0", "T__1", "INT", "AND", "OR", "IMP", "IFF", "NOT", "BOX", "DIA", 
      "IDXBOX", "IDXDIA", "NAME"
    },
    std::vector<std::string>{
      "DEFAULT_TOKEN_CHANNEL", "HIDDEN"
    },
    std::vector<std::string>{
      "DEFAULT_MODE"
    },
    std::vector<std::string>{
      "", "'('", "')'", "", "'&'", "'|'", "'=>'", "'<=>'", "'~'", "'[]'", 
      "'<>'"
    },
    std::vector<std::string>{
      "", "", "", "INT", "AND", "OR", "IMP", "IFF", "NOT", "BOX", "DIA", 
      "IDXBOX", "IDXDIA", "NAME"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,0,13,74,6,-1,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,
  	6,2,7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,1,0,1,0,1,1,1,
  	1,1,2,4,2,33,8,2,11,2,12,2,34,1,3,1,3,1,4,1,4,1,5,1,5,1,5,1,6,1,6,1,6,
  	1,6,1,7,1,7,1,8,1,8,1,8,1,9,1,9,1,9,1,10,1,10,3,10,58,8,10,1,10,1,10,
  	1,11,1,11,3,11,64,8,11,1,11,1,11,1,12,1,12,5,12,70,8,12,10,12,12,12,73,
  	9,12,0,0,13,1,1,3,2,5,3,7,4,9,5,11,6,13,7,15,8,17,9,19,10,21,11,23,12,
  	25,13,1,0,3,1,0,48,57,2,0,65,90,97,122,3,0,48,57,65,90,97,122,77,0,1,
  	1,0,0,0,0,3,1,0,0,0,0,5,1,0,0,0,0,7,1,0,0,0,0,9,1,0,0,0,0,11,1,0,0,0,
  	0,13,1,0,0,0,0,15,1,0,0,0,0,17,1,0,0,0,0,19,1,0,0,0,0,21,1,0,0,0,0,23,
  	1,0,0,0,0,25,1,0,0,0,1,27,1,0,0,0,3,29,1,0,0,0,5,32,1,0,0,0,7,36,1,0,
  	0,0,9,38,1,0,0,0,11,40,1,0,0,0,13,43,1,0,0,0,15,47,1,0,0,0,17,49,1,0,
  	0,0,19,52,1,0,0,0,21,55,1,0,0,0,23,61,1,0,0,0,25,67,1,0,0,0,27,28,5,40,
  	0,0,28,2,1,0,0,0,29,30,5,41,0,0,30,4,1,0,0,0,31,33,7,0,0,0,32,31,1,0,
  	0,0,33,34,1,0,0,0,34,32,1,0,0,0,34,35,1,0,0,0,35,6,1,0,0,0,36,37,5,38,
  	0,0,37,8,1,0,0,0,38,39,5,124,0,0,39,10,1,0,0,0,40,41,5,61,0,0,41,42,5,
  	62,0,0,42,12,1,0,0,0,43,44,5,60,0,0,44,45,5,61,0,0,45,46,5,62,0,0,46,
  	14,1,0,0,0,47,48,5,126,0,0,48,16,1,0,0,0,49,50,5,91,0,0,50,51,5,93,0,
  	0,51,18,1,0,0,0,52,53,5,60,0,0,53,54,5,62,0,0,54,20,1,0,0,0,55,57,5,91,
  	0,0,56,58,3,5,2,0,57,56,1,0,0,0,57,58,1,0,0,0,58,59,1,0,0,0,59,60,5,93,
  	0,0,60,22,1,0,0,0,61,63,5,60,0,0,62,64,3,5,2,0,63,62,1,0,0,0,63,64,1,
  	0,0,0,64,65,1,0,0,0,65,66,5,62,0,0,66,24,1,0,0,0,67,71,7,1,0,0,68,70,
  	7,2,0,0,69,68,1,0,0,0,70,73,1,0,0,0,71,69,1,0,0,0,71,72,1,0,0,0,72,26,
  	1,0,0,0,73,71,1,0,0,0,5,0,34,57,63,71,0
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  formulaklexerLexerStaticData = staticData.release();
}

}

FormulaKLexer::FormulaKLexer(CharStream *input) : Lexer(input) {
  FormulaKLexer::initialize();
  _interpreter = new atn::LexerATNSimulator(this, *formulaklexerLexerStaticData->atn, formulaklexerLexerStaticData->decisionToDFA, formulaklexerLexerStaticData->sharedContextCache);
}

FormulaKLexer::~FormulaKLexer() {
  delete _interpreter;
}

std::string FormulaKLexer::getGrammarFileName() const {
  return "FormulaK.g4";
}

const std::vector<std::string>& FormulaKLexer::getRuleNames() const {
  return formulaklexerLexerStaticData->ruleNames;
}

const std::vector<std::string>& FormulaKLexer::getChannelNames() const {
  return formulaklexerLexerStaticData->channelNames;
}

const std::vector<std::string>& FormulaKLexer::getModeNames() const {
  return formulaklexerLexerStaticData->modeNames;
}

const dfa::Vocabulary& FormulaKLexer::getVocabulary() const {
  return formulaklexerLexerStaticData->vocabulary;
}

antlr4::atn::SerializedATNView FormulaKLexer::getSerializedATN() const {
  return formulaklexerLexerStaticData->serializedATN;
}

const atn::ATN& FormulaKLexer::getATN() const {
  return *formulaklexerLexerStaticData->atn;
}




void FormulaKLexer::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  formulaklexerLexerInitialize();
#else
  ::antlr4::internal::call_once(formulaklexerLexerOnceFlag, formulaklexerLexerInitialize);
#endif
}
