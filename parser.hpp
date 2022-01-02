#ifndef PARSER_HPP

enum class NodeType {
	Nonterminal, Terminal
};

class Node {
	public:
		NodeType type;
		string val; // available when NodeType is `Terminal`
		TokenType tt; // available when NodeType is `Terminal`
		
		typedef function<void(Node*)> F_ACT; // parent node, self node
		vector<pair<vector<Node*>, F_ACT>> child; // use for CFG
		
		vector<Node*> astchild; // use for AST
		
		string nonterminal_name; // use for debug(print `Nonterminal` name)
		bool emptynode; // emptynode for string `""`

		class F_ACTs { // basic function for user use
			public:
				static void remPriTail(Node *nd) { // remove CFG tail
					if(nd -> astchild.empty()) return ;
					Node *tail = nd -> astchild.back();
					nd -> astchild.pop_back();
					walka(x, tail -> astchild) {
						nd -> astchild.push_back(x);
					}
					delete tail;
				};
				static void remPriTail2rd(Node *nd) { // remove CFG last second
					if((int) nd -> astchild.size() < 2) return ;
					Node *tail = nd -> astchild.back();
					nd -> astchild.pop_back();
					remPriTail(nd);
					nd -> astchild.push_back(tail);
				};
		};

		typedef function<void(Node*, int)> FF_ACT;
		FF_ACT act; // maintain self after matched
	public:
		static string printAST(Node *nd) {
			function<string(Node*, int)> dfs = [&] (Node *nd, int dep) {
				string res;
				for(int i = 1 ; i <= dep ; ++ i) {
					res += "  ";
				}
				if(nd -> type == NodeType :: Nonterminal) {
					res += "<" + nd -> nonterminal_name + ">\n";
				} else {
					res += "[" + nd -> val + "]\n";
				}
				walka(x, nd -> astchild) {
					res += dfs(x, dep + 1);
				}
				return res;
			};
			string res;
			res += "============================= AST ==============================\n";
			res += dfs(nd, 0);
			res += "================================================================\n";
			return res;
		}

		static void freeASTNode(Node *nd) { // free `astchild`
			walka(c, nd -> astchild) {
				freeASTNode(c);
			}
			delete nd;
		};

		static Node* match(Node *S) {
			int ptr = 0;
			function<Node*(Node*)> dfs = [&] (Node *sta) {
				int ptrmem = ptr;		
				if(sta -> type == NodeType :: Terminal) {
					if(sta -> tt == TokenType :: __reserved) { // reserved for raw `string`
						if(sta -> val == "") {
							return (new Node(NodeType :: Terminal, "")) -> setEmptyNode(true);
						} else if(sta -> val == tokens[ptr].val) {
							Node *res = new Node(NodeType :: Terminal, tokens[ptr].val, tokens[ptr].type); // remain lexer's tokentype
							++ ptr;
							return res;
						} else {
							return (Node *)NULL;
						}
					} else {
						if(sta -> tt == tokens[ptr].type) {
							Node *res = new Node(NodeType :: Terminal, tokens[ptr].val, tokens[ptr].type);
							++ ptr;
							return res;
						} else {
							return (Node *)NULL;
						}
					}
				} else if(sta -> type == NodeType :: Nonterminal) {
					int ruleidx = 0;
					for(auto &c: sta -> child) {
						Node *res = new Node(NodeType :: Nonterminal);
						res -> nonterminal_name = sta -> nonterminal_name;
						for(auto ele: c.first) {
							Node *tmp = dfs(ele);
							if(tmp == NULL) {
								goto nxt;
							} else if(tmp -> emptynode == false) {
								res -> astchild.push_back(tmp);
							} else {
								Node :: freeASTNode(tmp);
							}
						}
						if(c.second) { // if has declared action function, then use it
							c.second(res);
						} else if(res -> astchild.empty()) {
							res -> setEmptyNode(true);
						}
						if(sta -> act) {
							sta -> act(res, ruleidx);
						}
						return res;
						nxt: ptr = ptrmem;
						++ ruleidx;
						Node :: freeASTNode(res);
					}
				}
				ptr = ptrmem;
				return (Node *)NULL;
			};
			return dfs(S);
		}
	public:
		Node() {
			type = NodeType :: Nonterminal;
			val = "";
			tt = TokenType :: __reserved;
			child.clear(), astchild.clear();
			emptynode = false;
			act = NULL;
		}
		Node(NodeType type, string val, TokenType tt) : type(type), val(val), tt(tt) { emptynode = false; }
		Node(NodeType type, string val) : type(type), val(val) { tt = TokenType :: __reserved; emptynode = false; }
		Node(NodeType type) : type(type) { tt = TokenType :: __reserved; emptynode = false; }

		Node* setTokenType(TokenType tt) {
			this -> tt = tt;
			return this;
		}
		Node* setNodeType(NodeType type) {
			this -> type = type;
			return this;
		}
		Node* setEmptyNode(bool flag) {
			this -> emptynode = flag;
			return this;
		}
		Node* setAct(FF_ACT act) {
			this -> act = act;
			return this;
		}
		Node* add(vector<Node*> args, F_ACT act = NULL) {
			child.push_back(make_pair(args, act));
			return this;
		}
};

class Parser {
	private:
		Node *nonterminal_nodes_pool;
		vector<Node*> terminal_nodes_pool;
		Node *astroot;

	public:
		Node* match(Node *S) {
			astroot = Node :: match(S);
			return astroot;
		}
		Node* getASTroot() {
			return astroot;
		}
		Node* getTerminalNode(string val) {
			terminal_nodes_pool.push_back(new Node(NodeType :: Terminal, val));
			return terminal_nodes_pool.back();
		}
		Node* getNonterminalNode(int idx) {
			return &nonterminal_nodes_pool[idx];
		}
		~Parser() {
		    // free memory
		    Node :: freeASTNode(astroot);
		    delete [] nonterminal_nodes_pool;
		    walka(x, terminal_nodes_pool) delete x;
		}
		Parser() {
			nonterminal_nodes_pool = new Node[__Nonterminal_Count];
			for(int i = 0 ; i < __Nonterminal_Count ; ++ i) {
                nonterminal_nodes_pool[i].nonterminal_name = string(symbols[i]);
            }
		}
};

#endif
#define PARSER_HPP
