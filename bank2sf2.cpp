#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "serialize.h"

#include <algorithm>
#include <memory>
#include <vector>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include "sf2cute.hpp"

//Uncomment the following defines to enable the use of a common samples directory
//(this might be necessary if some samples are being referenced from other preloaded groups)
//#define USE_COMMON_SAMPLES_DIR 1
//#define COMMON_SAMPLES_DIR "ALL_SAMPLES"

using namespace sf2cute;

FILE * fpPool;
FILE * fpProj;
SoundFont sf2;
uint32_t macroOffs;
uint32_t tableOffs;
uint32_t keymapOffs;
uint32_t layerOffs;
char samplesDir[0x100];
std::vector<int16_t> sampleData[0x4000];
uint16_t sampleLoopModes[0x4000];
std::shared_ptr<SFSample> usedSamples[0x4000];
std::shared_ptr<SFInstrument> usedMacros[0x4000];

//Milliseconds to timecent units (attack/decay/release time)
int32_t ms_to_tc(uint32_t ms) {
	if(ms<=1) return -12000;
	double msd = (double)ms;
	int32_t tc = (int32_t)round(4000.0*log10(msd/1000.0));
	if(tc>8000) tc = 8000;
	return tc;
}
//Timecent to timecent units (attack/decay/release time)
int32_t tc_to_tc(int32_t tc) {
	double tcd = (double)tc;
	double secs = exp2(tcd/(1200.0*65536.0));
	return ms_to_tc((int32_t)round(1000.0*secs));
}
//Percent to centibel units (sustain level)
int32_t pc_to_cb(int32_t pc) {
	if(pc==0) return 1440;
	double pcd = (double)pc;
	int32_t cb = (int32_t)round(-100.0*log10(pcd/4096.0));
	return cb;
}
//Percent to centibel units (volume)
int32_t pc8_to_cb(int32_t pc) {
	if(pc==0) return 1440;
	double pcd = (double)pc;
	int32_t cb = (int32_t)round(-100.0*log10(pcd/127.0));
	return cb;
}
//Pan range convert (0-127)->(-500,500)
int32_t pan_range(uint32_t pan) {
	//Clamp center to 0
	if(pan==64) return 0;
	//Map range (left)
	if(pan<64) return ((pan-64)*500)/64;
	//Map range (right)
	if(pan>64) return ((pan-64)*500)/63;
	//Default
	return 0;
}

