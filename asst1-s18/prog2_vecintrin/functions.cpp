#include <stdio.h>
#include <algorithm>
#include <math.h>
#include "CMU418intrin.h"
#include "logger.h"
using namespace std;


void absSerial(float* values, float* output, int N) {
    for (int i=0; i<N; i++) {
	float x = values[i];
	if (x < 0) {
	    output[i] = -x;
	} else {
	    output[i] = x;
	}
    }
}

// implementation of absolute value using 15418 instrinsics
void absVector(float* values, float* output, int N) {
	// x
	// 都是只有八位
    __cmu418_vec_float x;
    __cmu418_vec_float result;
    __cmu418_vec_float zero = _cmu418_vset_float(0.f);
    __cmu418_mask maskAll, maskIsNegative, maskIsNotNegative;

    //  Note: Take a careful look at this loop indexing.  This example
    //  code is not guaranteed to work when (N % VECTOR_WIDTH) != 0.
    //  Why is that the case?
	// 想要对不是VECTOR_WIDTH整数倍的N进行处理，像下面一样创建一个有效掩码即可maskValid，
	// 然后将下面的maskAll改为maskValid
    for (int i=0; i<N; i+=VECTOR_WIDTH) {

	// All ones
	maskAll = _cmu418_init_ones();

	// All zeros
	maskIsNegative = _cmu418_init_ones(0);

	// Load vector of values from contiguous memory addresses
	_cmu418_vload_float(x, values+i, maskAll);               // x = values[i];

	// Set mask according to predicate
	_cmu418_vlt_float(maskIsNegative, x, zero, maskAll);     // if (x < 0) {

	// Execute instruction using mask ("if" clause)
	_cmu418_vsub_float(result, zero, x, maskIsNegative);      //   output[i] = -x;

	// Inverse maskIsNegative to generate "else" mask
	maskIsNotNegative = _cmu418_mask_not(maskIsNegative);     // } else {

	// Execute instruction ("else" clause)
	_cmu418_vload_float(result, values+i, maskIsNotNegative); //   output[i] = x; }

	// Write results back to memory
	_cmu418_vstore_float(output+i, result, maskAll);
    }
}

// Accepts an array of values and an array of exponents
// For each element, compute values[i]^exponents[i] and clamp value to
// 4.18.  Store result in outputs.
// Uses iterative squaring, so that total iterations is proportional
// to the log_2 of the exponent
void clampedExpSerial(float* values, int* exponents, float* output, int N) {
    for (int i=0; i<N; i++) {
	float x = values[i];
	float result = 1.f;
	int y = exponents[i];
	float xpower = x;
	//经典的二进制幂算法
	//y每退一位，xpower都平方，导致xpower始终是对齐的
	// 是否将xpower乘到result里取决于y最低位是否为1
	while (y > 0) {
	    if (y & 0x1)	//注意i这里没有大括号
		result *= xpower;
	    xpower = xpower * xpower;
	    y >>= 1;
	}
	if (result > 4.18f) {
	    result = 4.18f;
	}
	output[i] = result;
    }
}

