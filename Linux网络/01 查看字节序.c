/*
#   查看字节序
*/

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

//通过联合体来查看本地字节序
union {
  short s;
  char c[sizeof(short)];
} unS;

union {
  int s;
  char c[sizeof(int)];
}unI;

//查看本地字节序
int test01()
{
  //测试short类型
  unS.s = 0x0102;
  printf("%d,%d,%d\n",unS.c[0],unS.c[1],unS.s);

	//测试int类型
	unI.s = 0x01020304;
	printf("%d,%d,%d,%d,%d\n", unI.c[0], unI.c[1], unI.c[2], unI.c[3], unI.s);
  return 0;
}

//本地字节序转网络字节序
int test02(void)
{
  //0x1234 --> 16进制数，一个数字占4bit，也就是一共占2byte，2字节
  short tmp = 0x1234;

  //从本地字节序转为网络字节序
  tmp = htons(tmp);

  if((*(char *)&tmp) == 0x34) 
  {
    printf("It's 小端法\n");
  } 
  else if((*(char *)&tmp) == 0x12) 
  {
    printf("It's 大端法\n");
  }

  return 0;
}

int main(void)
{
  //查看本地字节序
  test01();

  //本地字节序转网络字节序
  test02();

  return 0;
}

