#ifndef LEXER_HPP

class Lexer {
    private:
        static void expect(bool flag) { if(!flag) { throw 1; } }
        class Regex;
        class NFA;
        class DFA;
        class Binder;

        #define eps 0
        typedef tuple<int, int, int> Edge; // (u, v, w)  w -> char, or 0(alias epsilon)

        class Regex {
            friend class NFA;
            private:
                typedef pair<int, int> headtail;
                int REstartnd, cnt;
                set<int> REendnd;
                vector<Edge> edg;
                void addedg(int u, int v, int w) {
                    edg.push_back(make_tuple(u, v, w));
                }
                int newnode() { return ++ cnt; }
                bool chk(char c) { return isdigit(c) || isalpha(c) || c == '\\'; }
            public:
                Regex() {
                    REstartnd = cnt = 0;
                    REendnd.clear();
                    edg.clear();
                }
                void linearScanner(string str) {
                    int len = str.length(), ptr = 0;
                    function<void(headtail)> dfs = [&] (headtail ht) -> void {
                        if(ptr >= len) return ;
                        int S = ht.first, T = ht.second;
                        if(str[ptr] == '(') {
                            // (w) (w)*
                            ++ ptr;
                            int ns = newnode(), nt = ns, lst = nt;
                            int starts = ns, endt = newnode();
                            addedg(S, starts, eps);
                            addedg(endt, T, eps);
                            while(str[ptr] != ')') {
                                if(str[ptr] == '+') {
                                    addedg(lst, endt, eps);
                                    ns = starts, nt = ns, lst = nt;
                                    ++ ptr;
                                    expect(str[ptr] != ')');
                                    continue;
                                }
                                
                                nt = newnode();
                                dfs({ns, nt});
                                ns = nt;
                                lst = nt;
                            }
                            addedg(lst, endt, eps);
                            expect(str[ptr ++] == ')');
                            if(str[ptr] == '*') {
                                ++ ptr;
                                // (w)*
                                addedg(endt, starts, eps);
                                addedg(S, T, eps);
                            } else {
                                // w^n? unacceptable, so do nothing
                            }
                        } else if(chk(str[ptr])) {
                            int lst = S;
                            while(ptr < len && chk(str[ptr])) {
                                char c = str[ptr];
                                if(str[ptr] == '\\') c = str[++ ptr];
                                int nd = T;
                                addedg(lst, nd, c);
                                lst = nd;
                                ++ ptr;
                            }
                        } else if(str[ptr] == ')' || str[ptr] == '*') {
                           // `++ ptr;` ? actually, do nothing
                        } else {
                            expect(0);
                        }
                    };

                    try {
                        int S = newnode(), T = S, lst = S;
                        while(ptr < len) {
                            int ns = lst, nt = newnode();
                            dfs({ns, nt});
                            lst = nt;
                        }
                        T = lst;
                        REstartnd = S;
                        REendnd.emplace(T);
                    } catch (...) {
                        errorlog("something error in `Lexer/Regex`");
                    }
                }
        };

        template<typename T_Delta, typename T_F, typename T_isend>
        class Automachine {
            public:
                int Q; // node count (index from 1)
                int Sigma; // alphabet size
                T_Delta Delta; // δ(q,w)
                int q0; // start state(node)
                T_F F; // finished state
                T_isend isend; // marked if is finished
                Automachine() {}
                Automachine(int Q, int Sigma, T_Delta Delta, int q0, T_F F, T_isend isend) : Q(Q), Sigma(Sigma), Delta(Delta), q0(q0), F(F), isend(isend) {};
        };
        
