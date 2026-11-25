#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

struct SymbolEntry {
    string name;
    string dataType;
    string scope;
    bool isFunction;
    bool isStruct;
    bool isArray;
    int arraySize;
    vector<string> parameterTypes;
};

struct Tok {
    int line;
    string type;
    string lex;
};

vector<SymbolEntry> symbolTable;
vector<Tok> toks;
vector<string> errors;
int pos = 0;

SymbolEntry makeEmptyEntry() {
    SymbolEntry e;
    e.name = "";
    e.dataType = "";
    e.scope = "";
    e.isFunction = false;
    e.isStruct = false;
    e.isArray = false;
    e.arraySize = 0;
    return e;
}

SymbolEntry makeVarEntry(string name, string dataType, string scope) {
    SymbolEntry e = makeEmptyEntry();
    e.name = name;
    e.dataType = dataType;
    e.scope = scope;
    return e;
}

SymbolEntry makeArrayEntry(string name, string dataType, string scope, int size) {
    SymbolEntry e = makeEmptyEntry();
    e.name = name;
    e.dataType = dataType;
    e.scope = scope;
    e.isArray = true;
    e.arraySize = size;
    return e;
}

SymbolEntry makeFuncEntry(string name, string dataType, string scope) {
    SymbolEntry e = makeEmptyEntry();
    e.name = name;
    e.dataType = dataType;
    e.scope = scope;
    e.isFunction = true;
    return e;
}

SymbolEntry makeStructEntry(string name, string scope) {
    SymbolEntry e = makeEmptyEntry();
    e.name = name;
    e.dataType = name;
    e.scope = scope;
    e.isStruct = true;
    return e;
}

void clearSymbolTable() {
    symbolTable.clear();
}

void clearErrors() {
    errors.clear();
}

int symbolTableSize() {
    return (int)symbolTable.size();
}

int errorCount() {
    return (int)errors.size();
}

void addError(string msg) {
    errors.push_back(msg);
}

void addErrorLine(int line, string msg) {
    string s = "line ";
    s = s + to_string(line);
    s = s + ": ";
    s = s + msg;
    addError(s);
}

void printErrors() {
    cout << "=== semantic errors (" << errorCount() << ") ===" << endl;
    for (int i = 0; i < (int)errors.size(); i++) {
        cout << errors[i] << endl;
    }
}

bool sameNameAndScope(const SymbolEntry& a, string name, string scope) {
    if (a.name != name) return false;
    if (a.scope != scope) return false;
    return true;
}

int findSymbolIndex(string name, string scope) {
    for (int i = 0; i < (int)symbolTable.size(); i++) {
        if (sameNameAndScope(symbolTable[i], name, scope)) return i;
    }
    return -1;
}

bool symbolExists(string name, string scope) {
    if (findSymbolIndex(name, scope) == -1) return false;
    return true;
}

SymbolEntry getSymbolAt(int index) {
    if (index < 0 || index >= (int)symbolTable.size()) return makeEmptyEntry();
    return symbolTable[index];
}

int findVisibleIndex(string name, string scope) {
    int idx = findSymbolIndex(name, scope);
    if (idx != -1) return idx;
    if (scope != "global") {
        idx = findSymbolIndex(name, "global");
        if (idx != -1) return idx;
    }
    return -1;
}

bool symbolVisible(string name, string scope) {
    return findVisibleIndex(name, scope) != -1;
}

SymbolEntry getVisibleSymbol(string name, string scope) {
    int idx = findVisibleIndex(name, scope);
    return getSymbolAt(idx);
}

void addSymbol(SymbolEntry e) {
    if (symbolExists(e.name, e.scope)) {
        string msg = "redeclared '";
        msg = msg + e.name;
        msg = msg + "' in scope ";
        msg = msg + e.scope;
        addError(msg);
        return;
    }
    symbolTable.push_back(e);
}

