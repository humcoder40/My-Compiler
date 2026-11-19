#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cctype>
using namespace std;

bool isLetter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

bool isSpaceChar(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

bool startsWith(const string& s, int i, const string& word) {
    if (i + (int)word.size() > (int)s.size()) return false;
    for (int k = 0; k < (int)word.size(); k++) {
        if (s[i + k] != word[k]) return false;
    }
    return true;
}

bool isKeywordWord(const string& w) {
    if (w == "int") return true;
    if (w == "float") return true;
    if (w == "char") return true;
    if (w == "khali") return true;
    if (w == "agar") return true;
    if (w == "warna") return true;
    if (w == "jabtak") return true;
    if (w == "dohrao") return true;
    if (w == "chuno") return true;
    if (w == "soorat") return true;
    if (w == "baqi") return true;
    if (w == "nikal") return true;
    if (w == "jari") return true;
    if (w == "wapis") return true;
    if (w == "dhancha") return true;
    if (w == "dikhao") return true;
    if (w == "poocho") return true;
    return false;
}

string keywordTypeName(const string& w) {
    if (w == "int") return "KEYWORD_INT";
    if (w == "float") return "KEYWORD_FLOAT";
    if (w == "char") return "KEYWORD_CHAR";
    if (w == "khali") return "KEYWORD_KHALI";
    if (w == "agar") return "KEYWORD_AGAR";
    if (w == "warna") return "KEYWORD_WARNA";
    if (w == "jabtak") return "KEYWORD_JABTAK";
    if (w == "dohrao") return "KEYWORD_DOHRAO";
    if (w == "chuno") return "KEYWORD_CHUNO";
    if (w == "soorat") return "KEYWORD_SOORAT";
    if (w == "baqi") return "KEYWORD_BAQI";
    if (w == "nikal") return "KEYWORD_NIKAL";
    if (w == "jari") return "KEYWORD_JARI";
    if (w == "wapis") return "KEYWORD_WAPIS";
    if (w == "dhancha") return "KEYWORD_DHANCHA";
    if (w == "dikhao") return "KEYWORD_DIKHAO";
    if (w == "poocho") return "KEYWORD_POOCHO";
    return "UNKNOWN";
}

bool isBlockWord(const string& w) {
    return w == "shro" || w == "khatam";
}

string blockTypeName(const string& w) {
    if (w == "shro") return "PUNCT_SHRO";
    if (w == "khatam") return "PUNCT_KHATAM";
    return "UNKNOWN";
}

bool isAssignWord(const string& w) {
    return w == "medalo";
}

string makeTokenLine(int line, const string& typeName, const string& lexeme) {
    string out = "<";
    out = out + to_string(line);
    out = out + ", ";
    out = out + typeName;
    out = out + ", ";
    out = out + lexeme;
    out = out + ">";
    return out;
}

void addToken(vector<string>& tokens, int line, const string& typeName, const string& lexeme) {
    tokens.push_back(makeTokenLine(line, typeName, lexeme));
}

string readWholeFile(const string& path) {
    ifstream fin(path.c_str());
    if (!fin.is_open()) {
        cout << "cant open input file: " << path << endl;
        return "";
    }
    string all = "";
    string line;
    while (getline(fin, line)) {
        all = all + line + "\n";
    }
    fin.close();
    return all;
}

void writeTokensFile(const string& path, const vector<string>& tokens) {
    ofstream fout(path.c_str());
    if (!fout.is_open()) {
        cout << "cant write output file: " << path << endl;
        return;
    }
    for (int i = 0; i < (int)tokens.size(); i++) {
        fout << tokens[i] << "\n";
    }
    fout.close();
}

void skipSpaces(const string& src, int& i, int& line) {
    while (i < (int)src.size() && isSpaceChar(src[i])) {
        if (src[i] == '\n') line++;
        i++;
    }
}

void skipLineComment(const string& src, int& i, int& line) {
    if (i + 1 < (int)src.size() && src[i] == '/' && src[i + 1] == '/') {
        while (i < (int)src.size() && src[i] != '\n') i++;
    }
}

string readIdentifier(const string& src, int& i) {
    string w = "";
    while (i < (int)src.size() && (isLetter(src[i]) || isDigit(src[i]))) {
        w = w + src[i];
        i++;
    }
    return w;
}

string readNumber(const string& src, int& i, bool& isFloat) {
    string w = "";
    isFloat = false;
    while (i < (int)src.size() && isDigit(src[i])) {
        w = w + src[i];
        i++;
    }
    if (i < (int)src.size() && src[i] == '.' && i + 1 < (int)src.size() && isDigit(src[i + 1])) {
        isFloat = true;
        w = w + src[i];
        i++;
        while (i < (int)src.size() && isDigit(src[i])) {
            w = w + src[i];
            i++;
        }
    }
    return w;
}

string readStringLiteral(const string& src, int& i, int line) {
    string w = "";
    i++;
    while (i < (int)src.size() && src[i] != '"') {
        if (src[i] == '\n') {
            cout << "warning: string not closed near line " << line << endl;
            break;
        }
        w = w + src[i];
        i++;
    }
    if (i < (int)src.size() && src[i] == '"') i++;
    return w;
}

bool tryTwoCharOp(const string& src, int& i, string& op) {
    if (i + 1 >= (int)src.size()) return false;
    char a = src[i];
    char b = src[i + 1];
    if (a == '=' && b == '=') { op = "=="; i += 2; return true; }
    if (a == '!' && b == '=') { op = "!="; i += 2; return true; }
    if (a == '<' && b == '=') { op = "<="; i += 2; return true; }
    if (a == '>' && b == '=') { op = ">="; i += 2; return true; }
    return false;
}

string singleOpType(char c) {
    if (c == '+') return "OP_PLUS";
    if (c == '-') return "OP_MINUS";
    if (c == '*') return "OP_MUL";
    if (c == '/') return "OP_DIV";
    if (c == '<') return "OP_LT";
    if (c == '>') return "OP_GT";
    if (c == ';') return "PUNCT_SEMI";
    if (c == ',') return "PUNCT_COMMA";
    if (c == '(') return "PUNCT_LPAREN";
    if (c == ')') return "PUNCT_RPAREN";
    if (c == '[') return "PUNCT_LBRACK";
    if (c == ']') return "PUNCT_RBRACK";
    if (c == '.') return "PUNCT_DOT";
    if (c == ':') return "PUNCT_COLON";
    return "UNKNOWN";
}

string twoCharOpType(const string& op) {
    if (op == "==") return "OP_EQ";
    if (op == "!=") return "OP_NEQ";
    if (op == "<=") return "OP_LTE";
    if (op == ">=") return "OP_GTE";
    return "UNKNOWN";
}

bool isSingleOpChar(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' ||
           c == '<' || c == '>' || c == ';' || c == ',' ||
           c == '(' || c == ')' || c == '[' || c == ']' ||
           c == '.' || c == ':';
}

void lexSource(const string& src, vector<string>& tokens) {
    int i = 0;
    int line = 1;

    while (i < (int)src.size()) {
        skipSpaces(src, i, line);
        if (i >= (int)src.size()) break;

        if (i + 1 < (int)src.size() && src[i] == '/' && src[i + 1] == '/') {
            skipLineComment(src, i, line);
            continue;
        }

        if (src[i] == '"') {
            string s = readStringLiteral(src, i, line);
            addToken(tokens, line, "STR_LITERAL", s);
            continue;
        }

        if (isLetter(src[i])) {
            string w = readIdentifier(src, i);
            if (isKeywordWord(w)) {
                addToken(tokens, line, keywordTypeName(w), w);
            } else if (isBlockWord(w)) {
                addToken(tokens, line, blockTypeName(w), w);
            } else if (isAssignWord(w)) {
                addToken(tokens, line, "OP_MEDALO", w);
            } else {
                addToken(tokens, line, "IDENTIFIER", w);
            }
            continue;
        }

        if (isDigit(src[i])) {
            bool isFloat = false;
            string n = readNumber(src, i, isFloat);
            if (isFloat) addToken(tokens, line, "FLOAT_LITERAL", n);
            else addToken(tokens, line, "NUM_LITERAL", n);
            continue;
        }

        string op2 = "";
        if (tryTwoCharOp(src, i, op2)) {
            addToken(tokens, line, twoCharOpType(op2), op2);
            continue;
        }

        if (isSingleOpChar(src[i])) {
            char c = src[i];
            string one = "";
            one = one + c;
            addToken(tokens, line, singleOpType(c), one);
            i++;
            continue;
        }

        cout << "unknown char '" << src[i] << "' at line " << line << endl;
        i++;
    }
}

int main() {
    string inputFile = "sample_input.citu";
    string outputFile = "tokens.txt";

    cout << "Citu Lexer" << endl;
    cout << "reading: " << inputFile << endl;

    string src = readWholeFile(inputFile);
    if (src == "") {
        return 1;
    }

    vector<string> tokens;
    lexSource(src, tokens);

    writeTokensFile(outputFile, tokens);

    cout << "done. wrote " << tokens.size() << " tokens to " << outputFile << endl;

    int show = 10;
    if ((int)tokens.size() < show) show = (int)tokens.size();
    cout << "first tokens:" << endl;
    for (int i = 0; i < show; i++) {
        cout << tokens[i] << endl;
    }

    return 0;
}