        typedef vector<vector<set<int>>> NFA_Delta;  // NFA_Delta: [node count] x [alphabet size]
        typedef set<int> NFA_F;
        typedef vector<bool> NFA_isend;
        class NFA : private Automachine<NFA_Delta, NFA_F, NFA_isend> { // NFA = (Q, Σ, δ, q0, F)
            friend class DFA;
            private:
                    void cleanEpsilon() {
                    // remove `ε-edges`
                    walk(rt, Q) {
                        set<int> eclo;
                        function<void(int)> dfs = [&] (int u) {
                            eclo.emplace(u);
                            walkac(v, Delta[u][eps], v && eclo.find(v) == eclo.end()) {
                                dfs(v);
                            }
                        };
                        dfs(rt);
                        walka(x, eclo) {
                            if(isend[x] && !isend[rt]) {
                                isend[rt] = 1;
                                F.emplace(rt);
                            }
                            walk(c, Sigma) {
                                Delta[rt][c].insert(vecrange(Delta[x][c]));
                            }
                        }
                    }
                    walk(rt, Q) {
                        Delta[rt][eps].clear();
                    }
                }
            public:
                NFA(Regex &reg) {
                    try {
                        Q = reg.cnt;
                        Sigma = 127; // default
                        Delta = vector<vector<set<int> > > (Q + 1, vector<set<int> > (Sigma + 1));
                        q0 = reg.REstartnd;
                        F = reg.REendnd;
                        isend.resize(Q + 1); walka(x, F) isend[x] = true;
                        walka(e, reg.edg) {
                            int u, v, w; tie(u, v, w) = e;
                            Delta[u][w].insert(v);
                        }
                        cleanEpsilon();
                    } catch(...) {
                        errorlog("something error in `Lexer/NFA`");
                    }
                }
        };

        typedef vector<vector<int>> DFA_Delta;  // DFA_Delta: δ(q,w)
        typedef set<int> DFA_F;
        typedef vector<bool> DFA_isend;    
        class DFA : private Automachine<DFA_Delta, DFA_F, DFA_isend> { // DFA = (Q, Σ, δ, q0, F)
            friend class Binder;
            private:
                int curstate; // current state(used in `feed`)
                bool feed(char c) {
                    curstate = Delta[curstate][c];
                    return isend[curstate];
                }
                bool test(char c) {
                    return Delta[curstate][c] != 0;
                }

