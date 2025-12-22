# 项目逻辑框架
## prog1
一个pthread的复习。没啥好讲的，看懂makefile即可
## prog2
关于SIMD的手动实现
makefile不用看, look main. 
### main
    上来先prase **commandline options**, struct option是cpp常用库的类，语法可以学学

    init

    运行串行版本：clampedExpSerial -> gold

    向量版本：   clampedExpVector ->  output

    验证结果 (verifyResult)

    打印性能统计 (CMU418Logger)

    验证 ARRAY SUM (可选bonus)

    delete

- 整个项目待完成的地方：

    - clampedExpVector
    
    - abs(作为参考，可以不实现maskvalid)

    - arraySumVector

- 要点
    - 掩码是 0，指令对这些 Lane **什么都不做**，而不是替换为0！
    - 要对什么进行操作，永远是先做一个掩码出来
    - 使用向量化接口和掩码来完成赋值操作
        __cmu418_vec_float x = _cmu418_vset_float(0.f);
        _cmu418_vload_float(x, values + i, mask_valid); // load
	- 我们无法直接对向量中的每个元素进行条件判断（因为每个元素的指数不同，循环次数可能不同）。所以我们需要一个循环，直到所有元素的指数都变为0。然而，向量操作是同时作用于所有通道的，所以我们需要一个循环，每次循环处理所有通道的当前最低位，然后同时右移。

## prog3
用ISPC来计算mandelbrot

## prog4
use ISPC to calculate RMS


## prog5
SAXPY的多种并行实现


# 语法速查
static struct option[]	        定义结构体数组，static 让其只对本文件可见

{"name", has_arg, flag, val}	选项结构：名字、是否需参、标志指针、返回值

getopt_long()	                同时支持短选项和长选项的函数

optarg	                        全局变量，存储选项的参数值

atoi()	                        字符串转整数

EOF	                            End Of File，值为 -1，表示输入结束static 

ANSI转义码，\e[1;31m表示粗体红色，\e[0m表示重置
  printf("\e[1;31mCLAMPED EXPONENT\e[0m (required) \n");

