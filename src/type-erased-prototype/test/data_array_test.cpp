#include <gtest/gtest.h>
#include <vector>

#include "data_array.h"
#include "dimensions.h"
#include "variable.h"

TEST(DataArray, construct) {
  ASSERT_NO_THROW(
      makeDataArray<Variable::Value>(Dimensions(Dimension::Tof, 2), 2));
  const auto a =
      makeDataArray<Variable::Value>(Dimensions(Dimension::Tof, 2), 2);
  const auto &data = a.get<Variable::Value>();
  EXPECT_EQ(data.size(), 2);
}

TEST(DataArray, construct_fail) {
  ASSERT_ANY_THROW(makeDataArray<Variable::Value>(Dimensions(), 2));
  ASSERT_ANY_THROW(
      makeDataArray<Variable::Value>(Dimensions(Dimension::Tof, 1), 2));
  ASSERT_ANY_THROW(
      makeDataArray<Variable::Value>(Dimensions(Dimension::Tof, 3), 2));
}

TEST(DataArray, sharing) {
  const auto a1 =
      makeDataArray<Variable::Value>(Dimensions(Dimension::Tof, 2), 2);
  const auto a2(a1);
  EXPECT_EQ(&a1.get<Variable::Value>(), &a2.get<Variable::Value>());
}

TEST(DataArray, copy) {
  const auto a1 =
      makeDataArray<Variable::Value>(Dimensions(Dimension::Tof, 2), {1.1, 2.2});
  const auto &data1 = a1.get<Variable::Value>();
  EXPECT_EQ(data1[0], 1.1);
  EXPECT_EQ(data1[1], 2.2);
  auto a2(a1);
  EXPECT_EQ(&a1.get<Variable::Value>(), &a2.get<const Variable::Value>());
  EXPECT_NE(&a1.get<Variable::Value>(), &a2.get<Variable::Value>());
  const auto &data2 = a2.get<Variable::Value>();
  EXPECT_EQ(data2[0], 1.1);
  EXPECT_EQ(data2[1], 2.2);
}

TEST(DataArray, ragged) {
  const auto raggedSize = makeDataArray<Variable::DimensionSize>(
      Dimensions(Dimension::SpectrumNumber, 2), {2l, 3l});
  EXPECT_EQ(raggedSize.dimensions().volume(), 2);
  Dimensions dimensions;
  dimensions.add(Dimension::Tof, raggedSize);
  dimensions.add(Dimension::SpectrumNumber, 2);
  EXPECT_EQ(dimensions.volume(), 5);
  ASSERT_NO_THROW(makeDataArray<Variable::Value>(dimensions, 5));
  ASSERT_ANY_THROW(makeDataArray<Variable::Value>(dimensions, 4));
}

TEST(DataArray, concatenate) {
  Dimensions dims(Dimension::Tof, 1);
  const auto a = makeDataArray<Variable::Value>(dims, {1.0});
  const auto b = makeDataArray<Variable::Value>(dims, {2.0});
  auto ab = concatenate(Dimension::Tof, a, b);
  ASSERT_EQ(ab.size(), 2);
  const auto &data = ab.get<Variable::Value>();
  EXPECT_EQ(data[0], 1.0);
  EXPECT_EQ(data[1], 2.0);
  auto ba = concatenate(Dimension::Tof, b, a);
  const auto abba = concatenate(Dimension::Q, ab, ba);
  ASSERT_EQ(abba.size(), 4);
  EXPECT_EQ(abba.dimensions().count(), 2);
  const auto &data2 = abba.get<Variable::Value>();
  EXPECT_EQ(data2[0], 1.0);
  EXPECT_EQ(data2[1], 2.0);
  EXPECT_EQ(data2[2], 2.0);
  EXPECT_EQ(data2[3], 1.0);
  const auto ababbaba = concatenate(Dimension::Tof, abba, abba);
  ASSERT_EQ(ababbaba.size(), 8);
  const auto &data3 = ababbaba.get<Variable::Value>();
  EXPECT_EQ(data3[0], 1.0);
  EXPECT_EQ(data3[1], 2.0);
  EXPECT_EQ(data3[2], 1.0);
  EXPECT_EQ(data3[3], 2.0);
  EXPECT_EQ(data3[4], 2.0);
  EXPECT_EQ(data3[5], 1.0);
  EXPECT_EQ(data3[6], 2.0);
  EXPECT_EQ(data3[7], 1.0);
  const auto abbaabba = concatenate(Dimension::Q, abba, abba);
  ASSERT_EQ(abbaabba.size(), 8);
  const auto &data4 = abbaabba.get<Variable::Value>();
  EXPECT_EQ(data4[0], 1.0);
  EXPECT_EQ(data4[1], 2.0);
  EXPECT_EQ(data4[2], 2.0);
  EXPECT_EQ(data4[3], 1.0);
  EXPECT_EQ(data4[4], 1.0);
  EXPECT_EQ(data4[5], 2.0);
  EXPECT_EQ(data4[6], 2.0);
  EXPECT_EQ(data4[7], 1.0);
}

#if 1
const auto spectrumPosition = makeDataArray<Variable::SpectrumPosition>(
    detectorPosition, detectorGrouping);
// dimensions given by linked?
// why not allow for extra dimensions? does not make sense, would return same data
// Internally in spectrum-pos variable:
// DatasetIterator<Variable::DetectorPosition, Variable::DetectorGrouping>
// does linking via DataArray make sense?
// should DataArray be able to live on its own, outside Dataset?
//
// How to concatenate Variable::SpectrumPosition? Does not make sense, unless
// DetectorPosition and DetectorGrouping are concatenated.
//
// We cannot drive this from Variable::SpectrumPosition, it cannot be the one
// doing the concatenation of the variables it derives from.(*)
// Ok if we are a passive observer of DetectorPosition and DetectorGrouping,
// however this implies that we must ensure that DetectorGrouping and
// DetectorPosition are under our control, i.e., contained within a Dataset.
// Basically this imples that DataArray may not exist outside Dataset.
//
// (*) Can it? If we make it the owner, even within Dataset it may be ok?
// Dataset concatenates only SpectrumPosition, which interally deals with
// DetectorPosition and DetectorGrouping. How can we expose the latter in
// Dataset?
// What if there are multiple owners, e.g., for Variable::DimensionSize which
// is required by all variables making up a histogram?
//
// How to concatenate Dataset? In general, we do not want to concatenate all
// variables, unless an existing dimension gets longer. If dimension is new, we
// do not want to concatenate variables that are constant in the new dimension.
// Use pointer comparison, assuming perfect sharing? Do full checks
// (optionally?)?)?
#endif
