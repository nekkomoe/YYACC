#ifndef UTILS_HPP
#define errorlog(args...) (printf(args), exit(0))

#define walk0(hd, arr) for(int hd = 0 ; (hd) <= (arr) ; ++ (hd))
#define walk0c(hd, arr, cond) for(int hd = 0 ; (hd) <= (arr) ; ++ (hd)) if((cond))
#define walk(hd, arr) for(int hd = 1 ; (hd) <= (arr) ; ++ (hd))
#define walkc(hd, arr, cond) for(int hd = 1 ; (hd) <= (arr) ; ++ (hd)) if((cond))
#define walka(hd, vec) for(auto hd: (vec)) 
#define walkac(hd, vec, cond) for(auto hd: (vec)) if((cond)) 
#define decl(hd, vec, size) vec(hd)(size + 1) // declare with pascal style(index from `1`)
#define vecrange(hd) (hd).begin(), (hd).end()

#include "table.hpp"
#include "filemanager.hpp"

#include "token.hpp"
#include "lexer.hpp"
#include "parser.hpp"

#endif
#define UTILS_HPP
