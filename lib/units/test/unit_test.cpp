// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Scipp contributors (https://github.com/scipp)
#include <gtest/gtest.h>

#include "test_macros.h"

#include "scipp/units/except.h"
#include "scipp/units/unit.h"

using namespace scipp;
using scipp::units::Unit;

TEST(UnitTest, c) {
  EXPECT_EQ(units::c.underlying().multiplier(), 299792458.0);
}

TEST(UnitTest, cancellation) {
  EXPECT_EQ(Unit(units::deg / units::deg), units::dimensionless);
  EXPECT_EQ(units::deg / units::deg, units::dimensionless);
  EXPECT_EQ(units::deg * Unit(units::rad / units::deg), units::rad);
}

TEST(UnitTest, construct) { ASSERT_NO_THROW(Unit{units::dimensionless}); }

TEST(UnitTest, construct_default) {
  Unit u;
  ASSERT_EQ(u, units::dimensionless);
}

TEST(UnitTest, construct_bad_string) {
  EXPECT_THROW(Unit("abcde"), except::UnitError);
}

TEST(UnitTest, overflows) {
  // These would run out of bits in llnl/units and wrap, ensure that scipp
  // prevents this and throws instead.
  Unit m64{pow(units::m, 64)};
  Unit inv_m128{units::one / m64 / m64};
  EXPECT_THROW(m64 * m64, except::UnitError);
  EXPECT_THROW(units::one / inv_m128, except::UnitError);
  EXPECT_THROW(inv_m128 / units::m, except::UnitError);
  EXPECT_THROW(pow(units::m, 128), except::UnitError);
}

TEST(UnitTest, compare) {
  Unit u1{units::dimensionless};
  Unit u2{units::m};
  ASSERT_TRUE(u1 == u1);
  ASSERT_TRUE(u1 != u2);
  ASSERT_TRUE(u2 == u2);
  ASSERT_FALSE(u1 == u2);
  ASSERT_FALSE(u2 != u2);
}

TEST(UnitTest, add) {
  Unit a{units::dimensionless};
  Unit b{units::m};
  Unit c{units::m * units::m};
  EXPECT_EQ(a + a, a);
  EXPECT_EQ(b + b, b);
  EXPECT_EQ(c + c, c);
  EXPECT_THROW(a + b, except::UnitError);
  EXPECT_THROW(a + c, except::UnitError);
  EXPECT_THROW(b + a, except::UnitError);
  EXPECT_THROW(b + c, except::UnitError);
  EXPECT_THROW(c + a, except::UnitError);
  EXPECT_THROW(c + b, except::UnitError);
}

TEST(UnitTest, multiply) {
  Unit a{units::dimensionless};
  Unit b{units::m};
  Unit c{units::m * units::m};
  EXPECT_EQ(a * a, a);
  EXPECT_EQ(a * b, b);
  EXPECT_EQ(b * a, b);
  EXPECT_EQ(a * c, c);
  EXPECT_EQ(c * a, c);
  EXPECT_EQ(b * b, c);
  EXPECT_EQ(b * c, units::m * units::m * units::m);
  EXPECT_EQ(c * b, units::m * units::m * units::m);
}

TEST(UnitTest, counts_variances) {
  Unit counts{units::counts};
  EXPECT_EQ(counts * counts, units::Unit("counts**2"));
}

TEST(UnitTest, multiply_counts) {
  Unit counts{units::counts};
  Unit none{units::dimensionless};
  EXPECT_EQ(counts * none, counts);
  EXPECT_EQ(none * counts, counts);
}

TEST(UnitTest, divide) {
  Unit one{units::dimensionless};
  Unit l{units::m};
  Unit t{units::s};
  Unit v{units::m / units::s};
  EXPECT_EQ(l / one, l);
  EXPECT_EQ(t / one, t);
  EXPECT_EQ(l / l, one);
  EXPECT_EQ(l / t, v);
}

TEST(UnitTest, divide_counts) {
  Unit counts{units::counts};
  EXPECT_EQ(counts / counts, units::dimensionless);
}

TEST(UnitTest, modulo) {
  Unit one{units::dimensionless};
  Unit l{units::m};
  Unit t{units::s};
  EXPECT_EQ(l % one, l);
  EXPECT_EQ(t % one, t);
  EXPECT_EQ(l % l, l);
  EXPECT_EQ(l % t, l);
}

TEST(UnitTest, pow) {
  EXPECT_EQ(pow(units::m, 0), units::one);
  EXPECT_EQ(pow(units::m, 1), units::m);
  EXPECT_EQ(pow(units::m, 2), units::m * units::m);
  EXPECT_EQ(pow(units::m, -1), units::one / units::m);
}

TEST(UnitTest, neutron_units) {
  Unit c(units::c);
  EXPECT_EQ(c * units::m, Unit(units::c * units::m));
  EXPECT_EQ(c * units::m / units::m, units::c);
  EXPECT_EQ(units::meV / c, Unit(units::meV / units::c));
  EXPECT_EQ(units::meV / c / units::meV, Unit(units::dimensionless / units::c));
}

TEST(UnitTest, isCounts) {
  EXPECT_FALSE(units::dimensionless.isCounts());
  EXPECT_TRUE(units::counts.isCounts());
  EXPECT_FALSE(Unit(units::counts / units::us).isCounts());
  EXPECT_FALSE(Unit(units::counts / units::meV).isCounts());
  EXPECT_FALSE(Unit(units::dimensionless / units::m).isCounts());
}

