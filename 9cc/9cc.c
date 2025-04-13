#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの種類を表す列挙型
typedef enum {
  TK_RESERVED, // 記号
  TK_NUM,      // 整数トークン
  TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;
char *user_input; // Input program

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
};

/**
 * Creates a new token and links it to the previous token.
 * Args:
 *   kind: Type of the token to create
 *   prev: Previous token to link to
 *   str: String representation of the token
 * Returns:
 *   Pointer to the newly created token
 */
Token *new_token(TokenKind kind, Token *prev, char *str) {
  Token *new_tok = calloc(1, sizeof(Token));
  new_tok->kind = kind;
  new_tok->str = str;
  prev->next = new_tok;
  return new_tok;
}

// 抽象構文木のノードの種類
typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_NUM, // 整数
} NodeKind;

typedef struct Node Node;

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

/**
 * Creates a new AST node for operators.
 * Args:
 *   kind: Type of the node
 *   left_hand_side: Left child node
 *   right_hand_side: Right child node
 * Returns:
 *   Pointer to the newly created node
 */
Node *new_node(NodeKind kind, Node *left_hand_side, Node *right_hand_side) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->left_hand_side = left_hand_side;
  node->right_hand_side = right_hand_side;
  return node;
}

/**
 * Creates a new AST node for numbers.
 * Args:
 *   value: Integer value for the node
 * Returns:
 *   Pointer to the newly created number node
 */
Node *new_node_num(int value) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->value = value;
  return node;
}

// 現在処理中のトークン
Token *current_token;

/**
 * Reports an error with formatted message and exits.
 * Args:
 *   fmt: Format string for the error message
 *   ...: Variable arguments for formatting
 */
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

/**
 * Reports an error with location information and exits.
 * Args:
 *   location: Pointer to the location in source where error occurred
 *   fmt: Format string for the error message
 *   ...: Variable arguments for formatting
 */
void error_at(char *location, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int position = location - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", position, ""); // print position spaces.
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

/**
 * Consumes the next token if it matches the expected operator.
 * Args:
 *   op: Expected operator character
 * Returns:
 *   true if token was consumed, false otherwise
 */
bool consume(char op) {
  if (current_token->kind != TK_RESERVED || current_token->str[0] != op)
    return false;
  current_token = current_token->next;
  return true;
}

/**
 * Ensures next token is the expected symbol and advances.
 * Args:
 *   op: Expected operator character
 */
void expect_symbol(char op) {
  if (current_token->kind != TK_RESERVED || current_token->str[0] != op)
    error_at(current_token->str, "expected: '%c'but got: '%s'", op,
             current_token->str);
  current_token = current_token->next;
}

/**
 * Ensures next token is a number and returns its value.
 * Returns:
 *   The value of the number token
 */
int expect_number() {
  if (current_token->kind != TK_NUM)
    error_at(current_token->str, "expected a number");
  int val = current_token->val;
  current_token = current_token->next;
  return val;
}

/**
 * Checks if the current token is EOF.
 * Returns:
 *   true if current token is EOF, false otherwise
 */
bool at_eof() { return current_token->kind == TK_EOF; }

/**
 * Tokenizes the input string into a linked list of tokens.
 * Returns:
 *   Pointer to the first token in the linked list
 */
Token *tokenize() {
  char *input_ptr = user_input;
  Token head;
  head.next = NULL;
  Token *tail = &head;

  while (*input_ptr) {
    if (isspace(*input_ptr)) {
      input_ptr++;
      continue;
    }

    if (*input_ptr == '+' || *input_ptr == '-') {
      tail = new_token(TK_RESERVED, tail, input_ptr++);
      continue;
    }

    if (isdigit(*input_ptr)) {
      tail = new_token(TK_NUM, tail, input_ptr);
      tail->val = strtol(input_ptr, &input_ptr, 10);
      continue;
    }

    error_at(input_ptr, "unexpected character");
  }

  new_token(TK_EOF, tail, input_ptr);
  return head.next;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    error("引数の数が正しくありません");
  }

  // 入力をトークナイズ
  user_input = argv[1];
  current_token = tokenize();

  // アセンブリのヘッダ出力
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // 最初の数値をRAXに移動
  printf("  mov rax, %d\n", expect_number());

  // 式の解析とコード生成
  while (!at_eof()) {
    if (consume('+')) {
      printf("  add rax, %d\n", expect_number());
      continue;
    }

    expect_symbol('-');
    printf("  sub rax, %d\n", expect_number());
  }

  printf("  ret\n");
  return 0;
}
