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

// トークンを表す構造体
struct Token {
  TokenKind kind; // トークンの種類
  Token *next;    // 次のトークンへのポインタ
  int val;        // kindがTK_NUMの場合の数値
  char *str;      // トークン文字列へのポインタ
};

// 新しいトークンを作成して連結
Token *new_token(TokenKind kind, Token *prev, char *str) {
  Token *new_tok = calloc(1, sizeof(Token));
  new_tok->kind = kind;
  new_tok->str = str;
  prev->next = new_tok;
  return new_tok;
}

// 現在処理中のトークン
Token *current_token;

// Reports an error and exits the program.
// Takes the same arguments as printf.
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// 次のトークンが期待する記号ならトークンを進めてtrueを返す
bool consume(char op) {
  if (current_token->kind != TK_RESERVED || current_token->str[0] != op)
    return false;
  current_token = current_token->next;
  return true;
}

// 次のトークンが期待する記号でなければエラー
void expect(char op) {
  if (current_token->kind != TK_RESERVED || current_token->str[0] != op)
    error("期待された記号: '%c'ですが、実際には '%s'でした", op,
          current_token->str);
  current_token = current_token->next;
}

// 次のトークンが数値でなければエラー
int expect_number() {
  if (current_token->kind != TK_NUM)
    error("数ではありません");
  int val = current_token->val;
  current_token = current_token->next;
  return val;
}

// トークンが終端かどうか
bool at_eof() { return current_token->kind == TK_EOF; }

// 入力文字列をトークンに分割
Token *tokenize(char *input_ptr) {
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

    error("トークナイズできません");
  }

  new_token(TK_EOF, tail, input_ptr);
  return head.next;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    error("引数の数が正しくありません");
  }

  // 入力をトークナイズ
  current_token = tokenize(argv[1]);

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

    expect('-');
    printf("  sub rax, %d\n", expect_number());
  }

  printf("  ret\n");
  return 0;
}
