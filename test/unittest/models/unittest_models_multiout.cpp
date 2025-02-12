// SPDX-License-Identifier: Apache-2.0
/**
 * Copyright (C) 2021 Jihoon Lee <jhoon.it.lee@samsung.com>
 *
 * @file unittest_models_multiout.cpp
 * @date 22 November 2021
 * @brief unittest models to cover multiinput, multioutput scenario
 * @see	https://github.com/nnstreamer/nntrainer
 * @author Jihoon Lee <jhoon.it.lee@samsung.com>
 * @bug No known bugs except for NYI items
 */

#include <gtest/gtest.h>

#include <memory>

#include <ini_wrapper.h>
#include <neuralnet.h>
#include <nntrainer_test_util.h>

#include <models_golden_test.h>

using namespace nntrainer;

static std::unique_ptr<NeuralNetwork> split_axis3_split_number5() {
  std::unique_ptr<NeuralNetwork> nn(new NeuralNetwork());
  nn->setProperty({"batch_size=2"});

  auto graph = makeGraph({
    {"conv2d",
     {"name=conv", "input_shape=3:4:5", "filters=3", "kernel_size=1,1"}},
    {"split", {"name=split", "input_layers=conv", "axis=3", "split_number=5"}},
    {"addition",
     {"name=add", "input_layers=split(0),split(1),split(2),split(3),split(4)"}},
    {"mse", {"name=loss", "input_layers=add"}},
  });
  for (auto &node : graph) {
    nn->addLayer(node);
  }

  nn->setOptimizer(ml::train::createOptimizer("sgd", {"learning_rate = 0.1"}));
  return nn;
}

static std::unique_ptr<NeuralNetwork> split_axis2_split_number4() {
  std::unique_ptr<NeuralNetwork> nn(new NeuralNetwork());
  nn->setProperty({"batch_size=2"});

  auto graph = makeGraph({
    {"conv2d",
     {"name=conv", "input_shape=3:4:5", "filters=3", "kernel_size=1,1"}},
    {"split", {"name=split", "input_layers=conv", "axis=2", "split_number=4"}},
    {"addition",
     {"name=add", "input_layers=split(0),split(1),split(2),split(3)"}},
    {"mse", {"name=loss", "input_layers=add"}},
  });
  for (auto &node : graph) {
    nn->addLayer(node);
  }

  nn->setOptimizer(ml::train::createOptimizer("sgd", {"learning_rate = 0.1"}));
  return nn;
}

static std::unique_ptr<NeuralNetwork> split_axis2_split_number2() {
  std::unique_ptr<NeuralNetwork> nn(new NeuralNetwork());
  nn->setProperty({"batch_size=2"});

  auto graph = makeGraph({
    {"conv2d",
     {"name=conv", "input_shape=3:4:5", "filters=3", "kernel_size=1,1"}},
    {"split", {"name=split", "input_layers=conv", "axis=2", "split_number=2"}},
    {"addition", {"name=add", "input_layers=split(0),split(1)"}},
    {"mse", {"name=loss", "input_layers=add"}},
  });
  for (auto &node : graph) {
    nn->addLayer(node);
  }

  nn->setOptimizer(ml::train::createOptimizer("sgd", {"learning_rate = 0.1"}));
  return nn;
}

/// A has two output tensor a1, a2 and B, C takes it
///     A
/// (a0, a1)
///  |    |
///  v    v
/// (a0) (a1)
///  B    C
///   \  /
///    v
///    D
static std::unique_ptr<NeuralNetwork> split_and_join() {
  std::unique_ptr<NeuralNetwork> nn(new NeuralNetwork());
  nn->setProperty({"batch_size=5"});

  auto graph = makeGraph({
    {"fully_connected", {"name=fc", "input_shape=1:1:3", "unit=2"}},
    {"split", {"name=a", "input_layers=fc", "axis=3"}},
    {"fully_connected", {"name=c", "input_layers=a(1)", "unit=3"}},
    {"fully_connected", {"name=b", "input_layers=a(0)", "unit=3"}},
    {"addition", {"name=d", "input_layers=b,c"}},
    {"mse", {"name=loss", "input_layers=d"}},
  });
  for (auto &node : graph) {
    nn->addLayer(node);
  }

  nn->setOptimizer(ml::train::createOptimizer("sgd", {"learning_rate = 0.1"}));
  return nn;
}

/// A has two output tensor a1, a2 and B takes it
///     A
/// (a0, a1)
///  |    |
///  v    v
/// (a0, a1)
///    B
static std::unique_ptr<NeuralNetwork> one_to_one() {
  std::unique_ptr<NeuralNetwork> nn(new NeuralNetwork());
  nn->setProperty({"batch_size=5"});

  auto graph = makeGraph({
    {"fully_connected", {"name=fc", "input_shape=1:1:3", "unit=2"}},
    {"split", {"name=a", "input_layers=fc", "axis=3"}},
    {"addition", {"name=b", "input_layers=a(0),a(1)"}},
    {"mse", {"name=loss", "input_layers=b"}},
  });
  for (auto &node : graph) {
    nn->addLayer(node);
  }

  nn->setOptimizer(ml::train::createOptimizer("sgd", {"learning_rate = 0.1"}));
  return nn;
}