                void minimize() { // minimize DFA to MFA
                    // though, maybe `0` is the `emptyset` before
                    // firstly, insert new `emptyset` into DFA
                    int es = ++ Q;
                    Delta.emplace_back(vector<int> (Sigma + 1));
                    isend.emplace_back(0);
                    set<int> sig;
                    walk(u, Q) {
                        walkc(c, Sigma, Delta[u][c]) {
                            sig.emplace(c);
                        }
                    }
                    walka(c, sig) {
                        Delta[es][c] = es;
                    }
                    walk(u, Q) {
                        walkac(c, sig, !Delta[u][c]) {
                            Delta[u][c] = es;
                        }
                    }

                    const int MAXN = 5000; // max nodes number of DFA
                    expect(Q < MAXN);

                    // merge similar states
                    do {
                        bitset<MAXN> R[Q + 1]; // `1` if can be merged
                        decl(fa, vector<int>, Q); // dsu
                        walk(i, Q) {
                            fa[i] = i;
                        }
                        function<int(int)> getfa = [&] (int x) {
                            return x == fa[x] ? x : fa[x] = getfa(fa[x]);
                        };
                        queue<tuple<int, int> > que;
                        walk(u, Q) {
                            walkc(v, Q, isend[u] ^ isend[v]) {
                                R[u][v] = 1;
                                que.emplace(make_tuple(u, v));
                            }
                        }
                        while(que.size()) {
                            int u, v; tie(u, v) = que.front(); que.pop();
                            walka(c, sig) {
                                walkc(x, Q, Delta[x][c] == u) {
                                    walkc(y, Q, Delta[y][c] == v) {
                                        if(R[x][y] == 0) {
                                            R[x][y] = R[y][x] = 1;
                                            que.emplace(make_tuple(x, y));
                                        }
                                    }
                                }
                            }
                        }
                        walk(u, Q) {
                            walkc(v, Q, R[u][v] == 0) {
                                fa[getfa(u)] = getfa(v);
                            }
                        }

                        decl(clo, vector<vector<int> >, Q);
                        walk(u, Q) {
                            int accu = getfa(u);
                            clo[accu].emplace_back(u);
                        }
                        set<tuple<int, int, int> > edgtmp; // (u, v, w)
                        set<int> nF;
                        walkc(x, Q, x == getfa(x)) {
                            walka(u, clo[x]) {
                                if(isend[u]) {
                                    nF.emplace(x);
                                }
                                walkac(c, sig, Delta[u][c]) {
                                    int v = Delta[u][c];
                                    edgtmp.emplace(make_tuple(getfa(u), getfa(v), c));
                                }
                            }
                        }
                        map<int, int> reidx; int reidx_cnt = 0;
                        walkc(i, Q, i == getfa(i)) reidx[i] = ++ reidx_cnt;
                        Q = reidx_cnt;
                        q0 = reidx[getfa(q0)];
                        Delta.clear(), Delta = vector<vector<int> > (Q + 1, vector<int> (Sigma + 1));
                        walka(e, edgtmp) {
                            int u, v, c; tie(u, v, c) = e;
                            u = reidx[u], v = reidx[v];
                            Delta[u][c] = v;
                        }
                        F.clear(); walka(x, nF) {
                            F.emplace(reidx[x]);
                        }
                        isend.clear(), isend.resize(Q + 1); walka(x, F) isend[x] = 1;
                    } while(0);

                    do {
                        bitset<MAXN> pre[Q + 1]; // `pre`: from q0
                        walk(u, Q) {
                            pre[u][u] = 1;
                            walkac(c, sig, Delta[u][c]) {
                                int v = Delta[u][c];
                                pre[u][v] = 1;
                            }
                        }
                        walk(k, Q) {
                            walkc(i, Q, pre[i][k]) {
                                pre[i] |= pre[k];
                            }
                        }
                        bitset<MAXN> rem;
                        walk(u, Q) {
                            if(!pre[q0][u]) {
                                rem[u] = 1;
                            } else {
                                int flag = 0;
                                walkc(v, Q, pre[u][v] && isend[v]) {
                                    flag = 1;
                                }
                                if(!flag) {
                                    rem[u] = 1;
                                }
                            }
                        }
                        map<int, int> reidx; int reidx_cnt = 0, Qmem = Q;
                        walkc(i, Q, !rem[i]) reidx[i] = ++ reidx_cnt;
                        Q = reidx_cnt;
                        q0 = reidx[q0];
                        vector<tuple<int, int, int> > edgtmp;
                        auto nDelta = vector<vector<int> > (Qmem + 1, vector<int> (Sigma + 1));
                        F.clear();
                        walkc(u, Qmem, !rem[u]) {
                            walkac(c, sig, Delta[u][c] && !rem[Delta[u][c]]) {
                                int v = Delta[u][c];
                                nDelta[reidx[u]][c] = reidx[v];
                            }
                            if(isend[u]) {
                                F.emplace(reidx[u]);
                            }
                        }
                        Delta = nDelta;
                        isend.clear(), isend.resize(Q + 1); walka(x, F) isend[x] = 1;
                    } while(0);
                }
            public:
                DFA(int Q, int Sigma, DFA_Delta Delta, int q0, DFA_F F, DFA_isend isend) : Automachine(Q, Sigma, Delta, q0, F, isend) {};
                DFA(NFA &nfa) { // trans NFA to DFA
                    try {
                        Sigma = nfa.Sigma;
                        map<set<int>, int> idx;

                        Q = 0;
                        auto getidx = [&] (set<int> &state) {
                            if(idx.find(state) != idx.end()) {
                                return idx[state];
                            }
                            return idx[state] = ++ Q;
                        };
                        queue<set<int> > que;
                        map<int, bool> vis;

                        do {
                            set<int> rt;
                            rt.insert(nfa.q0);
                            que.emplace(rt);
                            vis[q0 = getidx(rt)] = 1;
                        } while(0);

                        while(que.size()) {
                            set<int> rt = que.front(); que.pop();
                            int rtidx = getidx(rt);
                            walkac(x, rt, nfa.isend[x]) {
                                F.emplace(rtidx);
                                break;
                            }
                            walk(c, Sigma) {
                                set<int> nxt;
                                walka(x, rt) {
                                    nxt.insert(vecrange(nfa.Delta[x][c]));
                                }
                                if(!nxt.empty()) {
                                    int nxtidx = getidx(nxt);
                                    while((int) Delta.size() <= Q * 2 + 1) {
                                        Delta.emplace_back(vector<int> (Sigma + 1));
                                    }
                                    Delta[rtidx][c] = nxtidx;
                                    if(!vis[nxtidx]) {
                                        vis[nxtidx] = 1;
                                        que.emplace(nxt);
                                    }
                                }
                            }
                        }
                        isend.clear(), isend.resize(Q + 1);
                        walka(x, F) {
                            isend[x] = true;
                        }
                        minimize();
                    } catch(...) {
                        errorlog("something error in `Lexer/DFA`");
                    }
                }
                void resetState() {
                    curstate = q0; // set initial value of `curstate`
                }
        };

