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
    
	for (int i=0; i<N; i+=VECTOR_WIDTH) {	
		// create valid_mask and maskall
		// init_ones不输入参数的时候默认为1
		__cmu418_mask maskAll = _cmu418_init_ones();
		__cmu418_mask maskValid = _cmu418_init_ones(0); // 从0开始，全0
		for (int j=0; j<VECTOR_WIDTH; ++j) { 
			maskValid.value[j] = (i + j < N); 
		}

		//load values and exp into x and y
		// 这里写得太乱了，看下面的
		// __cmu418_vec_float x = _cmu418_vset_float(0.f);
		// _cmu418_vload_float(x, values + i, mask_valid); // load
		// __cmu418_vec_float xpower = x;
		// __cmu418_vec_float result = _cmu418_vset_float(1.0f);
		// __cmu418_vec_int y = _cmu418_vset_int(0);
		// _cmu418_vload_int(y, exponents + i, mask_valid);
		__cmu418_vec_float x;
        __cmu418_vec_int y;
        __cmu418_vec_float result = _cmu418_vset_float(1.0f);
        __cmu418_vec_float xpower;
        
        _cmu418_vload_float(x, values + i, maskValid);
        _cmu418_vload_int(y, exponents + i, maskValid);
		_cmu418_vmove_float(xpower, x, maskValid);     //xpower = x;

		for (int j=0; j<VECTOR_WIDTH; ++j) { 
			// 注意i的步长是VECTOR_WIDTH
			int current_index = i + j;
			//越界检查
			if (current_index >= N) break;
			result.value[j] = 1.0f;	
			xpower.value[j] = x.value[j] = values[current_index];
			y.value[j] = exponents[current_index];
			while (y.value[j] > 0) { 
				if (y.value[j] & 0x1) { 
					result.value[j] *= xpower.value[j];
				}
				y.value[j] >>= 1;
				xpower.value[j] = xpower.value[j] * xpower.value[j];
			}
			if (result.value[j] > 4.18f) { 
				result.value[j] = 4.18f;
			}
			output[current_index] = result.value[j];
		}
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
}
