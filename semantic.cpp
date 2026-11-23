#include <iostream>
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

vector<SymbolEntry> symbolTable;

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
    e.isFunction = false;
    e.isStruct = false;
    e.isArray = false;
    e.arraySize = 0;
    return e;
}

SymbolEntry makeArrayEntry(string name, string dataType, string scope, int size) {
    SymbolEntry e = makeEmptyEntry();
    e.name = name;
    e.dataType = dataType;
    e.scope = scope;
    e.isFunction = false;
    e.isStruct = false;
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
    e.isStruct = false;
    e.isArray = false;
    e.arraySize = 0;
    return e;
}

SymbolEntry makeStructEntry(string name, string scope) {
    SymbolEntry e = makeEmptyEntry();
    e.name = name;
    e.dataType = name;
    e.scope = scope;
    e.isFunction = false;
    e.isStruct = true;
    e.isArray = false;
    e.arraySize = 0;
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

string paramTypesToString(const SymbolEntry& e) {
    string s = "";
    for (int i = 0; i < (int)e.parameterTypes.size(); i++) {
        s = s + e.parameterTypes[i];
        if (i + 1 < (int)e.parameterTypes.size()) s = s + ",";
    }
    return s;
}

void demoFillFakeSymbols() {
    clearSymbolTable();

    SymbolEntry f = makeFuncEntry("asal", "khali", "global");
    addSymbol(f);

    SymbolEntry v1 = makeVarEntry("qeemat1", "int", "asal");
    addSymbol(v1);

    SymbolEntry v2 = makeVarEntry("jama", "int", "asal");
    addSymbol(v2);

    SymbolEntry arr = makeArrayEntry("a", "int", "asal", 5);
    addSymbol(arr);

    SymbolEntry st = makeStructEntry("Student", "global");
    addSymbol(st);
}

int main() {
    cout << "Citu Semantic Analyzer - commit 1" << endl;
    cout << "symbol table skeleton only" << endl;

    demoFillFakeSymbols();
    printSymbolTable();

    cout << "lookup jama in asal: ";
    if (symbolExists("jama", "asal")) {
        int idx = findSymbolIndex("jama", "asal");
        printOneSymbol(getSymbolAt(idx));
    } else {
        cout << "not found" << endl;
    }

    cout << "lookup noor in asal: ";
    if (symbolExists("noor", "asal")) cout << "found" << endl;
    else cout << "not found" << endl;

    return 0;
}
