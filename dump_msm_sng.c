#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "serialize.h"

//Replace this with your path to amuseconv
#define PATH_TO_AMUSECONV "amuse-2018_09_08-win64\\amuseconv.exe"

int main(int argc, char ** argv) {
	//Open MSM file and get chunk info
	FILE * fp = fopen(argv[1],"rb");
	fseek(fp,0x20,SEEK_SET);
	uint32_t chk2Offs = read_u32_be(fp);
	uint32_t chk2Size = read_u32_be(fp);
	uint32_t chk3Offs = read_u32_be(fp);
	uint32_t chk3Size = read_u32_be(fp);
	fseek(fp,8,SEEK_CUR);
	uint32_t chk5Offs = read_u32_be(fp);
	
	//Extract sequences
	for(int i=1; i<(chk3Size>>4); i++) {
		//Get SNG offset/size
		fseek(fp,chk3Offs+(i<<4),SEEK_SET);
		uint16_t SNGgrpId = read_u16_be(fp);
		fseek(fp,2,SEEK_CUR);
		uint32_t SNGgrpOffs = read_u32_be(fp);
		uint32_t SNGgrpSize = read_u32_be(fp);
		
		fseek(fp,chk2Offs,SEEK_SET);
		uint32_t CMPgrpOffs = -1;
		for(int j=0; j<(chk2Size>>5); j++) {
			uint16_t CMPgrpId = read_u16_be(fp);
			if(CMPgrpId==SNGgrpId) {
				fseek(fp,2,SEEK_CUR);
				CMPgrpOffs = read_u32_be(fp);
				break;
			} else {
				fseek(fp,0x1E,SEEK_CUR);
			}
		}
		if(CMPgrpOffs==-1) continue;
		
		//Dump SNG
		uint8_t * buf = (uint8_t*)malloc(SNGgrpSize);
		fseek(fp,chk5Offs+CMPgrpOffs+0xC,SEEK_SET);
		uint32_t extraOff = read_u32_be(fp);
		fseek(fp,chk5Offs+CMPgrpOffs+SNGgrpOffs+extraOff,SEEK_SET);
		fread(buf,1,SNGgrpSize,fp);
		char fname[0x100];
		snprintf(fname,0x100,"%04X.sng",i);
		FILE * out = fopen(fname,"wb");
		fwrite(buf,1,SNGgrpSize,out);
		fclose(out);
		free(buf);
		
		//Convert to MIDI
		char syscmd[0x100];
		snprintf(syscmd,0x100,PATH_TO_AMUSECONV " %04X.sng %04X.mid gcn",i,i);
		system(syscmd);
	}
	
	//Exit
	return 0;
}
