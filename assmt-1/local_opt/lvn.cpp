// Local Value Numbering (LVN)
// Standalone tool: reads quadruples, emits optimized quadruples.
//
// Supported optimizations within a single basic block:
//   * Constant folding
//   * Constant propagation
//   * Copy propagation
//   * Common Subexpression Elimination (CSE)
//   * Commutativity handling (a op b == b op a for + and *)
//   * Array reference handling (x = arr[idx], arr[idx] = x)
//
// Input quadruple syntax (one per line, '#' or '//' start a comment line):
//   dst = value                 ; constant / copy   (e.g. a = 10 , e = i)
//   dst = op1 <op> op2          ; binary            (e.g. t1 = i * j)
//   dst = arr[idx]              ; array load        (e.g. t3 = t2[t1])
//   arr[idx] = src              ; array store       (e.g. t2[t1] = x)
// where <op> is one of + - * /

#include <cctype>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

//===----------------------------------------------------------------------===//
// Data structures for value numbering
//===----------------------------------------------------------------------===//

// NameTable entry, indexed by value number.
struct NameTableEntry {
  std::vector<std::string> names; // all names sharing this value number
  long constValue = 0;            // constant value (if constFlag)
  bool constFlag = false;         // true if this value is a known constant
};

// The three tables of the classic LVN algorithm.
static std::map<std::string, int> ValnumTable; // name        -> value number
static std::map<std::string, int> HashTable;   // canonical expr -> value number
static std::vector<NameTableEntry> NameTable;  // value number -> entry

static int NextValueNumber = 0;

// Allocate a fresh value number with an (optional) initial name.
static int newValueNumber(const std::string &name = "") {
  int vn = NextValueNumber++;
  NameTable.emplace_back();
  if (!name.empty())
    NameTable[vn].names.push_back(name);
  return vn;
}

//===----------------------------------------------------------------------===//
// Small helpers
//===----------------------------------------------------------------------===//

static std::string trim(const std::string &s) {
  size_t b = s.find_first_not_of(" \t\r\n");
  if (b == std::string::npos)
    return "";
  size_t e = s.find_last_not_of(" \t\r\n");
  return s.substr(b, e - b + 1);
}

static bool isIntLiteral(const std::string &s, long &out) {
  if (s.empty())
    return false;
  size_t i = 0;
  if (s[i] == '+' || s[i] == '-')
    ++i;
  if (i >= s.size())
    return false;
  for (size_t j = i; j < s.size(); ++j)
    if (!std::isdigit((unsigned char)s[j]))
      return false;
  out = std::stol(s);
  return true;
}

// A name is treated as a compiler temporary if it starts with 't' followed by
// digits (e.g. t1, t23).  Temporaries that become constants or CSE duplicates
// are eligible for deletion.
static bool isTemp(const std::string &name) {
  if (name.size() < 2 || name[0] != 't')
    return false;
  for (size_t i = 1; i < name.size(); ++i)
    if (!std::isdigit((unsigned char)name[i]))
      return false;
  return true;
}

// Representative (first) name of a value number.
static std::string repName(int vn) {
  if (vn >= 0 && vn < (int)NameTable.size() && !NameTable[vn].names.empty())
    return NameTable[vn].names.front();
  return "?";
}

//===----------------------------------------------------------------------===//
// Operand resolution
//===----------------------------------------------------------------------===//

// An operand resolves either to a known constant, or to a value number.
struct Operand {
  bool isConst = false;
  long constValue = 0;
  int vn = -1;
  std::string origText; // as written in the source quad
};

// Resolve a textual operand: literal -> constant; existing name -> its vn
// (and its constant, if any); unseen name -> fresh value number.
static Operand resolveOperand(const std::string &tok) {
  Operand op;
  op.origText = tok;
  long lit;
  if (isIntLiteral(tok, lit)) {
    op.isConst = true;
    op.constValue = lit;
    return op;
  }
  auto it = ValnumTable.find(tok);
  int vn;
  if (it == ValnumTable.end()) {
    vn = newValueNumber(tok);
    ValnumTable[tok] = vn;
  } else {
    vn = it->second;
  }
  op.vn = vn;
  if (NameTable[vn].constFlag) {
    op.isConst = true;
    op.constValue = NameTable[vn].constValue;
  }
  return op;
}

