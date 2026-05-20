/*******************************************************************************************************************
Copyright (c) 2023 Cycling '74

The code that Max generates automatically and that end users are capable of
exporting and using, and any associated documentation files (the “Software”)
is a work of authorship for which Cycling '74 is the author and owner for
copyright purposes.

This Software is dual-licensed either under the terms of the Cycling '74
License for Max-Generated Code for Export, or alternatively under the terms
of the General Public License (GPL) Version 3. You may use the Software
according to either of these licenses as it is most appropriate for your
project on a case-by-case basis (proprietary or not).

A) Cycling '74 License for Max-Generated Code for Export

A license is hereby granted, free of charge, to any person obtaining a copy
of the Software (“Licensee”) to use, copy, modify, merge, publish, and
distribute copies of the Software, and to permit persons to whom the Software
is furnished to do so, subject to the following conditions:

The Software is licensed to Licensee for all uses that do not include the sale,
sublicensing, or commercial distribution of software that incorporates this
source code. This means that the Licensee is free to use this software for
educational, research, and prototyping purposes, to create musical or other
creative works with software that incorporates this source code, or any other
use that does not constitute selling software that makes use of this source
code. Commercial distribution also includes the packaging of free software with
other paid software, hardware, or software-provided commercial services.

For entities with UNDER $200k in annual revenue or funding, a license is hereby
granted, free of charge, for the sale, sublicensing, or commercial distribution
of software that incorporates this source code, for as long as the entity's
annual revenue remains below $200k annual revenue or funding.

For entities with OVER $200k in annual revenue or funding interested in the
sale, sublicensing, or commercial distribution of software that incorporates
this source code, please send inquiries to licensing@cycling74.com.

The above copyright notice and this license shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Please see
https://support.cycling74.com/hc/en-us/articles/10730637742483-RNBO-Export-Licensing-FAQ
for additional information

B) General Public License Version 3 (GPLv3)
Details of the GPLv3 license can be found at: https://www.gnu.org/licenses/gpl-3.0.html
*******************************************************************************************************************/

#ifdef RNBO_LIB_PREFIX
#define STR_IMPL(A) #A
#define STR(A) STR_IMPL(A)
#define RNBO_LIB_INCLUDE(X) STR(RNBO_LIB_PREFIX/X)
#else
#define RNBO_LIB_INCLUDE(X) #X
#endif // RNBO_LIB_PREFIX
#ifdef RNBO_INJECTPLATFORM
#define RNBO_USECUSTOMPLATFORM
#include RNBO_INJECTPLATFORM
#endif // RNBO_INJECTPLATFORM

#include RNBO_LIB_INCLUDE(RNBO_Common.h)
#include RNBO_LIB_INCLUDE(RNBO_AudioSignal.h)

namespace RNBO {


#define trunc(x) ((Int)(x))
#define autoref auto&

#if defined(__GNUC__) || defined(__clang__)
    #define RNBO_RESTRICT __restrict__
#elif defined(_MSC_VER)
    #define RNBO_RESTRICT __restrict
#endif

#define FIXEDSIZEARRAYINIT(...) { }

template <class ENGINE = INTERNALENGINE> class rnbomatic : public PatcherInterfaceImpl {

friend class EngineCore;
friend class Engine;
friend class MinimalEngine<>;
public:

rnbomatic()
: _internalEngine(this)
{
}

~rnbomatic()
{
    deallocateSignals();
}

Index getNumMidiInputPorts() const {
    return 0;
}

void processMidiEvent(MillisecondTime , int , ConstByteArray , Index ) {}

Index getNumMidiOutputPorts() const {
    return 0;
}

void process(
    const SampleValue * const* inputs,
    Index numInputs,
    SampleValue * const* outputs,
    Index numOutputs,
    Index n
) {
    RNBO_UNUSED(numInputs);
    RNBO_UNUSED(inputs);
    this->vs = n;
    this->updateTime(this->getEngine()->getCurrentTime(), (ENGINE*)nullptr, true);
    SampleValue * out1 = (numOutputs >= 1 && outputs[0] ? outputs[0] : this->dummyBuffer);

    this->cycle_tilde_01_perform(
        this->cycle_tilde_01_frequency,
        this->cycle_tilde_01_phase_offset,
        this->signals[0],
        this->dummyBuffer,
        n
    );

    this->cycle_tilde_02_perform(
        this->cycle_tilde_02_frequency,
        this->cycle_tilde_02_phase_offset,
        this->signals[1],
        this->dummyBuffer,
        n
    );

    this->cycle_tilde_03_perform(
        this->cycle_tilde_03_frequency,
        this->cycle_tilde_03_phase_offset,
        this->signals[2],
        this->dummyBuffer,
        n
    );

    this->signaladder_01_perform(this->signals[2], this->signals[0], this->signals[1], this->signals[1], n);
    this->dspexpr_01_perform(this->signals[1], this->dspexpr_01_in2, out1, n);
    this->stackprotect_perform(n);
    this->globaltransport_advance();
    this->advanceTime((ENGINE*)nullptr);
    this->audioProcessSampleCount += this->vs;
}

void prepareToProcess(number sampleRate, Index maxBlockSize, bool force) {
    RNBO_ASSERT(this->_isInitialized);

    if (this->maxvs < maxBlockSize || !this->didAllocateSignals) {
        Index i;

        for (i = 0; i < 3; i++) {
            this->signals[i] = resizeSignal(this->signals[i], this->maxvs, maxBlockSize);
        }

        this->globaltransport_tempo = resizeSignal(this->globaltransport_tempo, this->maxvs, maxBlockSize);
        this->globaltransport_state = resizeSignal(this->globaltransport_state, this->maxvs, maxBlockSize);
        this->zeroBuffer = resizeSignal(this->zeroBuffer, this->maxvs, maxBlockSize);
        this->dummyBuffer = resizeSignal(this->dummyBuffer, this->maxvs, maxBlockSize);
        this->didAllocateSignals = true;
    }

    const bool sampleRateChanged = sampleRate != this->sr;
    const bool maxvsChanged = maxBlockSize != this->maxvs;
    const bool forceDSPSetup = sampleRateChanged || maxvsChanged || force;

    if (sampleRateChanged || maxvsChanged) {
        this->vs = maxBlockSize;
        this->maxvs = maxBlockSize;
        this->sr = sampleRate;
        this->invsr = 1 / sampleRate;
    }

    this->cycle_tilde_01_dspsetup(forceDSPSetup);
    this->cycle_tilde_02_dspsetup(forceDSPSetup);
    this->cycle_tilde_03_dspsetup(forceDSPSetup);
    this->globaltransport_dspsetup(forceDSPSetup);

    if (sampleRateChanged)
        this->onSampleRateChanged(sampleRate);
}

number msToSamps(MillisecondTime ms, number sampleRate) {
    return ms * sampleRate * 0.001;
}

MillisecondTime sampsToMs(SampleIndex samps) {
    return samps * (this->invsr * 1000);
}

Index getNumInputChannels() const {
    return 0;
}

Index getNumOutputChannels() const {
    return 1;
}

DataRef* getDataRef(DataRefIndex index)  {
    switch (index) {
    case 0:
        {
        return addressOf(this->RNBODefaultSinus);
        break;
        }
    case 1:
        {
        return addressOf(this->RNBODefaultMtofLookupTable256);
        break;
        }
    default:
        {
        return nullptr;
        }
    }
}

DataRefIndex getNumDataRefs() const {
    return 2;
}

void processDataViewUpdate(DataRefIndex index, MillisecondTime time) {
    this->updateTime(time, (ENGINE*)nullptr);

    if (index == 0) {
        this->cycle_tilde_01_buffer = reInitDataView(this->cycle_tilde_01_buffer, this->RNBODefaultSinus);
        this->cycle_tilde_02_buffer = reInitDataView(this->cycle_tilde_02_buffer, this->RNBODefaultSinus);
        this->cycle_tilde_03_buffer = reInitDataView(this->cycle_tilde_03_buffer, this->RNBODefaultSinus);
        this->cycle_tilde_01_bufferUpdated();
        this->cycle_tilde_02_bufferUpdated();
        this->cycle_tilde_03_bufferUpdated();
    }

    if (index == 1) {
        this->mtof_01_innerMtoF_buffer = reInitDataView(this->mtof_01_innerMtoF_buffer, this->RNBODefaultMtofLookupTable256);
        this->mtof_02_innerMtoF_buffer = reInitDataView(this->mtof_02_innerMtoF_buffer, this->RNBODefaultMtofLookupTable256);
        this->mtof_03_innerMtoF_buffer = reInitDataView(this->mtof_03_innerMtoF_buffer, this->RNBODefaultMtofLookupTable256);
    }
}

void initialize() {
    RNBO_ASSERT(!this->_isInitialized);

    this->RNBODefaultSinus = initDataRef(
        this->RNBODefaultSinus,
        this->dataRefStrings->name0,
        true,
        this->dataRefStrings->file0,
        this->dataRefStrings->tag0
    );

    this->RNBODefaultMtofLookupTable256 = initDataRef(
        this->RNBODefaultMtofLookupTable256,
        this->dataRefStrings->name1,
        true,
        this->dataRefStrings->file1,
        this->dataRefStrings->tag1
    );

    this->assign_defaults();
    this->applyState();
    this->RNBODefaultSinus->setIndex(0);
    this->cycle_tilde_01_buffer = new SampleBuffer(this->RNBODefaultSinus);
    this->cycle_tilde_02_buffer = new SampleBuffer(this->RNBODefaultSinus);
    this->cycle_tilde_03_buffer = new SampleBuffer(this->RNBODefaultSinus);
    this->RNBODefaultMtofLookupTable256->setIndex(1);
    this->mtof_01_innerMtoF_buffer = new SampleBuffer(this->RNBODefaultMtofLookupTable256);
    this->mtof_02_innerMtoF_buffer = new SampleBuffer(this->RNBODefaultMtofLookupTable256);
    this->mtof_03_innerMtoF_buffer = new SampleBuffer(this->RNBODefaultMtofLookupTable256);
    this->initializeObjects();
    this->allocateDataRefs();
    this->startup();
    this->_isInitialized = true;
}

void getPreset(PatcherStateInterface& preset) {
    this->updateTime(this->getEngine()->getCurrentTime(), (ENGINE*)nullptr);
    preset["__presetid"] = "rnbo";
    this->param_01_getPresetValue(getSubState(preset, "CM"));
    this->param_02_getPresetValue(getSubState(preset, "Cm"));
    this->param_03_getPresetValue(getSubState(preset, "DfM"));
    this->param_04_getPresetValue(getSubState(preset, "Dfm"));
    this->param_05_getPresetValue(getSubState(preset, "DM"));
    this->param_06_getPresetValue(getSubState(preset, "Dm"));
    this->param_07_getPresetValue(getSubState(preset, "EfM"));
    this->param_08_getPresetValue(getSubState(preset, "Efm"));
    this->param_09_getPresetValue(getSubState(preset, "EM"));
    this->param_10_getPresetValue(getSubState(preset, "Em"));
    this->param_11_getPresetValue(getSubState(preset, "FM"));
    this->param_12_getPresetValue(getSubState(preset, "Fm"));
    this->param_13_getPresetValue(getSubState(preset, "GfM"));
    this->param_14_getPresetValue(getSubState(preset, "Gfm"));
    this->param_15_getPresetValue(getSubState(preset, "GM"));
    this->param_16_getPresetValue(getSubState(preset, "Gm"));
    this->param_17_getPresetValue(getSubState(preset, "AfM"));
    this->param_18_getPresetValue(getSubState(preset, "Afm"));
    this->param_19_getPresetValue(getSubState(preset, "AM"));
    this->param_20_getPresetValue(getSubState(preset, "Am"));
    this->param_21_getPresetValue(getSubState(preset, "BfM"));
    this->param_22_getPresetValue(getSubState(preset, "Bfm"));
    this->param_23_getPresetValue(getSubState(preset, "BM"));
    this->param_24_getPresetValue(getSubState(preset, "Bm"));
}

void setPreset(MillisecondTime time, PatcherStateInterface& preset) {
    this->updateTime(time, (ENGINE*)nullptr);
    this->param_01_setPresetValue(getSubState(preset, "CM"));
    this->param_02_setPresetValue(getSubState(preset, "Cm"));
    this->param_03_setPresetValue(getSubState(preset, "DfM"));
    this->param_04_setPresetValue(getSubState(preset, "Dfm"));
    this->param_05_setPresetValue(getSubState(preset, "DM"));
    this->param_06_setPresetValue(getSubState(preset, "Dm"));
    this->param_07_setPresetValue(getSubState(preset, "EfM"));
    this->param_08_setPresetValue(getSubState(preset, "Efm"));
    this->param_09_setPresetValue(getSubState(preset, "EM"));
    this->param_10_setPresetValue(getSubState(preset, "Em"));
    this->param_11_setPresetValue(getSubState(preset, "FM"));
    this->param_12_setPresetValue(getSubState(preset, "Fm"));
    this->param_13_setPresetValue(getSubState(preset, "GfM"));
    this->param_14_setPresetValue(getSubState(preset, "Gfm"));
    this->param_15_setPresetValue(getSubState(preset, "GM"));
    this->param_16_setPresetValue(getSubState(preset, "Gm"));
    this->param_17_setPresetValue(getSubState(preset, "AfM"));
    this->param_18_setPresetValue(getSubState(preset, "Afm"));
    this->param_19_setPresetValue(getSubState(preset, "AM"));
    this->param_20_setPresetValue(getSubState(preset, "Am"));
    this->param_21_setPresetValue(getSubState(preset, "BfM"));
    this->param_22_setPresetValue(getSubState(preset, "Bfm"));
    this->param_23_setPresetValue(getSubState(preset, "BM"));
    this->param_24_setPresetValue(getSubState(preset, "Bm"));
}

void setParameterValue(ParameterIndex index, ParameterValue v, MillisecondTime time) {
    this->updateTime(time, (ENGINE*)nullptr);

    switch (index) {
    case 0:
        {
        this->param_01_value_set(v);
        break;
        }
    case 1:
        {
        this->param_02_value_set(v);
        break;
        }
    case 2:
        {
        this->param_03_value_set(v);
        break;
        }
    case 3:
        {
        this->param_04_value_set(v);
        break;
        }
    case 4:
        {
        this->param_05_value_set(v);
        break;
        }
    case 5:
        {
        this->param_06_value_set(v);
        break;
        }
    case 6:
        {
        this->param_07_value_set(v);
        break;
        }
    case 7:
        {
        this->param_08_value_set(v);
        break;
        }
    case 8:
        {
        this->param_09_value_set(v);
        break;
        }
    case 9:
        {
        this->param_10_value_set(v);
        break;
        }
    case 10:
        {
        this->param_11_value_set(v);
        break;
        }
    case 11:
        {
        this->param_12_value_set(v);
        break;
        }
    case 12:
        {
        this->param_13_value_set(v);
        break;
        }
    case 13:
        {
        this->param_14_value_set(v);
        break;
        }
    case 14:
        {
        this->param_15_value_set(v);
        break;
        }
    case 15:
        {
        this->param_16_value_set(v);
        break;
        }
    case 16:
        {
        this->param_17_value_set(v);
        break;
        }
    case 17:
        {
        this->param_18_value_set(v);
        break;
        }
    case 18:
        {
        this->param_19_value_set(v);
        break;
        }
    case 19:
        {
        this->param_20_value_set(v);
        break;
        }
    case 20:
        {
        this->param_21_value_set(v);
        break;
        }
    case 21:
        {
        this->param_22_value_set(v);
        break;
        }
    case 22:
        {
        this->param_23_value_set(v);
        break;
        }
    case 23:
        {
        this->param_24_value_set(v);
        break;
        }
    }
}

void processParameterEvent(ParameterIndex index, ParameterValue value, MillisecondTime time) {
    this->setParameterValue(index, value, time);
}

void processParameterBangEvent(ParameterIndex index, MillisecondTime time) {
    this->setParameterValue(index, this->getParameterValue(index), time);
}

void processNormalizedParameterEvent(ParameterIndex index, ParameterValue value, MillisecondTime time) {
    this->setParameterValueNormalized(index, value, time);
}

ParameterValue getParameterValue(ParameterIndex index)  {
    switch (index) {
    case 0:
        {
        return this->param_01_value;
        }
    case 1:
        {
        return this->param_02_value;
        }
    case 2:
        {
        return this->param_03_value;
        }
    case 3:
        {
        return this->param_04_value;
        }
    case 4:
        {
        return this->param_05_value;
        }
    case 5:
        {
        return this->param_06_value;
        }
    case 6:
        {
        return this->param_07_value;
        }
    case 7:
        {
        return this->param_08_value;
        }
    case 8:
        {
        return this->param_09_value;
        }
    case 9:
        {
        return this->param_10_value;
        }
    case 10:
        {
        return this->param_11_value;
        }
    case 11:
        {
        return this->param_12_value;
        }
    case 12:
        {
        return this->param_13_value;
        }
    case 13:
        {
        return this->param_14_value;
        }
    case 14:
        {
        return this->param_15_value;
        }
    case 15:
        {
        return this->param_16_value;
        }
    case 16:
        {
        return this->param_17_value;
        }
    case 17:
        {
        return this->param_18_value;
        }
    case 18:
        {
        return this->param_19_value;
        }
    case 19:
        {
        return this->param_20_value;
        }
    case 20:
        {
        return this->param_21_value;
        }
    case 21:
        {
        return this->param_22_value;
        }
    case 22:
        {
        return this->param_23_value;
        }
    case 23:
        {
        return this->param_24_value;
        }
    default:
        {
        return 0;
        }
    }
}

ParameterIndex getNumSignalInParameters() const {
    return 0;
}

ParameterIndex getNumSignalOutParameters() const {
    return 0;
}

ParameterIndex getNumParameters() const {
    return 24;
}

ConstCharPointer getParameterName(ParameterIndex index) const {
    switch (index) {
    case 0:
        {
        return "CM";
        }
    case 1:
        {
        return "Cm";
        }
    case 2:
        {
        return "DfM";
        }
    case 3:
        {
        return "Dfm";
        }
    case 4:
        {
        return "DM";
        }
    case 5:
        {
        return "Dm";
        }
    case 6:
        {
        return "EfM";
        }
    case 7:
        {
        return "Efm";
        }
    case 8:
        {
        return "EM";
        }
    case 9:
        {
        return "Em";
        }
    case 10:
        {
        return "FM";
        }
    case 11:
        {
        return "Fm";
        }
    case 12:
        {
        return "GfM";
        }
    case 13:
        {
        return "Gfm";
        }
    case 14:
        {
        return "GM";
        }
    case 15:
        {
        return "Gm";
        }
    case 16:
        {
        return "AfM";
        }
    case 17:
        {
        return "Afm";
        }
    case 18:
        {
        return "AM";
        }
    case 19:
        {
        return "Am";
        }
    case 20:
        {
        return "BfM";
        }
    case 21:
        {
        return "Bfm";
        }
    case 22:
        {
        return "BM";
        }
    case 23:
        {
        return "Bm";
        }
    default:
        {
        return "bogus";
        }
    }
}

ConstCharPointer getParameterId(ParameterIndex index) const {
    switch (index) {
    case 0:
        {
        return "CM";
        }
    case 1:
        {
        return "Cm";
        }
    case 2:
        {
        return "DfM";
        }
    case 3:
        {
        return "Dfm";
        }
    case 4:
        {
        return "DM";
        }
    case 5:
        {
        return "Dm";
        }
    case 6:
        {
        return "EfM";
        }
    case 7:
        {
        return "Efm";
        }
    case 8:
        {
        return "EM";
        }
    case 9:
        {
        return "Em";
        }
    case 10:
        {
        return "FM";
        }
    case 11:
        {
        return "Fm";
        }
    case 12:
        {
        return "GfM";
        }
    case 13:
        {
        return "Gfm";
        }
    case 14:
        {
        return "GM";
        }
    case 15:
        {
        return "Gm";
        }
    case 16:
        {
        return "AfM";
        }
    case 17:
        {
        return "Afm";
        }
    case 18:
        {
        return "AM";
        }
    case 19:
        {
        return "Am";
        }
    case 20:
        {
        return "BfM";
        }
    case 21:
        {
        return "Bfm";
        }
    case 22:
        {
        return "BM";
        }
    case 23:
        {
        return "Bm";
        }
    default:
        {
        return "bogus";
        }
    }
}

void getParameterInfo(ParameterIndex index, ParameterInfo * info) const {
    {
        switch (index) {
        case 0:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 0;
            info->min = 0;
            info->max = 1;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 1:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 0;
            info->min = 0;
            info->max = 1;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 2:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 0;
            info->min = 0;
            info->max = 1;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 3:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 0;
            info->min = 0;
            info->max = 1;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 4:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 0;
            info->min = 0;
            info->max = 1;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 5:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 0;
            info->min = 0;
            info->max = 1;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 6:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 0;
            info->min = 0;
            info->max = 1;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 7:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 0;
            info->min = 0;
            info->max = 1;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 8:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 0;
            info->min = 0;
            info->max = 1;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 9:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 0;
            info->min = 0;
            info->max = 1;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 10:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 0;
            info->min = 0;
            info->max = 1;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 11:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 0;
            info->min = 0;
            info->max = 1;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 12:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 0;
            info->min = 0;
            info->max = 1;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 13:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 0;
            info->min = 0;
            info->max = 1;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 14:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 0;
            info->min = 0;
            info->max = 1;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 15:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 0;
            info->min = 0;
            info->max = 1;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 16:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 0;
            info->min = 0;
            info->max = 1;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 17:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 0;
            info->min = 0;
            info->max = 1;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 18:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 0;
            info->min = 0;
            info->max = 1;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 19:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 0;
            info->min = 0;
            info->max = 1;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 20:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 0;
            info->min = 0;
            info->max = 1;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 21:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 0;
            info->min = 0;
            info->max = 1;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 22:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 0;
            info->min = 0;
            info->max = 1;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        case 23:
            {
            info->type = ParameterTypeNumber;
            info->initialValue = 0;
            info->min = 0;
            info->max = 1;
            info->exponent = 1;
            info->steps = 0;
            info->debug = false;
            info->saveable = true;
            info->transmittable = true;
            info->initialized = true;
            info->visible = true;
            info->displayName = "";
            info->unit = "";
            info->ioType = IOTypeUndefined;
            info->signalIndex = INVALID_INDEX;
            break;
            }
        }
    }
}

ParameterValue applyStepsToNormalizedParameterValue(ParameterValue normalizedValue, int steps) const {
    if (steps == 1) {
        if (normalizedValue > 0) {
            normalizedValue = 1.;
        }
    } else {
        ParameterValue oneStep = (number)1. / (steps - 1);
        ParameterValue numberOfSteps = rnbo_fround(normalizedValue / oneStep * 1 / (number)1) * (number)1;
        normalizedValue = numberOfSteps * oneStep;
    }

    return normalizedValue;
}

ParameterValue convertToNormalizedParameterValue(ParameterIndex index, ParameterValue value) const {
    switch (index) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
        {
        {
            value = (value < 0 ? 0 : (value > 1 ? 1 : value));
            ParameterValue normalizedValue = (value - 0) / (1 - 0);
            return normalizedValue;
        }
        }
    default:
        {
        return value;
        }
    }
}

ParameterValue convertFromNormalizedParameterValue(ParameterIndex index, ParameterValue value) const {
    value = (value < 0 ? 0 : (value > 1 ? 1 : value));

    switch (index) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
        {
        {
            {
                return 0 + value * (1 - 0);
            }
        }
        }
    default:
        {
        return value;
        }
    }
}

