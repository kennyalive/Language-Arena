package main

const n = 624
const m = 397
const matrixA = 0x9908b0df
const upperMask = 0x80000000
const lowerMask = 0x7fffffff

var mt [n]uint32
var mti int = n+1

var mag01 [2]uint32 = [2]uint32{0, matrixA}

func InitGenRand(seed uint32) {
	mt[0] = seed
	for mti = 1; mti < n; mti++ {
		mt[mti] = 1812433253 * (mt[mti-1] ^ (mt[mti-1] >> 30)) + uint32(mti)
	}
}

func RandUint32() uint32 {
	if mti >= n {
		if mti == n+1 {
			InitGenRand(5489)
		}

		kk := 0
		for ; kk < n-m; kk++ {
			y := mt[kk]&upperMask | mt[kk+1]&lowerMask
            mt[kk] = mt[kk+m] ^ y>>1 ^ mag01[y & 0x1]
		}
		for ; kk < n-1; kk++ {
            y := mt[kk]&upperMask | mt[kk+1]&lowerMask
            mt[kk] = mt[kk+(m-n)] ^ y>>1 ^ mag01[y & 0x1]
        }
        y := mt[n-1]&upperMask | mt[0]&lowerMask
        mt[n-1] = mt[m-1] ^ y>>1 ^ mag01[y & 0x1]

		mti = 0
	}

	y := mt[mti]
	mti++

    y ^= (y >> 11)
    y ^= (y << 7) & 0x9d2c5680
    y ^= (y << 15) & 0xefc60000
    y ^= (y >> 18)

    return y
}

func RandFloat64() float64 {
	return float64(RandUint32()) / 4294967296.0;
}

func RandForRange(a, b float64) float64 {
	return a + (b - a)*RandFloat64()
}
