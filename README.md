# YYACC

Yet Yet Another Compiler Compiler

---

# 小型编译器框架构建与PL/0语言解释器实例化

## 设计Token的正则表达式

- 数字：`[0-9][0-9]*`
- 标识符：`[_a-zA-Z][_a-zA-Z0-9]*`
- 赋值符号：`\:\=`
- 小于等于符号：`\<\=`
- 大于等于符号：`\>\=`
- 其它符号：`[!?:;.,#<=>+-\*/\(\)]`
- 注释：`\{[\f\n\r\t\v -z|~]*\}`
- 空白字符：`[ \f\n\r\t\v][ \f\n\r\t\v]*`

## 设计自动机

一个自动机通过五元组 $(Q,\Sigma,\delta,q _ 0,F)$ 表示，其中：

- $Q$ 表示状态集
- $\Sigma$ 表示字符集
- $\delta(q,c)$ 表示二元转移函数
  - 从状态 $q$ 通过转移字符 $c$ 所到达的状态（集）
- $q _ 0$ 表示起始状态，$F$ 表示终止状态集。

为了简便起见，记 $\hat\delta(q,cw)=\hat\delta(\delta(q,c),w),(w \in \Sigma^*)$。

容易观察到，DFA和NFA都属于自动机，但它们的转移函数的定义不同（NFA为 `vector<vector<set<int>>>`，DFA为 `vector<vector<int>>`）。

可以考虑使用模板类进行不同类型继承。

``` cpp
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
class NFA : private Automachine<NFA_Delta, NFA_F, NFA_isend>; // NFA = (Q, Σ, δ, q0, F)
class DFA : private Automachine<DFA_Delta, DFA_F, DFA_isend>; // DFA = (Q, Σ, δ, q0, F)
```

### 从正则表达式到ε-NFA（Thompson构造法）

通过下列转化，可以将正则表达式构造成等价的 $\varepsilon-NFA$。