ParameterValue constrainParameterValue(ParameterIndex index, ParameterValue value) const {
    switch (index) {
    case 0:
        {
        return this->param_01_value_constrain(value);
        }
    case 1:
        {
        return this->param_02_value_constrain(value);
        }
    case 2:
        {
        return this->param_03_value_constrain(value);
        }
    case 3:
        {
        return this->param_04_value_constrain(value);
        }
    case 4:
        {
        return this->param_05_value_constrain(value);
        }
    case 5:
        {
        return this->param_06_value_constrain(value);
        }
    case 6:
        {
        return this->param_07_value_constrain(value);
        }
    case 7:
        {
        return this->param_08_value_constrain(value);
        }
    case 8:
        {
        return this->param_09_value_constrain(value);
        }
    case 9:
        {
        return this->param_10_value_constrain(value);
        }
    case 10:
        {
        return this->param_11_value_constrain(value);
        }
    case 11:
        {
        return this->param_12_value_constrain(value);
        }
    case 12:
        {
        return this->param_13_value_constrain(value);
        }
    case 13:
        {
        return this->param_14_value_constrain(value);
        }
    case 14:
        {
        return this->param_15_value_constrain(value);
        }
    case 15:
        {
        return this->param_16_value_constrain(value);
        }
    case 16:
        {
        return this->param_17_value_constrain(value);
        }
    case 17:
        {
        return this->param_18_value_constrain(value);
        }
    case 18:
        {
        return this->param_19_value_constrain(value);
        }
    case 19:
        {
        return this->param_20_value_constrain(value);
        }
    case 20:
        {
        return this->param_21_value_constrain(value);
        }
    case 21:
        {
        return this->param_22_value_constrain(value);
        }
    case 22:
        {
        return this->param_23_value_constrain(value);
        }
    case 23:
        {
        return this->param_24_value_constrain(value);
        }
    default:
        {
        return value;
        }
    }
}

void processNumMessage(MessageTag tag, MessageTag objectId, MillisecondTime time, number payload) {
    this->updateTime(time, (ENGINE*)nullptr);

    switch (tag) {
    case TAG("listin"):
        {
        if (TAG("message_obj-33") == objectId)
            this->message_01_listin_number_set(payload);

        if (TAG("message_obj-68") == objectId)
            this->message_02_listin_number_set(payload);

        if (TAG("message_obj-341") == objectId)
            this->message_03_listin_number_set(payload);

        if (TAG("message_obj-711") == objectId)
            this->message_04_listin_number_set(payload);

        if (TAG("message_obj-378") == objectId)
            this->message_05_listin_number_set(payload);

        if (TAG("message_obj-736") == objectId)
            this->message_06_listin_number_set(payload);

        if (TAG("message_obj-423") == objectId)
            this->message_07_listin_number_set(payload);

        if (TAG("message_obj-762") == objectId)
            this->message_08_listin_number_set(payload);

        if (TAG("message_obj-416") == objectId)
            this->message_09_listin_number_set(payload);

        if (TAG("message_obj-812") == objectId)
            this->message_10_listin_number_set(payload);

        if (TAG("message_obj-430") == objectId)
            this->message_11_listin_number_set(payload);

        if (TAG("message_obj-74") == objectId)
            this->message_12_listin_number_set(payload);

        if (TAG("message_obj-468") == objectId)
            this->message_13_listin_number_set(payload);

        if (TAG("message_obj-856") == objectId)
            this->message_14_listin_number_set(payload);

        if (TAG("message_obj-475") == objectId)
            this->message_15_listin_number_set(payload);

        if (TAG("message_obj-57") == objectId)
            this->message_16_listin_number_set(payload);

        if (TAG("message_obj-514") == objectId)
            this->message_17_listin_number_set(payload);

        if (TAG("message_obj-902") == objectId)
            this->message_18_listin_number_set(payload);

        if (TAG("message_obj-927") == objectId)
            this->message_19_listin_number_set(payload);

        if (TAG("message_obj-551") == objectId)
            this->message_20_listin_number_set(payload);

        if (TAG("message_obj-588") == objectId)
            this->message_21_listin_number_set(payload);

        if (TAG("message_obj-952") == objectId)
            this->message_22_listin_number_set(payload);

        if (TAG("message_obj-625") == objectId)
            this->message_23_listin_number_set(payload);

        if (TAG("message_obj-1002") == objectId)
            this->message_24_listin_number_set(payload);

        break;
        }
    }
}

void processListMessage(
    MessageTag tag,
    MessageTag objectId,
    MillisecondTime time,
    const list& payload
) {
    this->updateTime(time, (ENGINE*)nullptr);

    switch (tag) {
    case TAG("listin"):
        {
        if (TAG("message_obj-33") == objectId)
            this->message_01_listin_list_set(payload);

        if (TAG("message_obj-68") == objectId)
            this->message_02_listin_list_set(payload);

        if (TAG("message_obj-341") == objectId)
            this->message_03_listin_list_set(payload);

        if (TAG("message_obj-711") == objectId)
            this->message_04_listin_list_set(payload);

        if (TAG("message_obj-378") == objectId)
            this->message_05_listin_list_set(payload);

        if (TAG("message_obj-736") == objectId)
            this->message_06_listin_list_set(payload);

        if (TAG("message_obj-423") == objectId)
            this->message_07_listin_list_set(payload);

        if (TAG("message_obj-762") == objectId)
            this->message_08_listin_list_set(payload);

        if (TAG("message_obj-416") == objectId)
            this->message_09_listin_list_set(payload);

        if (TAG("message_obj-812") == objectId)
            this->message_10_listin_list_set(payload);

        if (TAG("message_obj-430") == objectId)
            this->message_11_listin_list_set(payload);

        if (TAG("message_obj-74") == objectId)
            this->message_12_listin_list_set(payload);

        if (TAG("message_obj-468") == objectId)
            this->message_13_listin_list_set(payload);

        if (TAG("message_obj-856") == objectId)
            this->message_14_listin_list_set(payload);

        if (TAG("message_obj-475") == objectId)
            this->message_15_listin_list_set(payload);

        if (TAG("message_obj-57") == objectId)
            this->message_16_listin_list_set(payload);

        if (TAG("message_obj-514") == objectId)
            this->message_17_listin_list_set(payload);

        if (TAG("message_obj-902") == objectId)
            this->message_18_listin_list_set(payload);

        if (TAG("message_obj-927") == objectId)
            this->message_19_listin_list_set(payload);

        if (TAG("message_obj-551") == objectId)
            this->message_20_listin_list_set(payload);

        if (TAG("message_obj-588") == objectId)
            this->message_21_listin_list_set(payload);

        if (TAG("message_obj-952") == objectId)
            this->message_22_listin_list_set(payload);

        if (TAG("message_obj-625") == objectId)
            this->message_23_listin_list_set(payload);

        if (TAG("message_obj-1002") == objectId)
            this->message_24_listin_list_set(payload);

        break;
        }
    }
}

void processBangMessage(MessageTag tag, MessageTag objectId, MillisecondTime time) {
    this->updateTime(time, (ENGINE*)nullptr);

    switch (tag) {
    case TAG("listin"):
        {
        if (TAG("message_obj-33") == objectId)
            this->message_01_listin_bang_bang();

        if (TAG("message_obj-68") == objectId)
            this->message_02_listin_bang_bang();

        if (TAG("message_obj-341") == objectId)
            this->message_03_listin_bang_bang();

        if (TAG("message_obj-711") == objectId)
            this->message_04_listin_bang_bang();

        if (TAG("message_obj-378") == objectId)
            this->message_05_listin_bang_bang();

        if (TAG("message_obj-736") == objectId)
            this->message_06_listin_bang_bang();

        if (TAG("message_obj-423") == objectId)
            this->message_07_listin_bang_bang();

        if (TAG("message_obj-762") == objectId)
            this->message_08_listin_bang_bang();

        if (TAG("message_obj-416") == objectId)
            this->message_09_listin_bang_bang();

        if (TAG("message_obj-812") == objectId)
            this->message_10_listin_bang_bang();

        if (TAG("message_obj-430") == objectId)
            this->message_11_listin_bang_bang();

        if (TAG("message_obj-74") == objectId)
            this->message_12_listin_bang_bang();

        if (TAG("message_obj-468") == objectId)
            this->message_13_listin_bang_bang();

        if (TAG("message_obj-856") == objectId)
            this->message_14_listin_bang_bang();

        if (TAG("message_obj-475") == objectId)
            this->message_15_listin_bang_bang();

        if (TAG("message_obj-57") == objectId)
            this->message_16_listin_bang_bang();

        if (TAG("message_obj-514") == objectId)
            this->message_17_listin_bang_bang();

        if (TAG("message_obj-902") == objectId)
            this->message_18_listin_bang_bang();

        if (TAG("message_obj-927") == objectId)
            this->message_19_listin_bang_bang();

        if (TAG("message_obj-551") == objectId)
            this->message_20_listin_bang_bang();

        if (TAG("message_obj-588") == objectId)
            this->message_21_listin_bang_bang();

        if (TAG("message_obj-952") == objectId)
            this->message_22_listin_bang_bang();

        if (TAG("message_obj-625") == objectId)
            this->message_23_listin_bang_bang();

        if (TAG("message_obj-1002") == objectId)
            this->message_24_listin_bang_bang();

        break;
        }
    }
}

MessageTagInfo resolveTag(MessageTag tag) const {
    switch (tag) {
    case TAG("listout"):
        {
        return "listout";
        }
    case TAG("message_obj-33"):
        {
        return "message_obj-33";
        }
    case TAG("message_obj-68"):
        {
        return "message_obj-68";
        }
    case TAG("message_obj-341"):
        {
        return "message_obj-341";
        }
    case TAG("message_obj-711"):
        {
        return "message_obj-711";
        }
    case TAG("message_obj-378"):
        {
        return "message_obj-378";
        }
    case TAG("message_obj-736"):
        {
        return "message_obj-736";
        }
    case TAG("message_obj-423"):
        {
        return "message_obj-423";
        }
    case TAG("message_obj-762"):
        {
        return "message_obj-762";
        }
    case TAG("message_obj-416"):
        {
        return "message_obj-416";
        }
    case TAG("message_obj-812"):
        {
        return "message_obj-812";
        }
    case TAG("message_obj-430"):
        {
        return "message_obj-430";
        }
    case TAG("message_obj-74"):
        {
        return "message_obj-74";
        }
    case TAG("message_obj-468"):
        {
        return "message_obj-468";
        }
    case TAG("message_obj-856"):
        {
        return "message_obj-856";
        }
    case TAG("message_obj-475"):
        {
        return "message_obj-475";
        }
    case TAG("message_obj-57"):
        {
        return "message_obj-57";
        }
    case TAG("message_obj-514"):
        {
        return "message_obj-514";
        }
    case TAG("message_obj-902"):
        {
        return "message_obj-902";
        }
    case TAG("message_obj-927"):
        {
        return "message_obj-927";
        }
    case TAG("message_obj-551"):
        {
        return "message_obj-551";
        }
    case TAG("message_obj-588"):
        {
        return "message_obj-588";
        }
    case TAG("message_obj-952"):
        {
        return "message_obj-952";
        }
    case TAG("message_obj-625"):
        {
        return "message_obj-625";
        }
    case TAG("message_obj-1002"):
        {
        return "message_obj-1002";
        }
    case TAG("listin"):
        {
        return "listin";
        }
    }

    return "";
}

MessageIndex getNumMessages() const {
    return 0;
}

const MessageInfo& getMessageInfo(MessageIndex index) const {
    switch (index) {

    }

    return NullMessageInfo;
}

protected:

		
void advanceTime(EXTERNALENGINE*) {}
void advanceTime(INTERNALENGINE*) {
	_internalEngine.advanceTime(sampstoms(this->vs));
}

void processInternalEvents(MillisecondTime time) {
	_internalEngine.processEventsUntil(time);
}

void updateTime(MillisecondTime time, INTERNALENGINE*, bool inProcess = false) {
	if (time == TimeNow) time = getPatcherTime();
	processInternalEvents(inProcess ? time + sampsToMs(this->vs) : time);
	updateTime(time, (EXTERNALENGINE*)nullptr);
}

rnbomatic* operator->() {
    return this;
}
const rnbomatic* operator->() const {
    return this;
}
rnbomatic* getTopLevelPatcher() {
    return this;
}

void cancelClockEvents()
{
}

template<typename LISTTYPE = list> void listquicksort(LISTTYPE& arr, LISTTYPE& sortindices, Int l, Int h, bool ascending) {
    if (l < h) {
        Int p = (Int)(this->listpartition(arr, sortindices, l, h, ascending));
        this->listquicksort(arr, sortindices, l, p - 1, ascending);
        this->listquicksort(arr, sortindices, p + 1, h, ascending);
    }
}

template<typename LISTTYPE = list> Int listpartition(LISTTYPE& arr, LISTTYPE& sortindices, Int l, Int h, bool ascending) {
    number x = arr[(Index)h];
    Int i = (Int)(l - 1);

    for (Int j = (Int)(l); j <= h - 1; j++) {
        bool asc = (bool)((bool)(ascending) && arr[(Index)j] <= x);
        bool desc = (bool)((bool)(!(bool)(ascending)) && arr[(Index)j] >= x);

        if ((bool)(asc) || (bool)(desc)) {
            i++;
            this->listswapelements(arr, i, j);
            this->listswapelements(sortindices, i, j);
        }
    }

    i++;
    this->listswapelements(arr, i, h);
    this->listswapelements(sortindices, i, h);
    return i;
}

template<typename LISTTYPE = list> void listswapelements(LISTTYPE& arr, Int a, Int b) {
    auto tmp = arr[(Index)a];
    arr[(Index)a] = arr[(Index)b];
    arr[(Index)b] = tmp;
}

inline number linearinterp(number frac, number x, number y) {
    return x + (y - x) * frac;
}

number mstosamps(MillisecondTime ms) {
    return ms * this->sr * 0.001;
}

number maximum(number x, number y) {
    return (x < y ? y : x);
}

MillisecondTime sampstoms(number samps) {
    return samps * 1000 / this->sr;
}

void param_01_value_set(number v) {
    v = this->param_01_value_constrain(v);
    this->param_01_value = v;
    this->sendParameter(0, false);

    if (this->param_01_value != this->param_01_lastValue) {
        {
            this->getEngine()->presetTouched();
        }

        this->param_01_lastValue = this->param_01_value;
    }

    this->message_01_trigger_bang();
}

void param_02_value_set(number v) {
    v = this->param_02_value_constrain(v);
    this->param_02_value = v;
    this->sendParameter(1, false);

    if (this->param_02_value != this->param_02_lastValue) {
        {
            this->getEngine()->presetTouched();
        }

        this->param_02_lastValue = this->param_02_value;
    }

    this->message_02_trigger_bang();
}

void param_03_value_set(number v) {
    v = this->param_03_value_constrain(v);
    this->param_03_value = v;
    this->sendParameter(2, false);

    if (this->param_03_value != this->param_03_lastValue) {
        {
            this->getEngine()->presetTouched();
        }

        this->param_03_lastValue = this->param_03_value;
    }

    this->message_03_trigger_bang();
}