        class Binder {
            friend class Lexer;
            private:
                typedef tuple<string, function<void(string)>, DFA> regexHook;
                vector<regexHook> hooks;
            public:
                Binder() { hooks.clear(); }
                void addHook(string reg, function<void(string)> act) {
                    auto regexPlus = [&] (string reg) {
                        string res; int len = reg.length();
                        for(int i = 0 ; i < len ; ++ i) {
                            if(reg[i] == '\\') {
                                res += reg[i];
                                ++ i;
                                res += reg[i];
                            } else if(reg[i] == '[') {
                                // [a-z] or [a-zA-Z] or [a-zA-Z_0-9] or [\[]
                                int L = i + 1, R = L;
                                while(R < len && reg[R] != ']') {
                                    if(reg[R] == '\\') ++ R;
                                    ++ R;
                                }
                                -- R;
                                vector<string> pak;
                                while(L <= R) {
                                    // printf("L = %d, R = %d\n", L, R);
                                    if(L + 1 <= R && reg[L + 1] == '-') {
                                        // a-z
                                        for(char c = reg[L] ; c <= reg[L + 2] ; ++ c) {
                                            if(isalpha(c) || isdigit(c)) pak.emplace_back(string("") + c);
                                            else pak.emplace_back(string("\\") + c);
                                        }
                                        L += 3;
                                    } else if(reg[L] == '\\') {
                                        pak.emplace_back(string("\\") + reg[L + 1]);
                                        L += 2;
                                    } else {
                                        char c = reg[L];
                                        if(isalpha(c) || isdigit(c)) pak.emplace_back(string("") + c);
                                        else pak.emplace_back(string("\\") + c);
                                        ++ L;
                                    }
                                }
                                // puts("loop end");
                                string tmp;
                                for(int j = 0 ; j < (int) pak.size() ; ++ j) {
                                    if(j != 0) tmp += "+";
                                    tmp += pak[j];
                                }
                                res += "(" + tmp + ")";
                                i = R + 1;
                            } else {
                                res += reg[i];
                            }
                        }
                        return res;
                    };
                    reg = regexPlus(reg);

                    Regex regScanner;
                    regScanner.linearScanner(reg);
                    NFA nfa(regScanner);
                    DFA dfa(nfa);
                    hooks.emplace_back(make_tuple(reg, act, dfa));
                }
                void match(string str) {
                    int len = str.length(), ptr = 0;
                    while(ptr < len) { // analyse raw code
                        walka(&hk, hooks) {
                            auto &raw = get<0>(hk);
                            auto &act = get<1>(hk);
                            auto &dfa = get<2>(hk);
                            raw = "", dfa.resetState();
                            int ptrmem = ptr, isendstate = 0;
                            while(ptr < len && dfa.test(str[ptr])) {
                                isendstate = dfa.feed(str[ptr]);
                                raw += str[ptr ++];
                            }
                            if(isendstate) { // state in `endF`, and we catched a string!
                                act(raw);
                                goto nxt;
                            } else {
                                ptr = ptrmem;
                            }
                        }
                        errorlog("error in `match`: can't match any regex, char: %c (ascii: %d)", str[ptr], (int) str[ptr]);
                        nxt: ;
                    }
                }
        };

    private:
        Binder binder;
    public:
        Lexer() { binder = Binder(); }
        Lexer& feed(string reg, function<void(string)> act) {
            reg = "(" + reg + ")";
            binder.addHook(reg, act);
            return *this;
        }
        Lexer& match(string str) {
            binder.match(str);
            return *this;
        }
};

#endif
#define LEXER_HPP
