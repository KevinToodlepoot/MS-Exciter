#ifndef WAVESHAPERS_H
#define WAVESHAPERS_H

#include <math.h>

namespace audio
{

    inline float signOf(float x)
    {
        if (x)
            return x / abs(x);
        else
            return 1.0f;
    }
    
    inline float clip(float x, float clipLevel = 1.f)
    {
        return fminf(abs(x), clipLevel) * signOf(x);
    }

    inline float fold(float x, float clipLevel = 1.f)
    {
        return fmodf(x * pow(-1, floor(x / clipLevel)), clipLevel) * signOf(x);
    }

    inline float wrap(float x, float clipLevel = 1.f)
    {
        return fmodf(abs(x), clipLevel) * signOf(x);
    }

    inline float arry(float x)
    {
        return ((3.f * x) / 2.f) * (1 - (pow(x, 2) / 3));
    }

    inline float sig(float x, float k)
    {
        return 2.f / (1 + exp(-k * x)) - 1.f;
    }

    inline float sig2(float x)
    {
        return ( (exp(x) - 1.f) * (exp(1.f) + 1.f) ) / ( (exp(x) + 1.f) * (exp(1.f) - 1.f) );
    }

    inline float tanh(float x, float k)
    {
        return tanhf(k * x) / tanhf(k);
    }

    inline float atan(float x, float k)
    {
        return atanf(k * x) / atanf(k);
    }

    inline float fExp1(float x, float k)
    {
        return signOf(x) * (1.f - exp(-1.f * abs(k * x))) / (1.f - exp(-1 * k));
    }
}

#endif
