#include <bits/stdc++.h>
using namespace std;

enum Nonterminal {
    program = 0, block, constdef, constdefpri, vardef, vardefpri, procdef,
    statement, statementpri, condition, expression, expressionpri,
    term, termpri, factor, ident, number
,__Nonterminal_Count};

const char* symbols[] = {
    "program",
    "block",
    "constdef",
    "constdefpri",
    "vardef",
    "vardefpri",
    "procdef",
    "statement",
    "statementpri",
    "condition",
    "expression",
    "expressionpri",
    "term",
    "termpri",
    "factor",
    "ident",
    "number"
};

#include "utils.hpp"

namespace PL_0 {
    class Stack {
        private:
            static const int STACK_MAX = 1e6;
            int *stack;
            int *__stack_top, *__stack_bottom;
        public:
            Stack(int *st, int *sb, int stack_max = STACK_MAX) {
                stack = new int[STACK_MAX];
                __stack_top = st;
                __stack_bottom = sb;
                (*__stack_top) = 0;
                (*__stack_bottom) = 1;
            }
            ~Stack() { delete [] stack; }
            inline void push(int x) { stack[++ (*__stack_top)] = x; }
            inline int pop() { return stack[(*__stack_top) --]; }
            inline int& get(int idx) { return stack[(*__stack_bottom) + idx]; }
            inline void print(int l, int r) { printf("[ "); for(int i = l ; i <= r ; ++ i) printf("%d, ", stack[i]); puts("]"); }
    };

    class Opcode {
        private:
            vector<vector<int>> bin; // {op, data1, data2, ...}
        public:
            enum class BasicCommand {
                push = 0, pop, jff,             // push, pop, jff
                plus, minus, multi, div,        // +, -, *, /
                odd, eq, neq, lt, le, gt, ge,   // odd, =, #, <, <=, >, >=
                inp, out                        // ?, !
            };

            inline int size() const { return (int) bin.size(); }
            inline Opcode& modify(int idx, vector<int> odt) { bin[idx] = odt; return *this; }
            inline Opcode& add(vector<int> odt) {
                bin.push_back(odt);
                return *this;
            }
            inline vector<int> get(int cnt) const {
                return 0 <= cnt && cnt < (int) bin.size() ? bin[cnt] : vector<int> ();
            }

            inline static vector<int> bc_push(int ty, int data) { // ty = 0/1/2: local_var/global_var/immmediate_number
                return { (int) BasicCommand :: push, ty, data };
            }
            inline static vector<int> bc_push_loc(int data) { return bc_push(0, data); }
            inline static vector<int> bc_push_glo(int data) { return bc_push(1, data); }
            inline static vector<int> bc_push_imm(int data) { return bc_push(2, data); }
            inline static vector<int> bc_pop(int ty, int data) { // ty = 0/1: local_var/global_var
                return { (int) BasicCommand :: pop, ty, data };
            }
            inline static vector<int> bc_pop_loc(int data) { return bc_pop(0, data); }
            inline static vector<int> bc_pop_glo(int data) { return bc_pop(1, data); }
            inline static vector<int> bc_jff(int data) {
                return { (int) BasicCommand :: jff, data };
            }

            inline static vector<int> bc_opr(string op) {
                if(op == "odd") return { (int) BasicCommand :: odd };
                else if(op == "=") return { (int) BasicCommand :: eq };
                else if(op == "#") return { (int) BasicCommand :: neq };
                else if(op == "<") return { (int) BasicCommand :: lt };
                else if(op == "<=") return { (int) BasicCommand :: le };
                else if(op == ">") return { (int) BasicCommand :: gt };
                else if(op == ">=") return { (int) BasicCommand :: ge };
                else if(op == "+") return { (int) BasicCommand :: plus };
                else if(op == "-") return { (int) BasicCommand :: minus };
                else if(op == "*") return { (int) BasicCommand :: multi };
                else if(op == "/") return { (int) BasicCommand :: div };
                else if(op == "?") return { (int) BasicCommand :: inp };
                else if(op == "!") return { (int) BasicCommand :: out };
                else errorlog("unknow operator: %s", op.c_str());
            }
    };


