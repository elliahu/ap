#include <iostream>
#include <chrono>
#include "affinity_propagation.h"
#include "parser.h"

/**
 * Affinity propagation implementation by Matěj Eliáš 2023
 *
 * Optimized for performace by using thread-pooling
 * Computationaly expensive loops are split into smallest possible independent jobs
 * These jobs are then being worked on by work threads
 * Number of work threads depends on specific hardware
 *
 * To run press F5 in VS Code or run 'make'
 * required environment: Linux, C++20, make build tools
 *
 * make sure the path to source data is relative to CWD when running the build binary directly (such as './main')
 */

int main()
{
    AP::Parser parser{};
    parser.parseCSV("../data/mnist_test.csv");
    AP::Matrix similarities = parser.getSimilarity(AP::Diagonal::Min);

    auto start = std::chrono::high_resolution_clock::now();
    AP::AffinityPropagation affinityPropagation(similarities, 10);
    affinityPropagation.fit();

    const std::vector<int> &labels = affinityPropagation.getLabels();

    std::cout << "\n-------------\nClusters:\n------------\n";
    for (size_t i = 0; i < labels.size(); ++i)
    {
        std::cout << "Data point " << i << " belongs to cluster " << labels[i] << "\n";
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Algorithm finished in " << duration.count() << " milliseconds" << std::endl;

    return 0;
}