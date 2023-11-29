#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <string>
#include <limits>
#include <algorithm>
#include "matrix.h"

namespace AP
{
    class Parser
    {
    public:
        inline void parseCSV(std::string filename)
        {
            std::ifstream file(filename);

            if (!file.is_open())
            {
                std::cerr << "Error opening file: " << filename << std::endl;
                exit(EXIT_FAILURE);
            }
            bool skipFirstColumn = true;
            std::string line;
            std::getline(file, line); // skip first row
            while (std::getline(file, line))
            {
                std::stringstream ss(line);
                std::string cell;
                std::vector<double> row;

                while (std::getline(ss, cell, ','))
                {
                    if(!skipFirstColumn)
                    {
                        row.push_back(std::stod(cell));
                        skipFirstColumn = false;
                    }
                    else
                    {
                        skipFirstColumn = false;
                    }
                    
                }
                points_.push_back(row);
                skipFirstColumn = true;
            }
        }

        inline Matrix getSimilarity()
        {
            auto width = MatrixWidth(points_);
            auto height = MatrixHeight(points_);
            Matrix similarityMatrix = CreateMatrix(width, height, 0.0);
            

            for (size_t i = 0; i < height; ++i)
            {
                for (size_t j = 0; j < width; ++j)
                {
                    if (i != j)
                    {
                        double nsqred = negSquaredEuclideanDistance(points_[i], points_[j]);
                        similarityMatrix[i][j] = nsqred;
                    }
                }
            }

            
            double median = Math::median(similarityMatrix);
            double min = Math::min(similarityMatrix);

            for (size_t i = 0; i < height; ++i)
            {
                for (size_t j = 0; j < width; ++j)
                {
                    if (i == j)
                    {
                        similarityMatrix[i][j] = min;
                    }
                }
            }

            return similarityMatrix;
        }

    private:

        inline double negSquaredEuclideanDistance(const std::vector<double> &point1, const std::vector<double> &point2)
        {
            if (point1.size() != point2.size())
            {
                throw std::invalid_argument("Point dimensions do not match");
            }

            double distance = 0.0;
            for (size_t i = 0; i < point1.size(); ++i)
            {
                double diff = point1[i] - point2[i];
                distance -= diff * diff;
            }

            return distance;
        }

        Matrix points_;
    };
}