void param_04_value_set(number v) {
    v = this->param_04_value_constrain(v);
    this->param_04_value = v;
    this->sendParameter(3, false);

    if (this->param_04_value != this->param_04_lastValue) {
        {
            this->getEngine()->presetTouched();
        }

        this->param_04_lastValue = this->param_04_value;
    }

    this->message_04_trigger_bang();
}

void param_05_value_set(number v) {
    v = this->param_05_value_constrain(v);
    this->param_05_value = v;
    this->sendParameter(4, false);

    if (this->param_05_value != this->param_05_lastValue) {
        {
            this->getEngine()->presetTouched();
        }

        this->param_05_lastValue = this->param_05_value;
    }

    this->message_05_trigger_bang();
}

void param_06_value_set(number v) {
    v = this->param_06_value_constrain(v);
    this->param_06_value = v;
    this->sendParameter(5, false);

    if (this->param_06_value != this->param_06_lastValue) {
        {
            this->getEngine()->presetTouched();
        }

        this->param_06_lastValue = this->param_06_value;
    }

    this->message_06_trigger_bang();
}

void param_07_value_set(number v) {
    v = this->param_07_value_constrain(v);
    this->param_07_value = v;
    this->sendParameter(6, false);

    if (this->param_07_value != this->param_07_lastValue) {
        {
            this->getEngine()->presetTouched();
        }

        this->param_07_lastValue = this->param_07_value;
    }

    this->message_07_trigger_bang();
}

void param_08_value_set(number v) {
    v = this->param_08_value_constrain(v);
    this->param_08_value = v;
    this->sendParameter(7, false);

    if (this->param_08_value != this->param_08_lastValue) {
        {
            this->getEngine()->presetTouched();
        }

        this->param_08_lastValue = this->param_08_value;
    }

    this->message_08_trigger_bang();
}

void param_09_value_set(number v) {
    v = this->param_09_value_constrain(v);
    this->param_09_value = v;
    this->sendParameter(8, false);

    if (this->param_09_value != this->param_09_lastValue) {
        {
            this->getEngine()->presetTouched();
        }

        this->param_09_lastValue = this->param_09_value;
    }

    this->message_09_trigger_bang();
}

void param_10_value_set(number v) {
    v = this->param_10_value_constrain(v);
    this->param_10_value = v;
    this->sendParameter(9, false);

    if (this->param_10_value != this->param_10_lastValue) {
        {
            this->getEngine()->presetTouched();
        }

        this->param_10_lastValue = this->param_10_value;
    }

    this->message_10_trigger_bang();
}

void param_11_value_set(number v) {
    v = this->param_11_value_constrain(v);
    this->param_11_value = v;
    this->sendParameter(10, false);

    if (this->param_11_value != this->param_11_lastValue) {
        {
            this->getEngine()->presetTouched();
        }

        this->param_11_lastValue = this->param_11_value;
    }

    this->message_11_trigger_bang();
}

void param_12_value_set(number v) {
    v = this->param_12_value_constrain(v);
    this->param_12_value = v;
    this->sendParameter(11, false);

    if (this->param_12_value != this->param_12_lastValue) {
        {
            this->getEngine()->presetTouched();
        }

        this->param_12_lastValue = this->param_12_value;
    }

    this->message_12_trigger_bang();
}

void param_13_value_set(number v) {
    v = this->param_13_value_constrain(v);
    this->param_13_value = v;
    this->sendParameter(12, false);

    if (this->param_13_value != this->param_13_lastValue) {
        {
            this->getEngine()->presetTouched();
        }

        this->param_13_lastValue = this->param_13_value;
    }

    this->message_13_trigger_bang();
}

void param_14_value_set(number v) {
    v = this->param_14_value_constrain(v);
    this->param_14_value = v;
    this->sendParameter(13, false);

    if (this->param_14_value != this->param_14_lastValue) {
        {
            this->getEngine()->presetTouched();
        }

        this->param_14_lastValue = this->param_14_value;
    }

    this->message_14_trigger_bang();
}

void param_15_value_set(number v) {
    v = this->param_15_value_constrain(v);
    this->param_15_value = v;
    this->sendParameter(14, false);

    if (this->param_15_value != this->param_15_lastValue) {
        {
            this->getEngine()->presetTouched();
        }

        this->param_15_lastValue = this->param_15_value;
    }

    this->message_15_trigger_bang();
}

void param_16_value_set(number v) {
    v = this->param_16_value_constrain(v);
    this->param_16_value = v;
    this->sendParameter(15, false);

    if (this->param_16_value != this->param_16_lastValue) {
        {
            this->getEngine()->presetTouched();
        }

        this->param_16_lastValue = this->param_16_value;
    }

    this->message_16_trigger_bang();
}

void param_17_value_set(number v) {
    v = this->param_17_value_constrain(v);
    this->param_17_value = v;
    this->sendParameter(16, false);

    if (this->param_17_value != this->param_17_lastValue) {
        {
            this->getEngine()->presetTouched();
        }

        this->param_17_lastValue = this->param_17_value;
    }

    this->message_17_trigger_bang();
}

void param_18_value_set(number v) {
    v = this->param_18_value_constrain(v);
    this->param_18_value = v;
    this->sendParameter(17, false);

    if (this->param_18_value != this->param_18_lastValue) {
        {
            this->getEngine()->presetTouched();
        }

        this->param_18_lastValue = this->param_18_value;
    }

    this->message_18_trigger_bang();
}

void param_19_value_set(number v) {
    v = this->param_19_value_constrain(v);
    this->param_19_value = v;
    this->sendParameter(18, false);

    if (this->param_19_value != this->param_19_lastValue) {
        {
            this->getEngine()->presetTouched();
        }

        this->param_19_lastValue = this->param_19_value;
    }

    this->message_20_trigger_bang();
}

void param_20_value_set(number v) {
    v = this->param_20_value_constrain(v);
    this->param_20_value = v;
    this->sendParameter(19, false);

    if (this->param_20_value != this->param_20_lastValue) {
        {
            this->getEngine()->presetTouched();
        }

        this->param_20_lastValue = this->param_20_value;
    }

    this->message_19_trigger_bang();
}

void param_21_value_set(number v) {
    v = this->param_21_value_constrain(v);
    this->param_21_value = v;
    this->sendParameter(20, false);

    if (this->param_21_value != this->param_21_lastValue) {
        {
            this->getEngine()->presetTouched();
        }

        this->param_21_lastValue = this->param_21_value;
    }

    this->message_21_trigger_bang();
}

void param_22_value_set(number v) {
    v = this->param_22_value_constrain(v);
    this->param_22_value = v;
    this->sendParameter(21, false);

    if (this->param_22_value != this->param_22_lastValue) {
        {
            this->getEngine()->presetTouched();
        }

        this->param_22_lastValue = this->param_22_value;
    }

    this->message_22_trigger_bang();
}

void param_23_value_set(number v) {
    v = this->param_23_value_constrain(v);
    this->param_23_value = v;
    this->sendParameter(22, false);

    if (this->param_23_value != this->param_23_lastValue) {
        {
            this->getEngine()->presetTouched();
        }

        this->param_23_lastValue = this->param_23_value;
    }

    this->message_23_trigger_bang();
}

void param_24_value_set(number v) {
    v = this->param_24_value_constrain(v);
    this->param_24_value = v;
    this->sendParameter(23, false);

    if (this->param_24_value != this->param_24_lastValue) {
        {
            this->getEngine()->presetTouched();
        }

        this->param_24_lastValue = this->param_24_value;
    }

    this->message_24_trigger_bang();
}

MillisecondTime getPatcherTime() const {
    return this->_currentTime;
}

template<typename LISTTYPE> void message_01_listin_list_set(const LISTTYPE& v) {
    this->message_01_set_set(v);
}

void message_01_listin_number_set(number v) {
    this->message_01_set_set(v);
}

void message_01_listin_bang_bang() {
    this->message_01_trigger_bang();
}

template<typename LISTTYPE> void message_02_listin_list_set(const LISTTYPE& v) {
    this->message_02_set_set(v);
}

void message_02_listin_number_set(number v) {
    this->message_02_set_set(v);
}

void message_02_listin_bang_bang() {
    this->message_02_trigger_bang();
}

template<typename LISTTYPE> void message_03_listin_list_set(const LISTTYPE& v) {
    this->message_03_set_set(v);
}

void message_03_listin_number_set(number v) {
    this->message_03_set_set(v);
}

void message_03_listin_bang_bang() {
    this->message_03_trigger_bang();
}

template<typename LISTTYPE> void message_04_listin_list_set(const LISTTYPE& v) {
    this->message_04_set_set(v);
}

void message_04_listin_number_set(number v) {
    this->message_04_set_set(v);
}

void message_04_listin_bang_bang() {
    this->message_04_trigger_bang();
}

template<typename LISTTYPE> void message_05_listin_list_set(const LISTTYPE& v) {
    this->message_05_set_set(v);
}

void message_05_listin_number_set(number v) {
    this->message_05_set_set(v);
}

void message_05_listin_bang_bang() {
    this->message_05_trigger_bang();
}

template<typename LISTTYPE> void message_06_listin_list_set(const LISTTYPE& v) {
    this->message_06_set_set(v);
}

void message_06_listin_number_set(number v) {
    this->message_06_set_set(v);
}

void message_06_listin_bang_bang() {
    this->message_06_trigger_bang();
}

template<typename LISTTYPE> void message_07_listin_list_set(const LISTTYPE& v) {
    this->message_07_set_set(v);
}

void message_07_listin_number_set(number v) {
    this->message_07_set_set(v);
}

void message_07_listin_bang_bang() {
    this->message_07_trigger_bang();
}

template<typename LISTTYPE> void message_08_listin_list_set(const LISTTYPE& v) {
    this->message_08_set_set(v);
}

void message_08_listin_number_set(number v) {
    this->message_08_set_set(v);
}

void message_08_listin_bang_bang() {
    this->message_08_trigger_bang();
}

template<typename LISTTYPE> void message_09_listin_list_set(const LISTTYPE& v) {
    this->message_09_set_set(v);
}

void message_09_listin_number_set(number v) {
    this->message_09_set_set(v);
}

void message_09_listin_bang_bang() {
    this->message_09_trigger_bang();
}

template<typename LISTTYPE> void message_10_listin_list_set(const LISTTYPE& v) {
    this->message_10_set_set(v);
}

void message_10_listin_number_set(number v) {
    this->message_10_set_set(v);
}

void message_10_listin_bang_bang() {
    this->message_10_trigger_bang();
}

template<typename LISTTYPE> void message_11_listin_list_set(const LISTTYPE& v) {
    this->message_11_set_set(v);
}

void message_11_listin_number_set(number v) {
    this->message_11_set_set(v);
}

void message_11_listin_bang_bang() {
    this->message_11_trigger_bang();
}

template<typename LISTTYPE> void message_12_listin_list_set(const LISTTYPE& v) {
    this->message_12_set_set(v);
}

void message_12_listin_number_set(number v) {
    this->message_12_set_set(v);
}

void message_12_listin_bang_bang() {
    this->message_12_trigger_bang();
}

template<typename LISTTYPE> void message_13_listin_list_set(const LISTTYPE& v) {
    this->message_13_set_set(v);
}

void message_13_listin_number_set(number v) {
    this->message_13_set_set(v);
}

void message_13_listin_bang_bang() {
    this->message_13_trigger_bang();
}

template<typename LISTTYPE> void message_14_listin_list_set(const LISTTYPE& v) {
    this->message_14_set_set(v);
}

void message_14_listin_number_set(number v) {
    this->message_14_set_set(v);
}

void message_14_listin_bang_bang() {
    this->message_14_trigger_bang();
}

template<typename LISTTYPE> void message_15_listin_list_set(const LISTTYPE& v) {
    this->message_15_set_set(v);
}

void message_15_listin_number_set(number v) {
    this->message_15_set_set(v);
}

void message_15_listin_bang_bang() {
    this->message_15_trigger_bang();
}

template<typename LISTTYPE> void message_16_listin_list_set(const LISTTYPE& v) {
    this->message_16_set_set(v);
}

void message_16_listin_number_set(number v) {
    this->message_16_set_set(v);
}

void message_16_listin_bang_bang() {
    this->message_16_trigger_bang();
}

template<typename LISTTYPE> void message_17_listin_list_set(const LISTTYPE& v) {
    this->message_17_set_set(v);
}

void message_17_listin_number_set(number v) {
    this->message_17_set_set(v);
}

void message_17_listin_bang_bang() {
    this->message_17_trigger_bang();
}

template<typename LISTTYPE> void message_18_listin_list_set(const LISTTYPE& v) {
    this->message_18_set_set(v);
}

void message_18_listin_number_set(number v) {
    this->message_18_set_set(v);
}

void message_18_listin_bang_bang() {
    this->message_18_trigger_bang();
}

template<typename LISTTYPE> void message_19_listin_list_set(const LISTTYPE& v) {
    this->message_19_set_set(v);
}

void message_19_listin_number_set(number v) {
    this->message_19_set_set(v);
}

void message_19_listin_bang_bang() {
    this->message_19_trigger_bang();
}

template<typename LISTTYPE> void message_20_listin_list_set(const LISTTYPE& v) {
    this->message_20_set_set(v);
}

void message_20_listin_number_set(number v) {
    this->message_20_set_set(v);
}

void message_20_listin_bang_bang() {
    this->message_20_trigger_bang();
}

template<typename LISTTYPE> void message_21_listin_list_set(const LISTTYPE& v) {
    this->message_21_set_set(v);
}

void message_21_listin_number_set(number v) {
    this->message_21_set_set(v);
}

void message_21_listin_bang_bang() {
    this->message_21_trigger_bang();
}

template<typename LISTTYPE> void message_22_listin_list_set(const LISTTYPE& v) {
    this->message_22_set_set(v);
}

void message_22_listin_number_set(number v) {
    this->message_22_set_set(v);
}

void message_22_listin_bang_bang() {
    this->message_22_trigger_bang();
}

template<typename LISTTYPE> void message_23_listin_list_set(const LISTTYPE& v) {
    this->message_23_set_set(v);
}

void message_23_listin_number_set(number v) {
    this->message_23_set_set(v);
}

void message_23_listin_bang_bang() {
    this->message_23_trigger_bang();
}

template<typename LISTTYPE> void message_24_listin_list_set(const LISTTYPE& v) {
    this->message_24_set_set(v);
}

void message_24_listin_number_set(number v) {
    this->message_24_set_set(v);
}

void message_24_listin_bang_bang() {
    this->message_24_trigger_bang();
}

void deallocateSignals() {
    Index i;

    for (i = 0; i < 3; i++) {
        this->signals[i] = freeSignal(this->signals[i]);
    }

    this->globaltransport_tempo = freeSignal(this->globaltransport_tempo);
    this->globaltransport_state = freeSignal(this->globaltransport_state);
    this->zeroBuffer = freeSignal(this->zeroBuffer);
    this->dummyBuffer = freeSignal(this->dummyBuffer);
}

Index getMaxBlockSize() const {
    return this->maxvs;
}

number getSampleRate() const {
    return this->sr;
}

bool hasFixedVectorSize() const {
    return false;
}

void setProbingTarget(MessageTag ) {}

void fillRNBODefaultSinus(DataRef& ref) {
    SampleBuffer buffer(ref);
    number bufsize = buffer->getSize();

    for (Index i = 0; i < bufsize; i++) {
        buffer[i] = rnbo_cos(i * 3.14159265358979323846 * 2. / bufsize);
    }
}

void fillRNBODefaultMtofLookupTable256(DataRef& ref) {
    SampleBuffer buffer(ref);
    number bufsize = buffer->getSize();

    for (Index i = 0; i < bufsize; i++) {
        number midivalue = -256. + (number)512. / (bufsize - 1) * i;
        buffer[i] = rnbo_exp(.057762265 * (midivalue - 69.0));
    }
}

void fillDataRef(DataRefIndex index, DataRef& ref) {
    switch (index) {
    case 0:
        {
        this->fillRNBODefaultSinus(ref);
        break;
        }
    case 1:
        {
        this->fillRNBODefaultMtofLookupTable256(ref);
        break;
        }
    }
}

void allocateDataRefs() {
    this->cycle_tilde_01_buffer->requestSize(16384, 1);
    this->cycle_tilde_01_buffer->setSampleRate(this->sr);
    this->cycle_tilde_02_buffer->requestSize(16384, 1);
    this->cycle_tilde_02_buffer->setSampleRate(this->sr);
    this->cycle_tilde_03_buffer->requestSize(16384, 1);
    this->cycle_tilde_03_buffer->setSampleRate(this->sr);
    this->mtof_01_innerMtoF_buffer->requestSize(65536, 1);
    this->mtof_01_innerMtoF_buffer->setSampleRate(this->sr);
    this->mtof_02_innerMtoF_buffer->requestSize(65536, 1);
    this->mtof_02_innerMtoF_buffer->setSampleRate(this->sr);
    this->mtof_03_innerMtoF_buffer->requestSize(65536, 1);
    this->mtof_03_innerMtoF_buffer->setSampleRate(this->sr);
    this->cycle_tilde_01_buffer = this->cycle_tilde_01_buffer->allocateIfNeeded();
    this->cycle_tilde_02_buffer = this->cycle_tilde_02_buffer->allocateIfNeeded();
    this->cycle_tilde_03_buffer = this->cycle_tilde_03_buffer->allocateIfNeeded();

    if (this->RNBODefaultSinus->hasRequestedSize()) {
        if (this->RNBODefaultSinus->wantsFill())
            this->fillRNBODefaultSinus(this->RNBODefaultSinus);

        this->getEngine()->sendDataRefUpdated(0);
    }

    this->mtof_01_innerMtoF_buffer = this->mtof_01_innerMtoF_buffer->allocateIfNeeded();
    this->mtof_02_innerMtoF_buffer = this->mtof_02_innerMtoF_buffer->allocateIfNeeded();
    this->mtof_03_innerMtoF_buffer = this->mtof_03_innerMtoF_buffer->allocateIfNeeded();

    if (this->RNBODefaultMtofLookupTable256->hasRequestedSize()) {
        if (this->RNBODefaultMtofLookupTable256->wantsFill())
            this->fillRNBODefaultMtofLookupTable256(this->RNBODefaultMtofLookupTable256);

        this->getEngine()->sendDataRefUpdated(1);
    }
}

void initializeObjects() {
    this->message_01_init();
    this->message_02_init();
    this->message_03_init();
    this->message_04_init();
    this->message_05_init();
    this->message_06_init();
    this->message_07_init();
    this->message_08_init();
    this->message_09_init();
    this->message_10_init();
    this->message_11_init();
    this->message_12_init();
    this->mtof_01_innerScala_init();
    this->mtof_01_init();
    this->mtof_02_innerScala_init();
    this->mtof_02_init();
    this->message_13_init();
    this->message_14_init();
    this->mtof_03_innerScala_init();
    this->mtof_03_init();
    this->message_15_init();
    this->message_16_init();
    this->message_17_init();
    this->message_18_init();
    this->message_19_init();
    this->message_20_init();
    this->message_21_init();
    this->message_22_init();
    this->message_23_init();
    this->message_24_init();
}

Index getIsMuted()  {
    return this->isMuted;
}

void setIsMuted(Index v)  {
    this->isMuted = v;
}

void onSampleRateChanged(double ) {}

void extractState(PatcherStateInterface& ) {}

void applyState() {}

void processClockEvent(MillisecondTime , ClockId , bool , ParameterValue ) {}

void processOutletAtCurrentTime(EngineLink* , OutletIndex , ParameterValue ) {}

