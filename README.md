# CompilationPrinciple-test
编译原理实验-词法分析(对简单c程序)

#### 实验目的
编制一个读单词过程，从输入的源程序中，识别出各个具有独立意义的单词，即基本保留字、标识符、常数、运算符、分隔符五大类。并依次输出各个单词的内部编码及单词符号自身值

#### 测试数据
#include "math.h"<br>
main()<br>
{<br>
float a,b,y1,y2;<br>
int c,d,e;<br>
a=3.1415926;b=25.0;<br>
scanf("%d,%d",&c,&d);<br>
y1=cos(a/3);<br>
y2=sqrt(b);<br>
e=d+(-1);<br>
if(c>d&&c>0)  e=c;<br>
printf("y1=%f,y2=%f,e=%d,c=%x\n",y1,y2,e,c);<br>
}<br>


