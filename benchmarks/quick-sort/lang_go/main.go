package main

import (
	"time"
	"os"
	"bufio"
	"strconv"
)

func ReadFile(filePath string) (numbers []int) {
	file, _ := os.Open(filePath)
	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		value, _ := strconv.Atoi(scanner.Text())
		numbers = append(numbers, value)
	}
	return
}

func Swap(slice []int, i, j int) {
	temp := slice[i]
	slice[i] = slice[j]
	slice[j] = temp
}

func QuickSort(slice []int) {
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

func ValidateResult(result []int, answers_file_name string) bool {
	answer := ReadFile(answers_file_name)
	for i, v := range result { 
		if v != answer[i] { 
			return false 
		} 
	}
	return true
}

func main() {
	// prepare input data
	file_name := os.Args[1] + "/random_numbers"
	array := ReadFile(file_name)

	// run benchmark
	start := time.Now()
	QuickSort(array)
	elapsed_time := int(time.Since(start) / time.Millisecond)

	// return benchmark results
	exit_code := elapsed_time
	if os.Args[len(os.Args) - 1] == "validate" {
		answers_file_name := os.Args[1] + "/sorted_random_numbers"
		if ValidateResult(array, answers_file_name) {
			exit_code = 0
		} else {
			exit_code = -1
		}
	}
	os.Exit(exit_code)
}
