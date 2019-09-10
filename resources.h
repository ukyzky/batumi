// Copyright 2015 Matthias Puech.
//
// Author: Matthias Puech (matthias.puech@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// Resources definitions.
//
// Automatically generated with:
// make resources


#ifndef _RESOURCES_H_
#define _RESOURCES_H_


#include "stmlib/stmlib.h"



namespace batumi {

typedef uint8_t ResourceId;

extern const char* string_table[];

extern const uint16_t* lookup_table_table[];

extern const uint32_t* lookup_table_32_table[];

extern const int16_t* waveform_table[];

extern const uint16_t lut_scale_pitch[];
extern const uint16_t lut_scale_phase[];
extern const uint16_t lut_scale_divide[];
extern const int16_t lut_scale_divide_multiply[];
extern const uint32_t lut_increments[];
extern const int16_t wav_sine[];
extern const int16_t wav_saw10[];
extern const int16_t wav_saw100[];
extern const int16_t wav_tri10[];
extern const int16_t wav_tri100[];
extern const int16_t wav_trap10[];
extern const int16_t wav_trap100[];
extern const int16_t wav_bl_step0[];
extern const int16_t wav_bl_step1[];
extern const int16_t wav_bl_step2[];
extern const int16_t wav_bl_step3[];
extern const int16_t wav_bl_step4[];
extern const int16_t wav_bl_step5[];
extern const int16_t wav_bl_step6[];
extern const int16_t wav_bl_step7[];
extern const int16_t wav_bl_step8[];
extern const int16_t wav_bl_step9[];
extern const int16_t wav_bl_step10[];
extern const int16_t wav_bl_step11[];
extern const int16_t wav_bl_step12[];
extern const int16_t wav_bl_step13[];
extern const int16_t wav_bl_step14[];
extern const int16_t wav_bl_step15[];
extern const int16_t wav_bl_step16[];
extern const int16_t wav_bl_step17[];
extern const int16_t wav_bl_step18[];
extern const int16_t wav_bl_step19[];
extern const int16_t wav_bl_step20[];
extern const int16_t wav_bl_step21[];
extern const int16_t wav_bl_step22[];
extern const int16_t wav_bl_step23[];
extern const int16_t wav_bl_step24[];
extern const int16_t wav_bl_step25[];
extern const int16_t wav_bl_step26[];
extern const int16_t wav_bl_step27[];
extern const int16_t wav_bl_step28[];
extern const int16_t wav_bl_step29[];
extern const int16_t wav_bl_step30[];
extern const int16_t wav_bl_step31[];
#define STR_DUMMY 0  // dummy
#define LUT_SCALE_PITCH 0
#define LUT_SCALE_PITCH_SIZE 257
#define LUT_SCALE_PHASE 1
#define LUT_SCALE_PHASE_SIZE 257
#define LUT_SCALE_DIVIDE 2
#define LUT_SCALE_DIVIDE_SIZE 257
#define LUT_INCREMENTS 0
#define LUT_INCREMENTS_SIZE 97
#define WAV_SINE 0
#define WAV_SINE_SIZE 1025
#define WAV_SAW10 1
#define WAV_SAW10_SIZE 1025
#define WAV_SAW100 2
#define WAV_SAW100_SIZE 1025
#define WAV_TRI10 3
#define WAV_TRI10_SIZE 1025
#define WAV_TRI100 4
#define WAV_TRI100_SIZE 1025
#define WAV_TRAP10 5
#define WAV_TRAP10_SIZE 1025
#define WAV_TRAP100 6
#define WAV_TRAP100_SIZE 1025
#define WAV_BL_STEP0 7
#define WAV_BL_STEP0_SIZE 8
#define WAV_BL_STEP1 8
#define WAV_BL_STEP1_SIZE 8
#define WAV_BL_STEP2 9
#define WAV_BL_STEP2_SIZE 8
#define WAV_BL_STEP3 10
#define WAV_BL_STEP3_SIZE 8
#define WAV_BL_STEP4 11
#define WAV_BL_STEP4_SIZE 8
#define WAV_BL_STEP5 12
#define WAV_BL_STEP5_SIZE 8
#define WAV_BL_STEP6 13
#define WAV_BL_STEP6_SIZE 8
#define WAV_BL_STEP7 14
#define WAV_BL_STEP7_SIZE 8
#define WAV_BL_STEP8 15
#define WAV_BL_STEP8_SIZE 8
#define WAV_BL_STEP9 16
#define WAV_BL_STEP9_SIZE 8
#define WAV_BL_STEP10 17
#define WAV_BL_STEP10_SIZE 8
#define WAV_BL_STEP11 18
#define WAV_BL_STEP11_SIZE 8
#define WAV_BL_STEP12 19
#define WAV_BL_STEP12_SIZE 8
#define WAV_BL_STEP13 20
#define WAV_BL_STEP13_SIZE 8
#define WAV_BL_STEP14 21
#define WAV_BL_STEP14_SIZE 8
#define WAV_BL_STEP15 22
#define WAV_BL_STEP15_SIZE 8
#define WAV_BL_STEP16 23
#define WAV_BL_STEP16_SIZE 8
#define WAV_BL_STEP17 24
#define WAV_BL_STEP17_SIZE 8
#define WAV_BL_STEP18 25
#define WAV_BL_STEP18_SIZE 8
#define WAV_BL_STEP19 26
#define WAV_BL_STEP19_SIZE 8
#define WAV_BL_STEP20 27
#define WAV_BL_STEP20_SIZE 8
#define WAV_BL_STEP21 28
#define WAV_BL_STEP21_SIZE 8
#define WAV_BL_STEP22 29
#define WAV_BL_STEP22_SIZE 8
#define WAV_BL_STEP23 30
#define WAV_BL_STEP23_SIZE 8
#define WAV_BL_STEP24 31
#define WAV_BL_STEP24_SIZE 8
#define WAV_BL_STEP25 32
#define WAV_BL_STEP25_SIZE 8
#define WAV_BL_STEP26 33
#define WAV_BL_STEP26_SIZE 8
#define WAV_BL_STEP27 34
#define WAV_BL_STEP27_SIZE 8
#define WAV_BL_STEP28 35
#define WAV_BL_STEP28_SIZE 8
#define WAV_BL_STEP29 36
#define WAV_BL_STEP29_SIZE 8
#define WAV_BL_STEP30 37
#define WAV_BL_STEP30_SIZE 8
#define WAV_BL_STEP31 38
#define WAV_BL_STEP31_SIZE 8

}  // namespace batumi

#endif  // _RESOURCES_H_
