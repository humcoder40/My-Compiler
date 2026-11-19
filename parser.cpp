#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

struct Tok {
    int line;
    string type;
    string lex;
};

struct Node {
    string type;
    string value;
    vector<Node> kids;
};

vector<Tok> toks;
int pos = 0;

bool isTypeName(const string& t) {
    if (t == "KEYWORD_INT") return true;
    if (t == "KEYWORD_FLOAT") return true;
    if (t == "KEYWORD_CHAR") return true;
    if (t == "KEYWORD_KHALI") return true;
    return false;
}

bool isBinOp(const string& t) {
    if (t == "OP_PLUS") return true;
    if (t == "OP_MINUS") return true;
    if (t == "OP_MUL") return true;
    if (t == "OP_DIV") return true;
    if (t == "OP_EQ") return true;
    if (t == "OP_NEQ") return true;
    if (t == "OP_LT") return true;
    if (t == "OP_GT") return true;
    if (t == "OP_LTE") return true;
    if (t == "OP_GTE") return true;
    return false;
}

string trimSpaces(string s) {
    while (s.size() > 0 && (s[0] == ' ' || s[0] == '\t')) s.erase(0, 1);
    while (s.size() > 0 && (s[s.size() - 1] == ' ' || s[s.size() - 1] == '\t')) s.erase(s.size() - 1, 1);
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

bool check(const string& typeName) {
    return cur().type == typeName;
}

bool checkLex(const string& lex) {
    return cur().lex == lex;
}

Tok eat() {
    Tok t = cur();
    if (!atEnd()) pos++;
    return t;
}

bool matchType(const string& typeName) {
    if (!check(typeName)) return false;
    eat();
    return true;
}

Tok need(const string& typeName, const string& msg) {
    if (!check(typeName)) {
        cout << "parse error: " << msg << " got " << cur().type << " '" << cur().lex << "' line " << cur().line << endl;
        Tok bad;
        bad.type = "ERROR";
        bad.lex = "";
        bad.line = cur().line;
        return bad;
    }
    return eat();
}

Node makeNode(const string& type) {
    Node n;
    n.type = type;
    n.value = "";
    return n;
}

Node makeNodeVal(const string& type, const string& value) {
    Node n;
    n.type = type;
    n.value = value;
    return n;
}

void addKid(Node& parent, const Node& kid) {
    parent.kids.push_back(kid);
}

string escapeJson(const string& s) {
    string out = "";
    for (int i = 0; i < (int)s.size(); i++) {
        char c = s[i];
        if (c == '\\') out = out + "\\\\";
        else if (c == '"') out = out + "\\\"";
        else if (c == '\n') out = out + "\\n";
        else out = out + c;
    }
    return out;
}

string spaces(int n) {
    string s = "";
    for (int i = 0; i < n; i++) s = s + " ";
    return s;
}

string nodeToJson(const Node& n, int indent) {
    string s = "";
    s = s + spaces(indent) + "{\n";
    s = s + spaces(indent + 2) + "\"nodeType\": \"" + n.type + "\"";
    if (n.value.size() > 0) {
        s = s + ",\n";
        s = s + spaces(indent + 2) + "\"value\": \"" + escapeJson(n.value) + "\"";
    }
    if (n.kids.size() > 0) {
        s = s + ",\n";
        s = s + spaces(indent + 2) + "\"children\": [\n";
        for (int i = 0; i < (int)n.kids.size(); i++) {
            s = s + nodeToJson(n.kids[i], indent + 4);
            if (i + 1 < (int)n.kids.size()) s = s + ",";
            s = s + "\n";
        }
        s = s + spaces(indent + 2) + "]\n";
    } else {
        s = s + "\n";
    }
    s = s + spaces(indent) + "}";
    return s;
}

void writeTreeFile(const string& path, const Node& root) {
    ofstream fout(path.c_str());
    if (!fout.is_open()) {
        cout << "cant write " << path << endl;
        return;
    }
    fout << nodeToJson(root, 0) << "\n";
    fout.close();
}

Node parseExpression();
Node parseStatement();
Node parseStatementList();

Node parseTypeNode() {
    Tok t = eat();
    return makeNodeVal("TYPE_NODE", t.lex);
}

Node parseLValueTail(Node base) {
    while (true) {
        if (matchType("PUNCT_DOT")) {
            Tok field = need("IDENTIFIER", "expected field name");
            Node mem = makeNodeVal("MEMBER_ACCESS_NODE", field.lex);
            addKid(mem, base);
            addKid(mem, makeNodeVal("IDENTIFIER_NODE", field.lex));
            base = mem;
        } else if (matchType("PUNCT_LBRACK")) {
            Node idx = parseExpression();
            need("PUNCT_RBRACK", "expected ]");
            Node acc = makeNode("ARRAY_ACCESS_NODE");
            addKid(acc, base);
            addKid(acc, idx);
            base = acc;
        } else {
            break;
        }
    }
    return base;
}

Node parseFactor() {
    if (check("IDENTIFIER")) {
        Tok id = eat();
        Node base = makeNodeVal("IDENTIFIER_NODE", id.lex);
        return parseLValueTail(base);
    }
    if (check("NUM_LITERAL")) {
        Tok n = eat();
        return makeNodeVal("NUM_LITERAL_NODE", n.lex);
    }
    if (check("FLOAT_LITERAL")) {
        Tok n = eat();
        return makeNodeVal("FLOAT_LITERAL_NODE", n.lex);
    }
    if (matchType("PUNCT_LPAREN")) {
        Node e = parseExpression();
        need("PUNCT_RPAREN", "expected )");
        return e;
    }
    cout << "parse error: bad factor at line " << cur().line << endl;
    return makeNode("EXPRESSION_NODE");
}

Node parseExpression() {
    Node left = parseFactor();
    while (isBinOp(cur().type)) {
        Tok op = eat();
        Node right = parseFactor();
        Node bin = makeNodeVal("EXPRESSION_NODE", op.lex);
        addKid(bin, left);
        addKid(bin, right);
        left = bin;
    }
    return left;
}

Node parseArg() {
    if (check("STR_LITERAL")) {
        Tok s = eat();
        return makeNodeVal("STR_LITERAL_NODE", s.lex);
    }
    return parseExpression();
}

Node parseVarDecl() {
    Node typ = parseTypeNode();
    Tok name = need("IDENTIFIER", "expected var name");
    if (matchType("PUNCT_LBRACK")) {
        Tok sz = need("NUM_LITERAL", "expected array size");
        need("PUNCT_RBRACK", "expected ]");
        need("PUNCT_SEMI", "expected ;");
        Node arr = makeNodeVal("ARRAY_DECL_NODE", name.lex);
        addKid(arr, typ);
        addKid(arr, makeNodeVal("NUM_LITERAL_NODE", sz.lex));
        return arr;
    }
    need("PUNCT_SEMI", "expected ;");
    Node var = makeNodeVal("VAR_DECL_NODE", name.lex);
    addKid(var, typ);
    return var;
}

Node parseStructVarDecl() {
    need("KEYWORD_DHANCHA", "expected dhancha");
    Tok tname = need("IDENTIFIER", "expected struct type");
    Tok vname = need("IDENTIFIER", "expected var name");
    Node n = makeNodeVal("STRUCT_VAR_DECL_NODE", vname.lex);
    addKid(n, makeNodeVal("TYPE_NODE", tname.lex));
    if (matchType("PUNCT_LBRACK")) {
        Tok sz = need("NUM_LITERAL", "expected size");
        need("PUNCT_RBRACK", "expected ]");
        addKid(n, makeNodeVal("NUM_LITERAL_NODE", sz.lex));
    }
    need("PUNCT_SEMI", "expected ;");
    return n;
}

Node parseAssignment() {
    Tok id = need("IDENTIFIER", "expected id");
    Node lhs = makeNodeVal("IDENTIFIER_NODE", id.lex);
    lhs = parseLValueTail(lhs);
    need("OP_MEDALO", "expected medalo");
    Node rhs = parseExpression();
    need("PUNCT_SEMI", "expected ;");
    Node asg = makeNodeVal("ASSIGNMENT_NODE", "medalo");
    addKid(asg, lhs);
    addKid(asg, rhs);
    return asg;
}

Node parseIf() {
    need("KEYWORD_AGAR", "expected agar");
    need("PUNCT_LPAREN", "expected (");
    Node cond = parseExpression();
    need("PUNCT_RPAREN", "expected )");
    need("PUNCT_SHRO", "expected shro");
    Node body = parseStatementList();
    need("PUNCT_KHATAM", "expected khatam");
    Node n = makeNode("IF_NODE");
    addKid(n, cond);
    addKid(n, body);
    if (matchType("KEYWORD_WARNA")) {
        need("PUNCT_SHRO", "expected shro");
        Node ebody = parseStatementList();
        need("PUNCT_KHATAM", "expected khatam");
        Node el = makeNode("ELSE_NODE");
        addKid(el, ebody);
        addKid(n, el);
    }
    return n;
}

Node parseWhile() {
    need("KEYWORD_JABTAK", "expected jabtak");
    need("PUNCT_LPAREN", "expected (");
    Node cond = parseExpression();
    need("PUNCT_RPAREN", "expected )");
    need("PUNCT_SHRO", "expected shro");
    Node body = parseStatementList();
    need("PUNCT_KHATAM", "expected khatam");
    Node n = makeNode("WHILE_NODE");
    addKid(n, cond);
    addKid(n, body);
    return n;
}

Node parseFor() {
    need("KEYWORD_DOHRAO", "expected dohrao");
    need("PUNCT_LPAREN", "expected (");
    Tok id1 = need("IDENTIFIER", "expected id");
    need("OP_MEDALO", "expected medalo");
    Node e1 = parseExpression();
    need("PUNCT_SEMI", "expected ;");
    Node cond = parseExpression();
    need("PUNCT_SEMI", "expected ;");
    Tok id2 = need("IDENTIFIER", "expected id");
    need("OP_MEDALO", "expected medalo");
    Node e2 = parseExpression();
    need("PUNCT_RPAREN", "expected )");
    need("PUNCT_SHRO", "expected shro");
    Node body = parseStatementList();
    need("PUNCT_KHATAM", "expected khatam");

    Node init = makeNodeVal("ASSIGNMENT_NODE", "medalo");
    addKid(init, makeNodeVal("IDENTIFIER_NODE", id1.lex));
    addKid(init, e1);

    Node step = makeNodeVal("ASSIGNMENT_NODE", "medalo");
    addKid(step, makeNodeVal("IDENTIFIER_NODE", id2.lex));
    addKid(step, e2);

    Node n = makeNode("FOR_NODE");
    addKid(n, init);
    addKid(n, cond);
    addKid(n, step);
    addKid(n, body);
    return n;
}

Node parseSwitchCases() {
    Node box = makeNode("BLOCK_NODE");
    while (check("KEYWORD_SOORAT") || check("KEYWORD_BAQI")) {
        if (matchType("KEYWORD_SOORAT")) {
            Tok num = need("NUM_LITERAL", "expected case num");
            need("PUNCT_COLON", "expected :");
            Node c = makeNodeVal("CASE_NODE", num.lex);
            addKid(c, parseStatementList());
            addKid(box, c);
        } else {
            need("KEYWORD_BAQI", "expected baqi");
            need("PUNCT_COLON", "expected :");
            Node d = makeNode("DEFAULT_NODE");
            addKid(d, parseStatementList());
            addKid(box, d);
            break;
        }
    }
    return box;
}

Node parseSwitch() {
    need("KEYWORD_CHUNO", "expected chuno");
    need("PUNCT_LPAREN", "expected (");
    Tok id = need("IDENTIFIER", "expected id");
    Node target = makeNodeVal("IDENTIFIER_NODE", id.lex);
    target = parseLValueTail(target);
    need("PUNCT_RPAREN", "expected )");
    need("PUNCT_SHRO", "expected shro");
    Node cases = parseSwitchCases();
    need("PUNCT_KHATAM", "expected khatam");
    Node n = makeNode("SWITCH_NODE");
    addKid(n, target);
    addKid(n, cases);
    return n;
}

Node parsePrint() {
    need("KEYWORD_DIKHAO", "expected dikhao");
    need("PUNCT_LPAREN", "expected (");
    Node n = makeNode("PRINT_NODE");
    addKid(n, parseArg());
    while (matchType("PUNCT_COMMA")) {
        addKid(n, parseArg());
    }
    need("PUNCT_RPAREN", "expected )");
    need("PUNCT_SEMI", "expected ;");
    return n;
}

Node parseInput() {
    need("KEYWORD_POOCHO", "expected poocho");
    need("PUNCT_LPAREN", "expected (");
    Tok id = need("IDENTIFIER", "expected id");
    Node target = makeNodeVal("IDENTIFIER_NODE", id.lex);
    target = parseLValueTail(target);
    need("PUNCT_RPAREN", "expected )");
    need("PUNCT_SEMI", "expected ;");
    Node n = makeNode("INPUT_NODE");
    addKid(n, target);
    return n;
}

Node parseStatement() {
    if (isTypeName(cur().type)) return parseVarDecl();
    if (check("KEYWORD_DHANCHA")) return parseStructVarDecl();
    if (check("KEYWORD_AGAR")) return parseIf();
    if (check("KEYWORD_JABTAK")) return parseWhile();
    if (check("KEYWORD_DOHRAO")) return parseFor();
    if (check("KEYWORD_CHUNO")) return parseSwitch();
    if (check("KEYWORD_DIKHAO")) return parsePrint();
    if (check("KEYWORD_POOCHO")) return parseInput();

    if (matchType("KEYWORD_WAPIS")) {
        Node e = parseExpression();
        need("PUNCT_SEMI", "expected ;");
        Node n = makeNode("RETURN_NODE");
        addKid(n, e);
        return n;
    }
    if (matchType("KEYWORD_NIKAL")) {
        need("PUNCT_SEMI", "expected ;");
        return makeNode("BREAK_NODE");
    }
    if (matchType("KEYWORD_JARI")) {
        need("PUNCT_SEMI", "expected ;");
        return makeNode("CONTINUE_NODE");
    }
    if (check("IDENTIFIER")) return parseAssignment();

    cout << "parse error: weird statement " << cur().type << " line " << cur().line << endl;
    eat();
    return makeNode("BLOCK_NODE");
}

Node parseStatementList() {
    Node block = makeNode("BLOCK_NODE");
    while (!atEnd() && !check("PUNCT_KHATAM") && !check("KEYWORD_SOORAT") && !check("KEYWORD_BAQI")) {
        addKid(block, parseStatement());
    }
    return block;
}

Node parseParamList() {
    Node list = makeNode("PARAM_LIST_NODE");
    if (check("PUNCT_RPAREN")) return list;
    while (true) {
        Node typ = parseTypeNode();
        Tok name = need("IDENTIFIER", "expected param name");
        Node p = makeNodeVal("VAR_DECL_NODE", name.lex);
        addKid(p, typ);
        addKid(list, p);
        if (!matchType("PUNCT_COMMA")) break;
    }
    return list;
}

Node parseStructDecl() {
    need("KEYWORD_DHANCHA", "expected dhancha");
    Tok name = need("IDENTIFIER", "expected struct name");
    need("PUNCT_SHRO", "expected shro");
    Node n = makeNodeVal("STRUCT_DECL_NODE", name.lex);
    while (isTypeName(cur().type)) {
        Node typ = parseTypeNode();
        Tok fname = need("IDENTIFIER", "expected field");
        if (matchType("PUNCT_LBRACK")) {
            Tok sz = need("NUM_LITERAL", "expected size");
            need("PUNCT_RBRACK", "expected ]");
            need("PUNCT_SEMI", "expected ;");
            Node f = makeNodeVal("ARRAY_DECL_NODE", fname.lex);
            addKid(f, typ);
            addKid(f, makeNodeVal("NUM_LITERAL_NODE", sz.lex));
            addKid(n, f);
        } else {
            need("PUNCT_SEMI", "expected ;");
            Node f = makeNodeVal("VAR_DECL_NODE", fname.lex);
            addKid(f, typ);
            addKid(n, f);
        }
    }
    need("PUNCT_KHATAM", "expected khatam");
    need("PUNCT_SEMI", "expected ;");
    return n;
}

Node parseDeclaration() {
    if (check("KEYWORD_DHANCHA")) {
        if (look(2).type == "PUNCT_SHRO") return parseStructDecl();
        return parseStructVarDecl();
    }

    Node typ = parseTypeNode();
    Tok name = need("IDENTIFIER", "expected name");

    if (matchType("PUNCT_LPAREN")) {
        Node fn = makeNodeVal("FUNC_DECL_NODE", name.lex);
        addKid(fn, typ);
        addKid(fn, parseParamList());
        need("PUNCT_RPAREN", "expected )");
        need("PUNCT_SHRO", "expected shro");
        Node body = parseStatementList();
        need("PUNCT_KHATAM", "expected khatam");
        addKid(fn, body);
        return fn;
    }

    if (matchType("PUNCT_LBRACK")) {
        Tok sz = need("NUM_LITERAL", "expected size");
        need("PUNCT_RBRACK", "expected ]");
        need("PUNCT_SEMI", "expected ;");
        Node arr = makeNodeVal("ARRAY_DECL_NODE", name.lex);
        addKid(arr, typ);
        addKid(arr, makeNodeVal("NUM_LITERAL_NODE", sz.lex));
        return arr;
    }

    need("PUNCT_SEMI", "expected ;");
    Node var = makeNodeVal("VAR_DECL_NODE", name.lex);
    addKid(var, typ);
    return var;
}

Node parseProgram() {
    Node prog = makeNode("PROGRAM_NODE");
    while (!atEnd()) {
        addKid(prog, parseDeclaration());
    }
    return prog;
}

int main() {
    string inputFile = "tokens.txt";
    string outputFile = "tree.json";

    cout << "Citu Parser" << endl;
    cout << "reading: " << inputFile << endl;

    toks = readTokensFile(inputFile);
    if (toks.size() == 0) {
        cout << "no tokens" << endl;
        return 1;
    }

    pos = 0;
    Node tree = parseProgram();
    writeTreeFile(outputFile, tree);

    cout << "done. wrote " << outputFile << endl;
    cout << "top kids: " << tree.kids.size() << endl;
    return 0;
}
