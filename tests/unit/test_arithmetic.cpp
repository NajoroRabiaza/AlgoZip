#include <gtest/gtest.h>
#include <stdexcept>
#include <cmath>
#include "algorithms/arithmetic/arithmetic.hpp"

using namespace algozip;

// tolerance pour les comparaisons de nombres flottants
// le codage arithmetique accumule des erreurs d'arrondi
static constexpr double EPSILON = 1e-4;

// Tests encode
// valeurs attendues tirees directement des fichiers de cours
TEST(ArithmeticEncode, EmptyStringThrows) {
    EXPECT_THROW(arithmetic_encode(""), std::invalid_argument);
}

TEST(ArithmeticEncode, CALCUL_CodeApprox) {
    // Dans le cours on a comme exemple : CALCUL = 0.144
    ArithmeticResult result = arithmetic_encode("CALCUL");
    EXPECT_GT(result.code, 0.140);
    EXPECT_LT(result.code, 0.150);
}

TEST(ArithmeticEncode, ABRACADABRA_CodeApprox) {
    // Dans le cours on a comme exemple : CALCUL = 0.2650
    ArithmeticResult result = arithmetic_encode("ABRACADABRA");
    EXPECT_NEAR(result.code, 0.2650, EPSILON);
}

TEST(ArithmeticEncode, CABRACADABRARRARRAD_CodeApprox) {
    // Dans le cours on a comme exemple : CABRACADABRARRARRAD = 0.0319717874
    ArithmeticResult result = arithmetic_encode("CABRACADABRARRARRAD");
    EXPECT_NEAR(result.code, 0.0319717874, EPSILON);
}

TEST(ArithmeticEncode, CALCUL_ModelSize) {
    ArithmeticResult result = arithmetic_encode("CALCUL");
    // 4 symboles disctincts : C, A, L, U
    EXPECT_EQ(result.model.size(), 4u);
}

TEST(ArithmeticEncode, CALCUL_Frequencies) {
    ArithmeticResult result = arithmetic_encode("CALCUL");
    // C apprait 2 fois, A 1 fois, L 2 fois, U 1 fois
    for (const SymbolModel& sym : result.model) {
        if (sym.symbol == 'C') EXPECT_EQ(sym.count, 2);
        if (sym.symbol == 'A') EXPECT_EQ(sym.count, 1);
        if (sym.symbol == 'L') EXPECT_EQ(sym.count, 2);
        if (sym.symbol == 'U') EXPECT_EQ(sym.count, 1);
    }
}

TEST(ArithmeticEncode, CALCUL_IntervalCoversZeroToOne) {
    ArithmeticResult result = arithmetic_encode("CALCUL");
    // la borne basse du 1er symbol doit etre 0
    EXPECT_NEAR(result.model.front().low, 0.0, EPSILON);
    // la borne haute du dernier symbol doit etre 1
    EXPECT_NEAR(result.model.back().high, 1.0, EPSILON);
}

TEST(ArithmeticEncode, CALCUL_N) {
    ArithmeticResult result = arithmetic_encode("CALCUL");
    EXPECT_EQ(result.n, 6);
}


// Tests decode

TEST(ArithmeticDecode, EmptyStringThrows) {
    std::vector<SymbolModel> empty;
    EXPECT_THROW(arithmetic_decode(0.5, empty, 3), std::invalid_argument);
}

TEST(ArithmeticDecode, ZeroNThrows) {
    ArithmeticResult result = arithmetic_encode("CALCUL");
    EXPECT_THROW(arithmetic_decode(result.code, result.model, 0), std::invalid_argument);
}

TEST(ArithmeticDecode, InvalidCodeThrows) {
    ArithmeticResult result = arithmetic_encode("CALCUL");
    EXPECT_THROW(arithmetic_decode(1.5, result.model, result.n), std::invalid_argument);
    EXPECT_THROW(arithmetic_decode(-0.1, result.model, result.n), std::invalid_argument);
}


// Tests roundtrip

TEST(ArithmeticRoundtrip, CALCUL) {
    std::string original = "CALCUL";
    ArithmeticResult encoded = arithmetic_encode(original);
    std::string decoded = arithmetic_decode(encoded.code, encoded.model, encoded.n);
    EXPECT_EQ(decoded, original);
}

TEST(ArithmeticRoundtrip, ABRACADABRA) {
    std::string original = "ABRACADABRA";
    ArithmeticResult encoded = arithmetic_encode(original);
    std::string decoded = arithmetic_decode(encoded.code, encoded.model, encoded.n);
    EXPECT_EQ(decoded, original);
}

TEST(ArithmeticRoundtrip, CABRACADABRARRARRAD) {
    std::string original = "CABRACADABRARRARRAD";
    ArithmeticResult encoded = arithmetic_encode(original);
    std::string decoded = arithmetic_decode(encoded.code, encoded.model, encoded.n);
    EXPECT_EQ(decoded, original);
}

TEST(ArithmeticRoundtrip, SingleChar) {
    std::string original = "AAAA";
    ArithmeticResult encoded = arithmetic_encode(original);
    std::string decoded = arithmetic_decode(encoded.code, encoded.model, encoded.n);
    EXPECT_EQ(decoded, original);
}