/// A has two output tensor a0, aa and B takes it but in reversed order
///     A
/// (a0, a1)
///     x
///  v    v
/// (a0, a1)
///    B
static std::unique_ptr<NeuralNetwork> one_to_one_reversed() {
  std::unique_ptr<NeuralNetwork> nn(new NeuralNetwork());
  nn->setProperty({"batch_size=5"});

  auto graph = makeGraph({
    {"fully_connected", {"name=fc", "input_shape=1:1:3", "unit=2"}},
    {"split", {"name=a", "input_layers=fc", "axis=3"}},
    {"addition", {"name=b", "input_layers=a(1),a(0)"}},
    {"mse", {"name=loss", "input_layers=b"}},
  });
  for (auto &node : graph) {
    nn->addLayer(node);
  }

  nn->setOptimizer(ml::train::createOptimizer("sgd", {"learning_rate = 0.1"}));
  return nn;
}

/// A has two output tensor a1, a2 and B and C takes it
///     A
/// (a0, a1, a2)---------------->(a2)
///  | \  |-------                E
///  |  - + - \   |              /
///  v    v   v   v             /
/// (a0, a1) (a0, a1)          /
///    B         C            /
///   (b0)      (c0)         /
///     \        /          /
///      \      /----------
///         v
///        (d0)
///          D
static std::unique_ptr<NeuralNetwork> one_to_many() {
  std::unique_ptr<NeuralNetwork> nn(new NeuralNetwork());
  nn->setProperty({"batch_size=5"});

  auto graph = makeGraph({
    {"fully_connected", {"name=fc", "input_shape=1:1:2", "unit=3"}},
    {"split", {"name=a", "input_layers=fc", "axis=3"}},
    {"addition", {"name=b", "input_layers=a(0),a(1)"}},
    {"addition", {"name=c", "input_layers=a(0),a(1)"}},
    {"addition", {"name=d", "input_layers=b,c,a(2)"}},
    {"mse", {"name=loss", "input_layers=d"}},
  });

  for (auto &node : graph) {
    nn->addLayer(node);
  }

  nn->setOptimizer(ml::train::createOptimizer("sgd", {"learning_rate = 0.1"}));
  return nn;
}

///         input
///        (split)
///   a0   a1   a2   a3
///  (fc)           (fc)
///   b0             b1
///         (add)
///          c0
///
/// only c0 is fed to loss, a1 and a2 is dangled
/// @note we are not supporting explicit layer with dangling output, It is also
/// unclear that if this should be supported.
static std::unique_ptr<NeuralNetwork> split_and_join_dangle() {
  std::unique_ptr<NeuralNetwork> nn(new NeuralNetwork());
  nn->setProperty({"batch_size=5"});

  auto graph = makeGraph({
    {"fully_connected", {"name=fc", "input_shape=1:1:3", "unit=4"}},
    {"split", {"name=a", "input_layers=fc", "axis=3"}},
    {"activation", {"name=a0", "activation=sigmoid", "input_layers=a(0)"}},
    {"activation", {"name=a3", "activation=sigmoid", "input_layers=a(3)"}},
    {"fully_connected",
     {"name=b0", "input_layers=a0", "unit=3", "shared_from=b0"}},
    {"fully_connected",
     {"name=b1", "input_layers=a3", "unit=3", "shared_from=b0"}},
    {"addition", {"name=c0", "input_layers=b0,b1", "activation=sigmoid"}},
    {"mse", {"name=loss", "input_layers=c0"}},
  });

  for (auto &node : graph) {
    nn->addLayer(node);
  }

  nn->setOptimizer(ml::train::createOptimizer("sgd", {"learning_rate = 0.1"}));
  return nn;
}

GTEST_PARAMETER_TEST(
  multiInoutModels, nntrainerModelTest,
  ::testing::ValuesIn({
    mkModelTc_V2(split_axis3_split_number5, "split_axis3_split_number5",
                 ModelTestOption::ALL_V2),
    mkModelTc_V2(split_axis2_split_number4, "split_axis2_split_number4",
                 ModelTestOption::ALL_V2),
    mkModelTc_V2(split_axis2_split_number2, "split_axis2_split_number2",
                 ModelTestOption::ALL_V2),
    mkModelTc_V2(split_and_join, "split_and_join", ModelTestOption::ALL_V2),
    mkModelTc_V2(one_to_one, "one_to_one", ModelTestOption::ALL_V2),
    mkModelTc_V2(one_to_one_reversed, "one_to_one__reversed",
                 ModelTestOption::ALL_V2),
    mkModelTc_V2(one_to_many, "one_to_many", ModelTestOption::ALL_V2),
    mkModelTc_V2(split_and_join_dangle, "split_and_join_dangle",
                 ModelTestOption::ALL_V2),
  }),
  [](const testing::TestParamInfo<nntrainerModelTest::ParamType> &info) {
    return std::get<1>(info.param);
  });