string boolToYesNo(bool v) {
    if (v) return "yes";
    return "no";
}

string entryKindLabel(const SymbolEntry& e) {
    if (e.isFunction) return "function";
    if (e.isStruct) return "dhancha";
    if (e.isArray) return "array";
    return "var";
}

void printOneSymbol(const SymbolEntry& e) {
    cout << "name=" << e.name;
    cout << " type=" << e.dataType;
    cout << " scope=" << e.scope;
    cout << " kind=" << entryKindLabel(e);
    cout << " func=" << boolToYesNo(e.isFunction);
    cout << " struct=" << boolToYesNo(e.isStruct);
    cout << " array=" << boolToYesNo(e.isArray);
    if (e.isArray) cout << " size=" << e.arraySize;
    cout << endl;
}

void printSymbolTable() {
    cout << "=== symbol table (" << symbolTableSize() << ") ===" << endl;
    for (int i = 0; i < (int)symbolTable.size(); i++) {
        cout << i << ": ";
        printOneSymbol(symbolTable[i]);
    }
}

void addParamType(SymbolEntry& e, string t) {
    e.parameterTypes.push_back(t);
}

string trimSpaces(string s) {
    while (s.size() > 0 && (s[0] == ' ' || s[0] == '\t')) s.erase(0, 1);
    while (s.size() > 0 && (s[s.size() - 1] == ' ' || s[s.size() - 1] == '\t' || s[s.size() - 1] == '\r')) {
        s.erase(s.size() - 1, 1);
    }
    return s;
}

bool parseTokenLine(const string& line, Tok& out) {
    if (line.size() < 5) return false;
    if (line[0] != '<') return false;
    if (line[line.size() - 1] != '>') return false;
    string inside = line.substr(1, line.size() - 2);
    int c1 = -1;
    int c2 = -1;
    for (int i = 0; i < (int)inside.size(); i++) {
        if (inside[i] == ',') {
            if (c1 == -1) c1 = i;
            else if (c2 == -1) {
                c2 = i;
                break;
            }
        }
    }
    if (c1 < 0 || c2 < 0) return false;
    string a = trimSpaces(inside.substr(0, c1));
    string b = trimSpaces(inside.substr(c1 + 1, c2 - c1 - 1));
    string c = trimSpaces(inside.substr(c2 + 1));
    out.line = 0;
    for (int i = 0; i < (int)a.size(); i++) {
        if (a[i] >= '0' && a[i] <= '9') out.line = out.line * 10 + (a[i] - '0');
    }
    out.type = b;
    out.lex = c;
    return true;
}

vector<Tok> readTokensFile(const string& path) {
    vector<Tok> result;
    ifstream fin(path.c_str());
    if (!fin.is_open()) {
        cout << "cant open " << path << endl;
        return result;
    }
    string line;
    while (getline(fin, line)) {
        if (line.size() == 0) continue;
        Tok t;
        if (parseTokenLine(line, t)) result.push_back(t);
    }
    fin.close();
    return result;
}

bool atEnd() {
    return pos >= (int)toks.size();
}

Tok cur() {
    if (atEnd()) {
        Tok t;
        t.line = -1;
        t.type = "EOF";
        t.lex = "";
        return t;
    }
    return toks[pos];
}

Tok look(int n) {
    if (pos + n >= (int)toks.size()) {
        Tok t;
        t.line = -1;
        t.type = "EOF";
        t.lex = "";
        return t;
    }
    return toks[pos + n];
}

Tok eat() {
    Tok t = cur();
    if (!atEnd()) pos++;
    return t;
}

bool isTypeTok(const string& t) {
    if (t == "KEYWORD_INT") return true;
    if (t == "KEYWORD_FLOAT") return true;
    if (t == "KEYWORD_CHAR") return true;
    if (t == "KEYWORD_KHALI") return true;
    return false;
}

bool isNumType(const string& t) {
    if (t == "int") return true;
    if (t == "float") return true;
    if (t == "NUM") return true;
    return false;
}

