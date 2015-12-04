package common

import (
	"fmt"
	"os"
)

func CheckForError(err error) {
	if err != nil {
		fmt.Println("runtime error:", err)
		os.Exit(-1)
	}
}

func ValidationError(message string) {
	fmt.Println("validation error:", message)
	os.Exit(-1)
}