void processOutletEvent(
    EngineLink* sender,
    OutletIndex index,
    ParameterValue value,
    MillisecondTime time
) {
    this->updateTime(time, (ENGINE*)nullptr);
    this->processOutletAtCurrentTime(sender, index, value);
}

void sendOutlet(OutletIndex index, ParameterValue value) {
    this->getEngine()->sendOutlet(this, index, value);
}

void startup() {
    this->updateTime(this->getEngine()->getCurrentTime(), (ENGINE*)nullptr);

    {
        this->scheduleParamInit(0, 0);
    }

    {
        this->scheduleParamInit(1, 0);
    }

    {
        this->scheduleParamInit(2, 0);
    }

    {
        this->scheduleParamInit(3, 0);
    }

    {
        this->scheduleParamInit(4, 0);
    }

    {
        this->scheduleParamInit(5, 0);
    }

    {
        this->scheduleParamInit(6, 0);
    }

    {
        this->scheduleParamInit(7, 0);
    }

    {
        this->scheduleParamInit(8, 0);
    }

    {
        this->scheduleParamInit(9, 0);
    }

    {
        this->scheduleParamInit(10, 0);
    }

    {
        this->scheduleParamInit(11, 0);
    }

    {
        this->scheduleParamInit(12, 0);
    }

    {
        this->scheduleParamInit(13, 0);
    }

    {
        this->scheduleParamInit(14, 0);
    }

    {
        this->scheduleParamInit(15, 0);
    }

    {
        this->scheduleParamInit(16, 0);
    }

    {
        this->scheduleParamInit(17, 0);
    }

    {
        this->scheduleParamInit(18, 0);
    }

    {
        this->scheduleParamInit(19, 0);
    }

    {
        this->scheduleParamInit(20, 0);
    }

    {
        this->scheduleParamInit(21, 0);
    }

    {
        this->scheduleParamInit(22, 0);
    }

    {
        this->scheduleParamInit(23, 0);
    }

    this->processParamInitEvents();
}

number param_01_value_constrain(number v) const {
    v = (v > 1 ? 1 : (v < 0 ? 0 : v));
    return v;
}

void cycle_tilde_03_frequency_set(number v) {
    this->cycle_tilde_03_frequency = v;
}

void cycle_tilde_03_phase_offset_set(number v) {
    this->cycle_tilde_03_phase_offset = v;
}

template<typename LISTTYPE> void mtof_03_out_set(const LISTTYPE& v) {
    {
        if (v->length > 1)
            this->cycle_tilde_03_phase_offset_set(v[1]);

        number converted = (v->length > 0 ? v[0] : 0);
        this->cycle_tilde_03_frequency_set(converted);
    }
}

template<typename LISTTYPE> void mtof_03_midivalue_set(const LISTTYPE& v) {
    this->mtof_03_midivalue = jsCreateListCopy(v);
    list tmp = list();

    for (Int i = 0; i < this->mtof_03_midivalue->length; i++) {
        tmp->push(
            this->mtof_03_innerMtoF_next(this->mtof_03_midivalue[(Index)i], this->mtof_03_base)
        );
    }

    this->mtof_03_out_set(tmp);
}

void expr_03_out1_set(number v) {
    this->expr_03_out1 = v;

    {
        listbase<number, 1> converted = {this->expr_03_out1};
        this->mtof_03_midivalue_set(converted);
    }
}

void expr_03_in1_set(number in1) {
    this->expr_03_in1 = in1;
    this->expr_03_out1_set(this->expr_03_in1 + this->expr_03_in2);//#map:+_obj-45:1
}

void unpack_01_out3_set(number v) {
    this->unpack_01_out3 = v;
    this->expr_03_in1_set(v);
}

void cycle_tilde_02_frequency_set(number v) {
    this->cycle_tilde_02_frequency = v;
}

void cycle_tilde_02_phase_offset_set(number v) {
    this->cycle_tilde_02_phase_offset = v;
}

template<typename LISTTYPE> void mtof_02_out_set(const LISTTYPE& v) {
    {
        if (v->length > 1)
            this->cycle_tilde_02_phase_offset_set(v[1]);

        number converted = (v->length > 0 ? v[0] : 0);
        this->cycle_tilde_02_frequency_set(converted);
    }
}

template<typename LISTTYPE> void mtof_02_midivalue_set(const LISTTYPE& v) {
    this->mtof_02_midivalue = jsCreateListCopy(v);
    list tmp = list();

    for (Int i = 0; i < this->mtof_02_midivalue->length; i++) {
        tmp->push(
            this->mtof_02_innerMtoF_next(this->mtof_02_midivalue[(Index)i], this->mtof_02_base)
        );
    }

    this->mtof_02_out_set(tmp);
}

void expr_02_out1_set(number v) {
    this->expr_02_out1 = v;

    {
        listbase<number, 1> converted = {this->expr_02_out1};
        this->mtof_02_midivalue_set(converted);
    }
}

void expr_02_in1_set(number in1) {
    this->expr_02_in1 = in1;
    this->expr_02_out1_set(this->expr_02_in1 + this->expr_02_in2);//#map:+_obj-40:1
}

void unpack_01_out2_set(number v) {
    this->unpack_01_out2 = v;
    this->expr_02_in1_set(v);
}

void cycle_tilde_01_frequency_set(number v) {
    this->cycle_tilde_01_frequency = v;
}

void cycle_tilde_01_phase_offset_set(number v) {
    this->cycle_tilde_01_phase_offset = v;
}

template<typename LISTTYPE> void mtof_01_out_set(const LISTTYPE& v) {
    {
        if (v->length > 1)
            this->cycle_tilde_01_phase_offset_set(v[1]);

        number converted = (v->length > 0 ? v[0] : 0);
        this->cycle_tilde_01_frequency_set(converted);
    }
}

template<typename LISTTYPE> void mtof_01_midivalue_set(const LISTTYPE& v) {
    this->mtof_01_midivalue = jsCreateListCopy(v);
    list tmp = list();

    for (Int i = 0; i < this->mtof_01_midivalue->length; i++) {
        tmp->push(
            this->mtof_01_innerMtoF_next(this->mtof_01_midivalue[(Index)i], this->mtof_01_base)
        );
    }

    this->mtof_01_out_set(tmp);
}

void expr_01_out1_set(number v) {
    this->expr_01_out1 = v;

    {
        listbase<number, 1> converted = {this->expr_01_out1};
        this->mtof_01_midivalue_set(converted);
    }
}

void expr_01_in1_set(number in1) {
    this->expr_01_in1 = in1;
    this->expr_01_out1_set(this->expr_01_in1 + this->expr_01_in2);//#map:+_obj-37:1
}

void unpack_01_out1_set(number v) {
    this->unpack_01_out1 = v;
    this->expr_01_in1_set(v);
}

template<typename LISTTYPE> void unpack_01_input_list_set(const LISTTYPE& v) {
    if (v->length > 2)
        this->unpack_01_out3_set(v[2]);

    if (v->length > 1)
        this->unpack_01_out2_set(v[1]);

    if (v->length > 0)
        this->unpack_01_out1_set(v[0]);
}

template<typename LISTTYPE> void message_01_out_set(const LISTTYPE& v) {
    this->unpack_01_input_list_set(v);
}

void message_01_trigger_bang() {
    if ((bool)(this->message_01_set->length) || (bool)(false)) {
        this->message_01_out_set(this->message_01_set);
    }
}

number param_02_value_constrain(number v) const {
    v = (v > 1 ? 1 : (v < 0 ? 0 : v));
    return v;
}

template<typename LISTTYPE> void message_02_out_set(const LISTTYPE& v) {
    this->unpack_01_input_list_set(v);
}

void message_02_trigger_bang() {
    if ((bool)(this->message_02_set->length) || (bool)(false)) {
        this->message_02_out_set(this->message_02_set);
    }
}

number param_03_value_constrain(number v) const {
    v = (v > 1 ? 1 : (v < 0 ? 0 : v));
    return v;
}

template<typename LISTTYPE> void message_03_out_set(const LISTTYPE& v) {
    this->unpack_01_input_list_set(v);
}

void message_03_trigger_bang() {
    if ((bool)(this->message_03_set->length) || (bool)(false)) {
        this->message_03_out_set(this->message_03_set);
    }
}

number param_04_value_constrain(number v) const {
    v = (v > 1 ? 1 : (v < 0 ? 0 : v));
    return v;
}

template<typename LISTTYPE> void message_04_out_set(const LISTTYPE& v) {
    this->unpack_01_input_list_set(v);
}

void message_04_trigger_bang() {
    if ((bool)(this->message_04_set->length) || (bool)(false)) {
        this->message_04_out_set(this->message_04_set);
    }
}

number param_05_value_constrain(number v) const {
    v = (v > 1 ? 1 : (v < 0 ? 0 : v));
    return v;
}

template<typename LISTTYPE> void message_05_out_set(const LISTTYPE& v) {
    this->unpack_01_input_list_set(v);
}

void message_05_trigger_bang() {
    if ((bool)(this->message_05_set->length) || (bool)(false)) {
        this->message_05_out_set(this->message_05_set);
    }
}

number param_06_value_constrain(number v) const {
    v = (v > 1 ? 1 : (v < 0 ? 0 : v));
    return v;
}

template<typename LISTTYPE> void message_06_out_set(const LISTTYPE& v) {
    this->unpack_01_input_list_set(v);
}

void message_06_trigger_bang() {
    if ((bool)(this->message_06_set->length) || (bool)(false)) {
        this->message_06_out_set(this->message_06_set);
    }
}

number param_07_value_constrain(number v) const {
    v = (v > 1 ? 1 : (v < 0 ? 0 : v));
    return v;
}

template<typename LISTTYPE> void message_07_out_set(const LISTTYPE& v) {
    this->unpack_01_input_list_set(v);
}

void message_07_trigger_bang() {
    if ((bool)(this->message_07_set->length) || (bool)(false)) {
        this->message_07_out_set(this->message_07_set);
    }
}

number param_08_value_constrain(number v) const {
    v = (v > 1 ? 1 : (v < 0 ? 0 : v));
    return v;
}

template<typename LISTTYPE> void message_08_out_set(const LISTTYPE& v) {
    this->unpack_01_input_list_set(v);
}

void message_08_trigger_bang() {
    if ((bool)(this->message_08_set->length) || (bool)(false)) {
        this->message_08_out_set(this->message_08_set);
    }
}

number param_09_value_constrain(number v) const {
    v = (v > 1 ? 1 : (v < 0 ? 0 : v));
    return v;
}

template<typename LISTTYPE> void message_09_out_set(const LISTTYPE& v) {
    this->unpack_01_input_list_set(v);
}

void message_09_trigger_bang() {
    if ((bool)(this->message_09_set->length) || (bool)(false)) {
        this->message_09_out_set(this->message_09_set);
    }
}

number param_10_value_constrain(number v) const {
    v = (v > 1 ? 1 : (v < 0 ? 0 : v));
    return v;
}

template<typename LISTTYPE> void message_10_out_set(const LISTTYPE& v) {
    this->unpack_01_input_list_set(v);
}

void message_10_trigger_bang() {
    if ((bool)(this->message_10_set->length) || (bool)(false)) {
        this->message_10_out_set(this->message_10_set);
    }
}

number param_11_value_constrain(number v) const {
    v = (v > 1 ? 1 : (v < 0 ? 0 : v));
    return v;
}

template<typename LISTTYPE> void message_11_out_set(const LISTTYPE& v) {
    this->unpack_01_input_list_set(v);
}

void message_11_trigger_bang() {
    if ((bool)(this->message_11_set->length) || (bool)(false)) {
        this->message_11_out_set(this->message_11_set);
    }
}

number param_12_value_constrain(number v) const {
    v = (v > 1 ? 1 : (v < 0 ? 0 : v));
    return v;
}

template<typename LISTTYPE> void message_12_out_set(const LISTTYPE& v) {
    this->unpack_01_input_list_set(v);
}

void message_12_trigger_bang() {
    if ((bool)(this->message_12_set->length) || (bool)(false)) {
        this->message_12_out_set(this->message_12_set);
    }
}

number param_13_value_constrain(number v) const {
    v = (v > 1 ? 1 : (v < 0 ? 0 : v));
    return v;
}

template<typename LISTTYPE> void message_13_out_set(const LISTTYPE& v) {
    this->unpack_01_input_list_set(v);
}

void message_13_trigger_bang() {
    if ((bool)(this->message_13_set->length) || (bool)(false)) {
        this->message_13_out_set(this->message_13_set);
    }
}

number param_14_value_constrain(number v) const {
    v = (v > 1 ? 1 : (v < 0 ? 0 : v));
    return v;
}

template<typename LISTTYPE> void message_14_out_set(const LISTTYPE& v) {
    this->unpack_01_input_list_set(v);
}

void message_14_trigger_bang() {
    if ((bool)(this->message_14_set->length) || (bool)(false)) {
        this->message_14_out_set(this->message_14_set);
    }
}

number param_15_value_constrain(number v) const {
    v = (v > 1 ? 1 : (v < 0 ? 0 : v));
    return v;
}

template<typename LISTTYPE> void message_15_out_set(const LISTTYPE& v) {
    this->unpack_01_input_list_set(v);
}

void message_15_trigger_bang() {
    if ((bool)(this->message_15_set->length) || (bool)(false)) {
        this->message_15_out_set(this->message_15_set);
    }
}

number param_16_value_constrain(number v) const {
    v = (v > 1 ? 1 : (v < 0 ? 0 : v));
    return v;
}

template<typename LISTTYPE> void message_16_out_set(const LISTTYPE& v) {
    this->unpack_01_input_list_set(v);
}

void message_16_trigger_bang() {
    if ((bool)(this->message_16_set->length) || (bool)(false)) {
        this->message_16_out_set(this->message_16_set);
    }
}

number param_17_value_constrain(number v) const {
    v = (v > 1 ? 1 : (v < 0 ? 0 : v));
    return v;
}

template<typename LISTTYPE> void message_17_out_set(const LISTTYPE& v) {
    this->unpack_01_input_list_set(v);
}

void message_17_trigger_bang() {
    if ((bool)(this->message_17_set->length) || (bool)(false)) {
        this->message_17_out_set(this->message_17_set);
    }
}

number param_18_value_constrain(number v) const {
    v = (v > 1 ? 1 : (v < 0 ? 0 : v));
    return v;
}

template<typename LISTTYPE> void message_18_out_set(const LISTTYPE& v) {
    this->unpack_01_input_list_set(v);
}

void message_18_trigger_bang() {
    if ((bool)(this->message_18_set->length) || (bool)(false)) {
        this->message_18_out_set(this->message_18_set);
    }
}

number param_19_value_constrain(number v) const {
    v = (v > 1 ? 1 : (v < 0 ? 0 : v));
    return v;
}

template<typename LISTTYPE> void message_20_out_set(const LISTTYPE& v) {
    this->unpack_01_input_list_set(v);
}

void message_20_trigger_bang() {
    if ((bool)(this->message_20_set->length) || (bool)(false)) {
        this->message_20_out_set(this->message_20_set);
    }
}

number param_20_value_constrain(number v) const {
    v = (v > 1 ? 1 : (v < 0 ? 0 : v));
    return v;
}

template<typename LISTTYPE> void message_19_out_set(const LISTTYPE& v) {
    this->unpack_01_input_list_set(v);
}

void message_19_trigger_bang() {
    if ((bool)(this->message_19_set->length) || (bool)(false)) {
        this->message_19_out_set(this->message_19_set);
    }
}

number param_21_value_constrain(number v) const {
    v = (v > 1 ? 1 : (v < 0 ? 0 : v));
    return v;
}

template<typename LISTTYPE> void message_21_out_set(const LISTTYPE& v) {
    this->unpack_01_input_list_set(v);
}

void message_21_trigger_bang() {
    if ((bool)(this->message_21_set->length) || (bool)(false)) {
        this->message_21_out_set(this->message_21_set);
    }
}

number param_22_value_constrain(number v) const {
    v = (v > 1 ? 1 : (v < 0 ? 0 : v));
    return v;
}

template<typename LISTTYPE> void message_22_out_set(const LISTTYPE& v) {
    this->unpack_01_input_list_set(v);
}

void message_22_trigger_bang() {
    if ((bool)(this->message_22_set->length) || (bool)(false)) {
        this->message_22_out_set(this->message_22_set);
    }
}

number param_23_value_constrain(number v) const {
    v = (v > 1 ? 1 : (v < 0 ? 0 : v));
    return v;
}

template<typename LISTTYPE> void message_23_out_set(const LISTTYPE& v) {
    this->unpack_01_input_list_set(v);
}

void message_23_trigger_bang() {
    if ((bool)(this->message_23_set->length) || (bool)(false)) {
        this->message_23_out_set(this->message_23_set);
    }
}

number param_24_value_constrain(number v) const {
    v = (v > 1 ? 1 : (v < 0 ? 0 : v));
    return v;
}

template<typename LISTTYPE> void message_24_out_set(const LISTTYPE& v) {
    this->unpack_01_input_list_set(v);
}

void message_24_trigger_bang() {
    if ((bool)(this->message_24_set->length) || (bool)(false)) {
        this->message_24_out_set(this->message_24_set);
    }
}

template<typename LISTTYPE> void message_01_set_set(const LISTTYPE& v) {
    this->message_01_set = jsCreateListCopy(v);
}

template<typename LISTTYPE> void message_02_set_set(const LISTTYPE& v) {
    this->message_02_set = jsCreateListCopy(v);
}

template<typename LISTTYPE> void message_03_set_set(const LISTTYPE& v) {
    this->message_03_set = jsCreateListCopy(v);
}

template<typename LISTTYPE> void message_04_set_set(const LISTTYPE& v) {
    this->message_04_set = jsCreateListCopy(v);
}

template<typename LISTTYPE> void message_05_set_set(const LISTTYPE& v) {
    this->message_05_set = jsCreateListCopy(v);
}

template<typename LISTTYPE> void message_06_set_set(const LISTTYPE& v) {
    this->message_06_set = jsCreateListCopy(v);
}

template<typename LISTTYPE> void message_07_set_set(const LISTTYPE& v) {
    this->message_07_set = jsCreateListCopy(v);
}

template<typename LISTTYPE> void message_08_set_set(const LISTTYPE& v) {
    this->message_08_set = jsCreateListCopy(v);
}

template<typename LISTTYPE> void message_09_set_set(const LISTTYPE& v) {
    this->message_09_set = jsCreateListCopy(v);
}

template<typename LISTTYPE> void message_10_set_set(const LISTTYPE& v) {
    this->message_10_set = jsCreateListCopy(v);
}

template<typename LISTTYPE> void message_11_set_set(const LISTTYPE& v) {
    this->message_11_set = jsCreateListCopy(v);
}

template<typename LISTTYPE> void message_12_set_set(const LISTTYPE& v) {
    this->message_12_set = jsCreateListCopy(v);
}

template<typename LISTTYPE> void message_13_set_set(const LISTTYPE& v) {
    this->message_13_set = jsCreateListCopy(v);
}

template<typename LISTTYPE> void message_14_set_set(const LISTTYPE& v) {
    this->message_14_set = jsCreateListCopy(v);
}

template<typename LISTTYPE> void message_15_set_set(const LISTTYPE& v) {
    this->message_15_set = jsCreateListCopy(v);
}

template<typename LISTTYPE> void message_16_set_set(const LISTTYPE& v) {
    this->message_16_set = jsCreateListCopy(v);
}