bool typesCompatible(string a, string b) {
    if (a == "") return true;
    if (b == "") return true;
    if (a == b) return true;
    if (isNumType(a) && isNumType(b)) return true;
    return false;
}

int stringToInt(const string& s) {
    int n = 0;
    for (int i = 0; i < (int)s.size(); i++) {
        if (s[i] >= '0' && s[i] <= '9') n = n * 10 + (s[i] - '0');
    }
    return n;
}

void collectParamsInto(SymbolEntry& fn) {
    while (!atEnd() && cur().type != "PUNCT_RPAREN") {
        if (isTypeTok(cur().type)) {
            string ptype = cur().lex;
            eat();
            if (cur().type == "IDENTIFIER") {
                string pname = cur().lex;
                eat();
                addParamType(fn, ptype);
                addSymbol(makeVarEntry(pname, ptype, fn.name));
            }
        } else {
            eat();
        }
    }
}

void collectStructFields(string structName) {
    while (!atEnd() && cur().type != "PUNCT_KHATAM") {
        if (isTypeTok(cur().type)) {
            string ftype = cur().lex;
            eat();
            if (cur().type != "IDENTIFIER") continue;
            string fname = cur().lex;
            eat();
            if (cur().type == "PUNCT_LBRACK") {
                eat();
                int sz = 0;
                if (cur().type == "NUM_LITERAL") {
                    sz = stringToInt(cur().lex);
                    eat();
                }
                if (cur().type == "PUNCT_RBRACK") eat();
                if (cur().type == "PUNCT_SEMI") eat();
                addSymbol(makeArrayEntry(fname, ftype, structName, sz));
            } else {
                if (cur().type == "PUNCT_SEMI") eat();
                addSymbol(makeVarEntry(fname, ftype, structName));
            }
        } else {
            eat();
        }
    }
}

void fillSymbolTableFromTokens() {
    clearSymbolTable();
    pos = 0;
    string scope = "global";
    int blockDepth = 0;

    while (!atEnd()) {
        if (cur().type == "KEYWORD_DHANCHA") {
            eat();
            if (cur().type != "IDENTIFIER") continue;
            string firstId = cur().lex;
            eat();

            if (cur().type == "PUNCT_SHRO") {
                addSymbol(makeStructEntry(firstId, "global"));
                eat();
                collectStructFields(firstId);
                if (cur().type == "PUNCT_KHATAM") eat();
                if (cur().type == "PUNCT_SEMI") eat();
                continue;
            }

            if (cur().type == "IDENTIFIER") {
                string varName = cur().lex;
                eat();
                if (!symbolVisible(firstId, "global")) {
                    addErrorLine(cur().line, "unknown dhancha type '" + firstId + "'");
                }
                if (cur().type == "PUNCT_LBRACK") {
                    eat();
                    int sz = 0;
                    if (cur().type == "NUM_LITERAL") {
                        sz = stringToInt(cur().lex);
                        eat();
                    }
                    if (cur().type == "PUNCT_RBRACK") eat();
                    if (cur().type == "PUNCT_SEMI") eat();
                    addSymbol(makeArrayEntry(varName, firstId, scope, sz));
                } else {
                    if (cur().type == "PUNCT_SEMI") eat();
                    addSymbol(makeVarEntry(varName, firstId, scope));
                }
                continue;
            }
            continue;
        }

        if (isTypeTok(cur().type)) {
            string dtype = cur().lex;
            eat();
            if (cur().type != "IDENTIFIER") continue;
            string name = cur().lex;
            eat();

            if (cur().type == "PUNCT_LPAREN") {
                SymbolEntry fn = makeFuncEntry(name, dtype, "global");
                eat();
                collectParamsInto(fn);
                if (cur().type == "PUNCT_RPAREN") eat();
                addSymbol(fn);
                if (cur().type == "PUNCT_SHRO") {
                    eat();
                    scope = name;
                    blockDepth = 1;
                }
                continue;
            }

            if (cur().type == "PUNCT_LBRACK") {
                eat();
                int sz = 0;
                if (cur().type == "NUM_LITERAL") {
                    sz = stringToInt(cur().lex);
                    eat();
                }
                if (cur().type == "PUNCT_RBRACK") eat();
                if (cur().type == "PUNCT_SEMI") eat();
                addSymbol(makeArrayEntry(name, dtype, scope, sz));
                continue;
            }

            if (cur().type == "PUNCT_SEMI") {
                eat();
                addSymbol(makeVarEntry(name, dtype, scope));
                continue;
            }
            continue;
        }

        if (cur().type == "PUNCT_SHRO") {
            eat();
            if (scope != "global") blockDepth++;
            continue;
        }

        if (cur().type == "PUNCT_KHATAM") {
            eat();
            if (scope != "global") {
                blockDepth--;
                if (blockDepth <= 0) {
                    scope = "global";
                    blockDepth = 0;
                }
            }
            continue;
        }

        eat();
    }
}

