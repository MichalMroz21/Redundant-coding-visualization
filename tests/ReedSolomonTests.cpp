#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/range/algorithm/transform.hpp>

#include "ReedSolomonCode.hpp"

TEST(ReedSolomon, RemainderTest){

    QList<std::pair<int, int>> expectedResults{
                               {3, 2}, {6, 4}, {5,6}, {2, 0}, {0, 1}, {1, 7},
                               {5, 2}, {7, 1}, {2, 3}, {6, 4}, {3, 0}, {1, 1},
                               {4, 5}, {2, 4}, {7, 0}, {3, 7}, {6, 3}, {7, 5},
                               {2, 7}, {0, 4}, {6, 1}, {4, 2}
    },
                               results(expectedResults.size());

    std::vector<QString> data{"12345", "12346", "12347", "12357",
                              "12367", "12467", "12567", "13567",
                              "14567", "24567", "34567", "22347",
                              "32347", "42347", "52347", "62347",
                              "72347", "13347", "14347", "15347",
                              "16347", "17347"
    };

    ReedSolomonCode rs;

    auto makeResults = [&rs](const QString& str) -> std::pair<int, int>{
        Poly remainder = rs.getRemainder(str);
        return {remainder.coef[0], remainder.coef[1]};
    };

    boost::transform(data, results.begin(), makeResults);

    ASSERT_THAT(
        expectedResults,
        ::testing::ElementsAreArray(results)
    );
}

TEST(ReedSolomon, ErrorTest){

    QList<QList<int>> data{
       {1, 3, 3, 4, 5, 3, 2}, {2, 2, 3, 4, 5, 3, 2}, {1, 2, 4, 4, 5, 3, 2},
       {1, 2, 3, 5, 5, 3, 2}, {1, 2, 3, 4, 6, 3, 2}, {2, 2, 3, 4, 6, 6, 4},
       {1, 3, 3, 4, 6, 6, 4}, {1, 2, 4, 4, 6, 6, 4}, {1, 2, 3, 5, 6, 6, 4},
       {1, 2, 3, 4, 7, 6, 4}, {3, 2, 3, 4, 5, 7, 5}, {2, 3, 3, 4, 5, 7, 5},
       {2, 2, 4, 4, 5, 7, 5}, {2, 2, 3, 5, 5, 7, 5}, {2, 2, 3, 4, 6, 7, 5},
       {4, 2, 3, 4, 5, 2, 1}, {3, 3, 3, 4, 5, 2, 1}, {3, 2, 4, 4, 5, 2, 1},
       {3, 2, 3, 5, 5, 2, 1}, {3, 2, 3, 4, 6, 2, 1}
    };

    QList<int> expectedResults{1, 0, 2, 3, 4, 0, 1, 2, 3, 4,
                               0, 1, 2, 3, 4, 0,1, 2, 3, 4},
               results(expectedResults.size());

    ReedSolomonCode rs;

    auto makeResults = [&rs](const QList<int>& data) -> int{
        return rs.getRet(data.size(), const_cast<int*>(data.data()));
    };

    boost::transform(data, results.begin(), makeResults);

    ASSERT_THAT(
        expectedResults,
        ::testing::ElementsAreArray(results)
    );
}