template<typename LISTTYPE> void message_17_set_set(const LISTTYPE& v) {
    this->message_17_set = jsCreateListCopy(v);
}

template<typename LISTTYPE> void message_18_set_set(const LISTTYPE& v) {
    this->message_18_set = jsCreateListCopy(v);
}

template<typename LISTTYPE> void message_19_set_set(const LISTTYPE& v) {
    this->message_19_set = jsCreateListCopy(v);
}

template<typename LISTTYPE> void message_20_set_set(const LISTTYPE& v) {
    this->message_20_set = jsCreateListCopy(v);
}

template<typename LISTTYPE> void message_21_set_set(const LISTTYPE& v) {
    this->message_21_set = jsCreateListCopy(v);
}

template<typename LISTTYPE> void message_22_set_set(const LISTTYPE& v) {
    this->message_22_set = jsCreateListCopy(v);
}

template<typename LISTTYPE> void message_23_set_set(const LISTTYPE& v) {
    this->message_23_set = jsCreateListCopy(v);
}

template<typename LISTTYPE> void message_24_set_set(const LISTTYPE& v) {
    this->message_24_set = jsCreateListCopy(v);
}

void cycle_tilde_01_perform(
    number frequency,
    number phase_offset,
    SampleValue * out1,
    SampleValue * out2,
    Index n
) {
    RNBO_UNUSED(phase_offset);
    auto __cycle_tilde_01_f2i = this->cycle_tilde_01_f2i;
    auto __cycle_tilde_01_buffer = this->cycle_tilde_01_buffer;
    auto __cycle_tilde_01_phasei = this->cycle_tilde_01_phasei;
    Index i;

    for (i = 0; i < (Index)n; i++) {
        {
            UInt32 uint_phase;

            {
                {
                    uint_phase = __cycle_tilde_01_phasei;
                }
            }

            UInt32 idx = (UInt32)(uint32_rshift(uint_phase, 18));
            number frac = ((BinOpInt)((BinOpInt)uint_phase & (BinOpInt)262143)) * 3.81471181759574e-6;
            number y0 = __cycle_tilde_01_buffer[(Index)idx];
            number y1 = __cycle_tilde_01_buffer[(Index)((BinOpInt)(idx + 1) & (BinOpInt)16383)];
            number y = y0 + frac * (y1 - y0);

            {
                UInt32 pincr = (UInt32)(uint32_trunc(frequency * __cycle_tilde_01_f2i));
                __cycle_tilde_01_phasei = uint32_add(__cycle_tilde_01_phasei, pincr);
            }

            out1[(Index)i] = y;
            out2[(Index)i] = uint_phase * 0.232830643653869629e-9;
            continue;
        }
    }

    this->cycle_tilde_01_phasei = __cycle_tilde_01_phasei;
}

void cycle_tilde_02_perform(
    number frequency,
    number phase_offset,
    SampleValue * out1,
    SampleValue * out2,
    Index n
) {
    RNBO_UNUSED(phase_offset);
    auto __cycle_tilde_02_f2i = this->cycle_tilde_02_f2i;
    auto __cycle_tilde_02_buffer = this->cycle_tilde_02_buffer;
    auto __cycle_tilde_02_phasei = this->cycle_tilde_02_phasei;
    Index i;

    for (i = 0; i < (Index)n; i++) {
        {
            UInt32 uint_phase;

            {
                {
                    uint_phase = __cycle_tilde_02_phasei;
                }
            }

            UInt32 idx = (UInt32)(uint32_rshift(uint_phase, 18));
            number frac = ((BinOpInt)((BinOpInt)uint_phase & (BinOpInt)262143)) * 3.81471181759574e-6;
            number y0 = __cycle_tilde_02_buffer[(Index)idx];
            number y1 = __cycle_tilde_02_buffer[(Index)((BinOpInt)(idx + 1) & (BinOpInt)16383)];
            number y = y0 + frac * (y1 - y0);

            {
                UInt32 pincr = (UInt32)(uint32_trunc(frequency * __cycle_tilde_02_f2i));
                __cycle_tilde_02_phasei = uint32_add(__cycle_tilde_02_phasei, pincr);
            }

            out1[(Index)i] = y;
            out2[(Index)i] = uint_phase * 0.232830643653869629e-9;
            continue;
        }
    }

    this->cycle_tilde_02_phasei = __cycle_tilde_02_phasei;
}

void cycle_tilde_03_perform(
    number frequency,
    number phase_offset,
    SampleValue * out1,
    SampleValue * out2,
    Index n
) {
    RNBO_UNUSED(phase_offset);
    auto __cycle_tilde_03_f2i = this->cycle_tilde_03_f2i;
    auto __cycle_tilde_03_buffer = this->cycle_tilde_03_buffer;
    auto __cycle_tilde_03_phasei = this->cycle_tilde_03_phasei;
    Index i;

    for (i = 0; i < (Index)n; i++) {
        {
            UInt32 uint_phase;

            {
                {
                    uint_phase = __cycle_tilde_03_phasei;
                }
            }

            UInt32 idx = (UInt32)(uint32_rshift(uint_phase, 18));
            number frac = ((BinOpInt)((BinOpInt)uint_phase & (BinOpInt)262143)) * 3.81471181759574e-6;
            number y0 = __cycle_tilde_03_buffer[(Index)idx];
            number y1 = __cycle_tilde_03_buffer[(Index)((BinOpInt)(idx + 1) & (BinOpInt)16383)];
            number y = y0 + frac * (y1 - y0);

            {
                UInt32 pincr = (UInt32)(uint32_trunc(frequency * __cycle_tilde_03_f2i));
                __cycle_tilde_03_phasei = uint32_add(__cycle_tilde_03_phasei, pincr);
            }

            out1[(Index)i] = y;
            out2[(Index)i] = uint_phase * 0.232830643653869629e-9;
            continue;
        }
    }

    this->cycle_tilde_03_phasei = __cycle_tilde_03_phasei;
}

void signaladder_01_perform(
    const SampleValue * in1,
    const SampleValue * in2,
    const SampleValue * in3,
    SampleValue * out,
    Index n
) {
    Index i;

    for (i = 0; i < (Index)n; i++) {
        out[(Index)i] = in1[(Index)i] + in2[(Index)i] + in3[(Index)i];
    }
}

void dspexpr_01_perform(const Sample * in1, number in2, SampleValue * out1, Index n) {
    RNBO_UNUSED(in2);
    Index i;

    for (i = 0; i < (Index)n; i++) {
        out1[(Index)i] = in1[(Index)i] * 0.1;//#map:_###_obj_###_:1
    }
}

void stackprotect_perform(Index n) {
    RNBO_UNUSED(n);
    auto __stackprotect_count = this->stackprotect_count;
    __stackprotect_count = 0;
    this->stackprotect_count = __stackprotect_count;
}

void param_01_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_01_value;
}

void param_01_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_01_value_set(preset["value"]);
}

void message_01_init() {
    this->message_01_set_set(listbase<number, 3>{1, 5, 8});
}

void param_02_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_02_value;
}

void param_02_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_02_value_set(preset["value"]);
}

void message_02_init() {
    this->message_02_set_set(listbase<number, 3>{1, 5, 10});
}

void param_03_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_03_value;
}

void param_03_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_03_value_set(preset["value"]);
}

void message_03_init() {
    this->message_03_set_set(listbase<number, 3>{2, 6, 9});
}

void param_04_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_04_value;
}

void param_04_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_04_value_set(preset["value"]);
}

void message_04_init() {
    this->message_04_set_set(listbase<number, 3>{2, 6, 11});
}

void param_05_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_05_value;
}

void param_05_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_05_value_set(preset["value"]);
}

void message_05_init() {
    this->message_05_set_set(listbase<number, 3>{3, 7, 10});
}

void param_06_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_06_value;
}

void param_06_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_06_value_set(preset["value"]);
}

void message_06_init() {
    this->message_06_set_set(listbase<number, 3>{3, 7, 12});
}

void param_07_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_07_value;
}

void param_07_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_07_value_set(preset["value"]);
}

void message_07_init() {
    this->message_07_set_set(listbase<number, 3>{4, 8, 11});
}

void param_08_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_08_value;
}

void param_08_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_08_value_set(preset["value"]);
}

void message_08_init() {
    this->message_08_set_set(listbase<number, 3>{1, 4, 8});
}

void param_09_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_09_value;
}

void param_09_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_09_value_set(preset["value"]);
}

void message_09_init() {
    this->message_09_set_set(listbase<number, 3>{5, 9, 12});
}

void param_10_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_10_value;
}

void param_10_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_10_value_set(preset["value"]);
}

void message_10_init() {
    this->message_10_set_set(listbase<number, 3>{2, 5, 9});
}

void param_11_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_11_value;
}

void param_11_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_11_value_set(preset["value"]);
}

void message_11_init() {
    this->message_11_set_set(listbase<number, 3>{1, 6, 10});
}

void param_12_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_12_value;
}

void param_12_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_12_value_set(preset["value"]);
}

void message_12_init() {
    this->message_12_set_set(listbase<number, 3>{3, 6, 10});
}

number cycle_tilde_01_ph_next(number freq, number reset) {
    {
        {
            if (reset >= 0.)
                this->cycle_tilde_01_ph_currentPhase = reset;
        }
    }

    number pincr = freq * this->cycle_tilde_01_ph_conv;

    if (this->cycle_tilde_01_ph_currentPhase < 0.)
        this->cycle_tilde_01_ph_currentPhase = 1. + this->cycle_tilde_01_ph_currentPhase;

    if (this->cycle_tilde_01_ph_currentPhase > 1.)
        this->cycle_tilde_01_ph_currentPhase = this->cycle_tilde_01_ph_currentPhase - 1.;

    number tmp = this->cycle_tilde_01_ph_currentPhase;
    this->cycle_tilde_01_ph_currentPhase += pincr;
    return tmp;
}

void cycle_tilde_01_ph_reset() {
    this->cycle_tilde_01_ph_currentPhase = 0;
}

void cycle_tilde_01_ph_dspsetup() {
    this->cycle_tilde_01_ph_conv = (number)1 / this->sr;
}

void cycle_tilde_01_dspsetup(bool force) {
    if ((bool)(this->cycle_tilde_01_setupDone) && (bool)(!(bool)(force)))
        return;

    this->cycle_tilde_01_phasei = 0;
    this->cycle_tilde_01_f2i = (number)4294967296 / this->sr;
    this->cycle_tilde_01_wrap = (Int)(this->cycle_tilde_01_buffer->getSize()) - 1;
    this->cycle_tilde_01_setupDone = true;
    this->cycle_tilde_01_ph_dspsetup();
}

void cycle_tilde_01_bufferUpdated() {
    this->cycle_tilde_01_wrap = (Int)(this->cycle_tilde_01_buffer->getSize()) - 1;
}

number mtof_01_innerMtoF_next(number midivalue, number tuning) {
    if (midivalue == this->mtof_01_innerMtoF_lastInValue && tuning == this->mtof_01_innerMtoF_lastTuning)
        return this->mtof_01_innerMtoF_lastOutValue;

    this->mtof_01_innerMtoF_lastInValue = midivalue;
    this->mtof_01_innerMtoF_lastTuning = tuning;
    number result = 0;

    {
        result = rnbo_exp(.057762265 * (midivalue - 69.0));
    }

    this->mtof_01_innerMtoF_lastOutValue = tuning * result;
    return this->mtof_01_innerMtoF_lastOutValue;
}

void mtof_01_innerMtoF_reset() {
    this->mtof_01_innerMtoF_lastInValue = 0;
    this->mtof_01_innerMtoF_lastOutValue = 0;
    this->mtof_01_innerMtoF_lastTuning = 0;
}

void mtof_01_innerScala_mid(Int v) {
    this->mtof_01_innerScala_kbmMid = v;
    this->mtof_01_innerScala_updateRefFreq();
}

void mtof_01_innerScala_ref(Int v) {
    this->mtof_01_innerScala_kbmRefNum = v;
    this->mtof_01_innerScala_updateRefFreq();
}

void mtof_01_innerScala_base(number v) {
    this->mtof_01_innerScala_kbmRefFreq = v;
    this->mtof_01_innerScala_updateRefFreq();
}

void mtof_01_innerScala_init() {
    list sclValid = {
        12,
        100,
        0,
        200,
        0,
        300,
        0,
        400,
        0,
        500,
        0,
        600,
        0,
        700,
        0,
        800,
        0,
        900,
        0,
        1000,
        0,
        1100,
        0,
        2,
        1
    };

    this->mtof_01_innerScala_updateScale(sclValid);
}

template<typename LISTTYPE = list> void mtof_01_innerScala_update(const LISTTYPE& scale, const LISTTYPE& map) {
    if (scale->length > 0) {
        this->mtof_01_innerScala_updateScale(scale);
    }

    if (map->length > 0) {
        this->mtof_01_innerScala_updateMap(map);
    }
}

number mtof_01_innerScala_mtof(number note) {
    if ((bool)(this->mtof_01_innerScala_lastValid) && this->mtof_01_innerScala_lastNote == note) {
        return this->mtof_01_innerScala_lastFreq;
    }

    array<Int, 2> degoct = this->mtof_01_innerScala_applyKBM(note);
    number out = 0;

    if (degoct[1] > 0) {
        out = this->mtof_01_innerScala_applySCL(degoct[0], fract(note), this->mtof_01_innerScala_refFreq);
    }

    this->mtof_01_innerScala_updateLast(note, out);
    return out;
}

number mtof_01_innerScala_ftom(number hz) {
    if (hz <= 0.0) {
        return 0.0;
    }

    if ((bool)(this->mtof_01_innerScala_lastValid) && this->mtof_01_innerScala_lastFreq == hz) {
        return this->mtof_01_innerScala_lastNote;
    }

    array<number, 2> df = this->mtof_01_innerScala_hztodeg(hz);
    Int degree = (Int)(df[0]);
    number frac = df[1];
    number out = 0;

    if (this->mtof_01_innerScala_kbmSize == 0) {
        out = this->mtof_01_innerScala_kbmMid + degree;
    } else {
        array<Int, 2> octdeg = this->mtof_01_innerScala_octdegree(degree, this->mtof_01_innerScala_kbmOctaveDegree);
        number oct = (number)(octdeg[0]);
        Int index = (Int)(octdeg[1]);
        Index entry = 0;

        for (Index i = 0; i < this->mtof_01_innerScala_kbmMapSize; i++) {
            if (index == this->mtof_01_innerScala_kbmValid[(Index)(i + this->mtof_01_innerScala_KBM_MAP_OFFSET)]) {
                entry = i;
                break;
            }
        }

        out = oct * this->mtof_01_innerScala_kbmSize + entry + this->mtof_01_innerScala_kbmMid;
    }

    out = out + frac;
    this->mtof_01_innerScala_updateLast(out, hz);
    return this->mtof_01_innerScala_lastNote;
}

template<typename LISTTYPE = list> Int mtof_01_innerScala_updateScale(const LISTTYPE& scl) {
    if (scl->length < 1) {
        return 0;
    }

    number sclDataEntries = scl[0] * 2 + 1;

    if (sclDataEntries <= scl->length) {
        this->mtof_01_innerScala_lastValid = false;
        this->mtof_01_innerScala_sclExpMul = {};
        number last = 1;

        for (Index i = 1; i < sclDataEntries; i += 2) {
            const number c = (const number)(scl[(Index)(i + 0)]);
            const number d = (const number)(scl[(Index)(i + 1)]);

            if (d <= 0) {
                last = c / (number)1200;
            } else {
                last = rnbo_log2(c / d);
            }

            this->mtof_01_innerScala_sclExpMul->push(last);
        }

        this->mtof_01_innerScala_sclOctaveMul = last;
        this->mtof_01_innerScala_sclEntryCount = (Int)(this->mtof_01_innerScala_sclExpMul->length);

        if (scl->length >= sclDataEntries + 3) {
            this->mtof_01_innerScala_kbmMid = (Int)(scl[(Index)(sclDataEntries + 2)]);
            this->mtof_01_innerScala_kbmRefNum = (Int)(scl[(Index)(sclDataEntries + 1)]);
            this->mtof_01_innerScala_kbmRefFreq = scl[(Index)(sclDataEntries + 0)];
            this->mtof_01_innerScala_kbmSize = (Int)(0);
        }

        this->mtof_01_innerScala_updateRefFreq();
        return 1;
    }

    return 0;
}

template<typename LISTTYPE = list> Int mtof_01_innerScala_updateMap(const LISTTYPE& kbm) {
    list _kbm = kbm;

    if (_kbm->length == 1 && _kbm[0] == 0.0) {
        _kbm = {0.0, 0.0, 0.0, 60.0, 69.0, 440.0};
    }

    if (_kbm->length >= 6 && _kbm[0] >= 0.0) {
        this->mtof_01_innerScala_lastValid = false;
        Index size = (Index)(_kbm[0]);
        Int octave = 12;

        if (_kbm->length > 6) {
            octave = (Int)(_kbm[6]);
        }

        if (size > 0 && _kbm->length < this->mtof_01_innerScala_KBM_MAP_OFFSET) {
            return 0;
        }

        this->mtof_01_innerScala_kbmSize = (Int)(size);
        this->mtof_01_innerScala_kbmMin = (Int)(_kbm[1]);
        this->mtof_01_innerScala_kbmMax = (Int)(_kbm[2]);
        this->mtof_01_innerScala_kbmMid = (Int)(_kbm[3]);
        this->mtof_01_innerScala_kbmRefNum = (Int)(_kbm[4]);
        this->mtof_01_innerScala_kbmRefFreq = _kbm[5];
        this->mtof_01_innerScala_kbmOctaveDegree = octave;
        this->mtof_01_innerScala_kbmValid = _kbm;
        this->mtof_01_innerScala_kbmMapSize = (_kbm->length - this->mtof_01_innerScala_KBM_MAP_OFFSET > _kbm->length ? _kbm->length : (_kbm->length - this->mtof_01_innerScala_KBM_MAP_OFFSET < 0 ? 0 : _kbm->length - this->mtof_01_innerScala_KBM_MAP_OFFSET));
        this->mtof_01_innerScala_updateRefFreq();
        return 1;
    }

    return 0;
}

void mtof_01_innerScala_updateLast(number note, number freq) {
    this->mtof_01_innerScala_lastValid = true;
    this->mtof_01_innerScala_lastNote = note;
    this->mtof_01_innerScala_lastFreq = freq;
}

array<number, 2> mtof_01_innerScala_hztodeg(number hz) {
    number hza = rnbo_abs(hz);

    number octave = rnbo_floor(
        rnbo_log2(hza / this->mtof_01_innerScala_refFreq) / this->mtof_01_innerScala_sclOctaveMul
    );

    Int i = 0;
    number frac = 0;
    number n = 0;

    for (; i < this->mtof_01_innerScala_sclEntryCount; i++) {
        number c = this->mtof_01_innerScala_applySCLOctIndex(octave, i + 0, 0.0, this->mtof_01_innerScala_refFreq);
        n = this->mtof_01_innerScala_applySCLOctIndex(octave, i + 1, 0.0, this->mtof_01_innerScala_refFreq);

        if (c <= hza && hza < n) {
            if (c != hza) {
                frac = rnbo_log2(hza / c) / rnbo_log2(n / c);
            }

            break;
        }
    }

    if (i == this->mtof_01_innerScala_sclEntryCount && n != hza) {
        number c = n;
        n = this->mtof_01_innerScala_applySCLOctIndex(octave + 1, 0, 0.0, this->mtof_01_innerScala_refFreq);
        frac = rnbo_log2(hza / c) / rnbo_log2(n / c);
    }

    number deg = i + octave * this->mtof_01_innerScala_sclEntryCount;

    {
        deg = rnbo_fround((deg + frac) * 1 / (number)1) * 1;
        frac = 0.0;
    }

    return {deg, frac};
}