//Add soundmacro as SF2 instrument (by ID)
void macro_to_instrument(uint16_t id) {
	//If this ID has already been added, don't add it again
	if(usedMacros[id]!=nullptr) return;
	//Add macro
	fseek(fpPool,macroOffs,SEEK_SET);
	//Find this macro
	while(true) {
		uint32_t macroSize = read_u32_be(fpPool);
		uint16_t macroID = read_u16_be(fpPool);
		fseek(fpPool,2,SEEK_CUR);
		//Found it!
		if(id==macroID) {
			uint16_t sampleID = 0;
			int8_t tuneCoarse = 0;
			int8_t tuneFine = 0;
			int32_t attack = -12000;
			int32_t decay = -12000;
			int32_t sustain = 0;
			int32_t release = -12000;
			//int32_t modVelAtk = 0;
			//int32_t modKeyDec = 0;
			//Process macro commands
			while(true) {
				uint8_t param1 = read_u8_be(fpPool);
				uint8_t param2 = read_u8_be(fpPool);
				uint8_t param3 = read_u8_be(fpPool);
				uint8_t cmd = read_u8_be(fpPool);
				uint8_t param4 = read_u8_be(fpPool);
				uint8_t param5 = read_u8_be(fpPool);
				uint8_t param6 = read_u8_be(fpPool);
				uint8_t param7 = read_u8_be(fpPool);
				//Command 0x00 is end
				if(cmd==0x00) break;
				//Command 0x0C is set ADSR
				if(cmd==0x0C) {
					uint16_t adsrID = (param2<<8)|param3;
					uint32_t saveOffs = ftell(fpPool);
					fseek(fpPool,tableOffs,SEEK_SET);
					//Find this table
					while(true) {
						uint32_t tableSize = read_u32_be(fpPool);
						uint16_t tableID = read_u16_be(fpPool);
						fseek(fpPool,2,SEEK_CUR);
						//Found it!
						if(adsrID==tableID) {
							if(param1) {
								//DLS mode
								attack = tc_to_tc(read_s32_le(fpPool));
								decay = tc_to_tc(read_s32_le(fpPool));
								sustain = pc_to_cb(read_u16_le(fpPool));
								release = ms_to_tc(read_u16_le(fpPool));
								//TODO: handle modVelAtk
								//TODO: handle modKeyDec
							} else {
								//Normal mode
								attack = ms_to_tc(read_u16_le(fpPool));
								decay = ms_to_tc(read_u16_le(fpPool));
								sustain = pc_to_cb(read_u16_le(fpPool));
								release = ms_to_tc(read_u16_le(fpPool));
							}
							break;
						}
						//Check next
						else fseek(fpPool,tableSize-8,SEEK_CUR);
					}
					fseek(fpPool,saveOffs,SEEK_SET);
				}
				//Command 0x10 is set sample
				if(cmd==0x10) {
					sampleID = (param2<<8)|param3;
				}
				//Command 0x18 is set tuning
				if(cmd==0x18) {
					tuneCoarse = param3;
					tuneFine = param2;
				}
			}
			//If this sample ID has already been added, don't add it again
			if(usedSamples[sampleID]==nullptr) {
				//Process sample .wav file
				char strBuf[0x100];
				snprintf(strBuf,0x100,"%s/sample%04X.wav",samplesDir,sampleID);
				FILE * fpWave = fopen(strBuf,"rb");
				
				fseek(fpWave,0x4,SEEK_CUR);
				int32_t waveSize = read_u32_le(fpWave)-4;
				fseek(fpWave,0x4,SEEK_CUR);
				
				//std::vector<int16_t> sampleData;
				uint32_t sampleRate = 32000;
				uint32_t loopStart = 0;
				uint32_t loopEnd = 0;
				uint8_t rootKey = 60;
				uint8_t detune = 0;
				sampleLoopModes[sampleID] = (uint16_t)SampleMode::kNoLoop;
				while(waveSize>0) {
					uint32_t fourcc = read_u32_be(fpWave);
					int32_t chkSize = read_u32_le(fpWave);
					//"fmt "
					if(fourcc==0x666D7420) {
						uint16_t wFormatTag = read_u16_le(fpWave);
						uint16_t wChannels = read_u16_le(fpWave);
						uint32_t dwSamplesPerSec = read_u32_le(fpWave);
						uint32_t dwAvgBytesPerSec = read_u32_le(fpWave);
						uint16_t wBlockAlign = read_u16_le(fpWave);
						uint16_t wBitsPerSample = read_u16_le(fpWave);
						//uint16_t cbSize = read_u16_le(fpWave);
						//fseek(fpWave,cbSize,SEEK_CUR);
						sampleRate = dwSamplesPerSec;
					}
					//"smpl"
					else if(fourcc==0x736D706C) {
						uint32_t dwManufacturer = read_u32_le(fpWave);
						uint32_t dwProduct = read_u32_le(fpWave);
						uint32_t dwSamplePeriod = read_u32_le(fpWave);
						uint32_t dwMIDIUnityNote = read_u32_le(fpWave);
						uint32_t dwMIDIPitchFraction = read_u32_le(fpWave);
						uint32_t dwSMPTEFormat = read_u32_le(fpWave);
						uint32_t dwSMPTEOffset = read_u32_le(fpWave);
						uint32_t cSampleLoops = read_u32_le(fpWave);
						uint32_t cbSamplerData = read_u32_le(fpWave);
						for(int i=0; i<cSampleLoops; i++) {
							uint32_t dwIdentifier = read_u32_le(fpWave);
							uint32_t dwType = read_u32_le(fpWave);
							uint32_t dwStart = read_u32_le(fpWave);
							uint32_t dwEnd = read_u32_le(fpWave);
							uint32_t dwFraction = read_u32_le(fpWave);
							uint32_t dwPlayCount = read_u32_le(fpWave);
							loopStart = dwStart;
							loopEnd = dwEnd+1;
							sampleLoopModes[sampleID] = (uint16_t)SampleMode::kLoopContinuously;
						}
						fseek(fpWave,cbSamplerData,SEEK_CUR);
						rootKey = dwMIDIUnityNote;
						detune = (dwMIDIPitchFraction*100)/0x100;
					}
					//"data"
					else if(fourcc==0x64617461) {
						sampleData[sampleID].resize(chkSize/2);
						fread(&sampleData[sampleID][0],sizeof(int16_t),chkSize/2,fpWave);
					}
					//anything else
					else {
						fseek(fpWave,chkSize,SEEK_CUR);
					}
					waveSize -= (chkSize+8);
				}
				fclose(fpWave);
				//Add sample
				snprintf(strBuf,0x100,"sample%04X",sampleID);
				std::shared_ptr<SFSample> sample = sf2.NewSample(strBuf,
					sampleData[sampleID],
					loopStart,loopEnd,
					sampleRate,
					rootKey,detune);
				//Set ID as having been processed
				usedSamples[sampleID] = sample;
			}
			//Add instrument
			char strBuf[0x100];
			snprintf(strBuf,0x100,"macro%04X",id);
			SFInstrumentZone zone(usedSamples[sampleID],
				std::vector<SFGeneratorItem>{
					SFGeneratorItem(SFGenerator::kSampleModes,sampleLoopModes[sampleID]),
					SFGeneratorItem(SFGenerator::kCoarseTune,tuneCoarse),
					SFGeneratorItem(SFGenerator::kFineTune,tuneFine),
					SFGeneratorItem(SFGenerator::kAttackVolEnv,attack),
					SFGeneratorItem(SFGenerator::kDecayVolEnv,decay),
					SFGeneratorItem(SFGenerator::kSustainVolEnv,sustain),
					SFGeneratorItem(SFGenerator::kReleaseVolEnv,release)},
				std::vector<SFModulatorItem>{});
			std::shared_ptr<SFInstrument> instrument = sf2.NewInstrument(strBuf,
				std::vector<SFInstrumentZone>{zone});
			//Set ID as having been processed
			usedMacros[id] = instrument;
			//Exit
			return;
		}
		//Check next
		else fseek(fpPool,macroSize-8,SEEK_CUR);
	}
}

