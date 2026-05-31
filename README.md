# Laboratory work 5

## Homework

### Создание CMakeLists.txt

```
cmake_minimum_required(VERSION 3.14)
project(banking)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(banking STATIC banking/Account.cpp banking/Transaction.cpp)
target_include_directories(banking PUBLIC banking)

option(BUILD_TESTS "Build tests" OFF)
option(ENABLE_COVERAGE "Enable coverage" OFF)

if(ENABLE_COVERAGE)
    target_compile_options(banking PRIVATE --coverage -O0)
    target_link_options(banking PRIVATE --coverage)
endif()

if(BUILD_TESTS)
    enable_testing()

    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
    )
    FetchContent_MakeAvailable(googletest)

    file(GLOB TEST_SOURCES tests/*.cpp)
    add_executable(check ${TEST_SOURCES})
    target_link_libraries(check banking gtest_main gmock_main)
    add_test(NAME check COMMAND check)

    if(ENABLE_COVERAGE)
        target_compile_options(check PRIVATE --coverage -O0)
        target_link_options(check PRIVATE --coverage)
    endif()
endif()
```

---

### Написание модульных тестов

**Тесты для Account**

```
TEST(Account, Change_balance_without_lock) 
{
    Account a(1, 500);
    EXPECT_THROW(a.ChangeBalance(100), std::runtime_error);
}


TEST(Account, Change_balance_with_lock)
{
    Account a(1, 500);
    a.Lock();
    a.ChangeBalance(100);
    EXPECT_EQ(a.GetBalance(), 600);
    a.Unlock();
}

TEST(Account, Double_locks) 
{
    Account a(1, 500);
    a.Lock();
    EXPECT_THROW(a.Lock(), std::runtime_error);
    a.Unlock();
}
```

**Mock-класс для Account:**

```
class MockAccount : public Account {
public:
    MockAccount(int id, int balance) : Account(id, balance) {}
    MOCK_METHOD(int,  GetBalance,    (), (const, override));
    MOCK_METHOD(void, ChangeBalance, (int diff), (override));
    MOCK_METHOD(void, Lock,          (), (override));
    MOCK_METHOD(void, Unlock,        (), (override));
};
```

**Тесты для Transaction**

```
TEST(Transaction, Same_src_and_dest) {
    Transaction t;
    Account a(1, 1000);
    EXPECT_THROW(t.Make(a, a, 200), std::logic_error);
}

TEST(Transaction, Successfull_make) {
    Transaction t;
    t.set_fee(1);
    MockAccount from(1, 10000), to(2, 0);

    EXPECT_CALL(from, Lock()).Times(1);
    EXPECT_CALL(from, Unlock()).Times(1);
    EXPECT_CALL(to,   Lock()).Times(1);
    EXPECT_CALL(to,   Unlock()).Times(1);
    EXPECT_CALL(to, ChangeBalance(100)).Times(1);
    EXPECT_CALL(to, GetBalance()).WillRepeatedly(testing::Return(10000));
    EXPECT_CALL(to, ChangeBalance(-101)).Times(1);

    EXPECT_TRUE(t.Make(from, to, 100));
}
```

### Настройка GitHub Actions

Файл `.github/workflows/ci.yml`:

```
name: CI

on:
  push:
    branches: [master, main]
  pull_request:
    branches: [master, main]

jobs:
  build:
    name: Build on Ubuntu
    runs-on: ubuntu-latest

    steps:
      - uses: actions/checkout@v4

      - name: Install dependencies
        run: sudo apt-get install -y cmake g++ lcov

      - name: Configure
        run: |
          cmake -S . -B build \
            -DBUILD_TESTS=ON \
            -DENABLE_COVERAGE=ON \
            -DCMAKE_BUILD_TYPE=Debug

      - name: Build
        run: cmake --build build

      - name: Test
        run: cmake --build build --target test -- ARGS=--verbose

      - name: Collect coverage
        run: |
          lcov --capture --directory build --output-file coverage.info --ignore-errors mismatch
          lcov --remove coverage.info '/usr/*' '*/googletest/*' '*/gtest/*' '*/gmock/*' '*/build_deps/*' \
          --output-file coverage.info --ignore-errors unused
          lcov --list coverage.info

      - name: Upload to Coveralls
        uses: coverallsapp/github-action@v2
        env:
          COVERALLS_REPO_TOKEN: ${{ secrets.COVERALLS_REPO_TOKEN }}
        with:
          path-to-lcov: coverage.info
```

### [Результат на Coveralls](https://coveralls.io/jobs/182276374)
