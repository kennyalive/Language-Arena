package common

import (
	"fmt"
	"os"
	"strconv"
)

func Check(err error) {
	if err != nil {
		RuntimeError(err.Error())
	}
}

func RuntimeError(message string) {
	fmt.Println("runtime error:", message)
	os.Exit(1)
}

func ValidationError(message string) {
	fmt.Println("validation error: ", message)
	os.Exit(2)
}

func StoreBenchmarkTiming(time int) {
	f, err := os.Create("timing")
	if err != nil {
		RuntimeError("failed to store benchmark timing")
	}
	defer f.Close()
	f.WriteString(strconv.Itoa(time))
}

func AssertEquals(actual, expected uint64, message string) {
	if actual != expected {
		ValidationError(fmt.Sprintf("%v\nactual value %v, expected value %v",
			message, actual, expected))
	}
}

func AssertEqualsHex(actual, expected uint64, message string) {
	if actual != expected {
		ValidationError(fmt.Sprintf("%v\nactual value %#x, expected value %#x",
			message, actual, expected))
	}
}

func CombineHashes(hash1, hash2 uint64) uint64 {
	return hash1 ^ (hash2 + 0x9e3779b9 + hash1<<6 + hash1>>2)
}