//MAIN PROGRAM
int main(int argc, char ** argv) {
	//Open bank files
	char fname[0x100];
	snprintf(fname,0x100,"%s.pool",argv[1]);
	fpPool = fopen(fname,"rb");
	snprintf(fname,0x100,"%s.proj",argv[1]);
	fpProj = fopen(fname,"rb");
#ifdef USE_COMMON_SAMPLES_DIR
	strncpy(samplesDir,COMMON_SAMPLES_DIR,0x100);
#else
	strncpy(samplesDir,argv[1],0x100);
#endif
	
	//Setup output SF2
	sf2.set_sound_engine("EMU8000");
	sf2.set_bank_name(argv[1]);
	
	//Clear pointer buffer
	for(int i=0; i<0x4000; i++) {
		usedSamples[i] = nullptr;
		usedMacros[i] = nullptr;
	}
	
	//Get offsets
	macroOffs = read_u32_be(fpPool);
	tableOffs = read_u32_be(fpPool);
	keymapOffs = read_u32_be(fpPool);
	layerOffs = read_u32_be(fpPool);
	fseek(fpProj,0x1C,SEEK_SET);
	uint32_t pageNormOffs = read_u32_be(fpProj);
	uint32_t pageDrumOffs = read_u32_be(fpProj);
	
	//Parse files
	for(int i=0; i<2; i++) {
		//Loop through normal page
		if(i==0) fseek(fpProj,pageNormOffs,SEEK_SET);
		//Loop through drum page
		if(i==1) fseek(fpProj,pageDrumOffs,SEEK_SET);
		
		//Main loop
		while(true) {
			uint16_t objectID = read_u16_be(fpProj);
			fseek(fpProj,2,SEEK_CUR);
			uint8_t programNo = read_u8_be(fpProj);
			fseek(fpProj,1,SEEK_CUR);
			if(objectID==0xFFFF) break;
			
			//Layer
			if(objectID&0x8000) {
				fseek(fpPool,layerOffs,SEEK_SET);
				//Find this layer
				while(true) {
					uint32_t layerSize = read_u32_be(fpPool);
					uint16_t layerID = read_u16_be(fpPool);
					fseek(fpPool,2,SEEK_CUR);
					//Found it!
					if(objectID==layerID) {
						//Init instrument
						std::vector<SFPresetZone> zones;
						//Setup samples
						uint32_t numRegions = read_u32_be(fpPool);
						for(int j=0; j<numRegions; j++) {
							uint16_t objectID2 = read_u16_be(fpPool);
							uint8_t keyLo = read_u8_be(fpPool);
							uint8_t keyHi = read_u8_be(fpPool);
							int8_t transpose = read_s8_be(fpPool);
							uint8_t volume = read_u8_be(fpPool);
							fseek(fpPool,2,SEEK_CUR);
							uint8_t pan = read_u8_be(fpPool);
							fseek(fpPool,3,SEEK_CUR);
							uint32_t saveOffs = ftell(fpPool);
							//Object ID could be anything (?)
							//In practice it's always a macro though
							macro_to_instrument(objectID2);
							SFPresetZone thiszone(usedMacros[objectID2],
								std::vector<SFGeneratorItem>{
									SFGeneratorItem(SFGenerator::kKeyRange,RangesType(keyLo,keyHi)),
									SFGeneratorItem(SFGenerator::kCoarseTune,transpose),
									SFGeneratorItem(SFGenerator::kInitialAttenuation,pc8_to_cb(volume)),
									SFGeneratorItem(SFGenerator::kPan,pan_range(pan))},
								std::vector<SFModulatorItem>{});
							zones.push_back(thiszone);
							fseek(fpPool,saveOffs,SEEK_SET);
						}
						//Add instrument
						char strBuf[0x100];
						snprintf(strBuf,0x100,"layer%04X",layerID);
						std::shared_ptr<SFPreset> preset = sf2.NewPreset(strBuf,programNo,i?128:0,zones);
						break;
					}
					//Check next
					else fseek(fpPool,layerSize-8,SEEK_CUR);
				}
			}
			//Keymap
			else if(objectID&0x4000) {
				fseek(fpPool,keymapOffs,SEEK_SET);
				//Find this keymap
				while(true) {
					uint32_t keymapSize = read_u32_be(fpPool);
					uint16_t keymapID = read_u16_be(fpPool);
					fseek(fpPool,2,SEEK_CUR);
					//Found it!
					if(objectID==keymapID) {
						//Init instrument
						std::vector<SFPresetZone> zones;
						//Setup samples
						for(int j=0; j<128; j++) {
							uint16_t objectID2 = read_u16_be(fpPool);
							int8_t transpose = read_s8_be(fpPool);
							uint8_t pan = read_u8_be(fpPool);
							fseek(fpPool,4,SEEK_CUR);
							uint32_t saveOffs = ftell(fpPool);
							//ID 0xFFFF means no use
							if(objectID2==0xFFFF) continue;
							//Object ID could be anything (?)
							//In practice it's always a macro though
							macro_to_instrument(objectID2);
							SFPresetZone thiszone(usedMacros[objectID2],
								std::vector<SFGeneratorItem>{
									SFGeneratorItem(SFGenerator::kKeyRange,RangesType(j,j)),
									SFGeneratorItem(SFGenerator::kCoarseTune,transpose),
									SFGeneratorItem(SFGenerator::kPan,pan_range(pan))},
								std::vector<SFModulatorItem>{});
							zones.push_back(thiszone);
							fseek(fpPool,saveOffs,SEEK_SET);
						}
						//Add instrument
						char strBuf[0x100];
						snprintf(strBuf,0x100,"keymap%04X",keymapID);
						std::shared_ptr<SFPreset> preset = sf2.NewPreset(strBuf,programNo,i?128:0,zones);
						break;
					}
					//Check next
					else fseek(fpPool,keymapSize-8,SEEK_CUR);
				}
			}
			//Macro
			else {
				//Init instrument
				std::vector<SFPresetZone> zones;
				//Setup macro
				macro_to_instrument(objectID);
				SFPresetZone zone(usedMacros[objectID],
					std::vector<SFGeneratorItem>{},
					std::vector<SFModulatorItem>{});
				//Add instrument
				char strBuf[0x100];
				snprintf(strBuf,0x100,"macro%04X",objectID);
				std::shared_ptr<SFPreset> preset = sf2.NewPreset(strBuf,programNo,i?128:0,std::vector<SFPresetZone>{zone});
			}
		}
	}
	
	//Output SF2 file
	snprintf(fname,0x100,"%s.sf2",argv[1]);
	fclose(fpPool);
	fclose(fpProj);
	std::ofstream ofs(fname,std::ios::binary);
	sf2.Write(ofs);
	
	//Exit
	return 0;
}
