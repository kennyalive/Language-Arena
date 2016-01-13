package common

import (
	"fmt"
	"os"
)

func Check(err error) {
	if err != nil {
		RuntimeError(err.Error())
	}
}

func RuntimeError(message string) {
	fmt.Println("runtime error: ", message)
	os.Exit(-1)
}

func ValidationError(message string) {
	fmt.Println("validation error: ", message)
	os.Exit(-2)
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
