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

int symbolTableSize() {
    return (int)symbolTable.size();
}

void addSymbol(SymbolEntry e) {
    symbolTable.push_back(e);
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
    if (e.isArray) {
        cout << " size=" << e.arraySize;
    }
    if (e.isFunction && e.parameterTypes.size() > 0) {
        cout << " params=";
        for (int i = 0; i < (int)e.parameterTypes.size(); i++) {
            cout << e.parameterTypes[i];
            if (i + 1 < (int)e.parameterTypes.size()) cout << ",";
        }
    }
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

int stringToInt(const string& s) {
    int n = 0;
    for (int i = 0; i < (int)s.size(); i++) {
        if (s[i] >= '0' && s[i] <= '9') n = n * 10 + (s[i] - '0');
    }
    return n;
}

void skipUntilKhatamSemi() {
    int depth = 0;
    while (!atEnd()) {
        if (cur().type == "PUNCT_SHRO") depth++;
        if (cur().type == "PUNCT_KHATAM") {
            eat();
            if (depth <= 1) {
                if (cur().type == "PUNCT_SEMI") eat();
                return;
            }
            depth--;
            continue;
        }
        eat();
    }
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
                if (cur().type == "PUNCT_LBRACK") {
                    eat();
                    int sz = 0;
                    if (cur().type == "NUM_LITERAL") {
                        sz = stringToInt(cur().lex);
                        eat();
                    }
                    if (cur().type == "PUNCT_RBRACK") eat();
                    if (cur().type == "PUNCT_SEMI") eat();
                    SymbolEntry e = makeArrayEntry(varName, firstId, scope, sz);
                    addSymbol(e);
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

int main() {
    string inputFile = "tokens.txt";
    string outputFile = "symbols.txt";

    cout << "Citu Semantic Analyzer - commit 2" << endl;
    cout << "fill symbol table from declarations" << endl;
    cout << "reading: " << inputFile << endl;

    toks = readTokensFile(inputFile);
    if (toks.size() == 0) {
        cout << "no tokens (run lexer first)" << endl;
        return 1;
    }

    fillSymbolTableFromTokens();
    printSymbolTable();
    writeSymbolsFile(outputFile);
    cout << "wrote " << outputFile << endl;

    return 0;
}