    void initial(string filename) {
        Lexer lexer;
        Parser parser;

        string src = FileManager :: readFileString(filename);

        do {    // init regex
            string title = "Lexer Output";
            vector<string> head = { "idx", "TokenType", "raw" };
            vector<vector<string> > body;
            int cnt = 0;
            #define bpb(args...) body.push_back({ to_string(++ cnt), args })
            #define tpb(args...) (tokens.push_back({ args }))
            lexer.feed("[0-9][0-9]*",
                        [&](string raw) {
                            bpb("[number]", raw);
                            tpb(TokenType :: number, raw);
                        })
                 .feed("[_a-zA-Z][_a-zA-Z0-9]*", // identifiers 
                        [&](string raw) {
                            bpb("[ident]", raw);
                            tpb(TokenType :: ident, raw);
                        })
                 .feed("\\:\\=",
                        [&](string raw) {
                            bpb("[sym_assign]", raw);
                            tpb(TokenType :: sym_assign, raw);
                        })
                 .feed("\\<\\=",
                        [&](string raw) {
                            bpb("[sym_le]", raw);
                            tpb(TokenType :: sym_le, raw);
                        })
                 .feed("\\>\\=",
                        [&](string raw) {
                            bpb("[sym_ge]", raw);
                            tpb(TokenType :: sym_ge, raw);
                        })
                 .feed("[!?:;.,#<=>+-\\*/\\(\\)]",
                        [&](string raw) {
                            bpb("[sym]", raw);
                            tpb(TokenType :: sym, raw);
                        })
                 .feed("\\{[\f\n\r\t\v -z|~]*\\}",
                        [&](string raw) {
                            bpb("[anno]", raw);
                        })
                 .feed("[ \f\n\r\t\v][ \f\n\r\t\v]*",
                        [&](string raw) {
                            // pass and go on because of the blank chars
                        })
                 .match(src);
            #undef bpb
            #undef tpb

            Table tab(title, head, body);
            tab.show();
            FileManager :: writeFileString("Lexer.out", tab.getshowstr());
        } while(0);

        do {    // init CFG
            #define T(val) (parser.getTerminalNode(val))    // Terminal
            #define N(name) (parser.getNonterminalNode(name)) // Nonterminal
            N(program) -> add({ N(block), T(".") });
            N(block) -> add({ N(constdef), N(vardef), N(procdef), N(statement) });
            N(constdef) -> add({ T("const"), N(ident), T("="), N(number), N(constdefpri), T(";") })
                        -> add({ T("") });
            N(constdefpri) -> add({ T(","), N(ident), T("="), N(number), N(constdefpri) })
                           -> add({ T("") });
            N(vardef) -> add({ T("var"), N(ident), N(vardefpri), T(";") })
                      -> add({ T("") });
            N(vardefpri) -> add({ T(","), N(ident), N(vardefpri) })
                         -> add({ T("") });
            N(procdef) -> add({ T("procedure"), N(ident), T(";"), N(constdef), N(vardef), N(statement), T(";"), N(procdef) })
                       -> add({ T("") });
            N(condition) -> add({ T("odd"), N(expression) })
                         -> add({ N(expression), T("="),  N(expression) })
                         -> add({ N(expression), T("#"),  N(expression) })
                         -> add({ N(expression), T("<"),  N(expression) })
                         -> add({ N(expression), T("<="), N(expression) })
                         -> add({ N(expression), T(">"),  N(expression) })
                         -> add({ N(expression), T(">="), N(expression) });
            N(expression) -> add({ N(term), N(expressionpri) })
                          -> add({ T("+"), N(term), N(expressionpri) })
                          -> add({ T("-"), N(term), N(expressionpri) });
            N(expressionpri) -> add({ T("+"), N(term), N(expressionpri) })
                             -> add({ T("-"), N(term), N(expressionpri) })
                             -> add({ T("") });
            N(term) -> add({ N(factor), N(termpri) });
            N(termpri) -> add({ T("*"), N(factor), N(termpri) })
                       -> add({ T("/"), N(factor), N(termpri) })
                       -> add({ T("") });
            N(factor) -> add({ N(ident) })
                      -> add({ N(number) })
                      -> add({ T("("), N(expression), T(")") })
                      -> add({ T("+"), T("("), N(expression), T(")") })
                      -> add({ T("-"), T("("), N(expression), T(")") })
                      -> add({ T("+"), N(number) })
                      -> add({ T("-"), N(number) })
                      -> add({ T("+"), N(ident) })
                      -> add({ T("-"), N(ident) });

            N(ident) -> setTokenType(TokenType :: ident)
                     -> setNodeType(NodeType :: Terminal);
            N(number) -> setTokenType(TokenType :: number)
                      -> setNodeType(NodeType :: Terminal);
            N(statement) -> add({ N(ident), T(":="), N(expression) })
                         -> add({ T("call"), N(ident) })
                         -> add({ T("?"), N(ident) })
                         -> add({ T("!"), N(expression) })
                         -> add({ T("begin"), N(statement), N(statementpri), T("end") })
                         -> add({ T("if"), N(condition), T("then"), N(statement) })
                         -> add({ T("while"), N(condition), T("do"), N(statement) });
            N(statementpri) -> add({ T(";"), N(statement), N(statementpri) })
                            -> add({ T("") });
            #undef T
            #undef N
        } while(0);

        Node* astroot = parser.match(parser.getNonterminalNode(program));
        assert(astroot);

        string ASTstr_before = Node :: printAST(astroot); // before purify

        do {    // purify AST
            function<void(Node*)> dfs = [&] (Node *nd) {
                if(nd -> type == NodeType :: Nonterminal) {
                    vector<Node*> nastc;
                    
                    // clear *pri part
                    // constdefpri vardefpri expressionpri termpri statementpri
                    walka(x, nd -> astchild) {
                        dfs(x);
                        if(x -> nonterminal_name == "termpri"
                        || x -> nonterminal_name == "expressionpri"
                        || x -> nonterminal_name == "statementpri"
                        || x -> nonterminal_name == "constdefpri"
                        || x -> nonterminal_name == "vardefpri") {
                            walka(y, x -> astchild) {
                                nastc.push_back(y);
                            }
                            x -> child.clear(), x -> astchild.clear();
                        } else { 
                            nastc.push_back(x);
                            continue;
                        }
                        delete x;
                    }
                    nd -> astchild = nastc;

                    if(nd -> nonterminal_name == "constdef"
                    || nd -> nonterminal_name == "vardef") {
                        nastc.clear();
                        walka(x, nd -> astchild) {
                            if(x -> tt == TokenType :: ident || x -> tt == TokenType :: number) {
                                nastc.push_back(x);
                            } else {
                                delete x;
                            }
                        }
                        delete nastc.front();
                        nastc.erase(nastc.begin());
                        nd -> astchild = nastc;
                    }
                }
            };
            dfs(astroot);
        } while(0);

        string ASTstr_after = Node :: printAST(astroot); // after purify

        FileManager :: writeFileString("AST.out", ASTstr_before);
        FileManager :: writeFileString("AST_purify.out", ASTstr_after);
        printf("%s\n", ASTstr_after.c_str());

        // compile to asm
        auto asmblr = ([&]() {
            int labelcnt = 0;
            auto getlabel = [&] () -> string {
                return string("L") + to_string(++ labelcnt);
            };
            function<string(Node *nd)> dfs = [&] (Node *nd) {
                string res;
                if(nd -> type == NodeType :: Terminal) {
                    if(nd -> tt == TokenType :: ident) {
                        res += "push @" + nd -> val + ";\n";
                    } else {
                        res += "push " + nd -> val + ";\n";
                    }
                } else if(nd -> type == NodeType :: Nonterminal) {
                    if(nd -> nonterminal_name == "factor") {    
                        if(nd -> astchild.size() == 3) { // "(" <expression> ")"
                            res += dfs(nd -> astchild[1]);
                        } else if(nd -> astchild.size() == 1) { // <number> or <ident>
                            res += dfs(nd -> astchild[0]);
                        } else if(nd -> astchild.size() == 2) { // "+"/"-" <ident/number>
                            if(nd -> astchild[0] -> val == "+") {
                                res += dfs(nd -> astchild[1]);
                            } else {
                                res += "push 0;\n"
                                     + dfs(nd -> astchild[1])
                                     + "-;\n";
                            }
                        } else { // "+"/"-" "(" <expression> ")"
                            if(nd -> astchild[0] -> val == "+") {
                                res += dfs(nd -> astchild[2]);
                            } else {
                                res += "push 0;\n"
                                     + dfs(nd -> astchild[2]);
                                     + "-;\n";
                            }
                        }
                    } else if(nd -> nonterminal_name == "term") {
                        // a * b * c / d / e
                        for(int i = 0 ; i < (int) nd -> astchild.size() ; i += 2) {
                            res += dfs(nd -> astchild[i]);
                            if(i) {
                                res += nd -> astchild[i - 1] -> val + ";\n";
                            }
                        }
                    } else if(nd -> nonterminal_name == "expression") {
                        // a + b - c - d
                        for(int i = 0 ; i < (int) nd -> astchild.size() ; i += 2) {
                            res += dfs(nd -> astchild[i]);
                            if(i) {
                                res += nd -> astchild[i - 1] -> val + ";\n";
                            }
                        }
                    } else if(nd -> nonterminal_name == "condition") {
                        if(nd -> astchild[0] -> val == "odd") {
                            res += dfs(nd -> astchild[1]);
                            res += "odd;\n";
                        } else {
                            string exp1 = dfs(nd -> astchild[0]);
                            string op = nd -> astchild[1] -> val;
                            string exp2 = dfs(nd -> astchild[2]);
                            res += exp1 + exp2 + op + "\n";
                        }
                    } else if(nd -> nonterminal_name == "statement") {
                        if(nd -> astchild[1] -> val == ":=") {
                            res += dfs(nd -> astchild[2]);
                            res += "pop @" + nd -> astchild[0] -> val + ";\n";
                        } else if(nd -> astchild[0] -> val == "call") {
                            res += "push @@__ptr;\n"
                                   "push 4;\n"
                                   "+;\n"
                                   "push 0;\n"
                                   "jff &" + nd -> astchild[1] -> val + ";\n";
                        } else if(nd -> astchild[0] -> val == "?") {
                            res += "?;\n"
                                   "pop @" + nd -> astchild[1] -> val + ";\n";
                        } else if(nd -> astchild[0] -> val == "!") {
                            res += dfs(nd -> astchild[1])
                                 + "!;\n";
                        } else if(nd -> astchild[0] -> val == "begin") {
                            for(int i = 0 ; i < (int) nd -> astchild.size()  ; ++ i) {
                                if(nd -> astchild[i] -> nonterminal_name == "statement") {
                                    res += dfs(nd -> astchild[i]);
                                }
                            }
                        } else if(nd -> astchild[0] -> val == "if") {
                            // if <condition> do <statement>
                            auto nxt = getlabel();
                            res += dfs(nd -> astchild[1])
                                 + "jff " + nxt + ";\n"
                                 + dfs(nd -> astchild[3])
                                 + nxt + ": ";
                        } else if(nd -> astchild[0] -> val == "while") {
                            // while <condition> do <statement>
                            auto beg = getlabel(), nxt = getlabel();
                            res += beg + ": "
                                 + dfs(nd -> astchild[1])
                                 + "jff " + nxt + ";\n"
                                 + dfs(nd -> astchild[3])
                                 + "push 0;\n"
                                   "jff " + beg + ";\n"
                                 + nxt + ": ";
                        } else {
                            errorlog("unexcept `statement` choice");
                        }
                    } else if(nd -> nonterminal_name == "procdef") {
                        auto nxt = getlabel();
                        res += "push 0;\n"
                               "jff " + nxt + ";\n"
                             + "&" + nd -> astchild[1] -> val + ": "
                             + "push @@__stack_top;\n"
                             + "push @@__stack_bottom;\n"
                             + "push @@__stack_top;\n"
                             + "push 1;\n"
                             + "+;\n"
                             + "pop @@__stack_bottom;\n";
                        walka(x, nd -> astchild) {
                            if(x -> nonterminal_name == "constdef") {
                                // init const table
                            } else if(x -> nonterminal_name == "vardef") {
                                // init var table
                                res += dfs(x);
                            } else if(x -> nonterminal_name == "statement") {
                                res += dfs(x);
                            }
                        }
                        res += "push @@__stack_bottom;\n"
                               "push 1;\n"
                               "-;\n"
                               "pop @@__stack_top;\n"
                               "pop @@__stack_bottom;\n"
                               "pop @@__stack_top;\n"
                               "pop @@__ptr;\n"
                              + nxt + ": ";
                        walka(x, nd -> astchild) {
                            if(x -> nonterminal_name == "procdef") {
                                res += dfs(x);
                            }
                        }
                    } else if(nd -> nonterminal_name == "constdef") {
                        // use when compile to opcode
                    } else if(nd -> nonterminal_name == "vardef") {
                        res += "push @@__stack_top;\n"
                               "push " + to_string(nd -> astchild.size()) + ";\n"
                               "+;\n"
                               "pop @@__stack_top;\n";
                    } else if(nd -> nonterminal_name == "program") {
                        res += dfs(nd -> astchild[0]);
                    } else if(nd -> nonterminal_name == "block") {
                        // <constdef> <vardef> <procdef> <statement>
                        walka(&x, nd -> astchild) {
                            if(x -> nonterminal_name == "constdef") {
                                // init global const table
                            } else if(x -> nonterminal_name == "vardef") {
                                // init global var table
                            } else if(x -> nonterminal_name == "procdef") {
                                // init procedure
                                res += dfs(x);
                            } else if(x -> nonterminal_name == "statement") {
                                res += dfs(x);
                            }
                        }
                    } else {
                        errorlog("unexcept `nonterminal_name` : %s\n", nd -> nonterminal_name.c_str());
                    }
                } else {
                    errorlog("error during compiling...");
                }
                return res;
            };
            auto res = dfs(astroot);
            return res;
        })();

        int globalvar_cnt = 0;
        map<string, int> globalvars;
        map<string, int> globalconsts;
        map<string, int> procs;
        map<string, stack<int>> localvars;   // save offset
        map<string, stack<int>> localconsts; // save offset

        // output opcode
        auto opcode = ([&]() {
            Opcode opcode;
            globalvars["@__ptr"] = ++ globalvar_cnt;
            globalvars["@__stack_top"] = ++ globalvar_cnt;
            globalvars["@__stack_bottom"] = ++ globalvar_cnt;

            function<void(Node *nd)> dfs = [&] (Node *nd) {
                if(nd -> type == NodeType :: Terminal) {
                    if(nd -> tt == TokenType :: ident) {
                        if(!localconsts[nd -> val].empty()) {
                            opcode.add(Opcode :: bc_push_imm(localconsts[nd -> val].top()));
                        } else if(!localvars[nd -> val].empty()) {
                            opcode.add(Opcode :: bc_push_loc(localvars[nd -> val].top()));
                        } else if(globalconsts[nd -> val]) {
                            opcode.add(Opcode :: bc_push_imm(globalconsts[nd -> val]));
                        } else if(globalvars[nd -> val]) {
                            opcode.add(Opcode :: bc_push_glo(globalvars[nd -> val]));
                        } else {
                            errorlog("unknown var ident: %s", nd -> val.c_str());
                        }
                    } else {
                        opcode.add(Opcode :: bc_push_imm(stoi(nd -> val)));
                    }
                } else if(nd -> type == NodeType :: Nonterminal) {
                    if(nd -> nonterminal_name == "factor") {
                        if(nd -> astchild.size() == 3) { // "(" <expression> ")"
                            dfs(nd -> astchild[1]);
                        }  else if(nd -> astchild.size() == 1) { // <number> or <ident>
                            dfs(nd -> astchild[0]);
                        } else if(nd -> astchild.size() == 2) { // "+"/"-" <ident/number>
                            if(nd -> astchild[0] -> val == "+") {
                                dfs(nd -> astchild[1]);
                            } else {
                                opcode.add(Opcode :: bc_push_imm(0));
                                dfs(nd -> astchild[1]);
                                opcode.add(Opcode :: bc_opr("-"));
                            }
                        } else { // "+"/"-" "(" <expression> ")"
                            if(nd -> astchild[0] -> val == "+") {
                                dfs(nd -> astchild[2]);
                            } else {
                                opcode.add(Opcode :: bc_push_imm(0));
                                dfs(nd -> astchild[2]);
                                opcode.add(Opcode :: bc_opr("-"));
                            }
                        }
                    } else if(nd -> nonterminal_name == "term") {
                        // a * b * c / d / e
                        for(int i = 0 ; i < (int) nd -> astchild.size() ; i += 2) {
                            dfs(nd -> astchild[i]);
                            if(i) {
                                opcode.add(Opcode :: bc_opr(nd -> astchild[i - 1] -> val));
                            }
                        }
                    } else if(nd -> nonterminal_name == "expression") {
                        // a + b - c - d
                        for(int i = 0 ; i < (int) nd -> astchild.size() ; i += 2) {
                            dfs(nd -> astchild[i]);
                            if(i) {
                                opcode.add(Opcode :: bc_opr(nd -> astchild[i - 1] -> val));
                            }
                        }
                    } else if(nd -> nonterminal_name == "condition") {
                        if(nd -> astchild[0] -> val == "odd") {
                            dfs(nd -> astchild[1]);
                            opcode.add(Opcode :: bc_opr("odd"));
                        } else {
                            dfs(nd -> astchild[0]); // exp1
                            dfs(nd -> astchild[2]); // exp2
                            opcode.add(Opcode :: bc_opr(nd -> astchild[1] -> val)); // opr
                        }
                    } else if(nd -> nonterminal_name == "statement") {
                        if(nd -> astchild[1] -> val == ":=") {
                            dfs(nd -> astchild[2]);
                            if(!localvars[nd -> astchild[0] -> val].empty()) {
                                opcode.add(Opcode :: bc_pop_loc(localvars[nd -> astchild[0] -> val].top()));
                            } else if(globalvars[nd -> astchild[0] -> val]) {
                                opcode.add(Opcode :: bc_pop_glo(globalvars[nd -> astchild[0] -> val]));
                            } else {
                                errorlog("unknown var ident: %s", nd -> astchild[0] -> val.c_str());
                            }
                        } else if(nd -> astchild[0] -> val == "call") {
                            if(!procs[nd -> astchild[1] -> val]) {
                                errorlog("unknow proc: %s", nd -> astchild[1] -> val.c_str());
                            }
                            opcode.add(Opcode :: bc_push_glo(globalvars["@__ptr"]))
                                  .add(Opcode :: bc_push_imm(4))
                                  .add(Opcode :: bc_opr("+"))
                                  .add(Opcode :: bc_push_imm(0))
                                  .add(Opcode :: bc_jff(procs[nd -> astchild[1] -> val]));
                        } else if(nd -> astchild[0] -> val == "?") {
                            opcode.add(Opcode :: bc_opr("?"));
                            if(!localvars[nd -> astchild[1] -> val].empty()) {
                                opcode.add(Opcode :: bc_pop_loc(localvars[nd -> astchild[1] -> val].top()));
                            } else if(globalvars[nd -> astchild[1] -> val]) {
                                opcode.add(Opcode :: bc_pop_glo(globalvars[nd -> astchild[1] -> val]));
                            } else {
                                errorlog("unknown var ident: %s", nd -> astchild[1] -> val.c_str());
                            }
                        } else if(nd -> astchild[0] -> val == "!") {
                            dfs(nd -> astchild[1]);
                            opcode.add(Opcode :: bc_opr("!"));
                        } else if(nd -> astchild[0] -> val == "begin") {
                            for(int i = 0 ; i < (int) nd -> astchild.size()  ; ++ i) {
                                if(nd -> astchild[i] -> nonterminal_name == "statement") {
                                    dfs(nd -> astchild[i]);
                                }
                            }
                        } else if(nd -> astchild[0] -> val == "if") {
                            // if <condition> do <statement>
                            auto nxt = 0; // need to modify
                            dfs(nd -> astchild[1]);
                            auto idx = opcode.size();
                            opcode.add(Opcode :: bc_jff(nxt)); // need to modify
                            dfs(nd -> astchild[3]);
                            opcode.modify(idx, Opcode :: bc_jff(opcode.size())); // here, rewrite `nxt` address
                        } else if(nd -> astchild[0] -> val == "while") {
                            // while <condition> do <statement>
                            auto beg = opcode.size(), nxt = 0;
                            dfs(nd -> astchild[1]);
                            auto idx = opcode.size();
                            opcode.add(Opcode :: bc_jff(nxt)); // need to modify
                            dfs(nd -> astchild[3]);
                            opcode.add(Opcode :: bc_push_imm(0))
                                  .add(Opcode :: bc_jff(beg));
                            opcode.modify(idx, Opcode :: bc_jff(opcode.size())); // here, rewrite `nxt` address
                        } else {
                            errorlog("unexcept `statement` choice");
                        }
                    } else if(nd -> nonterminal_name == "procdef") {
                        auto nxt = 0;
                        opcode.add(Opcode :: bc_push_imm(0));
                        auto idx = opcode.size();
                        opcode.add(Opcode :: bc_jff(nxt));  // need to modify
                        procs[nd -> astchild[1] -> val] = opcode.size(); // `proc` address
                        opcode.add(Opcode :: bc_push_glo(globalvars["@__stack_top"]))
                              .add(Opcode :: bc_push_glo(globalvars["@__stack_bottom"]))
                              .add(Opcode :: bc_push_glo(globalvars["@__stack_top"]))
                              .add(Opcode :: bc_push_imm(1))
                              .add(Opcode :: bc_opr("+"))
                              .add(Opcode :: bc_pop_glo(globalvars["@__stack_bottom"]));
                        walka(x, nd -> astchild) {
                            if(x -> nonterminal_name == "constdef") {
                                // init const table
                                for(int i = 0 ; i < (int) x -> astchild.size() ; i += 2) {
                                    localconsts[x -> astchild[i] -> val].push(stoi(x -> astchild[i + 1] -> val));
                                }
                            } else if(x -> nonterminal_name == "vardef") {
                                // init var table
                                int cnt = 0;
                                walka(y, x -> astchild) {
                                    localvars[y -> val].push(cnt ++); // offset address, (absolute address: __stack_bottom + cnt)
                                }
                                opcode.add(Opcode :: bc_push_glo(globalvars["@__stack_top"]))
                                      .add(Opcode :: bc_push_imm(x -> astchild.size()))
                                      .add(Opcode :: bc_opr("+"))
                                      .add(Opcode :: bc_pop_glo(globalvars["@__stack_top"]));
                            } else if(x -> nonterminal_name == "statement") {
                                dfs(x);
                            }
                        }
                        opcode.add(Opcode :: bc_push_glo(globalvars["@__stack_bottom"]))
                              .add(Opcode :: bc_push_imm(1))
                              .add(Opcode :: bc_opr("-"))
                              .add(Opcode :: bc_pop_glo(globalvars["@__stack_top"]))
                              .add(Opcode :: bc_pop_glo(globalvars["@__stack_bottom"]))
                              .add(Opcode :: bc_pop_glo(globalvars["@__stack_top"]))
                              .add(Opcode :: bc_pop_glo(globalvars["@__ptr"]));
                        opcode.modify(idx, Opcode :: bc_jff(opcode.size())); // here, rewrite `nxt` address
                        walka(x, nd -> astchild) {  // clear var table
                            if(x -> nonterminal_name == "constdef") {
                                // clear const table
                                for(int i = 0 ; i < (int) x -> astchild.size() ; i += 2) {
                                    localconsts[x -> astchild[i] -> val].pop();
                                }
                            } else if(x -> nonterminal_name == "vardef") {
                                // clear var table
                                walka(y, x -> astchild) {
                                    localvars[y -> val].pop();
                                }
                            }
                        }
                        walka(x, nd -> astchild) {
                            if(x -> nonterminal_name == "procdef") {
                                dfs(x);
                            }
                        }
                    } else if(nd -> nonterminal_name == "constdef") {
                        // global consts
                        for(int i = 0 ; i < (int) nd -> astchild.size() ; i += 2) {
                            globalconsts[nd -> astchild[i] -> val] = stoi(nd -> astchild[i + 1] -> val);
                        }
                    } else if(nd -> nonterminal_name == "vardef") {
                        // global vars
                        walka(x, nd -> astchild) {
                            globalvars[x -> val] = ++ globalvar_cnt;
                        }
                    } else if(nd -> nonterminal_name == "program") {
                        dfs(nd -> astchild[0]);
                    } else if(nd -> nonterminal_name == "block") {
                        // <constdef> <vardef> <procdef> <statement>
                        walka(&x, nd -> astchild) {
                            if(x -> nonterminal_name == "constdef") {
                                // init const table
                                dfs(x);
                            } else if(x -> nonterminal_name == "vardef") {
                                // init var table
                                dfs(x);
                            } else if(x -> nonterminal_name == "procdef") {
                                // init procedure
                                dfs(x);
                            } else if(x -> nonterminal_name == "statement") {
                                dfs(x);
                            }
                        }
                    } else {
                        errorlog("unexcept `nonterminal_name` : %s\n", nd -> nonterminal_name.c_str());
                    }
                } else {
                    errorlog("error during compiling...");
                }
            };
            dfs(astroot);
            return opcode;
        }) ();

        do {    // build Assembly Output
            stringstream ss(asmblr);
            string title = "Assembly Output";
            vector<string> head = { "Line", "Asm", "Bin" };
            vector<vector<string>> body;
            int linenumber = 0;
            for(string tmp ; getline(ss, tmp) ; ++ linenumber) {
                string sop = "";
                walka(x, opcode.get(linenumber)) sop += to_string(x) + ",";
                sop = sop.substr(0, (int) sop.length() - 1);
                body.push_back({ string(to_string(linenumber)), tmp, sop });
            }
            Table tab(title, head, body);
            tab.show();
            FileManager :: writeFileString("Assembly.out", tab.getshowstr());
        } while(0);

        do {
            puts("=========================== Program ============================");
            int* gvars = new int[globalvar_cnt + 1];
            Stack stack(&gvars[globalvars["@__stack_top"]], &gvars[globalvars["@__stack_bottom"]]);
            gvars[globalvars["@__ptr"]] = 0;
            gvars[globalvars["@__stack_top"]] = 0;
            gvars[globalvars["@__stack_bottom"]] = 1;
            for(int &__ptr = gvars[globalvars["@__ptr"]] ; __ptr < (int) opcode.size() ; ) {
                vector<int> ops = opcode.get(__ptr ++);
                if(ops[0] == (int) Opcode :: BasicCommand :: push) {
                    if(ops[1] == 0) { // local
                        stack.push(stack.get(ops[2]));
                    } else if(ops[1] == 1) { // global
                        stack.push(gvars[ops[2]]);
                    } else if(ops[1] == 2) { // immediate number
                        stack.push(ops[2]);
                    } else {
                        errorlog("unknown vartype %d", ops[1]);
                    }
                } else if(ops[0] == (int) Opcode :: BasicCommand :: pop) {
                    if(ops[1] == 0) { // local
                        int val = stack.pop();
                        stack.get(ops[2]) = val;
                    } else if(ops[1] == 1) { // global
                        int val = stack.pop();
                        gvars[ops[2]] = val;
                    } else {
                        errorlog("unknown vartype %d", ops[1]);
                    }
                } else if(ops[0] == (int) Opcode :: BasicCommand :: jff) {
                    int flag = stack.pop();
                    if(flag == false) __ptr = ops[1];
                } else if(ops[0] == (int) Opcode :: BasicCommand :: plus) {
                    int y = stack.pop(); int x = stack.pop();
                    stack.push(x + y);
                } else if(ops[0] == (int) Opcode :: BasicCommand :: minus) {
                    int y = stack.pop(); int x = stack.pop();
                    stack.push(x - y);
                } else if(ops[0] == (int) Opcode :: BasicCommand :: multi) {
                    int y = stack.pop(); int x = stack.pop();
                    stack.push(x * y);
                } else if(ops[0] == (int) Opcode :: BasicCommand :: div) {
                    int y = stack.pop(); int x = stack.pop();
                    stack.push(x / y);
                } else if(ops[0] == (int) Opcode :: BasicCommand :: odd) {
                    int x = stack.pop();
                    stack.push(abs(x) & 1);
                } else if(ops[0] == (int) Opcode :: BasicCommand :: eq) {
                    int y = stack.pop(); int x = stack.pop();
                    stack.push(x == y);
                } else if(ops[0] == (int) Opcode :: BasicCommand :: neq) {
                    int y = stack.pop(); int x = stack.pop();
                    stack.push(x != y);
                } else if(ops[0] == (int) Opcode :: BasicCommand :: lt) {
                    int y = stack.pop(); int x = stack.pop();
                    stack.push(x < y);
                } else if(ops[0] == (int) Opcode :: BasicCommand :: le) {
                    int y = stack.pop(); int x = stack.pop();
                    stack.push(x <= y);
                } else if(ops[0] == (int) Opcode :: BasicCommand :: gt) {
                    int y = stack.pop(); int x = stack.pop();
                    stack.push(x > y);
                } else if(ops[0] == (int) Opcode :: BasicCommand :: ge) {
                    int y = stack.pop(); int x = stack.pop();
                    stack.push(x >= y);
                } else if(ops[0] == (int) Opcode :: BasicCommand :: inp) {
                    printf(">> ");
                    int x; scanf("%d", &x); stack.push(x);
                } else if(ops[0] == (int) Opcode :: BasicCommand :: out) {
                    printf("<< ");
                    printf("%d\n", stack.pop());                
                } else {
                    errorlog("unknow op: %d", ops[0]);
                }
            }
            delete [] gvars;
            puts("=========================== Finished ===========================");
        } while(0);

    }
}

int main(int argc, char **argv) {
    if(argc != 2) {
        errorlog("usage: PL-0 [src].pas");
    }
    PL_0 :: initial(argv[1]);
}
