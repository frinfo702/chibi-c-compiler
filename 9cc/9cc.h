#include <stdio.h>
#include <stdlib.h>

#ifndef D8370612_C791_4E70_AED9_6EAABCCCCA3D
#define D8370612_C791_4E70_AED9_6EAABCCCCA3D

extern char *user_input; // Input program
typedef struct Node Node;

// Token
typedef enum {
  TK_RESERVED, // 記号
  TK_NUM,      // 整数トークン
  TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

// Token type
typedef struct Token Token;
/**
 * Token represents a token in the input program.
 * Args:
 *   kind: Type of the token (reserved symbol, number, or EOF)
 *   next: Pointer to the next token
 *   val: Value if the token is a number
 *   str: String representation of the token
 */
struct Token {
  TokenKind kind; // トークンの種類
  Token *next;    // 次のトークンへのポインタ
  int val;        // kindがTK_NUMの場合の数値
  char *str;      // トークン文字列へのポインタ
  int length;     // トークンの長さ
};

//
// parse.c
//

// AST node kind
typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_EQ,  // ==
  ND_NE,  // !=
  ND_LT,  // <
  ND_LE,  // <=
  ND_NUM, // 整数
} NodeKind;

/**
 * Node represents a node in the Abstract Syntax Tree.
 * Args:
 *   kind: Type of the node (operator or number)
 *   left_hand_side: Left child node
 *   right_hand_side: Right child node
 *   value: Value if the node is a number
 */
struct Node {
  NodeKind kind;         // ノードの型
  Node *left_hand_side;  // 左辺
  Node *right_hand_side; // 右辺
  int value;             // kindがND_NUMの場合のみ使用
};

//
// codegen.c
//

void error(char *fmt, ...);
void gen(Node *node);

//
// main.c
//
int main(int argc, char **argv);

//
// 9cc.c
//

// 現在処理中のトークン
extern Token *current_token;

Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

Token *tokenize();

#endif /* D8370612_C791_4E70_AED9_6EAABCCCCA3D */
