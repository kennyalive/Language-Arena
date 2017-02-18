private immutable n = 624;
private immutable m = 397;
private immutable matrixA = 0x9908b0df;
private immutable upperMask = 0x80000000;
private immutable lowerMask = 0x7fffffff;

private immutable uint[2] mag01 = [0, matrixA];

private uint[n] mt;
private int mti = n+1;

void initRand(uint seed)
{
    mt[0] = seed;
    for (mti = 1; mti < n; mti++)
    {
        mt[mti] = 1812433253 * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti;
    }
}

uint randUint()
{
    if (mti >= n) {
        if (mti == n+1)
            initRand(5489);

        int kk = 0;
        for (; kk < n-m; kk++)
        {
            uint y = (mt[kk]&upperMask) | (mt[kk+1]&lowerMask);
            mt[kk] = mt[kk+m] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (; kk < n-1; kk++)
        {
            uint y = (mt[kk]&upperMask) | (mt[kk+1]&lowerMask);
            mt[kk] = mt[kk+(m-n)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        uint y = (mt[n-1]&upperMask) | (mt[0]&lowerMask);
        mt[n-1] = mt[m-1] ^ (y >> 1) ^ mag01[y & 0x1UL];
        mti = 0;
    }

    uint y = mt[mti++];
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);
    return y;
}

double randDouble() 
{
    static const double scale = 1.0 / 4294967296.0;
    return randUint() * scale;
}

double randFromRange(double a, double b)
{
    return a + (b - a)*randDouble();
}
