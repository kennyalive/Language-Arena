package common

import (
	"fmt"
	"os"
)

func Check(err error) {
	if err != nil {
		fmt.Println("runtime error:", err)
		os.Exit(-1)
	}
}

func ValidationError(message string) {
	fmt.Println("validation error:", message)
	os.Exit(-1)
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
