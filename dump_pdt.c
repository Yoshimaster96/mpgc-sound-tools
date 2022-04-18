#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "serialize.h"

uint32_t nibblesToSamples(uint32_t nibbles) {
	uint32_t nibInt = nibbles>>4;
	uint32_t nibFrac = nibbles&0xF;
	
	if(nibFrac>0) return (nibInt*14)+nibFrac-2;
	else return nibInt*14;
}

int main(int argc, char ** argv) {
	FILE * fp = fopen(argv[1],"rb");
	
	uint16_t unk00		= read_u16_be(fp);
	uint16_t numFiles	= read_u16_be(fp);
	uint32_t unk04		= read_u32_be(fp);
	uint32_t unk08		= read_u32_be(fp); //sample rate?
	uint32_t unk0C		= read_u32_be(fp); //channel count?
	uint32_t entryOffs	= read_u32_be(fp);
	uint32_t coeffOffs	= read_u32_be(fp);
	uint32_t headerOffs	= read_u32_be(fp);
	uint32_t streamOffs	= read_u32_be(fp);
	
	for(int i=0; i<numFiles; i++) {
		//Decode
		fseek(fp,entryOffs+(i<<2),SEEK_SET);
		
		uint32_t thisHeaderOffs		= read_u32_be(fp);
		if(thisHeaderOffs==0) continue;
		fseek(fp,thisHeaderOffs,SEEK_SET);
		
		uint32_t flags		= read_u32_be(fp);
		uint32_t sampleRate	= read_u32_be(fp);
		uint32_t nibbleCount	= read_u32_be(fp);
		uint32_t loopStart	= read_u32_be(fp);
		uint32_t ch1Start	= read_u32_be(fp);
		uint16_t ch1CoefEntry	= read_u16_be(fp);
		uint16_t unk116		= read_u16_be(fp);
		uint32_t ch1CoefOffs	= coeffOffs+(ch1CoefEntry<<5);
		
		uint32_t ch2Start	= 0;
		uint16_t ch2CoefEntry	= 0;
		uint32_t ch2CoefOffs	= 0;
		int chanCount = 1;
		
		if(flags&0x01000000) {
			ch2Start		= read_u32_be(fp);
			ch2CoefEntry		= read_u16_be(fp);
			uint16_t unk11A		= read_u16_be(fp);
			ch2CoefOffs		= coeffOffs+(ch2CoefEntry<<5);
			chanCount = 2;
		}
		
		int loopFlag = 0;
		if(flags&0x02000000) loopFlag = 1;
		
		char fname[0x100];
		
		//Write left channel
		snprintf(fname,0x100,"%04X_L.dsp",i);
		FILE * out = fopen(fname,"wb");
		
		write_u32_be(out,nibblesToSamples(nibbleCount));
		write_u32_be(out,nibbleCount);
		write_u32_be(out,sampleRate);
		write_u16_be(out,loopFlag);
		write_u16_be(out,0);
		write_u32_be(out,loopStart);
		write_u32_be(out,nibbleCount-1);
		write_u32_be(out,0);
		
		fseek(fp,ch1CoefOffs,SEEK_SET);
		for(int j=0; j<16; j++) {
			write_u16_be(out,read_u16_be(fp));
		}
		
		write_u16_be(out,0);
		fseek(fp,ch1Start,SEEK_SET);
		write_u16_be(out,read_u8_be(fp));
		write_u16_be(out,0);
		write_u16_be(out,0);
		write_u16_be(out,0);
		write_u16_be(out,0);
		write_u16_be(out,0);
		
		for(int j=0; j<11; j++) {
			write_u16_be(out,0);
		}
		
		fseek(fp,ch1Start,SEEK_SET);
		for(int j=0; j<(nibbleCount<<1); j++) {
			putc(getc(fp),out);
		}
		
		fclose(out);
		
		//Write right channel
		snprintf(fname,0x100,"%04X_R.dsp",i);
		out = fopen(fname,"wb");
		
		write_u32_be(out,nibblesToSamples(nibbleCount));
		write_u32_be(out,nibbleCount);
		write_u32_be(out,sampleRate);
		write_u16_be(out,loopFlag);
		write_u16_be(out,0);
		write_u32_be(out,loopStart);
		write_u32_be(out,nibbleCount-1);
		write_u32_be(out,0);
		
		fseek(fp,ch2CoefOffs,SEEK_SET);
		for(int j=0; j<16; j++) {
			write_u16_be(out,read_u16_be(fp));
		}
		
		write_u16_be(out,0);
		fseek(fp,ch2Start,SEEK_SET);
		write_u16_be(out,read_u8_be(fp));
		write_u16_be(out,0);
		write_u16_be(out,0);
		write_u16_be(out,0);
		write_u16_be(out,0);
		write_u16_be(out,0);
		
		for(int j=0; j<11; j++) {
			write_u16_be(out,0);
		}
		
		fseek(fp,ch2Start,SEEK_SET);
		for(int j=0; j<(nibbleCount<<1); j++) {
			putc(getc(fp),out);
		}
		
		fclose(out);
	}
	
	fclose(fp);
	return 0;
}
