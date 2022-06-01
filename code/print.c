#include <stdio.h>

int main (void) {
    // insert code here...
    printf("ab\b\bcd\n");    // \b

    printf("abcd\x7f;fg\n");   // 输出 abcd;fg

    printf("Hello, World!Hello, World!Hello, World!\n");  // 普通：输出Hello, World!
    // 输出1    12    123    1234    12345    123456    1234567
    printf("1\t12\t123\t1234\t12345\t123456\t1234567\t1\t12\t123\t1234\t12345\t123456\t1234567\n");


    printf("a23\b45\n");    // \b 输出a245
    printf("\0337Hello,word!\0338123\n");  // ESC 7,8 输出123lo,word!
    printf("\033[sHello,word!\033[u123\n");  // 输出123lo,word!, 没用！！！

//    printf("\033[31;5mHello,word!\033[32;25;39m123\n");  // ESC [pn m, Hello,world红色，其余绿色
    printf("123\033[2DHello,word!\n");  // 光标左移，1Hello,word!
    printf("123\033[2CHello,word!\n");  // 光标右移，123  Hello,word!
   
    printf("\033[31m");  // ESC [pn m, Hello,world红色，其余绿色
    printf("\033[10;10H test!\n");  // 定位到10, 10，test!
    printf("\033[20;20H test!\n");  // 定位到20, 20，test!
    printf("\033[32;25;39m123\n");  // ESC [pn m, Hello,world红色，其余绿色  

 //   printf("\033[32;25;39m123\n");  // ESC [pn m, Hello,world红色，其余绿色
//    printf("232\033[2J");  // 清屏
//    printf("Hello, World!Hello, World!Hello, World!\n");  // 普通：输出:有空格   Hello, World!

	return 0;
}

