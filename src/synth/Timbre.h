/*
 * Copyright 2013 Xavier Hosxe
 *
 * Author: Xavier Hosxe (xavier <.> hosxe < a t > gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */



#ifndef TIMBRE_H_
#define TIMBRE_H_

#include "Common.h"
#include "Osc.h"
#include "Env.h"
#include "Lfo.h"
#include "LfoOsc.h"
#include "LfoEnv.h"
#include "LfoEnv2.h"
#include "LfoStepSeq.h"
#include "Matrix.h"
#include "core_cmInstr.h"

#define NUMBER_OF_OPERATORS 6

extern float panTable[];


class Timbre {
    friend class Synth;
    friend class Voice;
public:
    Timbre();
    virtual ~Timbre();
    void init();

    void prepareForNextBlock();
    void fxAfterBlock();
    void afterNewParamsLoad();
    void setNewValue(int index, struct ParameterDisplay* param, float newValue);

    void noteOn() {
        for (int k=0; k<NUMBER_OF_LFO; k++) {
            lfo[k]->noteOn();
        }
    }

    void calculateFrequencyWithMatrix(struct OscState* oscState[NUMBER_OF_OPERATORS]) {
        for (int k=0; k<algoInformation[(int)params.engine1.algo].osc; k++) {
            osc[k]->calculateFrequencyWithMatrix(oscState[k]);
        }
    }

    void updateAllModulationIndexes() {
    	int numberOfIMs = algoInformation[(int)(params.engine1.algo)].im;
    	modulationIndex1 = params.engineIm1.modulationIndex1 + matrix.getDestination(INDEX_MODULATION1) + matrix.getDestination(INDEX_ALL_MODULATION);
    	if (unlikely(modulationIndex1 < 0.0f)) {
    		modulationIndex1 = 0.0f;
    	}
        modulationIndex2 = params.engineIm1.modulationIndex2 + matrix.getDestination(INDEX_MODULATION2) + matrix.getDestination(INDEX_ALL_MODULATION);
    	if (unlikely(modulationIndex2 < 0.0f)) {
    		modulationIndex2 = 0.0f;
    	}

        modulationIndex3 = params.engineIm1.modulationIndex3 + matrix.getDestination(INDEX_MODULATION3) + matrix.getDestination(INDEX_ALL_MODULATION);
    	if (unlikely(modulationIndex3 < 0.0f)) {
    		modulationIndex3 = 0.0f;
    	}
        modulationIndex4 = params.engineIm1.modulationIndex4 + matrix.getDestination(INDEX_MODULATION4) + matrix.getDestination(INDEX_ALL_MODULATION);
    	if (unlikely(modulationIndex4 < 0.0f)) {
    		modulationIndex4 = 0.0f;
    	}

        if (likely(numberOfIMs < 4)) {
        	return;
        }

        modulationIndex5 = params.engineIm2.modulationIndex5 + matrix.getDestination(INDEX_ALL_MODULATION);;
    	if (unlikely(modulationIndex5 < 0.0f)) {
    		modulationIndex5 = 0.0f;
    	}
        modulationIndex6 = params.engineIm2.modulationIndex6 + matrix.getDestination(INDEX_ALL_MODULATION);
    	if (unlikely(modulationIndex6 < 0.0f)) {
    		modulationIndex6 = 0.0f;
    	}

    }

