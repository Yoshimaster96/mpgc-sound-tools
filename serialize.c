#include "serialize.h"

//Read int little-endian
uint8_t read_u8_le(FILE * fp) {
	uint8_t b0 = getc(fp)&0xFF;
	return b0;
}
uint16_t read_u16_le(FILE * fp) {
	uint16_t b0 = getc(fp)&0xFF;
	uint16_t b1 = getc(fp)&0xFF;
	return b0|(b1<<8);
}
uint32_t read_u32_le(FILE * fp) {
	uint32_t b0 = getc(fp)&0xFF;
	uint32_t b1 = getc(fp)&0xFF;
	uint32_t b2 = getc(fp)&0xFF;
	uint32_t b3 = getc(fp)&0xFF;
	return b0|(b1<<8)|(b2<<16)|(b3<<24);
}
uint64_t read_u64_le(FILE * fp) {
	uint64_t b0 = getc(fp)&0xFF;
	uint64_t b1 = getc(fp)&0xFF;
	uint64_t b2 = getc(fp)&0xFF;
	uint64_t b3 = getc(fp)&0xFF;
	uint64_t b4 = getc(fp)&0xFF;
	uint64_t b5 = getc(fp)&0xFF;
	uint64_t b6 = getc(fp)&0xFF;
	uint64_t b7 = getc(fp)&0xFF;
	return b0|(b1<<8)|(b2<<16)|(b3<<24)|(b4<<32)|(b5<<40)|(b6<<48)|(b7<<56);
}
int8_t read_s8_le(FILE * fp) {
	return (int8_t)read_u8_le(fp);
}
int16_t read_s16_le(FILE * fp) {
	return (int16_t)read_u16_le(fp);
}
int32_t read_s32_le(FILE * fp) {
	return (int32_t)read_u32_le(fp);
}
int64_t read_s64_le(FILE * fp) {
	return (int64_t)read_u64_le(fp);
}
//Read int big-endian
uint8_t read_u8_be(FILE * fp) {
	uint8_t b0 = getc(fp)&0xFF;
	return b0;
}
uint16_t read_u16_be(FILE * fp) {
	uint16_t b0 = getc(fp)&0xFF;
	uint16_t b1 = getc(fp)&0xFF;
	return (b0<<8)|b1;
}
uint32_t read_u32_be(FILE * fp) {
	uint32_t b0 = getc(fp)&0xFF;
	uint32_t b1 = getc(fp)&0xFF;
	uint32_t b2 = getc(fp)&0xFF;
	uint32_t b3 = getc(fp)&0xFF;
	return (b0<<24)|(b1<<16)|(b2<<8)|b3;
}
uint64_t read_u64_be(FILE * fp) {
	uint64_t b0 = getc(fp)&0xFF;
	uint64_t b1 = getc(fp)&0xFF;
	uint64_t b2 = getc(fp)&0xFF;
	uint64_t b3 = getc(fp)&0xFF;
	uint64_t b4 = getc(fp)&0xFF;
	uint64_t b5 = getc(fp)&0xFF;
	uint64_t b6 = getc(fp)&0xFF;
	uint64_t b7 = getc(fp)&0xFF;
	return (b0<<56)|(b1<<48)|(b2<<40)|(b3<<32)|(b4<<24)|(b5<<16)|(b6<<8)|b7;
}
int8_t read_s8_be(FILE * fp) {
	return (int8_t)read_u8_be(fp);
}
int16_t read_s16_be(FILE * fp) {
	return (int16_t)read_u16_be(fp);
}
int32_t read_s32_be(FILE * fp) {
	return (int32_t)read_u32_be(fp);
}
int64_t read_s64_be(FILE * fp) {
	return (int64_t)read_u64_be(fp);
}
//Write int little-endian
void write_u8_le(FILE * fp,uint8_t val) {
	putc(val&0xFF,fp);
}
void write_u16_le(FILE * fp,uint16_t val) {
	putc(val&0xFF,fp);
	putc((val>>8)&0xFF,fp);
}
void write_u32_le(FILE * fp,uint32_t val) {
	putc(val&0xFF,fp);
	putc((val>>8)&0xFF,fp);
	putc((val>>16)&0xFF,fp);
	putc((val>>24)&0xFF,fp);
}
void write_u64_le(FILE * fp,uint64_t val) {
	putc(val&0xFF,fp);
	putc((val>>8)&0xFF,fp);
	putc((val>>16)&0xFF,fp);
	putc((val>>24)&0xFF,fp);
	putc((val>>32)&0xFF,fp);
	putc((val>>40)&0xFF,fp);
	putc((val>>48)&0xFF,fp);
	putc((val>>56)&0xFF,fp);
}
void write_s8_le(FILE * fp,int8_t val) {
	write_u8_le(fp,(uint8_t)val);
}
void write_s16_le(FILE * fp,int16_t val) {
	write_u16_le(fp,(uint16_t)val);
}
void write_s32_le(FILE * fp,int32_t val) {
	write_u32_le(fp,(uint32_t)val);
}
void write_s64_le(FILE * fp,int64_t val) {
	write_u64_le(fp,(uint64_t)val);
}
//Write int big-endian
void write_u8_be(FILE * fp,uint8_t val) {
	putc(val&0xFF,fp);
}
void write_u16_be(FILE * fp,uint16_t val) {
	putc((val>>8)&0xFF,fp);
	putc(val&0xFF,fp);
}
void write_u32_be(FILE * fp,uint32_t val) {
	putc((val>>24)&0xFF,fp);
	putc((val>>16)&0xFF,fp);
	putc((val>>8)&0xFF,fp);
	putc(val&0xFF,fp);
}
void write_u64_be(FILE * fp,uint64_t val) {
	putc((val>>56)&0xFF,fp);
	putc((val>>48)&0xFF,fp);
	putc((val>>40)&0xFF,fp);
	putc((val>>32)&0xFF,fp);
	putc((val>>24)&0xFF,fp);
	putc((val>>16)&0xFF,fp);
	putc((val>>8)&0xFF,fp);
	putc(val&0xFF,fp);
}
void write_s8_be(FILE * fp,int8_t val) {
	write_u8_be(fp,(uint8_t)val);
}
void write_s16_be(FILE * fp,int16_t val) {
	write_u16_be(fp,(uint16_t)val);
}
void write_s32_be(FILE * fp,int32_t val) {
	write_u32_be(fp,(uint32_t)val);
}
void write_s64_be(FILE * fp,int64_t val) {
	write_u64_be(fp,(uint64_t)val);
}
//Read float little-endian
float read_f32_le(FILE * fp) {
	T_f32_cnv cnv;
	cnv.i = read_u32_le(fp);
	return cnv.f;
}
double read_f64_le(FILE * fp) {
	T_f64_cnv cnv;
	cnv.i = read_u64_le(fp);
	return cnv.f;
}
//Read float big-endian
float read_f32_be(FILE * fp) {
	T_f32_cnv cnv;
	cnv.i = read_u32_be(fp);
	return cnv.f;
}
float read_f64_be(FILE * fp) {
	T_f64_cnv cnv;
	cnv.i = read_u64_be(fp);
	return cnv.f;
}
//Write float little-endian
void write_f32_le(FILE * fp,float val) {
	T_f32_cnv cnv;
	cnv.f = val;
	write_u32_le(fp,cnv.i);
}
void write_f64_le(FILE * fp,double val) {
	T_f64_cnv cnv;
	cnv.f = val;
	write_u64_le(fp,cnv.i);
}
//Write float big-endian
void write_f32_be(FILE * fp,float val) {
	T_f32_cnv cnv;
	cnv.f = val;
	write_u32_be(fp,cnv.i);
}
void write_f64_be(FILE * fp,double val) {
	T_f64_cnv cnv;
	cnv.f = val;
	write_u64_be(fp,cnv.i);
}
