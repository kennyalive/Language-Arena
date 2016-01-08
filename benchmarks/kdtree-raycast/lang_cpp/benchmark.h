#pragma once

class KdTree;

enum { benchmarkRaysCount = 10000000 };

int BenchmarkKdTree(const KdTree& kdTree);
void ValidateKdTree(const KdTree& kdTree, int raysCount);