bool isDeclNamePosition(int i) {
    if (i <= 0) return false;
    string prev = toks[i - 1].type;
    if (isTypeTok(prev)) return true;
    if (prev == "KEYWORD_DHANCHA") return true;
    if (prev == "IDENTIFIER" && i >= 2 && toks[i - 2].type == "KEYWORD_DHANCHA") return true;
    return false;
}

string typeOfPrimaryAt(int i, string scope, int& endPos) {
    endPos = i;
    if (i >= (int)toks.size()) return "";

    if (toks[i].type == "NUM_LITERAL") {
        endPos = i;
        return "int";
    }
    if (toks[i].type == "FLOAT_LITERAL") {
        endPos = i;
        return "float";
    }
    if (toks[i].type == "STR_LITERAL") {
        endPos = i;
        return "str";
    }

    if (toks[i].type == "IDENTIFIER") {
        string name = toks[i].lex;
        int line = toks[i].line;
        if (!symbolVisible(name, scope)) {
            addErrorLine(line, "undeclared '" + name + "'");
            endPos = i;
            return "";
        }
        SymbolEntry e = getVisibleSymbol(name, scope);
        string t = e.dataType;
        endPos = i;
        int j = i + 1;
        while (j < (int)toks.size()) {
            if (toks[j].type == "PUNCT_DOT") {
                j++;
                if (j < (int)toks.size() && toks[j].type == "IDENTIFIER") {
                    endPos = j;
                    j++;
                } else break;
            } else if (toks[j].type == "PUNCT_LBRACK") {
                j++;
                int idxEnd = j;
                string idxType = "";
                if (j < (int)toks.size()) {
                    if (toks[j].type == "NUM_LITERAL") {
                        idxType = "int";
                        idxEnd = j;
                    } else if (toks[j].type == "IDENTIFIER") {
                        string iname = toks[j].lex;
                        if (!symbolVisible(iname, scope)) {
                            addErrorLine(toks[j].line, "undeclared '" + iname + "'");
                        } else {
                            SymbolEntry ie = getVisibleSymbol(iname, scope);
                            idxType = ie.dataType;
                        }
                        idxEnd = j;
                    }
                }
                if (idxType != "" && idxType != "int") {
                    addErrorLine(toks[i].line, "array index should be int");
                }
                j = idxEnd + 1;
                if (j < (int)toks.size() && toks[j].type == "PUNCT_RBRACK") {
                    endPos = j;
                    j++;
                } else break;
                if (!e.isArray && e.dataType != "int" && e.dataType != "float" && e.dataType != "char") {
                }
            } else break;
        }
        return t;
    }

    if (toks[i].type == "PUNCT_LPAREN") {
        int j = i + 1;
        int depth = 1;
        string inner = "";
        int innerEnd = j;
        if (j < (int)toks.size()) {
            inner = typeOfPrimaryAt(j, scope, innerEnd);
        }
        j = innerEnd + 1;
        while (j < (int)toks.size() && depth > 0) {
            if (toks[j].type == "PUNCT_LPAREN") depth++;
            if (toks[j].type == "PUNCT_RPAREN") depth--;
            if (depth == 0) {
                endPos = j;
                return inner;
            }
            j++;
        }
        endPos = i;
        return inner;
    }

    return "";
}

