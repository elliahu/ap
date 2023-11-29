#pragma once
#include <iostream>
#include <vector>
#include <cmath>

namespace AP
{

    typedef std::vector<std::vector<double>> Matrix;

    inline Matrix CreateMatrix(size_t width, size_t height, double value = 0.0)
    {
        return std::vector<std::vector<double>>(height, std::vector<double>(width, value));
    }

    inline void ResizeMatrix(Matrix &m, size_t new_width, size_t new_height, double value = 0.0)
    {
        m.resize(new_height, std::vector<double>(new_width, value));
    }

    inline size_t MatrixWidth(const Matrix &m)
    {
        return m[0].size();
    }

    inline size_t MatrixHeight(const Matrix &m)
    {
        return m.size();
    }

    namespace Math
    {
        inline Matrix MatrixMulScalar(const Matrix &m, const double &s)
        {
            auto width = MatrixWidth(m);
            auto height = MatrixHeight(m);

            Matrix result = CreateMatrix(width, height);

            for (size_t i = 0; i < width; i++)
            {
                for (size_t j = 0; j < height; j++)
                {
                    result[j][i] = m[j][i] * s;
                }
            }

            return result;
        }

        inline double median(const Matrix &m)
        {
            auto width = MatrixWidth(m);
            auto height = MatrixHeight(m);
            std::vector<double> medianArray{};

            for (size_t i = 0; i < height; ++i)
            {
                for (size_t j = 0; j < width; ++j)
                {
                    medianArray.push_back(m[i][j]);
                }
            }

            std::sort(medianArray.begin(), medianArray.end());

            return medianArray[medianArray.size()/ 2];
        }

        inline double min(const Matrix &m)
        {
             auto width = MatrixWidth(m);
            auto height = MatrixHeight(m);
            std::vector<double> t{};

            for (size_t i = 0; i < height; ++i)
            {
                for (size_t j = 0; j < width; ++j)
                {
                    t.push_back(m[i][j]);
                }
            }
            return *std::min_element(t.begin(), t.end());
        }

        inline double max(const Matrix &m)
        {
             auto width = MatrixWidth(m);
            auto height = MatrixHeight(m);
            std::vector<double> t{};

            for (size_t i = 0; i < height; ++i)
            {
                for (size_t j = 0; j < width; ++j)
                {
                    t.push_back(m[i][j]);
                }
            }
            return *std::max_element(t.begin(), t.end());
        }
    }
}