array<Int, 2> mtof_01_innerScala_octdegree(Int degree, Int count) {
    Int octave = 0;
    Int index = 0;

    if (degree < 0) {
        octave = -(1 + (-1 - degree) / count);
        index = -degree % count;

        if (index > 0) {
            index = count - index;
        }
    } else {
        octave = degree / count;
        index = degree % count;
    }

    return {octave, index};
}

array<Int, 2> mtof_01_innerScala_applyKBM(number note) {
    if ((this->mtof_01_innerScala_kbmMin == this->mtof_01_innerScala_kbmMax && this->mtof_01_innerScala_kbmMax == 0) || (note >= this->mtof_01_innerScala_kbmMin && note <= this->mtof_01_innerScala_kbmMax)) {
        Int degree = (Int)(rnbo_floor(note - this->mtof_01_innerScala_kbmMid));

        if (this->mtof_01_innerScala_kbmSize == 0) {
            return {degree, 1};
        }

        array<Int, 2> octdeg = this->mtof_01_innerScala_octdegree(degree, this->mtof_01_innerScala_kbmSize);
        Int octave = (Int)(octdeg[0]);
        Index index = (Index)(octdeg[1]);

        if (this->mtof_01_innerScala_kbmMapSize > index) {
            degree = (Int)(this->mtof_01_innerScala_kbmValid[(Index)(this->mtof_01_innerScala_KBM_MAP_OFFSET + index)]);

            if (degree >= 0) {
                return {degree + octave * this->mtof_01_innerScala_kbmOctaveDegree, 1};
            }
        }
    }

    return {-1, 0};
}

number mtof_01_innerScala_applySCL(Int degree, number frac, number refFreq) {
    array<Int, 2> octdeg = this->mtof_01_innerScala_octdegree(degree, this->mtof_01_innerScala_sclEntryCount);
    return this->mtof_01_innerScala_applySCLOctIndex(octdeg[0], octdeg[1], frac, refFreq);
}

number mtof_01_innerScala_applySCLOctIndex(number octave, Int index, number frac, number refFreq) {
    number p = 0;

    if (index > 0) {
        p = this->mtof_01_innerScala_sclExpMul[(Index)(index - 1)];
    }

    if (frac > 0) {
        p = this->linearinterp(frac, p, this->mtof_01_innerScala_sclExpMul[(Index)index]);
    } else if (frac < 0) {
        p = this->linearinterp(-frac, this->mtof_01_innerScala_sclExpMul[(Index)index], p);
    }

    return refFreq * rnbo_pow(2, p + octave * this->mtof_01_innerScala_sclOctaveMul);
}

void mtof_01_innerScala_updateRefFreq() {
    this->mtof_01_innerScala_lastValid = false;
    Int refOffset = (Int)(this->mtof_01_innerScala_kbmRefNum - this->mtof_01_innerScala_kbmMid);

    if (refOffset == 0) {
        this->mtof_01_innerScala_refFreq = this->mtof_01_innerScala_kbmRefFreq;
    } else {
        Int base = (Int)(this->mtof_01_innerScala_kbmSize);

        if (base < 1) {
            base = this->mtof_01_innerScala_sclEntryCount;
        }

        array<Int, 2> octdeg = this->mtof_01_innerScala_octdegree(refOffset, base);
        number oct = (number)(octdeg[0]);
        Int index = (Int)(octdeg[1]);

        if (base > 0) {
            oct = oct + rnbo_floor(index / base);
            index = index % base;
        }

        if (index >= 0 && index < this->mtof_01_innerScala_kbmSize) {
            if (index < (Int)(this->mtof_01_innerScala_kbmMapSize)) {
                index = (Int)(this->mtof_01_innerScala_kbmValid[(Index)((Index)(index) + this->mtof_01_innerScala_KBM_MAP_OFFSET)]);
            } else {
                index = -1;
            }
        }

        if (index < 0 || index > (Int)(this->mtof_01_innerScala_sclExpMul->length))
            {} else {
            number p = 0;

            if (index > 0) {
                p = this->mtof_01_innerScala_sclExpMul[(Index)(index - 1)];
            }

            this->mtof_01_innerScala_refFreq = this->mtof_01_innerScala_kbmRefFreq / rnbo_pow(2, p + oct * this->mtof_01_innerScala_sclOctaveMul);
        }
    }
}

void mtof_01_innerScala_reset() {
    this->mtof_01_innerScala_lastValid = false;
    this->mtof_01_innerScala_lastNote = 0;
    this->mtof_01_innerScala_lastFreq = 0;
    this->mtof_01_innerScala_sclEntryCount = 0;
    this->mtof_01_innerScala_sclOctaveMul = 1;
    this->mtof_01_innerScala_sclExpMul = {};
    this->mtof_01_innerScala_kbmValid = {0, 0, 0, 60, 69, 440};
    this->mtof_01_innerScala_kbmMid = 60;
    this->mtof_01_innerScala_kbmRefNum = 69;
    this->mtof_01_innerScala_kbmRefFreq = 440;
    this->mtof_01_innerScala_kbmSize = 0;
    this->mtof_01_innerScala_kbmMin = 0;
    this->mtof_01_innerScala_kbmMax = 0;
    this->mtof_01_innerScala_kbmOctaveDegree = 12;
    this->mtof_01_innerScala_kbmMapSize = 0;
    this->mtof_01_innerScala_refFreq = 261.63;
}

void mtof_01_init() {
    this->mtof_01_innerScala_update(this->mtof_01_scale, this->mtof_01_map);
}

number cycle_tilde_02_ph_next(number freq, number reset) {
    {
        {
            if (reset >= 0.)
                this->cycle_tilde_02_ph_currentPhase = reset;
        }
    }

    number pincr = freq * this->cycle_tilde_02_ph_conv;

    if (this->cycle_tilde_02_ph_currentPhase < 0.)
        this->cycle_tilde_02_ph_currentPhase = 1. + this->cycle_tilde_02_ph_currentPhase;

    if (this->cycle_tilde_02_ph_currentPhase > 1.)
        this->cycle_tilde_02_ph_currentPhase = this->cycle_tilde_02_ph_currentPhase - 1.;

    number tmp = this->cycle_tilde_02_ph_currentPhase;
    this->cycle_tilde_02_ph_currentPhase += pincr;
    return tmp;
}

void cycle_tilde_02_ph_reset() {
    this->cycle_tilde_02_ph_currentPhase = 0;
}

void cycle_tilde_02_ph_dspsetup() {
    this->cycle_tilde_02_ph_conv = (number)1 / this->sr;
}

void cycle_tilde_02_dspsetup(bool force) {
    if ((bool)(this->cycle_tilde_02_setupDone) && (bool)(!(bool)(force)))
        return;

    this->cycle_tilde_02_phasei = 0;
    this->cycle_tilde_02_f2i = (number)4294967296 / this->sr;
    this->cycle_tilde_02_wrap = (Int)(this->cycle_tilde_02_buffer->getSize()) - 1;
    this->cycle_tilde_02_setupDone = true;
    this->cycle_tilde_02_ph_dspsetup();
}

void cycle_tilde_02_bufferUpdated() {
    this->cycle_tilde_02_wrap = (Int)(this->cycle_tilde_02_buffer->getSize()) - 1;
}

number mtof_02_innerMtoF_next(number midivalue, number tuning) {
    if (midivalue == this->mtof_02_innerMtoF_lastInValue && tuning == this->mtof_02_innerMtoF_lastTuning)
        return this->mtof_02_innerMtoF_lastOutValue;

    this->mtof_02_innerMtoF_lastInValue = midivalue;
    this->mtof_02_innerMtoF_lastTuning = tuning;
    number result = 0;

    {
        result = rnbo_exp(.057762265 * (midivalue - 69.0));
    }

    this->mtof_02_innerMtoF_lastOutValue = tuning * result;
    return this->mtof_02_innerMtoF_lastOutValue;
}

void mtof_02_innerMtoF_reset() {
    this->mtof_02_innerMtoF_lastInValue = 0;
    this->mtof_02_innerMtoF_lastOutValue = 0;
    this->mtof_02_innerMtoF_lastTuning = 0;
}

void mtof_02_innerScala_mid(Int v) {
    this->mtof_02_innerScala_kbmMid = v;
    this->mtof_02_innerScala_updateRefFreq();
}

void mtof_02_innerScala_ref(Int v) {
    this->mtof_02_innerScala_kbmRefNum = v;
    this->mtof_02_innerScala_updateRefFreq();
}

void mtof_02_innerScala_base(number v) {
    this->mtof_02_innerScala_kbmRefFreq = v;
    this->mtof_02_innerScala_updateRefFreq();
}

void mtof_02_innerScala_init() {
    list sclValid = {
        12,
        100,
        0,
        200,
        0,
        300,
        0,
        400,
        0,
        500,
        0,
        600,
        0,
        700,
        0,
        800,
        0,
        900,
        0,
        1000,
        0,
        1100,
        0,
        2,
        1
    };

    this->mtof_02_innerScala_updateScale(sclValid);
}

template<typename LISTTYPE = list> void mtof_02_innerScala_update(const LISTTYPE& scale, const LISTTYPE& map) {
    if (scale->length > 0) {
        this->mtof_02_innerScala_updateScale(scale);
    }

    if (map->length > 0) {
        this->mtof_02_innerScala_updateMap(map);
    }
}

number mtof_02_innerScala_mtof(number note) {
    if ((bool)(this->mtof_02_innerScala_lastValid) && this->mtof_02_innerScala_lastNote == note) {
        return this->mtof_02_innerScala_lastFreq;
    }

    array<Int, 2> degoct = this->mtof_02_innerScala_applyKBM(note);
    number out = 0;

    if (degoct[1] > 0) {
        out = this->mtof_02_innerScala_applySCL(degoct[0], fract(note), this->mtof_02_innerScala_refFreq);
    }

    this->mtof_02_innerScala_updateLast(note, out);
    return out;
}

number mtof_02_innerScala_ftom(number hz) {
    if (hz <= 0.0) {
        return 0.0;
    }

    if ((bool)(this->mtof_02_innerScala_lastValid) && this->mtof_02_innerScala_lastFreq == hz) {
        return this->mtof_02_innerScala_lastNote;
    }

    array<number, 2> df = this->mtof_02_innerScala_hztodeg(hz);
    Int degree = (Int)(df[0]);
    number frac = df[1];
    number out = 0;

    if (this->mtof_02_innerScala_kbmSize == 0) {
        out = this->mtof_02_innerScala_kbmMid + degree;
    } else {
        array<Int, 2> octdeg = this->mtof_02_innerScala_octdegree(degree, this->mtof_02_innerScala_kbmOctaveDegree);
        number oct = (number)(octdeg[0]);
        Int index = (Int)(octdeg[1]);
        Index entry = 0;

        for (Index i = 0; i < this->mtof_02_innerScala_kbmMapSize; i++) {
            if (index == this->mtof_02_innerScala_kbmValid[(Index)(i + this->mtof_02_innerScala_KBM_MAP_OFFSET)]) {
                entry = i;
                break;
            }
        }

        out = oct * this->mtof_02_innerScala_kbmSize + entry + this->mtof_02_innerScala_kbmMid;
    }

    out = out + frac;
    this->mtof_02_innerScala_updateLast(out, hz);
    return this->mtof_02_innerScala_lastNote;
}

template<typename LISTTYPE = list> Int mtof_02_innerScala_updateScale(const LISTTYPE& scl) {
    if (scl->length < 1) {
        return 0;
    }

    number sclDataEntries = scl[0] * 2 + 1;

    if (sclDataEntries <= scl->length) {
        this->mtof_02_innerScala_lastValid = false;
        this->mtof_02_innerScala_sclExpMul = {};
        number last = 1;

        for (Index i = 1; i < sclDataEntries; i += 2) {
            const number c = (const number)(scl[(Index)(i + 0)]);
            const number d = (const number)(scl[(Index)(i + 1)]);

            if (d <= 0) {
                last = c / (number)1200;
            } else {
                last = rnbo_log2(c / d);
            }

            this->mtof_02_innerScala_sclExpMul->push(last);
        }

        this->mtof_02_innerScala_sclOctaveMul = last;
        this->mtof_02_innerScala_sclEntryCount = (Int)(this->mtof_02_innerScala_sclExpMul->length);

        if (scl->length >= sclDataEntries + 3) {
            this->mtof_02_innerScala_kbmMid = (Int)(scl[(Index)(sclDataEntries + 2)]);
            this->mtof_02_innerScala_kbmRefNum = (Int)(scl[(Index)(sclDataEntries + 1)]);
            this->mtof_02_innerScala_kbmRefFreq = scl[(Index)(sclDataEntries + 0)];
            this->mtof_02_innerScala_kbmSize = (Int)(0);
        }

        this->mtof_02_innerScala_updateRefFreq();
        return 1;
    }

    return 0;
}

template<typename LISTTYPE = list> Int mtof_02_innerScala_updateMap(const LISTTYPE& kbm) {
    list _kbm = kbm;

    if (_kbm->length == 1 && _kbm[0] == 0.0) {
        _kbm = {0.0, 0.0, 0.0, 60.0, 69.0, 440.0};
    }

    if (_kbm->length >= 6 && _kbm[0] >= 0.0) {
        this->mtof_02_innerScala_lastValid = false;
        Index size = (Index)(_kbm[0]);
        Int octave = 12;

        if (_kbm->length > 6) {
            octave = (Int)(_kbm[6]);
        }

        if (size > 0 && _kbm->length < this->mtof_02_innerScala_KBM_MAP_OFFSET) {
            return 0;
        }

        this->mtof_02_innerScala_kbmSize = (Int)(size);
        this->mtof_02_innerScala_kbmMin = (Int)(_kbm[1]);
        this->mtof_02_innerScala_kbmMax = (Int)(_kbm[2]);
        this->mtof_02_innerScala_kbmMid = (Int)(_kbm[3]);
        this->mtof_02_innerScala_kbmRefNum = (Int)(_kbm[4]);
        this->mtof_02_innerScala_kbmRefFreq = _kbm[5];
        this->mtof_02_innerScala_kbmOctaveDegree = octave;
        this->mtof_02_innerScala_kbmValid = _kbm;
        this->mtof_02_innerScala_kbmMapSize = (_kbm->length - this->mtof_02_innerScala_KBM_MAP_OFFSET > _kbm->length ? _kbm->length : (_kbm->length - this->mtof_02_innerScala_KBM_MAP_OFFSET < 0 ? 0 : _kbm->length - this->mtof_02_innerScala_KBM_MAP_OFFSET));
        this->mtof_02_innerScala_updateRefFreq();
        return 1;
    }

    return 0;
}

void mtof_02_innerScala_updateLast(number note, number freq) {
    this->mtof_02_innerScala_lastValid = true;
    this->mtof_02_innerScala_lastNote = note;
    this->mtof_02_innerScala_lastFreq = freq;
}

array<number, 2> mtof_02_innerScala_hztodeg(number hz) {
    number hza = rnbo_abs(hz);

    number octave = rnbo_floor(
        rnbo_log2(hza / this->mtof_02_innerScala_refFreq) / this->mtof_02_innerScala_sclOctaveMul
    );

    Int i = 0;
    number frac = 0;
    number n = 0;

    for (; i < this->mtof_02_innerScala_sclEntryCount; i++) {
        number c = this->mtof_02_innerScala_applySCLOctIndex(octave, i + 0, 0.0, this->mtof_02_innerScala_refFreq);
        n = this->mtof_02_innerScala_applySCLOctIndex(octave, i + 1, 0.0, this->mtof_02_innerScala_refFreq);

        if (c <= hza && hza < n) {
            if (c != hza) {
                frac = rnbo_log2(hza / c) / rnbo_log2(n / c);
            }

            break;
        }
    }

    if (i == this->mtof_02_innerScala_sclEntryCount && n != hza) {
        number c = n;
        n = this->mtof_02_innerScala_applySCLOctIndex(octave + 1, 0, 0.0, this->mtof_02_innerScala_refFreq);
        frac = rnbo_log2(hza / c) / rnbo_log2(n / c);
    }

    number deg = i + octave * this->mtof_02_innerScala_sclEntryCount;

    {
        deg = rnbo_fround((deg + frac) * 1 / (number)1) * 1;
        frac = 0.0;
    }

    return {deg, frac};
}

array<Int, 2> mtof_02_innerScala_octdegree(Int degree, Int count) {
    Int octave = 0;
    Int index = 0;

    if (degree < 0) {
        octave = -(1 + (-1 - degree) / count);
        index = -degree % count;

        if (index > 0) {
            index = count - index;
        }
    } else {
        octave = degree / count;
        index = degree % count;
    }

    return {octave, index};
}

array<Int, 2> mtof_02_innerScala_applyKBM(number note) {
    if ((this->mtof_02_innerScala_kbmMin == this->mtof_02_innerScala_kbmMax && this->mtof_02_innerScala_kbmMax == 0) || (note >= this->mtof_02_innerScala_kbmMin && note <= this->mtof_02_innerScala_kbmMax)) {
        Int degree = (Int)(rnbo_floor(note - this->mtof_02_innerScala_kbmMid));

        if (this->mtof_02_innerScala_kbmSize == 0) {
            return {degree, 1};
        }

        array<Int, 2> octdeg = this->mtof_02_innerScala_octdegree(degree, this->mtof_02_innerScala_kbmSize);
        Int octave = (Int)(octdeg[0]);
        Index index = (Index)(octdeg[1]);

        if (this->mtof_02_innerScala_kbmMapSize > index) {
            degree = (Int)(this->mtof_02_innerScala_kbmValid[(Index)(this->mtof_02_innerScala_KBM_MAP_OFFSET + index)]);

            if (degree >= 0) {
                return {degree + octave * this->mtof_02_innerScala_kbmOctaveDegree, 1};
            }
        }
    }

    return {-1, 0};
}

number mtof_02_innerScala_applySCL(Int degree, number frac, number refFreq) {
    array<Int, 2> octdeg = this->mtof_02_innerScala_octdegree(degree, this->mtof_02_innerScala_sclEntryCount);
    return this->mtof_02_innerScala_applySCLOctIndex(octdeg[0], octdeg[1], frac, refFreq);
}

number mtof_02_innerScala_applySCLOctIndex(number octave, Int index, number frac, number refFreq) {
    number p = 0;

    if (index > 0) {
        p = this->mtof_02_innerScala_sclExpMul[(Index)(index - 1)];
    }

    if (frac > 0) {
        p = this->linearinterp(frac, p, this->mtof_02_innerScala_sclExpMul[(Index)index]);
    } else if (frac < 0) {
        p = this->linearinterp(-frac, this->mtof_02_innerScala_sclExpMul[(Index)index], p);
    }

    return refFreq * rnbo_pow(2, p + octave * this->mtof_02_innerScala_sclOctaveMul);
}

