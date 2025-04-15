#include "9cc.h"

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