    void updateAllMixOscsAndPans() {
    	int numberOfMixes = algoInformation[(int)(params.engine1.algo)].mix;
    	int mix;
    	float inv65535 = .0000152587890625; // 1/ 65535

    	mix1 = params.engineMix1.mixOsc1 + matrix.getDestination(MIX_OSC1) + matrix.getDestination(ALL_MIX);
    	// Optimization to check mix1 is between 0 and 1
    	mix1 = __USAT((int)(mix1 * 65536) , 16) * inv65535;
        float pan1 = params.engineMix1.panOsc1 + matrix.getDestination(PAN_OSC1) + matrix.getDestination(ALL_PAN) + 1.0f;
        // pan1 is between -1 and 1 : Scale from 0.0 to 256
        int pan = __USAT((int)(pan1 * 128), 8);
		pan1Left = panTable[256 - pan];
		pan1Right = panTable[pan];


        mix2 = params.engineMix1.mixOsc2 + matrix.getDestination(MIX_OSC2) + matrix.getDestination(ALL_MIX);
        mix2 = __USAT((int)(mix2 * 65535) , 16) * inv65535;

        float pan2 = params.engineMix1.panOsc2 + matrix.getDestination(PAN_OSC2) + matrix.getDestination(ALL_PAN) + 1.0f;
        pan = __USAT((int)(pan2 * 128), 8);
		pan2Left = panTable[256 - pan];
		pan2Right = panTable[pan];

        // A bit lighter for algo with 1 or 2 mix...
        if (numberOfMixes <=2) {
        	return;
        }

        mix3 = params.engineMix2.mixOsc3 + matrix.getDestination(MIX_OSC3) + matrix.getDestination(ALL_MIX);
        mix3 = __USAT((int)(mix3 * 65535) , 16) * inv65535;

        float pan3 = params.engineMix2.panOsc3 + matrix.getDestination(PAN_OSC3) + matrix.getDestination(ALL_PAN) + 1.0f;
        pan = __USAT((int)(pan3 * 128), 8);
		pan3Left = panTable[256 - pan];
		pan3Right = panTable[pan];

        // No matrix for mix4 and pan4
        mix4 = params.engineMix2.mixOsc4 + matrix.getDestination(ALL_MIX);
        mix4 = __USAT((int)(mix4 * 65535) , 16) * inv65535;
        float pan4 = params.engineMix2.panOsc4 + matrix.getDestination(ALL_PAN) + 1.0f;
        pan = __USAT((int)(pan4 * 128), 8);
		pan4Left = panTable[256 - pan];
		pan4Right = panTable[pan];


        // A bit lighter for algo with 5 or 6 mix...
        if (likely(numberOfMixes <=4)) {
        	return;
        }

        // No more matrix....

        mix5 = params.engineMix3.mixOsc5  + matrix.getDestination(ALL_MIX);
        mix5 = __USAT((int)(mix5 * 65535) , 16) * inv65535;
        float pan5 = params.engineMix3.panOsc5 + matrix.getDestination(ALL_PAN)  + 1.0f;
        pan = __USAT((int)(pan5 * 128), 8);
		pan5Left = panTable[256 - pan];
		pan5Right = panTable[pan];

        mix6 = params.engineMix3.mixOsc6 + matrix.getDestination(ALL_MIX);
        mix6 = __USAT((int)(mix6 * 65535) , 16) * inv65535;
        float pan6 = params.engineMix3.panOsc6 + matrix.getDestination(ALL_PAN) + 1.0f;
        pan = __USAT((int)(pan6 * 128), 8);
		pan6Left = panTable[256 - pan];
		pan6Right = panTable[pan];

    }

    Matrix* getMatrix() {
        return &matrix;
    }

    void midiClockContinue(int songPosition) {
        for (int k=0; k<NUMBER_OF_LFO; k++) {
            lfo[k]->midiClock(songPosition, false);
        }
        this->recomputeNext = ((songPosition&0x1)==0);
    }


    void midiClockStart() {
        for (int k=0; k<NUMBER_OF_LFO; k++) {
            lfo[k]->midiContinue();
        }
        this->recomputeNext = true;
    }

    void midiClockStop() {
    }

    void midiClockSongPositionStep(int songPosition) {
        for (int k=0; k<NUMBER_OF_LFO; k++) {
            lfo[k]->midiClock(songPosition, this->recomputeNext);
        }
        if ((songPosition & 0x1)==0) {
            this->recomputeNext = true;
        }
    }
    uint8_t* getParamRaw() {
        return (uint8_t*)&params;
    }

    float* getSampleBlock() {
        return sampleBlock;
    }

    // optimization
    float modulationIndex1, modulationIndex2, modulationIndex3, modulationIndex4, modulationIndex5, modulationIndex6;
    float mix1, mix2, mix3, mix4, mix5, mix6;
    float pan1Left, pan2Left, pan3Left, pan4Left, pan5Left, pan6Left  ;
    float pan1Right, pan2Right, pan3Right, pan4Right, pan5Right, pan6Right ;

private:
    struct OneSynthParams params;
    Matrix matrix;
    float sampleBlock[BLOCK_SIZE * 2];
    float *sbMax;

    LfoOsc lfoOsc[NUMBER_OF_LFO_OSC];
    LfoEnv lfoEnv[NUMBER_OF_LFO_ENV];
    LfoEnv2 lfoEnv2[NUMBER_OF_LFO_ENV2];
    LfoStepSeq lfoStepSeq[NUMBER_OF_LFO_STEP];
    Lfo* lfo[NUMBER_OF_LFO];

    // 6 oscillators Max
    Osc osc1;
    Osc osc2;
    Osc osc3;
    Osc osc4;
    Osc osc5;
    Osc osc6;
    Osc *osc[NUMBER_OF_OPERATORS];
    // And their 6 envelopes
    Env env1;
    Env env2;
    Env env3;
    Env env4;
    Env env5;
    Env env6;
    Env *env[NUMBER_OF_OPERATORS];

    // Must recompute LFO steps ?
    bool recomputeNext;

    float currentGate;

};

#endif /* TIMBRE_H_ */