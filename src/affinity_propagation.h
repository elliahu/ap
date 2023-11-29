#pragma once
#include <iostream>
#include <vector>
#include <cmath>
#include "matrix.h"
#include "threadpool.h"

namespace AP
{
    const double NEG_INFINITY = -std::numeric_limits<double>::infinity();

    class AffinityPropagation
    {
    public:
        AffinityPropagation(const Matrix &similarities, unsigned int max_iter = 200)
            : similarities_(similarities), max_iter_(max_iter) {}

        inline void fit()
        {
            thread_pool_.start();

            initialize();

            for (unsigned int iter = 0; iter < max_iter_; ++iter)
            {
                updateResponsibility();
                updateAvailability();
            }

            identifyClusters();

            thread_pool_.stop();
        }

        inline const std::vector<int> &getLabels() const
        {
            return labels_;
        }

    private:
        inline void initialize()
        {
            unsigned int n = similarities_.size();

            responsibilities_.resize(n, std::vector<double>(n, 0.0));
            availabilities_.resize(n, std::vector<double>(n, 0.0));

            std::vector<double> s_max(n, NEG_INFINITY);
            std::vector<double> a_max(n, NEG_INFINITY);

            for (unsigned int i = 0; i < n; ++i)
            {
                for (unsigned int k = 0; k < n; ++k)
                {
                    if (similarities_[i][k] > s_max[i])
                        s_max[i] = similarities_[i][k];
                }
            }

            for (unsigned int i = 0; i < n; ++i)
            {
                for (unsigned int k = 0; k < n; ++k)
                {
                    if (similarities_[i][k] > a_max[k])
                        a_max[k] = similarities_[i][k];
                }
            }

            for (unsigned int i = 0; i < n; ++i)
            {
                thread_pool_.queue_job(
                    [&, i]()
                    {
                        for (unsigned int k = 0; k < n; ++k)
                        {
                            responsibilities_[i][k] = similarities_[i][k] - s_max[i];
                            availabilities_[i][k] = similarities_[i][k] - a_max[k];
                        }
                    });
            }

            thread_pool_.wait();
        }

        inline void updateResponsibility()
        {
            unsigned int n = similarities_.size();

            for (unsigned int i = 0; i < n; ++i)
            {

                for (unsigned int k = 0; k < n; ++k)
                {
                    thread_pool_.queue_job(
                        [&, i, k, n]()
                        {
                            double max_val = NEG_INFINITY;

                            for (unsigned int kk = 0; kk < k; ++kk)
                            {
                                double val = availabilities_[i][kk] + similarities_[i][kk];
                                if (val > max_val)
                                    max_val = val;
                            }

                            for (unsigned int kk = k + 1; kk < n; ++kk)
                            {
                                double val = availabilities_[i][kk] + similarities_[i][kk];
                                if (val > max_val)
                                    max_val = val;
                            }

                            responsibilities_[i][k] = similarities_[i][k] - max_val;
                        });
                }
            }

            thread_pool_.wait();
        }

        inline void updateAvailability()
        {
            unsigned int n = similarities_.size();

            Matrix r_plus_similarity = CreateMatrix(n, n);

            for (unsigned int i = 0; i < n; ++i)
            {

                for (unsigned int k = 0; k < n; ++k)
                {
                    if (i != k)
                    {
                        thread_pool_.queue_job(
                            [&, i, k, n]()
                            {
                                double sum = 0.0;
                                for (unsigned int ii = 0; ii < i; ++ii)
                                {
                                    sum += std::max(0.0, responsibilities_[ii][k]);
                                }
                                for (unsigned int ii = i + 1; ii < k; ++ii)
                                {
                                    sum += std::max(0.0, responsibilities_[ii][k]);
                                }
                                for (unsigned int ii = k + 1; ii < n; ++ii)
                                {
                                    sum += std::max(0.0, responsibilities_[ii][k]);
                                }

                                r_plus_similarity[i][k] = sum;
                            });
                    }
                }
            }

            thread_pool_.wait();

            for (unsigned int i = 0; i < n; ++i)
            {

                for (unsigned int k = 0; k < n; ++k)
                {
                    thread_pool_.queue_job(
                        [&, i, k, n, r_plus_similarity]()
                        {
                            double sum = 0.0;
                            for (unsigned int kk = 0; kk < k; ++kk)
                            {
                                sum += std::max(0.0, r_plus_similarity[kk][i]);
                            }
                            for (unsigned int kk = k + 1; kk < n; ++kk)
                            {
                                sum += std::max(0.0, r_plus_similarity[kk][i]);
                            }

                            availabilities_[i][k] = r_plus_similarity[i][k] + sum;
                        });
                }
            }

            thread_pool_.wait();
        }

        inline void identifyClusters()
        {
            unsigned int n = similarities_.size();

            labels_.resize(n, -1);
            for (unsigned int i = 0; i < n; ++i)
            {
                double max_val = NEG_INFINITY;
                int exemplar = -1;

                for (unsigned int k = 0; k < n; ++k)
                {
                    double val = responsibilities_[i][k] + availabilities_[i][k];
                    if (val > max_val)
                    {
                        max_val = val;
                        exemplar = k;
                    }
                }

                labels_[i] = exemplar;
            }
        }

    private:
        Threading::ThreadPool thread_pool_{};
        const Matrix &similarities_;
        unsigned int max_iter_;

        Matrix responsibilities_;
        Matrix availabilities_;
        std::vector<int> labels_;
    };
}
