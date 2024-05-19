#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "ReedSolomonCode.hpp"

TEST(ReedSolomon, RemainderTest)
{
    std::vector<std::tuple<QString, int, int>> vTest;
    vTest.push_back(std::make_tuple("12345", 3, 2));
    vTest.push_back(std::make_tuple("12346", 6, 4));
    vTest.push_back(std::make_tuple("12347", 5, 6));
    vTest.push_back(std::make_tuple("12357", 2, 0));
    vTest.push_back(std::make_tuple("12367", 0, 1));
    vTest.push_back(std::make_tuple("12467", 1, 7));
    vTest.push_back(std::make_tuple("12567", 5, 2));
    vTest.push_back(std::make_tuple("13567", 7, 1));
    vTest.push_back(std::make_tuple("14567", 2, 3));
    vTest.push_back(std::make_tuple("24567", 6, 4));
    vTest.push_back(std::make_tuple("34567", 3, 0));
    vTest.push_back(std::make_tuple("22347", 1, 1));
    vTest.push_back(std::make_tuple("32347", 4, 5));
    vTest.push_back(std::make_tuple("42347", 2, 4));
    vTest.push_back(std::make_tuple("52347", 7, 0));
    vTest.push_back(std::make_tuple("62347", 3, 7));
    vTest.push_back(std::make_tuple("72347", 6, 3));
    vTest.push_back(std::make_tuple("13347", 7, 5));
    vTest.push_back(std::make_tuple("14347", 2, 7));
    vTest.push_back(std::make_tuple("15347", 0, 4));
    vTest.push_back(std::make_tuple("16347", 6, 1));
    vTest.push_back(std::make_tuple("17347", 4, 2));

    ReedSolomonCode rs;

    for(std::tuple<QString, int, int> elem : vTest)
    {
        Poly remainder = rs.getRemainder(std::get<0>(elem));
        ASSERT_EQ(remainder.coef[0], std::get<1>(elem));
        ASSERT_EQ(remainder.coef[1], std::get<2>(elem));
    }
}


TEST(ReedSolomon, ErrorTest)
{
    std::vector<std::pair<int*, int>> vTest;
    int arr2[] = {1, 3, 3, 4, 5, 3, 2};
    vTest.push_back(std::make_pair(arr2, 1));
    int arr1[] = {2, 2, 3, 4, 5, 3, 2};
    vTest.push_back(std::make_pair(arr1, 0));
    int arr3[] = {1, 2, 4, 4, 5, 3, 2};
    vTest.push_back(std::make_pair(arr3, 2));
    int arr4[] = {1, 2, 3, 5, 5, 3, 2};
    vTest.push_back(std::make_pair(arr4, 3));
    int arr5[] = {1, 2, 3, 4, 6, 3, 2};
    vTest.push_back(std::make_pair(arr5, 4));
    int arr6[] = {2, 2, 3, 4, 6, 6, 4};
    vTest.push_back(std::make_pair(arr6, 0));
    int arr7[] = {1, 3, 3, 4, 6, 6, 4};
    vTest.push_back(std::make_pair(arr7, 1));
    int arr8[] = {1, 2, 4, 4, 6, 6, 4};
    vTest.push_back(std::make_pair(arr8, 2));
    int arr9[] = {1, 2, 3, 5, 6, 6, 4};
    vTest.push_back(std::make_pair(arr9, 3));
    int arr10[] = {1, 2, 3, 4, 7, 6, 4};
    vTest.push_back(std::make_pair(arr10, 4));
    int arr11[] = {3, 2, 3, 4, 5, 7, 5};
    vTest.push_back(std::make_pair(arr11, 0));
    int arr12[] = {2, 3, 3, 4, 5, 7, 5};
    vTest.push_back(std::make_pair(arr12, 1));
    int arr13[] = {2, 2, 4, 4, 5, 7, 5};
    vTest.push_back(std::make_pair(arr13, 2));
    int arr14[] = {2, 2, 3, 5, 5, 7, 5};
    vTest.push_back(std::make_pair(arr14, 3));
    int arr15[] = {2, 2, 3, 4, 6, 7, 5};
    vTest.push_back(std::make_pair(arr15, 4));
    int arr16[] = {4, 2, 3, 4, 5, 2, 1};
    vTest.push_back(std::make_pair(arr16, 0));
    int arr17[] = {3, 3, 3, 4, 5, 2, 1};
    vTest.push_back(std::make_pair(arr17, 1));
    int arr18[] = {3, 2, 4, 4, 5, 2, 1};
    vTest.push_back(std::make_pair(arr18, 2));
    int arr19[] = {3, 2, 3, 5, 5, 2, 1};
    vTest.push_back(std::make_pair(arr19, 3));
    int arr20[] = {3, 2, 3, 4, 6, 2, 1};
    vTest.push_back(std::make_pair(arr20, 4));

    ReedSolomonCode rs;

    for(std::pair<int*, int> elem : vTest)
    {
        int ret = rs.getRet(7, elem.first);
        ASSERT_EQ(ret, elem.second);
    }
}
