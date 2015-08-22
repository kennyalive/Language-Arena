package main

import (
	"encoding/binary"
	"time"
	"os"
)

func ReadNextBytes(file *os.File, number int32) []byte {
    bytes := make([]byte, number)
    file.Read(bytes)
    return bytes
}

func ReadFile(filePath string) []int32 {
	file, _ := os.Open(filePath)
	defer file.Close()

	numbersCount := int32(binary.LittleEndian.Uint32(ReadNextBytes(file, 4)))
	numbers := make([]int32, numbersCount)

	int32Bytes := ReadNextBytes(file, numbersCount * 4)
	for i := range numbers {
		numbers[i] = int32(binary.LittleEndian.Uint32(int32Bytes[i*4:(i+1)*4]))
	}
	return numbers
}

func Swap(slice []int32, i, j int) {
	temp := slice[i]
	slice[i] = slice[j]
	slice[j] = temp
}

func QuickSort(slice []int32) {
	right := len(slice) - 1

	storeIndex := 0
	for i := 0; i < right; i++ {
		if slice[i] <= slice[right] {
			Swap(slice, i, storeIndex)
			storeIndex++
		}
	}
	Swap(slice, right, storeIndex)

	if storeIndex > 0 {
		QuickSort(slice[0:storeIndex])
	}
	if right > storeIndex {
		QuickSort(slice[storeIndex+1 : right+1])
	}
}

func main() {
	// prepare input data
	file_name := os.Args[1] + "/random_numbers"
	array := ReadFile(file_name)

	// run benchmark
	start := time.Now()
	QuickSort(array)
	os.Exit(int(time.Since(start) / time.Millisecond))
}
