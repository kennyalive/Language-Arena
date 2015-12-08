package main

import (
	"bufio"
	"common"
	"encoding/binary"
	"os"
	"path"
	"time"
)

func ReadNumbersFromFile(fileName string) []int32 {
	file, err := os.Open(fileName)
	common.Check(err)
	defer file.Close()

	reader := bufio.NewReader(file)

	var numbersCount int32
	err = binary.Read(reader, binary.LittleEndian, &numbersCount)
	common.Check(err)

	numbers := make([]int32, numbersCount)
	err = binary.Read(reader, binary.LittleEndian, &numbers)
	common.Check(err)
	return numbers
}

func QuickSort(numbers []int32) {
	right := len(numbers) - 1
	storeIndex := 0
	for i := 0; i < right; i++ {
		if numbers[i] <= numbers[right] {
			numbers[storeIndex], numbers[i] = numbers[i], numbers[storeIndex]
			storeIndex++
		}
	}
	numbers[storeIndex], numbers[right] = numbers[right], numbers[storeIndex]

	if storeIndex > 1 {
		QuickSort(numbers[0:storeIndex])
	}
	if right-storeIndex > 1 {
		QuickSort(numbers[storeIndex+1 : right+1])
	}
}

func main() {
	// prepare input data
	fileName := path.Join(os.Args[1], "random_numbers")
	array := ReadNumbersFromFile(fileName)

	// run benchmark
	start := time.Now()
	QuickSort(array)
	elapsedTime := int(time.Since(start) / time.Millisecond)

	// validation
	if len(array) != 4000000 {
		common.ValidationError("invalid size")
	}
	prevValue := array[0]
	for _, value := range array[1:] {
		if prevValue > value {
			common.ValidationError("array is not sorted")
		}
		prevValue = value
	}
	os.Exit(elapsedTime)
}
