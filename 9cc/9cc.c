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
  int length;     // トークンの長さ
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
Token *new_token(TokenKind kind, Token *current_token, char *str, int length) {
  Token *new_tok = calloc(1, sizeof(Token));
  new_tok->kind = kind;
  new_tok->str = str;
  new_tok->length = length;
  current_token->next = new_tok;
  return new_tok;
}

// 抽象構文木のノードの種類
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
bool consume(char *op) {
  if (current_token->kind != TK_RESERVED ||
      strlen(op) !=
          current_token
              ->length || // 先にチェックしないと">="を">"と"="に認識する可能性あり
      memcmp(current_token->str, op, current_token->length))
    return false;
  current_token = current_token->next;
  return true;
}

/**
 * Ensures current token is the expected symbol and advances.
 * Args:
 *   op: Expected operator character
 */
void expect_symbol(char *op) {
  if (current_token->kind != TK_RESERVED ||
      strlen(op) != current_token->length ||
      memcmp(current_token->str, op, current_token->length))
    error_at(current_token->str, "expected: \"%s\" but got: \"%s\"", op,
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

bool startswith(char *target_str, char *prefix) {
  return memcmp(target_str, prefix, strlen(prefix)) == 0;
}

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

    if (startswith(input_ptr, "==") || startswith(input_ptr, "!=") ||
        startswith(input_ptr, "<=") || startswith(input_ptr, ">=")) {
      tail = new_token(TK_RESERVED, tail, input_ptr, 2);
      input_ptr += 2;
      continue;
    }

    // 1文字の演算子を処理
    if (strchr("+-*/()<>", *input_ptr)) {
      tail = new_token(TK_RESERVED, tail, input_ptr, 1);
      input_ptr++;
      continue;
    }

    if (isdigit(*input_ptr)) {
      tail = new_token(TK_NUM, tail, input_ptr, 0);
      char *token_start = input_ptr;
      tail->val = strtol(input_ptr, &input_ptr, 10);
      tail->length = input_ptr - token_start;
      continue;
    }

    error_at(input_ptr, "unexpected character");
  }

  new_token(TK_EOF, tail, input_ptr, 0);
  return head.next;
}

// 関数プロトタイプ宣言
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

/**
 * Parses expressions.
 * Follows the grammar rule: expr = equality
 * Returns:
 *   Pointer to the root node of the parsed expression
 */
Node *expr() { return equality(); }

/**
 * Parses equality expressions with left-associative operators (==, !=).
 * Follows the grammar rule: equality = relational ("==" relational | "!="
 * relational)* Returns: Pointer to the root node of the parsed equality
 * expression
 */
Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("=="))
      node = new_node(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_node(ND_NE, node, relational());
    else
      return node;
  }
}

/**
 * Parses relational expressions with left-associative operators (<, <=, >, >=).
 * Follows the grammar rule: relational = add ("<" add | "<=" add | ">" add |
 * ">=" add)* Returns: Pointer to the root node of the parsed relational
 * expression
 */
Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<"))
      node = new_node(ND_LT, node, add());
    else if (consume("<="))
      node = new_node(ND_LE, node, add());
    else if (consume(">"))
      node = new_node(ND_LT, add(), node);
    else if (consume(">="))
      node = new_node(ND_LE, add(), node);
    else
      return node;
  }
}

/**
 * Parses additive expressions with left-associative operators (+, -).
 * Follows the grammar rule: add = mul ("+" mul | "-" mul)*
 * Returns:
 *   Pointer to the root node of the parsed additive expression
 */
Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+"))
      node = new_node(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_node(ND_SUB, node, mul());
    else
      return node;
  }
}

/**
 * Parses multiplicative expressions with left-associative operators (*, /).
 * Follows the grammar rule: mul = unary ("*" unary | "/" unary)*
 * Returns:
 *   Pointer to the root node of the parsed multiplicative expression
 */
Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*"))
      node = new_node(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_node(ND_DIV, node, unary());
    else
      return node;
  }
}

/**
 * Parses unary expressions with optional unary operators (+, -).
 * Follows the grammar rule: unary = ("+" | "-")? primary
 * Returns:
 *   Pointer to the root node of the parsed unary expression
 */
Node *unary() {
  if (consume("+"))
    return unary();
  if (consume("-"))
    return new_node(ND_SUB, new_node_num(0), unary());
  return primary();
}

/**
 * Parses primary expressions (numbers or parenthesized expressions).
 * Follows the grammar rule: primary = "(" expr ")" | num
 * Returns:
 *   Pointer to the root node of the parsed primary expression
 */
Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect_symbol(")");
    return node;
  }

  return new_node_num(expect_number());
}

void gen(Node *node) {
  if (node->kind == ND_NUM) {
    printf("  push %d\n", node->value);
    return;
  }

  gen(node->left_hand_side);
  gen(node->right_hand_side);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  case ND_EQ:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_NE:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LT:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  default:
    error("Unsupported node kind: %d", node->kind);
  }

  printf("  push rax\n");
}

int main(int argc, char **argv) {
  if (argc != 2) {
    error("%s: invalid number of arguments", argv[0]);
  }

  // 入力をトークナイズ
  user_input = argv[1];
  current_token = tokenize();
  Node *node = expr();

  // アセンブリのヘッダ出力
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // 抽象構文木を降りながらコード生成
  gen(node);

  // stackトップに式全体の値が残っているはずなので
  // それをRAXにロードして関数からの返り値とする
  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}
