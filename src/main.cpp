#include <iostream>
#include "affinity_propagation.h"
#include "parser.h"

int main() {
    AP::Matrix _similarities = {
        {0.9, 0.3, 0.1},
        {0.4, 0.8, 0.6},
        {0.2, 0.5, 0.7}
    };


    AP::Parser parser{};
    parser.parseCSV("../data/test.csv");
    AP::Matrix similarities = parser.getSimilarity();


    AP::AffinityPropagation affinityPropagation(similarities);
    affinityPropagation.fit();

    const std::vector<int>& labels = affinityPropagation.getLabels();

    std::cout << "Clusters:\n";
    for (size_t i = 0; i < labels.size(); ++i) {
        std::cout << "Data point " << i << " belongs to cluster " << labels[i] << "\n";
    }

    return 0;
}