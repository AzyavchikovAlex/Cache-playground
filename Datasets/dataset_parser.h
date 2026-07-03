#pragma once

#include "dataset.h"

size_t GetDatasetSize(const AnyDataset& any_dataset);

AnyDataset ParseDataset(const std::string& path);