void clampedExpVector(float* values, int* exponents, float* output, int N) {
    // Implement your vectorized version of clampedExpSerial here
    __cmu418_vec_float x;
    __cmu418_vec_int y;
    __cmu418_vec_float result = _cmu418_vset_float(0.0f);
    __cmu418_vec_float xpower;
    
    __cmu418_mask maskAll = _cmu418_init_ones();
    __cmu418_mask maskValid;
    __cmu418_mask maskContinue; 
    
    __cmu418_vec_int zero_Int = _cmu418_vset_int(0);
    __cmu418_vec_int oneInt = _cmu418_vset_int(1);
    __cmu418_vec_float limit = _cmu418_vset_float(4.18f);
	for (int i=0; i<N; i+=VECTOR_WIDTH) {	
		// create valid mask
		// init_ones不输入参数的时候默认为1
		__cmu418_mask maskValid = _cmu418_init_ones(0); // 从0开始，全0
		for (int j=0; j<VECTOR_WIDTH; ++j) { 
			maskValid.value[j] = (i + j < N); 
		}
        
        _cmu418_vload_float(x, values + i, maskValid);
        _cmu418_vload_int(y, exponents + i, maskValid);
		_cmu418_vset_float(result, 1.0f, maskValid); // result = 1.0f
		_cmu418_vmove_float(xpower, x, maskValid);     //xpower = x;

		// 创建常数向量
		__cmu418_vec_int zero_Int = _cmu418_vset_int(0);
		__cmu418_vec_int oneInt = _cmu418_vset_int(1);
		__cmu418_vec_float limit = _cmu418_vset_float(4.18f);

		// 向量实现
		// 开始循环
		// maskContinue是用来判断哪些元素还大于0，是要不断更新的
		// 至于哪些要幂次，是maskLsbSet的事

		//检查y>0作为初始循环条件
		//// IMPORTANT: Must start as all zeros so garbage in invalid lanes doesn't trigger logic.
		__cmu418_mask maskContinue = _cmu418_init_ones(0);
		_cmu418_vgt_int(maskContinue, y, zero_Int, maskValid);
		while (_cmu418_cntbits(maskContinue) > 0) { 
			// create mask, ,对>0的y取出y为1的最低位
			__cmu418_vec_int lsb;
			_cmu418_vbitand_int(lsb, y, oneInt, maskContinue); // lsb = y & 0x1;

			// 日你妈这一步init浪费了我2个小时
			//一定要初始全0然后将lsb设为1，如果没有初始全0，上一轮的状态会影响到这一轮
			// mask的保护机制
			__cmu418_mask maskLsbSet = _cmu418_init_ones(0);
			_cmu418_veq_int(maskLsbSet, lsb, oneInt, maskContinue);

			//如果最低位为1，xpower乘到result里
			_cmu418_vmult_float(result, result, xpower, maskLsbSet);

			//xpower**2
			_cmu418_vmult_float(xpower, xpower, xpower, maskContinue);

			// 对所有大于0的y >>= 1
			_cmu418_vshiftright_int(y, y, oneInt, maskContinue);

			// update maskContinue
			_cmu418_vgt_int(maskContinue, y, zero_Int, maskContinue);

		}

	// 	// 标量实现
	// 	for (int j=0; j<VECTOR_WIDTH; ++j) { 
	// 		// 注意i的步长是VECTOR_WIDTH
	// 		int current_index = i + j;
	// 		//越界检查
	// 		if (current_index >= N) break;
	// 		result.value[j] = 1.0f;	
	// 		xpower.value[j] = x.value[j] = values[current_index];
	// 		y.value[j] = exponents[current_index];
	// 		while (y.value[j] > 0) { 
	// 			if (y.value[j] & 0x1) { 
	// 				result.value[j] *= xpower.value[j];
	// 			}
	// 			y.value[j] >>= 1;
	// 			xpower.value[j] = xpower.value[j] * xpower.value[j];
	// 		}
	// 		if (result.value[j] > 4.18f) { 
	// 			result.value[j] = 4.18f;
	// 		}
	// 		output[current_index] = result.value[j];
	// 	}

		// limit result to 4.18f
		__cmu418_mask maskOverLimit;
		_cmu418_vgt_float(maskOverLimit, result, limit, maskValid);
		_cmu418_vmove_float(result, limit, maskOverLimit);

		// store
		_cmu418_vstore_float(output + i, result, maskValid);
	}
}


float arraySumSerial(float* values, int N) {
    float sum = 0;
    for (int i=0; i<N; i++) {
	sum += values[i];
    }

    return sum;
}

// Assume N % VECTOR_WIDTH == 0
// Assume VECTOR_WIDTH is a power of 2
float arraySumVector(float* values, int N) {
    // Implement your vectorized version here
    //  ...
	__cmu418_vec_float sumVec = _cmu418_vset_float(0.f);
	__cmu418_vec_float valVec;
	__cmu418_mask maskAll = _cmu418_init_ones();
	float sum = 0.0f;
	for (int i=0; i<N; i+=VECTOR_WIDTH) {
		// 把所有的值load进valVec
		// 然后再加奥sumVec上
		// sumVec的宽度为VECTOR_WIDTH
		_cmu418_vload_float(valVec, values + i,  maskAll);
		_cmu418_vadd_float(sumVec, sumVec, valVec, maskAll);
	}
	for (int i=0; i<VECTOR_WIDTH; ++i) {
		sum += sumVec.value[i];
	}
	return sum;

}
