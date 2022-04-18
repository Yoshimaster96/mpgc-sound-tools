#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "serialize.h"

int main(int argc, char ** argv) {
	//Open MSM file and get chunk info
	FILE * fp = fopen("mpgcsnd.msm","rb");
	fseek(fp,0x20,SEEK_SET);
	uint32_t chk2Offs = read_u32_be(fp);
	uint32_t chk2Size = read_u32_be(fp);
	fseek(fp,0x10,SEEK_CUR);
	uint32_t chk5Offs = read_u32_be(fp);
	uint32_t chk5Size = read_u32_be(fp);
	uint32_t chk6Offs = read_u32_be(fp);
	uint32_t chk6Size = read_u32_be(fp);
	
	//Extract banks
	for(int i=1; i<(chk2Size>>5); i++) {
		//Get offset/size data
		fseek(fp,chk2Offs+(i<<5),SEEK_SET);
		uint16_t groupId = read_u16_be(fp);
		fseek(fp,2,SEEK_CUR);
		uint32_t groupDataOffs = read_u32_be(fp);
		uint32_t groupDataSize = read_u32_be(fp);
		uint32_t sampOffs = read_u32_be(fp);
		uint32_t sampSize = read_u32_be(fp);
		groupDataOffs += chk5Offs;
		sampOffs += chk6Offs;
		
		fseek(fp,groupDataOffs,SEEK_SET);
		uint32_t poolOffs = read_u32_be(fp);
		uint32_t projOffs = read_u32_be(fp);
		uint32_t sdirOffs = read_u32_be(fp);
		uint32_t SNGOffs = read_u32_be(fp);
		uint32_t poolSize = projOffs-poolOffs;
		uint32_t projSize = sdirOffs-projOffs;
		uint32_t sdirSize = SNGOffs-sdirOffs;
		poolOffs += groupDataOffs;
		projOffs += groupDataOffs;
		sdirOffs += groupDataOffs;
		
		uint8_t * buf;
		char fname[0x100];
		FILE * out;
		
		//Dump .pool
		buf = (uint8_t*)malloc(poolSize);
		fseek(fp,poolOffs,SEEK_SET);
		fread(buf,1,poolSize,fp);
		snprintf(fname,0x100,"%04X.pool",groupId);
		out = fopen(fname,"wb");
		fwrite(buf,1,poolSize,out);
		fclose(out);
		free(buf);
		
		//Dump .proj
		buf = (uint8_t*)malloc(projSize);
		fseek(fp,projOffs,SEEK_SET);
		fread(buf,1,projSize,fp);
		snprintf(fname,0x100,"%04X.proj",groupId);
		out = fopen(fname,"wb");
		fwrite(buf,1,projSize,out);
		fclose(out);
		free(buf);
		
		//Dump .sdir
		buf = (uint8_t*)malloc(sdirOffs);
		fseek(fp,sdirOffs,SEEK_SET);
		fread(buf,1,sdirOffs,fp);
		snprintf(fname,0x100,"%04X.sdir",groupId);
		out = fopen(fname,"wb");
		fwrite(buf,1,sdirOffs,out);
		fclose(out);
		free(buf);
		
		//Dump .samp
		buf = (uint8_t*)malloc(sampSize);
		fseek(fp,sampOffs,SEEK_SET);
		fread(buf,1,sampSize,fp);
		snprintf(fname,0x100,"%04X.samp",groupId);
		out = fopen(fname,"wb");
		fwrite(buf,1,sampSize,out);
		fclose(out);
		free(buf);
	}
	
	//Exit
	return 0;
}
