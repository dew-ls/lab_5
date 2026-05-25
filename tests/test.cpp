#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Account.h"
#include "Transaction.h"

class MockAccount : public Account 
{
public:
	MockAccount(int id, int balance) : Account(id, balance) {}
	MOCK_METHOD(int,  GetBalance,    (), (const, override));
	MOCK_METHOD(void, ChangeBalance, (int diff), (override));
	MOCK_METHOD(void, Lock,          (), (override));
	MOCK_METHOD(void, Unlock,        (), (override));
};

// account tests
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
	Account a(1,500);
	a.Lock();
	EXPECT_THROW(a.Lock(), std::runtime_error);
}
// transaction tests
TEST(Transaction, Same_src_and_dest)
{
	Account a(1, 500);
	Transaction t;
	EXPECT_THROW(t.Make(a, a, 200), std::logic_error);
}
TEST(Transaction, Negative_value)
{
	Account a(1, 500);
	Account b(2, 500);
	Transaction t;
	EXPECT_THROW(t.Make(a, b, -8), std::invalid_argument);
}
TEST(Transaction, Small_value)
{
	Account a(1, 500);
	Account b(2, 500);
	Transaction t;
	EXPECT_THROW(t.Make(a, b, 11), std::logic_error);
}
TEST(Transaction, Unaccessible_action)
{
	Account a(1, 500);
	Account b(2, 500);
	Transaction t;
	t.set_fee(80);
	EXPECT_FALSE(t.Make(a, b, 110));
}
TEST(Transaction, Successfull_make) 
{
	Transaction t;
	t.set_fee(1);
	MockAccount a(1, 1000);
	MockAccount b(2, 0);
    EXPECT_CALL(a, Lock()).Times(1);
    EXPECT_CALL(a, Unlock()).Times(1);
    EXPECT_CALL(b, Lock()).Times(1);
    EXPECT_CALL(b, Unlock()).Times(1);
    EXPECT_CALL(b, ChangeBalance(100)).Times(1);
    EXPECT_CALL(b, ChangeBalance(-101)).Times(1);
	EXPECT_CALL(b, GetBalance()).WillRepeatedly(testing::Return(10000));
	EXPECT_TRUE(t.Make(a, b, 100));
}
TEST(Transaction, Pay_denied)
{
	Transaction t;
	t.set_fee(1);
	MockAccount a(1, 0);
	MockAccount b(2, 0);
    EXPECT_CALL(a, Lock()).Times(1);
    EXPECT_CALL(a, Unlock()).Times(1);
    EXPECT_CALL(b, Lock()).Times(1);
    EXPECT_CALL(b, Unlock()).Times(1);
    EXPECT_CALL(b, ChangeBalance(100)).Times(1);
    EXPECT_CALL(b, ChangeBalance(-100)).Times(1);
	EXPECT_CALL(b, GetBalance()).WillRepeatedly(testing::Return(0));
	EXPECT_FALSE(t.Make(a,b,100));
}
TEST(Transaction, Save_to_database)
{
    Transaction t;
    t.set_fee(1);
    Account a(1, 1000);
    Account b(2, 15);
    t.Make(a, b, 500);
}