void mtof_02_innerScala_updateRefFreq() {
    this->mtof_02_innerScala_lastValid = false;
    Int refOffset = (Int)(this->mtof_02_innerScala_kbmRefNum - this->mtof_02_innerScala_kbmMid);

    if (refOffset == 0) {
        this->mtof_02_innerScala_refFreq = this->mtof_02_innerScala_kbmRefFreq;
    } else {
        Int base = (Int)(this->mtof_02_innerScala_kbmSize);

        if (base < 1) {
            base = this->mtof_02_innerScala_sclEntryCount;
        }

        array<Int, 2> octdeg = this->mtof_02_innerScala_octdegree(refOffset, base);
        number oct = (number)(octdeg[0]);
        Int index = (Int)(octdeg[1]);

        if (base > 0) {
            oct = oct + rnbo_floor(index / base);
            index = index % base;
        }

        if (index >= 0 && index < this->mtof_02_innerScala_kbmSize) {
            if (index < (Int)(this->mtof_02_innerScala_kbmMapSize)) {
                index = (Int)(this->mtof_02_innerScala_kbmValid[(Index)((Index)(index) + this->mtof_02_innerScala_KBM_MAP_OFFSET)]);
            } else {
                index = -1;
            }
        }

        if (index < 0 || index > (Int)(this->mtof_02_innerScala_sclExpMul->length))
            {} else {
            number p = 0;

            if (index > 0) {
                p = this->mtof_02_innerScala_sclExpMul[(Index)(index - 1)];
            }

            this->mtof_02_innerScala_refFreq = this->mtof_02_innerScala_kbmRefFreq / rnbo_pow(2, p + oct * this->mtof_02_innerScala_sclOctaveMul);
        }
    }
}

void mtof_02_innerScala_reset() {
    this->mtof_02_innerScala_lastValid = false;
    this->mtof_02_innerScala_lastNote = 0;
    this->mtof_02_innerScala_lastFreq = 0;
    this->mtof_02_innerScala_sclEntryCount = 0;
    this->mtof_02_innerScala_sclOctaveMul = 1;
    this->mtof_02_innerScala_sclExpMul = {};
    this->mtof_02_innerScala_kbmValid = {0, 0, 0, 60, 69, 440};
    this->mtof_02_innerScala_kbmMid = 60;
    this->mtof_02_innerScala_kbmRefNum = 69;
    this->mtof_02_innerScala_kbmRefFreq = 440;
    this->mtof_02_innerScala_kbmSize = 0;
    this->mtof_02_innerScala_kbmMin = 0;
    this->mtof_02_innerScala_kbmMax = 0;
    this->mtof_02_innerScala_kbmOctaveDegree = 12;
    this->mtof_02_innerScala_kbmMapSize = 0;
    this->mtof_02_innerScala_refFreq = 261.63;
}

void mtof_02_init() {
    this->mtof_02_innerScala_update(this->mtof_02_scale, this->mtof_02_map);
}

void param_13_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_13_value;
}

void param_13_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_13_value_set(preset["value"]);
}

void message_13_init() {
    this->message_13_set_set(listbase<number, 3>{2, 7, 11});
}

void param_14_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_14_value;
}

void param_14_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_14_value_set(preset["value"]);
}

void message_14_init() {
    this->message_14_set_set(listbase<number, 3>{4, 7, 11});
}

number cycle_tilde_03_ph_next(number freq, number reset) {
    {
        {
            if (reset >= 0.)
                this->cycle_tilde_03_ph_currentPhase = reset;
        }
    }

    number pincr = freq * this->cycle_tilde_03_ph_conv;

    if (this->cycle_tilde_03_ph_currentPhase < 0.)
        this->cycle_tilde_03_ph_currentPhase = 1. + this->cycle_tilde_03_ph_currentPhase;

    if (this->cycle_tilde_03_ph_currentPhase > 1.)
        this->cycle_tilde_03_ph_currentPhase = this->cycle_tilde_03_ph_currentPhase - 1.;

    number tmp = this->cycle_tilde_03_ph_currentPhase;
    this->cycle_tilde_03_ph_currentPhase += pincr;
    return tmp;
}

void cycle_tilde_03_ph_reset() {
    this->cycle_tilde_03_ph_currentPhase = 0;
}

void cycle_tilde_03_ph_dspsetup() {
    this->cycle_tilde_03_ph_conv = (number)1 / this->sr;
}

void cycle_tilde_03_dspsetup(bool force) {
    if ((bool)(this->cycle_tilde_03_setupDone) && (bool)(!(bool)(force)))
        return;

    this->cycle_tilde_03_phasei = 0;
    this->cycle_tilde_03_f2i = (number)4294967296 / this->sr;
    this->cycle_tilde_03_wrap = (Int)(this->cycle_tilde_03_buffer->getSize()) - 1;
    this->cycle_tilde_03_setupDone = true;
    this->cycle_tilde_03_ph_dspsetup();
}

void cycle_tilde_03_bufferUpdated() {
    this->cycle_tilde_03_wrap = (Int)(this->cycle_tilde_03_buffer->getSize()) - 1;
}

number mtof_03_innerMtoF_next(number midivalue, number tuning) {
    if (midivalue == this->mtof_03_innerMtoF_lastInValue && tuning == this->mtof_03_innerMtoF_lastTuning)
        return this->mtof_03_innerMtoF_lastOutValue;

    this->mtof_03_innerMtoF_lastInValue = midivalue;
    this->mtof_03_innerMtoF_lastTuning = tuning;
    number result = 0;

    {
        result = rnbo_exp(.057762265 * (midivalue - 69.0));
    }

    this->mtof_03_innerMtoF_lastOutValue = tuning * result;
    return this->mtof_03_innerMtoF_lastOutValue;
}

void mtof_03_innerMtoF_reset() {
    this->mtof_03_innerMtoF_lastInValue = 0;
    this->mtof_03_innerMtoF_lastOutValue = 0;
    this->mtof_03_innerMtoF_lastTuning = 0;
}

void mtof_03_innerScala_mid(Int v) {
    this->mtof_03_innerScala_kbmMid = v;
    this->mtof_03_innerScala_updateRefFreq();
}

void mtof_03_innerScala_ref(Int v) {
    this->mtof_03_innerScala_kbmRefNum = v;
    this->mtof_03_innerScala_updateRefFreq();
}

void mtof_03_innerScala_base(number v) {
    this->mtof_03_innerScala_kbmRefFreq = v;
    this->mtof_03_innerScala_updateRefFreq();
}

void mtof_03_innerScala_init() {
    list sclValid = {
        12,
        100,
        0,
        200,
        0,
        300,
        0,
        400,
        0,
        500,
        0,
        600,
        0,
        700,
        0,
        800,
        0,
        900,
        0,
        1000,
        0,
        1100,
        0,
        2,
        1
    };

    this->mtof_03_innerScala_updateScale(sclValid);
}

template<typename LISTTYPE = list> void mtof_03_innerScala_update(const LISTTYPE& scale, const LISTTYPE& map) {
    if (scale->length > 0) {
        this->mtof_03_innerScala_updateScale(scale);
    }

    if (map->length > 0) {
        this->mtof_03_innerScala_updateMap(map);
    }
}

number mtof_03_innerScala_mtof(number note) {
    if ((bool)(this->mtof_03_innerScala_lastValid) && this->mtof_03_innerScala_lastNote == note) {
        return this->mtof_03_innerScala_lastFreq;
    }

    array<Int, 2> degoct = this->mtof_03_innerScala_applyKBM(note);
    number out = 0;

    if (degoct[1] > 0) {
        out = this->mtof_03_innerScala_applySCL(degoct[0], fract(note), this->mtof_03_innerScala_refFreq);
    }

    this->mtof_03_innerScala_updateLast(note, out);
    return out;
}

number mtof_03_innerScala_ftom(number hz) {
    if (hz <= 0.0) {
        return 0.0;
    }

    if ((bool)(this->mtof_03_innerScala_lastValid) && this->mtof_03_innerScala_lastFreq == hz) {
        return this->mtof_03_innerScala_lastNote;
    }

    array<number, 2> df = this->mtof_03_innerScala_hztodeg(hz);
    Int degree = (Int)(df[0]);
    number frac = df[1];
    number out = 0;

    if (this->mtof_03_innerScala_kbmSize == 0) {
        out = this->mtof_03_innerScala_kbmMid + degree;
    } else {
        array<Int, 2> octdeg = this->mtof_03_innerScala_octdegree(degree, this->mtof_03_innerScala_kbmOctaveDegree);
        number oct = (number)(octdeg[0]);
        Int index = (Int)(octdeg[1]);
        Index entry = 0;

        for (Index i = 0; i < this->mtof_03_innerScala_kbmMapSize; i++) {
            if (index == this->mtof_03_innerScala_kbmValid[(Index)(i + this->mtof_03_innerScala_KBM_MAP_OFFSET)]) {
                entry = i;
                break;
            }
        }

        out = oct * this->mtof_03_innerScala_kbmSize + entry + this->mtof_03_innerScala_kbmMid;
    }

    out = out + frac;
    this->mtof_03_innerScala_updateLast(out, hz);
    return this->mtof_03_innerScala_lastNote;
}

template<typename LISTTYPE = list> Int mtof_03_innerScala_updateScale(const LISTTYPE& scl) {
    if (scl->length < 1) {
        return 0;
    }

    number sclDataEntries = scl[0] * 2 + 1;

    if (sclDataEntries <= scl->length) {
        this->mtof_03_innerScala_lastValid = false;
        this->mtof_03_innerScala_sclExpMul = {};
        number last = 1;

        for (Index i = 1; i < sclDataEntries; i += 2) {
            const number c = (const number)(scl[(Index)(i + 0)]);
            const number d = (const number)(scl[(Index)(i + 1)]);

            if (d <= 0) {
                last = c / (number)1200;
            } else {
                last = rnbo_log2(c / d);
            }

            this->mtof_03_innerScala_sclExpMul->push(last);
        }

        this->mtof_03_innerScala_sclOctaveMul = last;
        this->mtof_03_innerScala_sclEntryCount = (Int)(this->mtof_03_innerScala_sclExpMul->length);

        if (scl->length >= sclDataEntries + 3) {
            this->mtof_03_innerScala_kbmMid = (Int)(scl[(Index)(sclDataEntries + 2)]);
            this->mtof_03_innerScala_kbmRefNum = (Int)(scl[(Index)(sclDataEntries + 1)]);
            this->mtof_03_innerScala_kbmRefFreq = scl[(Index)(sclDataEntries + 0)];
            this->mtof_03_innerScala_kbmSize = (Int)(0);
        }

        this->mtof_03_innerScala_updateRefFreq();
        return 1;
    }

    return 0;
}

template<typename LISTTYPE = list> Int mtof_03_innerScala_updateMap(const LISTTYPE& kbm) {
    list _kbm = kbm;

    if (_kbm->length == 1 && _kbm[0] == 0.0) {
        _kbm = {0.0, 0.0, 0.0, 60.0, 69.0, 440.0};
    }

    if (_kbm->length >= 6 && _kbm[0] >= 0.0) {
        this->mtof_03_innerScala_lastValid = false;
        Index size = (Index)(_kbm[0]);
        Int octave = 12;

        if (_kbm->length > 6) {
            octave = (Int)(_kbm[6]);
        }

        if (size > 0 && _kbm->length < this->mtof_03_innerScala_KBM_MAP_OFFSET) {
            return 0;
        }

        this->mtof_03_innerScala_kbmSize = (Int)(size);
        this->mtof_03_innerScala_kbmMin = (Int)(_kbm[1]);
        this->mtof_03_innerScala_kbmMax = (Int)(_kbm[2]);
        this->mtof_03_innerScala_kbmMid = (Int)(_kbm[3]);
        this->mtof_03_innerScala_kbmRefNum = (Int)(_kbm[4]);
        this->mtof_03_innerScala_kbmRefFreq = _kbm[5];
        this->mtof_03_innerScala_kbmOctaveDegree = octave;
        this->mtof_03_innerScala_kbmValid = _kbm;
        this->mtof_03_innerScala_kbmMapSize = (_kbm->length - this->mtof_03_innerScala_KBM_MAP_OFFSET > _kbm->length ? _kbm->length : (_kbm->length - this->mtof_03_innerScala_KBM_MAP_OFFSET < 0 ? 0 : _kbm->length - this->mtof_03_innerScala_KBM_MAP_OFFSET));
        this->mtof_03_innerScala_updateRefFreq();
        return 1;
    }

    return 0;
}

void mtof_03_innerScala_updateLast(number note, number freq) {
    this->mtof_03_innerScala_lastValid = true;
    this->mtof_03_innerScala_lastNote = note;
    this->mtof_03_innerScala_lastFreq = freq;
}

array<number, 2> mtof_03_innerScala_hztodeg(number hz) {
    number hza = rnbo_abs(hz);

    number octave = rnbo_floor(
        rnbo_log2(hza / this->mtof_03_innerScala_refFreq) / this->mtof_03_innerScala_sclOctaveMul
    );

    Int i = 0;
    number frac = 0;
    number n = 0;

    for (; i < this->mtof_03_innerScala_sclEntryCount; i++) {
        number c = this->mtof_03_innerScala_applySCLOctIndex(octave, i + 0, 0.0, this->mtof_03_innerScala_refFreq);
        n = this->mtof_03_innerScala_applySCLOctIndex(octave, i + 1, 0.0, this->mtof_03_innerScala_refFreq);

        if (c <= hza && hza < n) {
            if (c != hza) {
                frac = rnbo_log2(hza / c) / rnbo_log2(n / c);
            }

            break;
        }
    }

    if (i == this->mtof_03_innerScala_sclEntryCount && n != hza) {
        number c = n;
        n = this->mtof_03_innerScala_applySCLOctIndex(octave + 1, 0, 0.0, this->mtof_03_innerScala_refFreq);
        frac = rnbo_log2(hza / c) / rnbo_log2(n / c);
    }

    number deg = i + octave * this->mtof_03_innerScala_sclEntryCount;

    {
        deg = rnbo_fround((deg + frac) * 1 / (number)1) * 1;
        frac = 0.0;
    }

    return {deg, frac};
}

array<Int, 2> mtof_03_innerScala_octdegree(Int degree, Int count) {
    Int octave = 0;
    Int index = 0;

    if (degree < 0) {
        octave = -(1 + (-1 - degree) / count);
        index = -degree % count;

        if (index > 0) {
            index = count - index;
        }
    } else {
        octave = degree / count;
        index = degree % count;
    }

    return {octave, index};
}

array<Int, 2> mtof_03_innerScala_applyKBM(number note) {
    if ((this->mtof_03_innerScala_kbmMin == this->mtof_03_innerScala_kbmMax && this->mtof_03_innerScala_kbmMax == 0) || (note >= this->mtof_03_innerScala_kbmMin && note <= this->mtof_03_innerScala_kbmMax)) {
        Int degree = (Int)(rnbo_floor(note - this->mtof_03_innerScala_kbmMid));

        if (this->mtof_03_innerScala_kbmSize == 0) {
            return {degree, 1};
        }

        array<Int, 2> octdeg = this->mtof_03_innerScala_octdegree(degree, this->mtof_03_innerScala_kbmSize);
        Int octave = (Int)(octdeg[0]);
        Index index = (Index)(octdeg[1]);

        if (this->mtof_03_innerScala_kbmMapSize > index) {
            degree = (Int)(this->mtof_03_innerScala_kbmValid[(Index)(this->mtof_03_innerScala_KBM_MAP_OFFSET + index)]);

            if (degree >= 0) {
                return {degree + octave * this->mtof_03_innerScala_kbmOctaveDegree, 1};
            }
        }
    }

    return {-1, 0};
}

number mtof_03_innerScala_applySCL(Int degree, number frac, number refFreq) {
    array<Int, 2> octdeg = this->mtof_03_innerScala_octdegree(degree, this->mtof_03_innerScala_sclEntryCount);
    return this->mtof_03_innerScala_applySCLOctIndex(octdeg[0], octdeg[1], frac, refFreq);
}

number mtof_03_innerScala_applySCLOctIndex(number octave, Int index, number frac, number refFreq) {
    number p = 0;

    if (index > 0) {
        p = this->mtof_03_innerScala_sclExpMul[(Index)(index - 1)];
    }

    if (frac > 0) {
        p = this->linearinterp(frac, p, this->mtof_03_innerScala_sclExpMul[(Index)index]);
    } else if (frac < 0) {
        p = this->linearinterp(-frac, this->mtof_03_innerScala_sclExpMul[(Index)index], p);
    }

    return refFreq * rnbo_pow(2, p + octave * this->mtof_03_innerScala_sclOctaveMul);
}

void mtof_03_innerScala_updateRefFreq() {
    this->mtof_03_innerScala_lastValid = false;
    Int refOffset = (Int)(this->mtof_03_innerScala_kbmRefNum - this->mtof_03_innerScala_kbmMid);

    if (refOffset == 0) {
        this->mtof_03_innerScala_refFreq = this->mtof_03_innerScala_kbmRefFreq;
    } else {
        Int base = (Int)(this->mtof_03_innerScala_kbmSize);

        if (base < 1) {
            base = this->mtof_03_innerScala_sclEntryCount;
        }

        array<Int, 2> octdeg = this->mtof_03_innerScala_octdegree(refOffset, base);
        number oct = (number)(octdeg[0]);
        Int index = (Int)(octdeg[1]);

        if (base > 0) {
            oct = oct + rnbo_floor(index / base);
            index = index % base;
        }

        if (index >= 0 && index < this->mtof_03_innerScala_kbmSize) {
            if (index < (Int)(this->mtof_03_innerScala_kbmMapSize)) {
                index = (Int)(this->mtof_03_innerScala_kbmValid[(Index)((Index)(index) + this->mtof_03_innerScala_KBM_MAP_OFFSET)]);
            } else {
                index = -1;
            }
        }

        if (index < 0 || index > (Int)(this->mtof_03_innerScala_sclExpMul->length))
            {} else {
            number p = 0;

            if (index > 0) {
                p = this->mtof_03_innerScala_sclExpMul[(Index)(index - 1)];
            }

            this->mtof_03_innerScala_refFreq = this->mtof_03_innerScala_kbmRefFreq / rnbo_pow(2, p + oct * this->mtof_03_innerScala_sclOctaveMul);
        }
    }
}

void mtof_03_innerScala_reset() {
    this->mtof_03_innerScala_lastValid = false;
    this->mtof_03_innerScala_lastNote = 0;
    this->mtof_03_innerScala_lastFreq = 0;
    this->mtof_03_innerScala_sclEntryCount = 0;
    this->mtof_03_innerScala_sclOctaveMul = 1;
    this->mtof_03_innerScala_sclExpMul = {};
    this->mtof_03_innerScala_kbmValid = {0, 0, 0, 60, 69, 440};
    this->mtof_03_innerScala_kbmMid = 60;
    this->mtof_03_innerScala_kbmRefNum = 69;
    this->mtof_03_innerScala_kbmRefFreq = 440;
    this->mtof_03_innerScala_kbmSize = 0;
    this->mtof_03_innerScala_kbmMin = 0;
    this->mtof_03_innerScala_kbmMax = 0;
    this->mtof_03_innerScala_kbmOctaveDegree = 12;
    this->mtof_03_innerScala_kbmMapSize = 0;
    this->mtof_03_innerScala_refFreq = 261.63;
}

void mtof_03_init() {
    this->mtof_03_innerScala_update(this->mtof_03_scale, this->mtof_03_map);
}

void param_15_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_15_value;
}

void param_15_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_15_value_set(preset["value"]);
}

void message_15_init() {
    this->message_15_set_set(listbase<number, 3>{3, 8, 12});
}

void param_16_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_16_value;
}

void param_16_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_16_value_set(preset["value"]);
}