string typeOfExpressionFrom(int start, string scope, int& endPos) {
    string left = typeOfPrimaryAt(start, scope, endPos);
    int j = endPos + 1;
    while (j < (int)toks.size()) {
        string tt = toks[j].type;
        if (tt == "OP_PLUS" || tt == "OP_MINUS" || tt == "OP_MUL" || tt == "OP_DIV" ||
            tt == "OP_EQ" || tt == "OP_NEQ" || tt == "OP_LT" || tt == "OP_GT" ||
            tt == "OP_LTE" || tt == "OP_GTE") {
            j++;
            int rEnd = j;
            string right = typeOfPrimaryAt(j, scope, rEnd);
            if (!typesCompatible(left, right)) {
                addErrorLine(toks[start].line, "type mismatch in expression");
            }
            if (tt == "OP_EQ" || tt == "OP_NEQ" || tt == "OP_LT" || tt == "OP_GT" ||
                tt == "OP_LTE" || tt == "OP_GTE") {
                left = "int";
            } else if (left == "float" || right == "float") {
                left = "float";
            } else if (left == "") left = right;
            endPos = rEnd;
            j = rEnd + 1;
        } else break;
    }
    return left;
}

void checkAssignment(int idPos, string scope) {
    int endPos = idPos;
    string lhs = typeOfPrimaryAt(idPos, scope, endPos);
    int j = endPos + 1;
    if (j >= (int)toks.size() || toks[j].type != "OP_MEDALO") return;
    j++;
    int rhsEnd = j;
    string rhs = typeOfExpressionFrom(j, scope, rhsEnd);
    if (!typesCompatible(lhs, rhs)) {
        addErrorLine(toks[idPos].line, "type mismatch in medalo assignment");
    }
}

void checkPoocho(int poochoPos, string scope) {
    int j = poochoPos + 1;
    if (j >= (int)toks.size() || toks[j].type != "PUNCT_LPAREN") return;
    j++;
    if (j >= (int)toks.size()) return;
    if (toks[j].type == "IDENTIFIER") {
        int endPos = j;
        typeOfPrimaryAt(j, scope, endPos);
    } else {
        addErrorLine(toks[poochoPos].line, "poocho needs a variable");
    }
}

void checkDikhao(int dikhaoPos, string scope) {
    int j = dikhaoPos + 1;
    if (j >= (int)toks.size() || toks[j].type != "PUNCT_LPAREN") return;
    j++;
    while (j < (int)toks.size() && toks[j].type != "PUNCT_RPAREN") {
        if (toks[j].type == "STR_LITERAL") {
            j++;
        } else if (toks[j].type == "IDENTIFIER" || toks[j].type == "NUM_LITERAL" ||
                   toks[j].type == "FLOAT_LITERAL" || toks[j].type == "PUNCT_LPAREN") {
            int endPos = j;
            typeOfExpressionFrom(j, scope, endPos);
            j = endPos + 1;
        } else if (toks[j].type == "PUNCT_COMMA") {
            j++;
        } else {
            j++;
        }
    }
}

void checkCondition(int afterParenPos, string scope) {
    int endPos = afterParenPos;
    string t = typeOfExpressionFrom(afterParenPos, scope, endPos);
    if (t == "str") {
        addErrorLine(toks[afterParenPos].line, "condition cannot be string");
    }
}

