#include <stdio.h>
#include <algorithm>
#include <getopt.h>
#include <math.h>
#include "CMU418intrin.h"
#include "logger.h"
using namespace std;

extern void absSerial(float* values, float* output, int N);
extern void absVector(float* values, float* output, int N);
extern void clampedExpSerial(float* values, int* exponents, float* output, int N);
extern void clampedExpVector(float* values, int* exponents, float* output, int N);
extern float arraySumSerial(float* values, int N);
extern float arraySumVector(float* values, int N);



#define EXP_MAX 1024

Logger CMU418Logger;
// declaration
void usage(const char* progname);
void initValue(float* values, int* exponents, float* output, float* gold, unsigned int N);
void absSerial(float* values, float* output, int N);
void absVector(float* values, float* output, int N);
void clampedExpSerial(float* values, int* exponents, float* output, int N);
void clampedExpVector(float* values, int* exponents, float* output, int N);
float arraySumSerial(float* values, int N);
float arraySumVector(float* values, int N);
bool verifyResult(float* values, int* exponents, float* output, float* gold, int N);

int main(int argc, char * argv[]) {
  // 默认数组大小和是否打印日志
  int N = 16;                     //N初始值为16，也可以由commandline设置size
  bool printLog = false;

  // parse commandline options ////////////////////////////////////////////
  // 这里定义选项，struct option是cpp自带的
  int opt;
  static struct option long_options[] = {
    {"size", 1, 0, 's'},      // flag=1表示需要一个参数，长选项为--size, -s for short.
    {"log", 0, 0, 'l'},
    {"help", 0, 0, '?'},
    {0 ,0, 0, 0}      //终止标记，告诉getopt_long这是最后一个元素
  };
  
  // 根据选项一个个给opt赋值，switch看选项是啥
  while ((opt = getopt_long(argc,                 //参数个数
                    argv,                         //参数数组 
                    "s:l?",                     //短选项格式 s:需要参数，冒号表示选项后必须跟值
                    long_options, //长选项定义，不需要记录长选项索引
                    NULL)
                  ) != EOF) {        //所有选项被解析完时返回EOF (-1)

    switch (opt) {
      case 's':                             
      // optarg是getopt的全局变量，extern过了，存储紧跟在有参数选项后面的值
        N = atoi(optarg);                    // 将参数转为整数
        if (N <= 0) {                        // 检查是否合法
          printf("Error: Workload size is set to %d (<0).\n", N);
          return -1;                         // 出错返回
        }
        break;

      case 'l':                              // 用户输入了 -l 或 --log
        printLog = true;                     // 设置打印日志标志
        break;

      case '?':                              // 用户输入了 -? 或 --help
      default:                               // 或输入了未定义的选项
        usage(argv[0]);                      // 打印使用说明
        return 1;                            // 退出
    }
  }

  // init
  //+VECTOR_WIDTH避免越界错误
  float* values = new float[N+VECTOR_WIDTH];
  int* exponents = new int[N+VECTOR_WIDTH];
  float* output = new float[N+VECTOR_WIDTH];
  float* gold = new float[N+VECTOR_WIDTH];
  initValue(values, exponents, output, gold, N);

  //clamped exponent: 受限指数运算
  //clamp: 紧紧抓住，被抓住，被夹紧
  clampedExpSerial(values, exponents, gold, N);
  clampedExpVector(values, exponents, output, N);

  //absSerial(values, gold, N);
  //absVector(values, output, N);

  //输出和验证结果
  //ANSI转义码，\e[1;31m表示粗体红色，\e[0m表示重置
  printf("\e[1;31mCLAMPED EXPONENT\e[0m (required) \n");
  bool clampedCorrect = verifyResult(values, exponents, output, gold, N);
  if (printLog) CMU418Logger.printLog();
  CMU418Logger.printStats();

  printf("************************ Result Verification *************************\n");
  if (!clampedCorrect) {
    printf("@@@ Failed!!!\n");
  } else {
    printf("Passed!!!\n");
  }

  printf("\n\e[1;31mARRAY SUM\e[0m (bonus) \n");
  if (N % VECTOR_WIDTH == 0) {
    float sumGold = arraySumSerial(values, N);
    float sumOutput = arraySumVector(values, N);
    float epsilon = 0.1;
    bool sumCorrect = abs(sumGold - sumOutput) < epsilon * 2;
    if (!sumCorrect) {
      printf("Expected %f, got %f\n.", sumGold, sumOutput);
      printf("@@@ Failed!!!\n");
    } else {
      printf("Passed!!!\n");
    }
  } else {
    printf("Must have N % VECTOR_WIDTH == 0 for this problem (VECTOR_WIDTH is %d)\n", VECTOR_WIDTH);
  }

  delete[] values;
  delete[] exponents;
  delete[] output;
  delete[] gold;

  return 0;
}

void usage(const char* progname) {
  printf("Usage: %s [options]\n", progname);
  printf("Program Options:\n");
  printf("  -s  --size <N>     Use workload size N (Default = 16)\n");
  printf("  -l  --log          Print vector unit execution log\n");
  printf("  -?  --help         This message\n");
}

/// @brief 初始化值
/// @param values 是-1 to -1.01之间的随机浮点数
/// @param exponents 随机指数，范围（0~1023）
/// @param output 
/// @param gold 
/// @param N 
void initValue(float* values, int* exponents, float* output, float* gold, unsigned int N) {

  for (unsigned int i=0; i<N+VECTOR_WIDTH; i++)
  {
    // random input values
    values[i] = -1.f - 0.01f * static_cast<float>(rand()) / RAND_MAX;
    exponents[i] = rand() % EXP_MAX;
    output[i] = 0.f;
    gold[i] = 0.f;
  }

}

bool verifyResult(float* values, int* exponents, float* output, float* gold, int N) {
  // init to -1, 如果经历一大坨还是-1的话说明是对的
  int incorrect = -1;
  float epsilon = 0.00001;
  for (int i=0; i<N+VECTOR_WIDTH; i++) {
    if ( abs(output[i] - gold[i]) > epsilon ) {
      incorrect = i;
      break;
    }
  }
  // 有大于epsilon的值，接下来根据incorrect的类型打印错误
  if (incorrect != -1) {
    if (incorrect >= N)
      printf("You have written to out of bound value!\n");
    printf("Wrong calculation at value[%d]!\n", incorrect);
    printf("value  = ");
    for (int i=0; i<N; i++) {
      printf("% f ", values[i]);
    } printf("\n");

    printf("exp    = ");
    for (int i=0; i<N; i++) {
      printf("% 9d ", exponents[i]);
    } printf("\n");

    printf("output = ");
    for (int i=0; i<N; i++) {
      printf("% f ", output[i]);
    } printf("\n");

    printf("gold   = ");
    for (int i=0; i<N; i++) {
      printf("% f ", gold[i]);
    } printf("\n");
    return false;
  }
  printf("Results matched with answer!\n");
  return true;
}