void message_16_init() {
    this->message_16_set_set(listbase<number, 3>{5, 8, 12});
}

void param_17_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_17_value;
}

void param_17_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_17_value_set(preset["value"]);
}

void message_17_init() {
    this->message_17_set_set(listbase<number, 3>{1, 4, 9});
}

void param_18_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_18_value;
}

void param_18_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_18_value_set(preset["value"]);
}

void message_18_init() {
    this->message_18_set_set(listbase<number, 3>{1, 6, 9});
}

void message_19_init() {
    this->message_19_set_set(listbase<number, 3>{2, 7, 10});
}

void param_19_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_19_value;
}

void param_19_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_19_value_set(preset["value"]);
}

void message_20_init() {
    this->message_20_set_set(listbase<number, 3>{2, 5, 10});
}

void param_20_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_20_value;
}

void param_20_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_20_value_set(preset["value"]);
}

void param_21_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_21_value;
}

void param_21_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_21_value_set(preset["value"]);
}

void message_21_init() {
    this->message_21_set_set(listbase<number, 3>{3, 6, 11});
}

void param_22_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_22_value;
}

void param_22_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_22_value_set(preset["value"]);
}

void message_22_init() {
    this->message_22_set_set(listbase<number, 3>{3, 8, 11});
}

void param_23_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_23_value;
}

void param_23_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_23_value_set(preset["value"]);
}

void message_23_init() {
    this->message_23_set_set(listbase<number, 3>{4, 7, 12});
}

void param_24_getPresetValue(PatcherStateInterface& preset) {
    preset["value"] = this->param_24_value;
}

void param_24_setPresetValue(PatcherStateInterface& preset) {
    if ((bool)(stateIsEmpty(preset)))
        return;

    this->param_24_value_set(preset["value"]);
}

void message_24_init() {
    this->message_24_set_set(listbase<number, 3>{4, 9, 12});
}

void globaltransport_advance() {}

void globaltransport_dspsetup(bool ) {}

bool stackprotect_check() {
    this->stackprotect_count++;

    if (this->stackprotect_count > 128) {
        console->log("STACK OVERFLOW DETECTED - stopped processing branch !");
        return true;
    }

    return false;
}

Index getPatcherSerial() const {
    return 0;
}

void sendParameter(ParameterIndex index, bool ignoreValue) {
    this->getEngine()->notifyParameterValueChanged(index, (ignoreValue ? 0 : this->getParameterValue(index)), ignoreValue);
}

void scheduleParamInit(ParameterIndex index, Index order) {
    this->paramInitIndices->push(index);
    this->paramInitOrder->push(order);
}

void processParamInitEvents() {
    this->listquicksort(
        this->paramInitOrder,
        this->paramInitIndices,
        0,
        (int)(this->paramInitOrder->length - 1),
        true
    );

    for (Index i = 0; i < this->paramInitOrder->length; i++) {
        this->getEngine()->scheduleParameterBang(this->paramInitIndices[i], 0);
    }
}

void updateTime(MillisecondTime time, EXTERNALENGINE* engine, bool inProcess = false) {
    RNBO_UNUSED(inProcess);
    RNBO_UNUSED(engine);
    this->_currentTime = time;
    auto offset = rnbo_fround(this->msToSamps(time - this->getEngine()->getCurrentTime(), this->sr));

    if (offset >= (SampleIndex)(this->vs))
        offset = (SampleIndex)(this->vs) - 1;

    if (offset < 0)
        offset = 0;

    this->sampleOffsetIntoNextAudioBuffer = (Index)(offset);
}

void assign_defaults()
{
    param_01_value = 0;
    param_02_value = 0;
    param_03_value = 0;
    param_04_value = 0;
    param_05_value = 0;
    param_06_value = 0;
    param_07_value = 0;
    param_08_value = 0;
    param_09_value = 0;
    param_10_value = 0;
    param_11_value = 0;
    unpack_01_out1 = 0;
    unpack_01_out2 = 0;
    unpack_01_out3 = 0;
    expr_01_in1 = 0;
    expr_01_in2 = 59;
    expr_01_out1 = 0;
    param_12_value = 0;
    cycle_tilde_01_frequency = 0;
    cycle_tilde_01_phase_offset = 0;
    mtof_01_base = 440;
    dspexpr_01_in1 = 0;
    dspexpr_01_in2 = 0.1;
    expr_02_in1 = 0;
    expr_02_in2 = 59;
    expr_02_out1 = 0;
    cycle_tilde_02_frequency = 0;
    cycle_tilde_02_phase_offset = 0;
    mtof_02_base = 440;
    param_13_value = 0;
    param_14_value = 0;
    expr_03_in1 = 0;
    expr_03_in2 = 59;
    expr_03_out1 = 0;
    cycle_tilde_03_frequency = 0;
    cycle_tilde_03_phase_offset = 0;
    mtof_03_base = 440;
    param_15_value = 0;
    param_16_value = 0;
    param_17_value = 0;
    param_18_value = 0;
    param_19_value = 0;
    param_20_value = 0;
    param_21_value = 0;
    param_22_value = 0;
    param_23_value = 0;
    param_24_value = 0;
    _currentTime = 0;
    audioProcessSampleCount = 0;
    sampleOffsetIntoNextAudioBuffer = 0;
    zeroBuffer = nullptr;
    dummyBuffer = nullptr;
    signals[0] = nullptr;
    signals[1] = nullptr;
    signals[2] = nullptr;
    didAllocateSignals = 0;
    vs = 0;
    maxvs = 0;
    sr = 44100;
    invsr = 0.000022675736961451248;
    param_01_lastValue = 0;
    param_02_lastValue = 0;
    param_03_lastValue = 0;
    param_04_lastValue = 0;
    param_05_lastValue = 0;
    param_06_lastValue = 0;
    param_07_lastValue = 0;
    param_08_lastValue = 0;
    param_09_lastValue = 0;
    param_10_lastValue = 0;
    param_11_lastValue = 0;
    param_12_lastValue = 0;
    cycle_tilde_01_wrap = 0;
    cycle_tilde_01_ph_currentPhase = 0;
    cycle_tilde_01_ph_conv = 0;
    cycle_tilde_01_setupDone = false;
    mtof_01_innerMtoF_lastInValue = 0;
    mtof_01_innerMtoF_lastOutValue = 0;
    mtof_01_innerMtoF_lastTuning = 0;
    mtof_01_innerScala_lastValid = false;
    mtof_01_innerScala_lastNote = 0;
    mtof_01_innerScala_lastFreq = 0;
    mtof_01_innerScala_sclEntryCount = 0;
    mtof_01_innerScala_sclOctaveMul = 1;
    mtof_01_innerScala_kbmValid = { 0, 0, 0, 60, 69, 440 };
    mtof_01_innerScala_kbmMid = 60;
    mtof_01_innerScala_kbmRefNum = 69;
    mtof_01_innerScala_kbmRefFreq = 440;
    mtof_01_innerScala_kbmSize = 0;
    mtof_01_innerScala_kbmMin = 0;
    mtof_01_innerScala_kbmMax = 0;
    mtof_01_innerScala_kbmOctaveDegree = 12;
    mtof_01_innerScala_kbmMapSize = 0;
    mtof_01_innerScala_refFreq = 261.63;
    cycle_tilde_02_wrap = 0;
    cycle_tilde_02_ph_currentPhase = 0;
    cycle_tilde_02_ph_conv = 0;
    cycle_tilde_02_setupDone = false;
    mtof_02_innerMtoF_lastInValue = 0;
    mtof_02_innerMtoF_lastOutValue = 0;
    mtof_02_innerMtoF_lastTuning = 0;
    mtof_02_innerScala_lastValid = false;
    mtof_02_innerScala_lastNote = 0;
    mtof_02_innerScala_lastFreq = 0;
    mtof_02_innerScala_sclEntryCount = 0;
    mtof_02_innerScala_sclOctaveMul = 1;
    mtof_02_innerScala_kbmValid = { 0, 0, 0, 60, 69, 440 };
    mtof_02_innerScala_kbmMid = 60;
    mtof_02_innerScala_kbmRefNum = 69;
    mtof_02_innerScala_kbmRefFreq = 440;
    mtof_02_innerScala_kbmSize = 0;
    mtof_02_innerScala_kbmMin = 0;
    mtof_02_innerScala_kbmMax = 0;
    mtof_02_innerScala_kbmOctaveDegree = 12;
    mtof_02_innerScala_kbmMapSize = 0;
    mtof_02_innerScala_refFreq = 261.63;
    param_13_lastValue = 0;
    param_14_lastValue = 0;
    cycle_tilde_03_wrap = 0;
    cycle_tilde_03_ph_currentPhase = 0;
    cycle_tilde_03_ph_conv = 0;
    cycle_tilde_03_setupDone = false;
    mtof_03_innerMtoF_lastInValue = 0;
    mtof_03_innerMtoF_lastOutValue = 0;
    mtof_03_innerMtoF_lastTuning = 0;
    mtof_03_innerScala_lastValid = false;
    mtof_03_innerScala_lastNote = 0;
    mtof_03_innerScala_lastFreq = 0;
    mtof_03_innerScala_sclEntryCount = 0;
    mtof_03_innerScala_sclOctaveMul = 1;
    mtof_03_innerScala_kbmValid = { 0, 0, 0, 60, 69, 440 };
    mtof_03_innerScala_kbmMid = 60;
    mtof_03_innerScala_kbmRefNum = 69;
    mtof_03_innerScala_kbmRefFreq = 440;
    mtof_03_innerScala_kbmSize = 0;
    mtof_03_innerScala_kbmMin = 0;
    mtof_03_innerScala_kbmMax = 0;
    mtof_03_innerScala_kbmOctaveDegree = 12;
    mtof_03_innerScala_kbmMapSize = 0;
    mtof_03_innerScala_refFreq = 261.63;
    param_15_lastValue = 0;
    param_16_lastValue = 0;
    param_17_lastValue = 0;
    param_18_lastValue = 0;
    param_19_lastValue = 0;
    param_20_lastValue = 0;
    param_21_lastValue = 0;
    param_22_lastValue = 0;
    param_23_lastValue = 0;
    param_24_lastValue = 0;
    globaltransport_tempo = nullptr;
    globaltransport_state = nullptr;
    stackprotect_count = 0;
    _voiceIndex = 0;
    _noteNumber = 0;
    isMuted = 1;
}

    // data ref strings
    struct DataRefStrings {
    	static constexpr auto& name0 = "RNBODefaultSinus";
    	static constexpr auto& file0 = "";
    	static constexpr auto& tag0 = "buffer~";
    	static constexpr auto& name1 = "RNBODefaultMtofLookupTable256";
    	static constexpr auto& file1 = "";
    	static constexpr auto& tag1 = "buffer~";
    	DataRefStrings* operator->() { return this; }
    	const DataRefStrings* operator->() const { return this; }
    };

    DataRefStrings dataRefStrings;

// member variables

    number param_01_value;
    list message_01_set;
    number param_02_value;
    list message_02_set;
    number param_03_value;
    list message_03_set;
    number param_04_value;
    list message_04_set;
    number param_05_value;
    list message_05_set;
    number param_06_value;
    list message_06_set;
    number param_07_value;
    list message_07_set;
    number param_08_value;
    list message_08_set;
    number param_09_value;
    list message_09_set;
    number param_10_value;
    list message_10_set;
    number param_11_value;
    list message_11_set;
    number unpack_01_out1;
    number unpack_01_out2;
    number unpack_01_out3;
    number expr_01_in1;
    number expr_01_in2;
    number expr_01_out1;
    number param_12_value;
    list message_12_set;
    number cycle_tilde_01_frequency;
    number cycle_tilde_01_phase_offset;
    list mtof_01_midivalue;
    list mtof_01_scale;
    list mtof_01_map;
    number mtof_01_base;
    number dspexpr_01_in1;
    number dspexpr_01_in2;
    number expr_02_in1;
    number expr_02_in2;
    number expr_02_out1;
    number cycle_tilde_02_frequency;
    number cycle_tilde_02_phase_offset;
    list mtof_02_midivalue;
    list mtof_02_scale;
    list mtof_02_map;
    number mtof_02_base;
    number param_13_value;
    list message_13_set;
    number param_14_value;
    list message_14_set;
    number expr_03_in1;
    number expr_03_in2;
    number expr_03_out1;
    number cycle_tilde_03_frequency;
    number cycle_tilde_03_phase_offset;
    list mtof_03_midivalue;
    list mtof_03_scale;
    list mtof_03_map;
    number mtof_03_base;
    number param_15_value;
    list message_15_set;
    number param_16_value;
    list message_16_set;
    number param_17_value;
    list message_17_set;
    number param_18_value;
    list message_18_set;
    list message_19_set;
    number param_19_value;
    list message_20_set;
    number param_20_value;
    number param_21_value;
    list message_21_set;
    number param_22_value;
    list message_22_set;
    number param_23_value;
    list message_23_set;
    number param_24_value;
    list message_24_set;
    MillisecondTime _currentTime;
    ENGINE _internalEngine;
    UInt64 audioProcessSampleCount;
    Index sampleOffsetIntoNextAudioBuffer;
    signal zeroBuffer;
    signal dummyBuffer;
    SampleValue * signals[3];
    bool didAllocateSignals;
    Index vs;
    Index maxvs;
    number sr;
    number invsr;
    number param_01_lastValue;
    number param_02_lastValue;
    number param_03_lastValue;
    number param_04_lastValue;
    number param_05_lastValue;
    number param_06_lastValue;
    number param_07_lastValue;
    number param_08_lastValue;
    number param_09_lastValue;
    number param_10_lastValue;
    number param_11_lastValue;
    number param_12_lastValue;
    SampleBufferRef cycle_tilde_01_buffer;
    Int cycle_tilde_01_wrap;
    UInt32 cycle_tilde_01_phasei;
    SampleValue cycle_tilde_01_f2i;
    number cycle_tilde_01_ph_currentPhase;
    number cycle_tilde_01_ph_conv;
    bool cycle_tilde_01_setupDone;
    number mtof_01_innerMtoF_lastInValue;
    number mtof_01_innerMtoF_lastOutValue;
    number mtof_01_innerMtoF_lastTuning;
    SampleBufferRef mtof_01_innerMtoF_buffer;
    const Index mtof_01_innerScala_KBM_MAP_OFFSET = 7;
    bool mtof_01_innerScala_lastValid;
    number mtof_01_innerScala_lastNote;
    number mtof_01_innerScala_lastFreq;
    Int mtof_01_innerScala_sclEntryCount;
    number mtof_01_innerScala_sclOctaveMul;
    list mtof_01_innerScala_sclExpMul;
    list mtof_01_innerScala_kbmValid;
    Int mtof_01_innerScala_kbmMid;
    Int mtof_01_innerScala_kbmRefNum;
    number mtof_01_innerScala_kbmRefFreq;
    Int mtof_01_innerScala_kbmSize;
    Int mtof_01_innerScala_kbmMin;
    Int mtof_01_innerScala_kbmMax;
    Int mtof_01_innerScala_kbmOctaveDegree;
    Index mtof_01_innerScala_kbmMapSize;
    number mtof_01_innerScala_refFreq;
    SampleBufferRef cycle_tilde_02_buffer;
    Int cycle_tilde_02_wrap;
    UInt32 cycle_tilde_02_phasei;
    SampleValue cycle_tilde_02_f2i;
    number cycle_tilde_02_ph_currentPhase;
    number cycle_tilde_02_ph_conv;
    bool cycle_tilde_02_setupDone;
    number mtof_02_innerMtoF_lastInValue;
    number mtof_02_innerMtoF_lastOutValue;
    number mtof_02_innerMtoF_lastTuning;
    SampleBufferRef mtof_02_innerMtoF_buffer;
    const Index mtof_02_innerScala_KBM_MAP_OFFSET = 7;
    bool mtof_02_innerScala_lastValid;
    number mtof_02_innerScala_lastNote;
    number mtof_02_innerScala_lastFreq;
    Int mtof_02_innerScala_sclEntryCount;
    number mtof_02_innerScala_sclOctaveMul;
    list mtof_02_innerScala_sclExpMul;
    list mtof_02_innerScala_kbmValid;
    Int mtof_02_innerScala_kbmMid;
    Int mtof_02_innerScala_kbmRefNum;
    number mtof_02_innerScala_kbmRefFreq;
    Int mtof_02_innerScala_kbmSize;
    Int mtof_02_innerScala_kbmMin;
    Int mtof_02_innerScala_kbmMax;
    Int mtof_02_innerScala_kbmOctaveDegree;
    Index mtof_02_innerScala_kbmMapSize;
    number mtof_02_innerScala_refFreq;
    number param_13_lastValue;
    number param_14_lastValue;
    SampleBufferRef cycle_tilde_03_buffer;
    Int cycle_tilde_03_wrap;
    UInt32 cycle_tilde_03_phasei;
    SampleValue cycle_tilde_03_f2i;
    number cycle_tilde_03_ph_currentPhase;
    number cycle_tilde_03_ph_conv;
    bool cycle_tilde_03_setupDone;
    number mtof_03_innerMtoF_lastInValue;
    number mtof_03_innerMtoF_lastOutValue;
    number mtof_03_innerMtoF_lastTuning;
    SampleBufferRef mtof_03_innerMtoF_buffer;
    const Index mtof_03_innerScala_KBM_MAP_OFFSET = 7;
    bool mtof_03_innerScala_lastValid;
    number mtof_03_innerScala_lastNote;
    number mtof_03_innerScala_lastFreq;
    Int mtof_03_innerScala_sclEntryCount;
    number mtof_03_innerScala_sclOctaveMul;
    list mtof_03_innerScala_sclExpMul;
    list mtof_03_innerScala_kbmValid;
    Int mtof_03_innerScala_kbmMid;
    Int mtof_03_innerScala_kbmRefNum;
    number mtof_03_innerScala_kbmRefFreq;
    Int mtof_03_innerScala_kbmSize;
    Int mtof_03_innerScala_kbmMin;
    Int mtof_03_innerScala_kbmMax;
    Int mtof_03_innerScala_kbmOctaveDegree;
    Index mtof_03_innerScala_kbmMapSize;
    number mtof_03_innerScala_refFreq;
    number param_15_lastValue;
    number param_16_lastValue;
    number param_17_lastValue;
    number param_18_lastValue;
    number param_19_lastValue;
    number param_20_lastValue;
    number param_21_lastValue;
    number param_22_lastValue;
    number param_23_lastValue;
    number param_24_lastValue;
    signal globaltransport_tempo;
    signal globaltransport_state;
    number stackprotect_count;
    DataRef RNBODefaultSinus;
    DataRef RNBODefaultMtofLookupTable256;
    Index _voiceIndex;
    Int _noteNumber;
    Index isMuted;
    indexlist paramInitIndices;
    indexlist paramInitOrder;
    bool _isInitialized = false;
};

static PatcherInterface* creaternbomatic()
{
    return new rnbomatic<EXTERNALENGINE>();
}

#ifndef RNBO_NO_PATCHERFACTORY
extern "C" PatcherFactoryFunctionPtr GetPatcherFactoryFunction()
#else
extern "C" PatcherFactoryFunctionPtr rnbomaticFactoryFunction()
#endif
{
    return creaternbomatic;
}

#ifndef RNBO_NO_PATCHERFACTORY
extern "C" void SetLogger(Logger* logger)
#else
void rnbomaticSetLogger(Logger* logger)
#endif
{
    console = logger;
}

} // end RNBO namespace