void runScopeAndTypeChecks() {
    string scope = "global";
    int blockDepth = 0;
    int i = 0;

    while (i < (int)toks.size()) {
        if (isTypeTok(toks[i].type) && i + 2 < (int)toks.size() &&
            toks[i + 1].type == "IDENTIFIER" && toks[i + 2].type == "PUNCT_LPAREN") {
            scope = toks[i + 1].lex;
            blockDepth = 0;
            i++;
            continue;
        }

        if (toks[i].type == "PUNCT_SHRO") {
            if (scope != "global") blockDepth++;
            i++;
            continue;
        }

        if (toks[i].type == "PUNCT_KHATAM") {
            if (scope != "global") {
                blockDepth--;
                if (blockDepth <= 0) {
                    scope = "global";
                    blockDepth = 0;
                }
            }
            i++;
            continue;
        }

        if (toks[i].type == "IDENTIFIER" && !isDeclNamePosition(i)) {
            int j = i + 1;
            bool isAssign = false;
            int k = i;
            int tmpEnd = i;
            typeOfPrimaryAt(i, scope, tmpEnd);
            k = tmpEnd + 1;
            if (k < (int)toks.size() && toks[k].type == "OP_MEDALO") {
                isAssign = true;
                checkAssignment(i, scope);
            } else if (!symbolVisible(toks[i].lex, scope)) {
                if (!(i + 1 < (int)toks.size() && toks[i + 1].type == "PUNCT_LPAREN")) {
                }
            }
            i = tmpEnd + 1;
            if (isAssign) {
                while (i < (int)toks.size() && toks[i].type != "PUNCT_SEMI") i++;
            }
            continue;
        }

        if (toks[i].type == "KEYWORD_POOCHO") {
            checkPoocho(i, scope);
            i++;
            continue;
        }

        if (toks[i].type == "KEYWORD_DIKHAO") {
            checkDikhao(i, scope);
            i++;
            continue;
        }

        if (toks[i].type == "KEYWORD_AGAR" || toks[i].type == "KEYWORD_JABTAK") {
            int j = i + 1;
            if (j < (int)toks.size() && toks[j].type == "PUNCT_LPAREN") {
                checkCondition(j + 1, scope);
            }
            i++;
            continue;
        }

        i++;
    }
}

void writeSymbolsFile(const string& path) {
    ofstream fout(path.c_str());
    if (!fout.is_open()) {
        cout << "cant write " << path << endl;
        return;
    }
    for (int i = 0; i < (int)symbolTable.size(); i++) {
        SymbolEntry e = symbolTable[i];
        fout << e.name << " | " << e.dataType << " | " << e.scope << " | " << entryKindLabel(e);
        if (e.isArray) fout << " | size=" << e.arraySize;
        fout << "\n";
    }
    fout.close();
}

void writeErrorsFile(const string& path) {
    ofstream fout(path.c_str());
    if (!fout.is_open()) return;
    for (int i = 0; i < (int)errors.size(); i++) {
        fout << errors[i] << "\n";
    }
    fout.close();
}

int main() {
    string inputFile = "tokens.txt";
    string outputFile = "symbols.txt";
    string errFile = "semantic_errors.txt";

    cout << "Citu Semantic Analyzer - commit 3" << endl;
    cout << "scope + undeclared/redeclare + basic type checks" << endl;
    cout << "reading: " << inputFile << endl;

    clearErrors();
    toks = readTokensFile(inputFile);
    if (toks.size() == 0) {
        cout << "no tokens (run lexer first)" << endl;
        return 1;
    }

    fillSymbolTableFromTokens();
    runScopeAndTypeChecks();

    printSymbolTable();
    printErrors();
    writeSymbolsFile(outputFile);
    writeErrorsFile(errFile);

    if (errorCount() == 0) cout << "semantic checks: OK" << endl;
    else cout << "semantic checks: FAILED" << endl;

    cout << "wrote " << outputFile << " and " << errFile << endl;
    return 0;
}
