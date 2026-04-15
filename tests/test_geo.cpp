#include <gtest/gtest.h>
#include "../include/geo.h"

using namespace geo;

TEST(ComputeDistanceTest, ZeroCoordinates)
{
    // Одинаковые координаты (нулевая дистанция)
    double distance = ComputeDistance({0.0, 0.0}, {0.0, 0.0});
    EXPECT_DOUBLE_EQ(distance, 0.0);
}

TEST(ComputeDistanceTest, SameCoordinates)
{
    // Остановка №1 → Остановка №1 (одна и та же точка)
    double distance = ComputeDistance({55.690431, 38.113266}, {55.690431, 38.113266});
    EXPECT_DOUBLE_EQ(distance, 0.0);
}

TEST(ComputeDistanceTest, Stop1_To_Stop2)
{
    // Остановка №1 → Остановка №2 (примерно 1500м по road_distance)
    double distance = ComputeDistance({55.690431, 38.113266}, {55.682256, 38.116579});
    EXPECT_NEAR(distance, 950.0, 100.0);
}

TEST(ComputeDistanceTest, Stop2_To_Stop3)
{
    // Остановка №2 → Остановка №3 (примерно 1200м по дороге)
    double distance = ComputeDistance({55.682256, 38.116579}, {55.679148, 38.110804});
    EXPECT_NEAR(distance, 500.0, 50.0);
}

TEST(ComputeDistanceTest, Stop3_To_Stop4)
{
    // Остановка №3 → Остановка №4 (примерно 800м по road_distance)
    double distance = ComputeDistance({55.679148, 38.110804}, {55.656141, 38.100587});
    EXPECT_NEAR(distance, 2600.0, 100.0);
}

TEST(ComputeDistanceTest, Stop8_To_Stop9)
{
    // Остановка №8 → Остановка №9 (короткий участок маршрута ~400м)
    double distance = ComputeDistance({55.661347, 38.011089}, {55.657520, 38.011055});
    EXPECT_NEAR(distance, 425.0, 10.0);
}

TEST(ComputeDistanceTest, Stop9_To_Stop10)
{
    // Остановка №9 → Остановка №10 (около 300м по дороге)
    double distance = ComputeDistance({55.657520, 38.011055}, {55.654491, 38.011085});
    EXPECT_NEAR(distance, 335.0, 10.0);
}

TEST(ComputeDistanceTest, Stop13_To_Stop14)
{
    // Остановка №13 → Остановка №14 (примерно 600м road_distance)
    double distance = ComputeDistance({55.644166, 38.032845}, {55.646193, 38.039987});
    EXPECT_NEAR(distance, 500.0, 50.0);
}

TEST(ComputeDistanceTest, DistantStops_1_To_5)
{
    // Далёкие точки: Остановка №1 → Остановка №5 (несколько промежуточных остановок)
    double distance = ComputeDistance({55.690431, 38.113266}, {55.654301, 38.093509});
    EXPECT_GT(distance, 4000.0);
}

TEST(ComputeDistanceTest, DistantStops_1_To_14)
{
    // Максимально удалённые точки маршрута №1 → №14
    double distance = ComputeDistance({55.690431, 38.113266}, {55.646193, 38.039987});
    EXPECT_GT(distance, 6000.0);
}

TEST(ComputeDistanceTest, Symmetry)
{
    // Проверка симметричности: A→B == B→A
    double from_start = ComputeDistance({55.690431, 38.113266}, {55.682256, 38.116579});
    double from_finish = ComputeDistance({55.682256, 38.116579}, {55.690431, 38.113266});
    
    EXPECT_DOUBLE_EQ(from_start, from_finish);
}