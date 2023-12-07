#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <string>
#include <limits>
#include <algorithm>
#include <chrono>
#include "matrix.h"
#include "threadpool.h"

namespace AP
{
    class Parser
    {
    public:
        inline void parseTXT(std::string filename, uint32_t limit_rows = 0)
        {
            std::ifstream file(filename);

            if (!file.is_open())
            {
                std::cerr << "Error opening file: " << filename << std::endl;
                exit(EXIT_FAILURE);
            }

            uint32_t rows_read = 0;
            auto start = std::chrono::high_resolution_clock::now();

            // Read the file line by line
            std::string line;
            while (std::getline(file, line))
            {
                std::istringstream iss(line);
                double value;
                std::vector<double> row;

                // Read each value in the line
                while (iss >> value)
                {
                    row.push_back(value);
                }

                // Store the row in the vector
                points_.push_back(row);
                rows_read++;
            }

            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

            std::cout << "File " << filename << " parsed and " << rows_read << " rows were retrieved in " << duration.count() << " milliseconds" << std::endl;
        }
    
        inline void parseCSV(std::string filename, uint32_t limit_rows = 0)
        {
            std::ifstream file(filename);

            if (!file.is_open())
            {
                std::cerr << "Error opening file: " << filename << std::endl;
                exit(EXIT_FAILURE);
            }
            bool skipFirstColumn = true;
            uint32_t rows_read = 0;
            std::string line;
            auto start = std::chrono::high_resolution_clock::now();

            std::getline(file, line); // skip first row
            while (std::getline(file, line))
            {
                if (rows_read >= limit_rows && limit_rows != 0)
                    break;

                std::stringstream ss(line);
                std::string cell;
                std::vector<double> row;

                while (std::getline(ss, cell, ','))
                {
                    if (!skipFirstColumn)
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
                rows_read++;
            }
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

            std::cout << "File " << filename << " parsed and " << rows_read << " rows were retrieved in " << duration.count() << " milliseconds" << std::endl;
        }

        inline Matrix getSimilarity(Diagonal diagonal = Median)
        {
            auto width = MatrixHeight(points_);
            auto height = MatrixHeight(points_);
            Matrix similarityMatrix = CreateMatrix(width, height, 0.0);

            auto start = std::chrono::high_resolution_clock::now();

            thread_pool_.start();

            for (size_t i = 0; i < height; ++i)
            {
                thread_pool_.queue_job(
                    [&, i]()
                    {
                        for (size_t j = 0; j < width; ++j)
                        {
                            if (i != j)
                            {
                                double nsqred = negSquaredEuclideanDistance(points_[i], points_[j]);
                                similarityMatrix[i][j] = nsqred;
                            }
                        }
                    });
            }

            thread_pool_.wait();

            double median = Math::median(similarityMatrix);
            double min = Math::min(similarityMatrix);
            double max = Math::max(similarityMatrix);
            double inf = std::numeric_limits<double>::infinity();
            double negInf = -std::numeric_limits<double>::infinity();

            for (size_t i = 0; i < height; ++i)
            {
                thread_pool_.queue_job(
                    [&, i]()
                    {
                        for (size_t j = 0; j < width; ++j)
                        {
                            if (i == j)
                            {
                                switch (diagonal)
                                {
                                case Min:
                                    similarityMatrix[i][j] = min;
                                    break;
                                case Max:
                                    similarityMatrix[i][j] = max;
                                    break;
                                case Median:
                                    similarityMatrix[i][j] = median;
                                    break;
                                case Inf:
                                    similarityMatrix[i][j] = inf;
                                    break;
                                case NegInf:
                                    similarityMatrix[i][j] = negInf;
                                    break;
                                case Zero:
                                    similarityMatrix[i][j] = 0.0;
                                    break;
                                default:
                                    break;
                                }
                            }
                        }
                    });
            }

            thread_pool_.wait();
            thread_pool_.stop();

            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << "Computed similarity matrix in " << duration.count() << " millisecond" << std::endl;

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
        Threading::ThreadPool thread_pool_{};
    };
}