// How an operand should be printed after propagation.
static std::string printOperand(const Operand &op) {
  if (op.isConst)
    return std::to_string(op.constValue);
  return repName(op.vn);
}

// Canonical hash-key token for an operand (value-number based).
static std::string keyToken(const Operand &op) {
  if (op.isConst)
    return "c" + std::to_string(op.constValue);
  return "#" + std::to_string(op.vn);
}

static bool isCommutative(char op) { return op == '+' || op == '*'; }

static bool foldConst(char op, long a, long b, long &out) {
  switch (op) {
  case '+': out = a + b; return true;
  case '-': out = a - b; return true;
  case '*': out = a * b; return true;
  case '/':
    if (b == 0)
      return false;
    out = a / b;
    return true;
  }
  return false;
}

//===----------------------------------------------------------------------===//
// Output record for each processed quadruple
//===----------------------------------------------------------------------===//

struct OutQuad {
  std::string text;    // optimized quadruple text
  bool deleted = false;// true if it can be removed
  std::string note;    // annotation (e.g. "same as t1", "constant")
};

static std::vector<OutQuad> Output;

// Assign / merge a name to a value number in the tables.
static void setNameVN(const std::string &name, int vn) {
  ValnumTable[name] = vn;
  auto &names = NameTable[vn].names;
  bool found = false;
  for (auto &n : names)
    if (n == name) { found = true; break; }
  if (!found)
    names.push_back(name);
}

static void makeConst(const std::string &name, int vn, long value) {
  NameTable[vn].constFlag = true;
  NameTable[vn].constValue = value;
  setNameVN(name, vn);
}

//===----------------------------------------------------------------------===//
// Core processing of one quadruple
//===----------------------------------------------------------------------===//

// Parse "arr[idx]" into (arr, idx).  Returns false if not an array ref.
static bool parseArrayRef(const std::string &s, std::string &arr,
                          std::string &idx) {
  size_t lb = s.find('[');
  size_t rb = s.find(']');
  if (lb == std::string::npos || rb == std::string::npos || rb < lb)
    return false;
  arr = trim(s.substr(0, lb));
  idx = trim(s.substr(lb + 1, rb - lb - 1));
  return !arr.empty() && !idx.empty();
}

// Invalidate cached loads for an array (on a store to that array).
static void killArrayLoads(const std::string &arrToken) {
  for (auto it = HashTable.begin(); it != HashTable.end();) {
    if (it->first.rfind(arrToken + "[", 0) == 0)
      it = HashTable.erase(it);
    else
      ++it;
  }
}