TEST(UnitTest, isCountDensity) {
  EXPECT_FALSE(units::dimensionless.isCountDensity());
  EXPECT_FALSE(units::counts.isCountDensity());
  EXPECT_TRUE(Unit(units::counts / units::us).isCountDensity());
  EXPECT_TRUE(Unit(units::counts / units::meV).isCountDensity());
  EXPECT_FALSE(Unit(units::dimensionless / units::m).isCountDensity());
}

TEST(UnitFunctionsTest, abs) {
  EXPECT_EQ(abs(units::one), units::one);
  EXPECT_EQ(abs(units::m), units::m);
}

TEST(UnitFunctionsTest, ceil) {
  EXPECT_EQ(ceil(units::one), units::one);
  EXPECT_EQ(ceil(units::m), units::m);
}

TEST(UnitFunctionsTest, floor) {
  EXPECT_EQ(floor(units::one), units::one);
  EXPECT_EQ(floor(units::m), units::m);
}

TEST(UnitFunctionsTest, rint) {
  EXPECT_EQ(rint(units::one), units::one);
  EXPECT_EQ(rint(units::m), units::m);
}

TEST(UnitFunctionsTest, sqrt) {
  EXPECT_EQ(sqrt(units::m * units::m), units::m);
  EXPECT_EQ(sqrt(units::counts * units::counts), units::counts);
  EXPECT_EQ(sqrt(units::one), units::one);
  EXPECT_THROW_MSG(sqrt(units::m), except::UnitError,
                   "Unsupported unit as result of sqrt: sqrt(m).");
  EXPECT_THROW_MSG(sqrt(units::Unit("J")), except::UnitError,
                   "Unsupported unit as result of sqrt: sqrt(J).");
  EXPECT_THROW_MSG(sqrt(units::Unit("eV")), except::UnitError,
                   "Unsupported unit as result of sqrt: sqrt(eV).");
}

TEST(UnitFunctionsTest, sin) {
  EXPECT_EQ(sin(units::rad), units::dimensionless);
  EXPECT_EQ(sin(units::deg), units::dimensionless);
  EXPECT_THROW(sin(units::m), except::UnitError);
  EXPECT_THROW(sin(units::dimensionless), except::UnitError);
}

TEST(UnitFunctionsTest, cos) {
  EXPECT_EQ(cos(units::rad), units::dimensionless);
  EXPECT_EQ(cos(units::deg), units::dimensionless);
  EXPECT_THROW(cos(units::m), except::UnitError);
  EXPECT_THROW(cos(units::dimensionless), except::UnitError);
}

TEST(UnitFunctionsTest, tan) {
  EXPECT_EQ(tan(units::rad), units::dimensionless);
  EXPECT_EQ(tan(units::deg), units::dimensionless);
  EXPECT_THROW(tan(units::m), except::UnitError);
  EXPECT_THROW(tan(units::dimensionless), except::UnitError);
}

TEST(UnitFunctionsTest, asin) {
  EXPECT_EQ(asin(units::dimensionless), units::rad);
  EXPECT_THROW(asin(units::m), except::UnitError);
  EXPECT_THROW(asin(units::rad), except::UnitError);
  EXPECT_THROW(asin(units::deg), except::UnitError);
}

TEST(UnitFunctionsTest, acos) {
  EXPECT_EQ(acos(units::dimensionless), units::rad);
  EXPECT_THROW(acos(units::m), except::UnitError);
  EXPECT_THROW(acos(units::rad), except::UnitError);
  EXPECT_THROW(acos(units::deg), except::UnitError);
}

TEST(UnitFunctionsTest, atan) {
  EXPECT_EQ(atan(units::dimensionless), units::rad);
  EXPECT_THROW(atan(units::m), except::UnitError);
  EXPECT_THROW(atan(units::rad), except::UnitError);
  EXPECT_THROW(atan(units::deg), except::UnitError);
}

TEST(UnitFunctionsTest, atan2) {
  EXPECT_EQ(atan2(units::m, units::m), units::rad);
  EXPECT_EQ(atan2(units::s, units::s), units::rad);
  EXPECT_THROW(atan2(units::m, units::s), except::UnitError);
}

TEST(UnitParseTest, singular_plural) {
  EXPECT_EQ(units::Unit("counts"), units::counts);
  EXPECT_EQ(units::Unit("count"), units::counts);
}

TEST(UnitFormatTest, roundtrip_string) {
  for (const auto &s :
       {"m", "m/s", "meV", "pAh", "mAh", "ns", "counts", "counts^2",
        "counts/meV", "1/counts", "counts/m", "Y", "M", "D"}) {
    const auto unit = units::Unit(s);
    EXPECT_EQ(to_string(unit), s);
    EXPECT_EQ(units::Unit(to_string(unit)), unit);
  }
}

TEST(UnitFormatTest, roundtrip_unit) {
  // Some formattings use special characters, e.g., for micro and Angstrom, but
  // some are actually formatted badly right now, but at least roundtrip works.
  for (const auto &s : {"us", "angstrom", "counts/us", "Y", "M", "D"}) {
    const auto unit = units::Unit(s);
    EXPECT_EQ(units::Unit(to_string(unit)), unit);
  }
}