- 空表达式 $\varepsilon$ 转化为：
  ![inline](https://upload.wikimedia.org/wikipedia/commons/thumb/7/7e/Thompson-epsilon.svg/278px-Thompson-epsilon.svg.png)
- 单个符号 $a$ 转化为：
  ![inline](https://upload.wikimedia.org/wikipedia/commons/thumb/9/93/Thompson-a-symbol.svg/278px-Thompson-a-symbol.svg.png)
- 表达式的并($s|t$)转化为：
  ![inline](https://upload.wikimedia.org/wikipedia/commons/thumb/2/25/Thompson-or.svg/453px-Thompson-or.svg.png)
- 表达式的连接($st$)转化为：
  ![inline](https://upload.wikimedia.org/wikipedia/commons/thumb/5/55/Thompson-concat.svg/398px-Thompson-concat.svg.png)
- Kleen闭包($s^ * $)转化为：
  ![inline](https://upload.wikimedia.org/wikipedia/commons/thumb/8/8e/Thompson-kleene-star.svg/503px-Thompson-kleene-star.svg.png)

### 从ε-NFA到NFA（化简ε-NFA）

记 $\epsilon(q)=\{p|\hat \delta(q,\varepsilon)=p\}$，表示状态 $q$ 只沿 $\varepsilon$ 边所能到达的状态集合。

则 $\forall c \in \Sigma,\delta_{NFA}(E(q),c)=\{\cup_{p \in S} E(p)|S=\{r|\delta(q,c)=r\} \}$。

实际上，就是将模拟 $\varepsilon-NFA$ 的过程用简化状态集表示。

### 从NFA到DFA（确定化NFA）

这里采用**子集构造**的方法。

与化简 $\varepsilon-NFA$ 的步骤类似，记 $\Gamma_c(q)=\{p|\delta(q,c)=p\}$，设计如下 DFA：起始状态为 $\{q_0\}$，状态转移函数 $\delta_{DFA}(S,c)=\cup_{p \in S} \Gamma_c(p)$。

需要注意的是：

1. 对状态集合的映射，可以采用哈希法，也可以采用 `map<set<int>, int>` 存储（`set<int>` 可以作为 `map` 的键值）。
2. 通过子集构造方法确定化NFA，这里使用的是**广度优先搜索**(bfs)算法，即通过 `queue<set<int>> que;` 来存储状态集合 $S$，每次通过取队列首端元素进行更新。

### 从DFA到MFA（最小化DFA）

最小化DFA有许多算法，这里选用比较直接的**填表算法**。

定义 $p,q$ 状态相同，当且仅当 $\forall w\in\Sigma^+,s.t.\hat \delta(p,w)=\hat \delta(q,w)$，换而言之：若 $\exists w \in \Sigma^+,s.t.\hat\delta(p,w) \ne \hat \delta(q,w)$，则 $p,q$ 状态不相同。

首先，若 $p \in F,q \not \in F$，则 $p,q$ 不同；其次，若 $\delta(p,c)=p',\delta(q,c)=q'$ 且 $p',q'$ 不同，则 $p,q$ 不同；同时，若 $p,q$ 不同，则存在串 $w$，使得 $\hat\delta(p,w) \in F,\hat\delta(q,w)\not\in F$ 或相反。

所以可以再次使用广度优先搜索算法，初始时将所有满足 $p \in F,q \ne F$ 的 $(p,q)$ 点对加入队列，然后依次消除所有的不等价点对，最后剩下的点对就是所有的等价类。

之后只需要删除所有的不可从初始节点到达的节点就可以转化为MFA了；为了更容易实现字符串匹配，这里可以稍微破坏DFA的结构，即将所有不能到达终止节点的节点删除。

### MFA匹配字符串

这里的技术难点主要分为如何利用预编译好的**DFA进行字符串匹配**，以及如何优美的**读入绑定结构**。

线性扫描字符串的时候，依次使用每一个DFA进行贪心匹配。

``` cpp
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
```

对于绑定匹配模式和匹配回调函数，这里通过 `Lexer& f() { return *this; }` 的方法，实现连续调用。

``` cpp
lexer.feed("[0-9][0-9]*",
           [&](string raw) {
               bpb("[number]", raw);
               tpb(TokenType :: number, raw);
           })
    .feed("[_a-zA-Z][_a-zA-Z0-9]*", // identifiers 
          [&](string raw) {
              bpb("[ident]", raw);
              tpb(TokenType :: ident, raw);
          });
```

## 设计LL(1)文法

这里先给出EBNF范式的PL/0文法：

``` pascal
program = block "." ;
block = [ "const" ident "=" number {"," ident "=" number} ";"]
        [ "var" ident {"," ident} ";"]
        { "procedure" ident ";" block ";" } statement;
statement = [ ident ":=" expression | "call" ident 
              | "?" ident | "!" expression 
              | "begin" statement {";" statement } "end" 
              | "if" condition "then" statement 
              | "while" condition "do" statement ];
condition = "odd" expression |
            expression ("="|"#"|"<"|"<="|">"|">=") expression;
expression = [ "+"|"-"] term { ("+"|"-") term};
term = factor {("*"|"/") factor};
factor = ident | number | "(" expression ")";
```

由于Parser部分使用RDP实现，所以只需要去除**左递归**即可（不一定需要完全达到LL(1)），只需要增加后缀部分拆分左递归式。

原始版本的PL/0对负数支持不太好，这里重写了部分定义。

``` pascal
<program> = <block> "."
<block> = <constdef> <vardef> <procdef> <statement>
<constdef>    = "const" <ident> "=" <number> <constdefpri> ";"
              | ""
<constdefpri> = "," <ident> "=" <number> <constdefpri>
              | ""
<vardef>    = "var" <ident> <vardefpri> ";"
            | ""
<vardefpri> = "," <ident> <vardefpri>
            | ""
<procdef> = "procedure" <ident> ";" <constdef> <vardef> <statement> ";" <procdef>
          | ""
<statement>    = <ident> ":=" <expression>
               | "call" <ident> 
               | "?" <ident>
               | "!" <expression> 
               | "begin" <statement> <statementpri> "end" 
               | "if" <condition> "then" <statement>
               | "while" <condition> "do" <statement>
<statementpri> = ";" <statement> <statementpri>
               | ""
<condition> = "odd" <expression>
            | <expression> "="  <expression>
            | <expression> "#"  <expression>
            | <expression> "<"  <expression>
            | <expression> "<=" <expression>
            | <expression> ">"  <expression>
            | <expression> ">=" <expression>
<expression>    = <term> <expressionpri>
                | "+" <term> <expressionpri>
                | "-" <term> <expressionpri>
<expressionpri> = "+" <term> <expressionpri>
                | "-" <term> <expressionpri>
                | ""
<term>    = <factor> <termpri>
<termpri> = "*" <factor> <termpri>
          | "/" <factor> <termpri>
          | ""
<factor> = <ident>
         | <number>
         | "(" <expression> ")"
         | "+" "(" <expression> ")" // extra
         | "-" "(" <expression> ")" // extra
         | "+" <number> // extra
         | "-" <number> // extra
         | "+" <ident>  // extra
         | "-" <ident>  // extra
<ident>  = `IDENT`
<number> = `NUMBER`
```

如何处理带符号表达式是这部分的主要难点，诸如 `-114` 或者 `+514` 之类的数字，又或者说是 `-homo` 这类的变量，再或者说是 `+(1919/810)` 这类的表达式前的符号。可以考虑将它们都归结到 `<factor>` 中实现，即增加前缀符号识别。

## 解析抽象语法树

这里使用的是**递归下降子程序法**解析抽象语法树，大体思路就是，通过递归函数栈来模拟匹配文法。

通过构造抽象转移图的方式存储CFG格式，进而模拟进行递归下降子程序法进行解析。

通过宏定义，将中间节点和终止节点的使用变得容易；Node通过返回自身地址，实现连续操作。

``` cpp
#define T(val) (parser.getTerminalNode(val))      // Terminal
#define N(name) (parser.getNonterminalNode(name)) // Nonterminal
N(statement) -> add({ N(ident), T(":="), N(expression) })
    -> add({ T("call"), N(ident) })
    -> add({ T("?"), N(ident) })
    -> add({ T("!"), N(expression) })
    -> add({ T("begin"), N(statement), N(statementpri), T("end") })
    -> add({ T("if"), N(condition), T("then"), N(statement) })
    -> add({ T("while"), N(condition), T("do"), N(statement) });
```

## 设计汇编原型与编译模式

首先设计基础汇编指令，一共分为 $16$ 个基础指令，依次表示数据入栈、将栈顶数据弹出到变量、条件跳转、四则运算、条件判断（`odd` 表示判断是否是奇数，`#` 表示不等于）、控制台输入（`?`）和控制台输出（`!`）。

``` text
push, pop, jff, +, -, *, /, odd, =, #, <, <=, >, >=, ?, !
```

- `push x` 会将立即数 $x$ 压入栈顶，`push @x` 会将变量 `@x` 的值压入栈顶。
- `pop @x` 会将栈顶元素弹出，并赋值给变量 `@x`。
- `jff label` 会先弹出栈顶元素，如果栈顶元素为 $0$，则跳转到位置 `label`。 
- `+, -, *, /, =, #, <, <=, >, >=` 会弹出栈顶两个元素 $x_1,x_2$，然后将 $x_1 \oplus x_2$ 压入栈顶。
- `odd` 会弹出栈顶元素 $x$，如果 $x$ 是奇数，则压入 $1$，否则压入 $0$。
- `?` 会从控制台读入一个整数（int类型），并压入栈顶。
- `!` 会弹出栈顶元素并将其输出。

以下是详细的编译模式。

``` pascal
"const" <ident> "=" <number> <constdefpri> ";"  {  }  // 这部分在编译部分直接进行变量替换
"var" <ident> <vardefpri> ";" { push @@__stack_top; push <vert_ident>; +; pop @@__stack_top; } // 间接填充
"procedure" <ident> ";" <constdef> <vardef> <statement> ";" <procdef>
{
    push 0;
    jff nxt;
    &<ident>:;
    push @@__stack_top;
    push @@__stack_bottom;
    push @@__stack_top;
    push 1;
    +;
    pop @@__stack_bottom;
    [<constdef>]
    <vardef>
    <statement>
    push @@__stack_bottom;
    push 1;
    -;
    pop @@__stack_top;
    pop @@__stack_bottom;
    pop @@__stack_top;
    pop @@__ptr;
    nxt:;
}
<ident> ":=" <expression>  { <expression>; pop @<ident>; }
"call" <ident>    { push @@__ptr; push 4; +; push 0; jff &<ident>; }
"?" <ident>       { ?; pop @<ident>; }  // ?读入到栈顶,复用pop
"!" <expression>  { <expression>; !; }  // !输出栈顶并弹出,复用<expression>
"begin" <statement> <statementpri> "end"  { <statement> }
"if" <condition> "then" <statement>       { <condition>; jff nxt; <statement>; nxt:; } // 如果<condition>为false则跳转(jump if false)
"while" <condition> "do" <statement>      { beg:; <condition>; jff nxt; <statement>; push 0; jff beg; nxt:; }
"odd" <expression>                { <expression>; odd; }
<expression> "="  <expression>    { <expression1>; <expression2>; =; }
<expression> "#"  <expression>    { <expression1>; <expression2>; #; }
<expression> "<"  <expression>    { <expression1>; <expression2>; <; }
<expression> "<=" <expression>    { <expression1>; <expression2>; <=; }
<expression> ">"  <expression>    { <expression1>; <expression2>; >; }
<expression> ">=" <expression>    { <expression1>; <expression2>; >=; }
"+" <term> <expressionpri>    { <factor1>; <factor2>; +; }  // 弹出栈顶f1,f2,之后计算f1+f2并放入栈顶
"-" <term> <expressionpri>    { <factor1>; <factor2>; -; }  // 弹出栈顶f1,f2,之后计算f1-f2并放入栈顶
"*" <factor> <termpri>  { <factor1>; <factor2>; *; }  // 弹出栈顶f1,f2,之后计算f1*f2并放入栈顶
"/" <factor> <termpri>  { <factor1>; <factor2>; /; }  // 弹出栈顶f1,f2,之后计算f1/f2并放入栈顶
<ident>   { push @<ident> }
<number>  { push <number> }
"(" <expression> ")"  { <expression> } // <expression>在计算结束后会把值放入栈顶
"+" "(" <expression> ")" { <expression> }
"-" "(" <expression> ")" { push 0; <expression>; -; }
"+" <number> { push <number>; }
"-" <number> { push 0; push <number>; -; }
"+" <ident>	 { push @<ident>; }
"-" <ident>  { push 0; push @<ident>; -; }
```

## 字节码的处理与执行

首先通过上述编译模式进行逐行翻译，获得汇编码；同样方式可以获得字节码。在编译完成后，通过虚拟机进行执行（这里用单栈机实现）。

``` cpp
if(nd -> astchild[0] -> val == "if") {
    // if <condition> do <statement>
    auto nxt = 0; // need to modify
    dfs(nd -> astchild[1]);
    auto idx = opcode.size();
    opcode.add(Opcode :: bc_jff(nxt)); // need to modify
    dfs(nd -> astchild[3]);
    opcode.modify(idx, Opcode :: bc_jff(opcode.size())); // here, rewrite `nxt` address
}
```

## 结果展示

<center>
    <img style="border-radius: 0.3125em;
    box-shadow: 0 2px 4px 0 rgba(34,36,38,.12),0 2px 10px 0 rgba(34,36,38,.08);" 
    src="https://user-images.githubusercontent.com/20121501/187109634-77818110-a7b1-4275-aaaa-cb96c8748f67.png">
    <br>
    <div style="color:orange; border-bottom: 1px solid #d9d9d9;
    display: inline-block;
    color: #999;
    padding: 2px;">Lexer Output</div>
</center>

<center>
    <img style="border-radius: 0.3125em;
    box-shadow: 0 2px 4px 0 rgba(34,36,38,.12),0 2px 10px 0 rgba(34,36,38,.08);" 
    src="https://user-images.githubusercontent.com/20121501/187109794-ca2ad1d8-dd0e-4fc6-8840-16a6c520b802.jpg">
    <br>
    <div style="color:orange; border-bottom: 1px solid #d9d9d9;
    display: inline-block;
    color: #999;
    padding: 2px;">Assembly Output</div>
</center>

## 参考资料

- [1] [wikipedia-Regular language](https://en.wikipedia.org/wiki/Regular_language)
- [2] [wikipedia-Nondeterministic finite automaton](https://en.wikipedia.org/wiki/Nondeterministic_finite_automaton)
- [3] [wikipedia-Deterministic finite automaton](https://en.wikipedia.org/wiki/Deterministic_finite_automaton)
- [4] [wikipedia-Context-free grammar](https://en.wikipedia.org/wiki/Context-free_grammar)
- [5] [wikipedia-Abstract syntax tree](https://en.wikipedia.org/wiki/Abstract_syntax_tree)
- [6] [wikipedia-LL parser](https://en.wikipedia.org/wiki/LL_parser)
- [7] [wikipedia-Recursive descent parser](https://en.wikipedia.org/wiki/Recursive_descent_parser)
- [8] [wikipedia-Powerset construction](https://en.wikipedia.org/wiki/Powerset_construction)
- [9] [wikipedia-Breadth-first search](https://en.wikipedia.org/wiki/Breadth-first_search)
- [10] [wikipedia-DFA minimization](https://en.wikipedia.org/wiki/DFA_minimization)
- [11] [wikipedia-Left recursion](https://en.wikipedia.org/wiki/Left_recursion)
