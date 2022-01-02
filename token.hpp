#ifndef TOKEN_HPP
enum class TokenType {	// defined `TokenType`
	__reserved, // reserved for parser(with raw `string`)
	number,
	ident,
	blank,
	sym_assign, // :=
	sym_le, // <=
	sym_ge, // >=
	sym
};
class Token {
	public:
		TokenType type;
		string val;
		Token(TokenType type, string val) : type(type), val(val) {}
};
vector<Token> tokens;
#endif

#define TOKEN_HPP
