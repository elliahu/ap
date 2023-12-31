#pragma once
#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
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
                auto start = std::chrono::high_resolution_clock::now();
                updateResponsibility();
                updateAvailability();
                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                std::cout << "Iteration " << iter << " out of " << max_iter_ << " finished in " << duration.count() << " milliseconds" << std::endl;
            }

            identifyClusters();

            thread_pool_.stop();
        }

        inline const std::vector<int> &getLabels() const
        {
            return labels_;
        }

        inline std::vector<int> getUniqueClusters()
        {
            std::vector<int> lbls_(labels_);
            std::sort(lbls_.begin(), lbls_.end());
            auto last = std::unique(lbls_.begin(), lbls_.end());
            lbls_.erase(last, lbls_.end());
            return lbls_;
        }

    private:
        inline void initialize()
        {
            auto start = std::chrono::high_resolution_clock::now();
            unsigned int n = similarities_.size();

            responsibilities_.resize(n, std::vector<double>(n, 0.0));
            availabilities_.resize(n, std::vector<double>(n, 0.0));

            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

            std::cout << "Prepared matrices of size " << n << "x" << n << " in " << duration.count() << " milliseconds" << std::endl;
            ;
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

            for (unsigned int i = 0; i < n; ++i)
            {
                for (unsigned int k = 0; k < n; ++k)
                {
                    thread_pool_.queue_job(
                        [&, i, k, n]()
                        {
                            if (i != k)
                            {
                                double sum = 0.0;
                                for (unsigned int ii = 0; ii < n; ++ii)
                                {
                                    if (ii != i && ii != k)
                                    {
                                        sum += std::max(0.0, responsibilities_[ii][k]);
                                    }
                                }
                                availabilities_[i][k] = std::min(0.0, responsibilities_[k][k] + sum);
                            }
                            else
                            {
                                double sum = 0.0;
                                for (unsigned int ii = 0; ii < n; ++ii)
                                {
                                    if (ii != k)
                                    {
                                        sum += std::max(0.0, responsibilities_[ii][k]);
                                    }
                                }
                                availabilities_[i][k] = sum;
                            }
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
