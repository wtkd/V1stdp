#pragma once

#include <concepts>
#include <cstddef>
#include <filesystem>
#include <vector>

#include <CLI/CLI.hpp>
#include <Eigen/Dense>

void setupClustering(CLI::App &app);

std::size_t countLine(std::filesystem::path const &file);
std::size_t countWord(std::filesystem::path const &file);

template <typename T>
  requires std::integral<T> || std::floating_point<T>
double euclideanDistanceSquare(Eigen::VectorX<T> const &x, Eigen::VectorX<T> const &y);

template <typename T>
  requires std::integral<T> || std::floating_point<T>
double correlationSquare(Eigen::VectorX<T> const &x, Eigen::VectorX<T> const &y);
template <typename T>
  requires std::integral<T> || std::floating_point<T>
double correlation(Eigen::VectorX<T> const &x, Eigen::VectorX<T> const &y);
template <typename T>
  requires std::integral<T> || std::floating_point<T>
double correlationDistance(Eigen::VectorX<T> const &x, Eigen::VectorX<T> const &y);
template <typename T>
  requires std::integral<T> || std::floating_point<T>
double correlationDistanceSquare(Eigen::VectorX<T> const &x, Eigen::VectorX<T> const &y);

template <typename F, typename T>
  requires std::regular_invocable<F, Eigen::VectorX<T>, Eigen::VectorX<T>>
std::vector<std::size_t> singleClusteringSortPermutation(Eigen::MatrixX<T> const &matrix, F const &distance2);

template <typename T>
  requires std::integral<T> || std::floating_point<T>
Eigen::MatrixX<T> applyPermutationCol(Eigen::MatrixX<T> const &matrix, std::ranges::range auto const &permutation);

template <typename T>
  requires std::integral<T> || std::floating_point<T>
Eigen::MatrixX<T> applyPermutationRow(Eigen::MatrixX<T> const &matrix, std::ranges::range auto const &permutation);