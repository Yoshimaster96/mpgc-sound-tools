#ifndef SERIALIZE_H
#define SERIALIZE_H

//Includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

//Unions
typedef union {
	uint32_t i;
	float f;
} T_f32_cnv;
typedef union {
	uint64_t i;
	double f;
} T_f64_cnv;

//Functions
//Read int little-endian
uint8_t read_u8_le(FILE * fp);
uint16_t read_u16_le(FILE * fp);
uint32_t read_u32_le(FILE * fp);
uint64_t read_u64_le(FILE * fp);
int8_t read_s8_le(FILE * fp);
int16_t read_s16_le(FILE * fp);
int32_t read_s32_le(FILE * fp);
int64_t read_s64_le(FILE * fp);
//Read int big-endian
uint8_t read_u8_be(FILE * fp);
uint16_t read_u16_be(FILE * fp);
uint32_t read_u32_be(FILE * fp);
uint64_t read_u64_be(FILE * fp);
int8_t read_s8_be(FILE * fp);
int16_t read_s16_be(FILE * fp);
int32_t read_s32_be(FILE * fp);
int64_t read_s64_be(FILE * fp);
//Write int little-endian
void write_u8_le(FILE * fp,uint8_t val);
void write_u16_le(FILE * fp,uint16_t val);
void write_u32_le(FILE * fp,uint32_t val);
void write_u64_le(FILE * fp,uint64_t val);
void write_s8_le(FILE * fp,int8_t val);
void write_s16_le(FILE * fp,int16_t val);
void write_s32_le(FILE * fp,int32_t val);
void write_s64_le(FILE * fp,int64_t val);
//Write int big-endian
void write_u8_be(FILE * fp,uint8_t val);
void write_u16_be(FILE * fp,uint16_t val);
void write_u32_be(FILE * fp,uint32_t val);
void write_u64_be(FILE * fp,uint64_t val);
void write_s8_be(FILE * fp,int8_t val);
void write_s16_be(FILE * fp,int16_t val);
void write_s32_be(FILE * fp,int32_t val);
void write_s64_be(FILE * fp,int64_t val);
//Read float little-endian
float read_f32_le(FILE * fp);
double read_f64_le(FILE * fp);
//Read float big-endian
float read_f32_be(FILE * fp);
float read_f64_be(FILE * fp);
//Write float little-endian
void write_f32_le(FILE * fp,float val);
void write_f64_le(FILE * fp,double val);
//Write float big-endian
void write_f32_be(FILE * fp,float val);
void write_f64_be(FILE * fp,double val);

#endif