static void processQuad(const std::string &line) {
  size_t eq = line.find('=');
  if (eq == std::string::npos)
    return; // ignore malformed lines

  std::string lhs = trim(line.substr(0, eq));
  std::string rhs = trim(line.substr(eq + 1));

  //--- Array store:  arr[idx] = src ------------------------------------------
  std::string arr, idx;
  if (parseArrayRef(lhs, arr, idx)) {
    Operand base = resolveOperand(arr);
    Operand index = resolveOperand(idx);
    Operand src = resolveOperand(rhs);
    // A store invalidates previously cached loads of this array.
    killArrayLoads("A" + keyToken(base));
    OutQuad q;
    q.text = arr + "[" + printOperand(index) + "] = " + printOperand(src);
    Output.push_back(q);
    return;
  }

  //--- Array load:  dst = arr[idx] -------------------------------------------
  if (parseArrayRef(rhs, arr, idx)) {
    Operand base = resolveOperand(arr);
    Operand index = resolveOperand(idx);
    std::string key = "A" + keyToken(base) + "[" + keyToken(index) + "]";
    OutQuad q;
    q.text = lhs + " = " + arr + "[" + printOperand(index) + "]";
    auto hit = HashTable.find(key);
    if (hit != HashTable.end()) {
      int vn = hit->second;
      setNameVN(lhs, vn);
      q.deleted = isTemp(lhs);
      q.note = "same as " + repName(vn) + " (redundant array load)";
    } else {
      int vn = newValueNumber(lhs);
      HashTable[key] = vn;
      setNameVN(lhs, vn);
    }
    Output.push_back(q);
    return;
  }

  //--- Tokenize RHS to detect binary vs copy/constant ------------------------
  std::istringstream iss(rhs);
  std::vector<std::string> toks;
  std::string t;
  while (iss >> t)
    toks.push_back(t);

  //--- Binary op:  dst = op1 <op> op2 ----------------------------------------
  if (toks.size() == 3 && toks[1].size() == 1 &&
      (toks[1][0] == '+' || toks[1][0] == '-' || toks[1][0] == '*' ||
       toks[1][0] == '/')) {
    char op = toks[1][0];
    Operand a = resolveOperand(toks[0]);
    Operand b = resolveOperand(toks[2]);

    // Constant folding.
    long folded;
    if (a.isConst && b.isConst && foldConst(op, a.constValue, b.constValue,
                                            folded)) {
      int vn = newValueNumber(lhs);
      makeConst(lhs, vn, folded);
      OutQuad q;
      q.text = lhs + " = " + std::to_string(folded);
      q.note = "constant";
      // A temporary that only holds a constant can be deleted (its uses are
      // propagated).
      q.deleted = isTemp(lhs);
      Output.push_back(q);
      return;
    }

    // Build canonical key (commutativity: order operand tokens).
    std::string ka = keyToken(a), kb = keyToken(b);
    std::string key;
    if (isCommutative(op) && kb < ka)
      key = kb + op + ka;
    else
      key = ka + std::string(1, op) + kb;

    OutQuad q;
    q.text = lhs + " = " + printOperand(a) + " " + op + " " + printOperand(b);

    auto hit = HashTable.find(key);
    if (hit != HashTable.end()) {
      // CSE: reuse the existing value number.
      int vn = hit->second;
      setNameVN(lhs, vn);
      q.deleted = isTemp(lhs);
      q.note = "same as " + repName(vn);
    } else {
      int vn = newValueNumber(lhs);
      HashTable[key] = vn;
      setNameVN(lhs, vn);
    }
    Output.push_back(q);
    return;
  }

  //--- Copy or constant:  dst = value ----------------------------------------
  if (toks.size() == 1) {
    long lit;
    if (isIntLiteral(toks[0], lit)) {
      int vn = newValueNumber(lhs);
      makeConst(lhs, vn, lit);
      OutQuad q;
      q.text = lhs + " = " + std::to_string(lit);
      Output.push_back(q);
      return;
    }
    // Copy: dst = src  -> copy propagation (dst gets src's value number).
    Operand src = resolveOperand(toks[0]);
    setNameVN(lhs, src.vn);
    OutQuad q;
    q.text = lhs + " = " + printOperand(src);
    Output.push_back(q);
    return;
  }

  // Fallback: emit unchanged.
  OutQuad q;
  q.text = line;
  Output.push_back(q);
}

//===----------------------------------------------------------------------===//
// Driver
//===----------------------------------------------------------------------===//

int main(int argc, char **argv) {
  std::istream *in = &std::cin;
  std::ifstream file;
  if (argc > 1) {
    file.open(argv[1]);
    if (!file) {
      std::cerr << "error: cannot open " << argv[1] << "\n";
      return 1;
    }
    in = &file;
  }

  std::string line;
  while (std::getline(*in, line)) {
    // Strip inline comments ('#' or '//').
    size_t hash = line.find('#');
    if (hash != std::string::npos)
      line = line.substr(0, hash);
    size_t dslash = line.find("//");
    if (dslash != std::string::npos)
      line = line.substr(0, dslash);
    std::string s = trim(line);
    if (s.empty())
      continue;
    processQuad(s);
  }

  // Print annotated optimized quadruples.
  std::cout << "=== Optimized quadruples (annotated) ===\n";
  int idx = 1;
  for (auto &q : Output) {
    std::cout << idx++ << ". " << q.text;
    if (q.deleted)
      std::cout << "   // deleted";
    if (!q.note.empty())
      std::cout << (q.deleted ? " " : "   // ") << q.note;
    std::cout << "\n";
  }

  // Print final renumbered quadruples (deleted quads removed).
  std::cout << "\n=== Final quadruples (renumbered) ===\n";
  int n = 1;
  for (auto &q : Output)
    if (!q.deleted)
      std::cout << n++ << ". " << q.text << "\n";

  return 0;
}
