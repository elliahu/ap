#include <iostream>
#include "affinity_propagation.h"
#include "parser.h"

int main() {
    AP::Parser parser{};
    parser.parseCSV("../data/test.csv");
    AP::Matrix similarities = parser.getSimilarity(AP::Diagonal::Min);

    AP::AffinityPropagation affinityPropagation(similarities, 10);
    affinityPropagation.fit();

    const std::vector<int>& labels = affinityPropagation.getLabels();

    std::cout << "Clusters:\n";
    for (size_t i = 0; i < labels.size(); ++i) {
        std::cout << "Data point " << i << " belongs to cluster " << labels[i] << "\n";
    }

    return 0;
}