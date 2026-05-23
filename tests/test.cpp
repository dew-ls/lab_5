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

class MockTransaction : public Transaction 
{
public:
	MOCK_METHOD(void, SaveToDataBase, (Account& from, Account& to, int sum), (override));

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
	MockTransaction t;
	EXPECT_THROW(t.Make(a, a, 200), std::logic_error);
}
TEST(Transaction, Negative_value)
{
	Account a(1, 500);
	Account b(1, 500);
	MockTransaction t;
	EXPECT_THROW(t.Make(a, b, -8), std::invalid_argument);
}
TEST(Transaction, Small_value)
{
	Account a(1, 500);
	Account b(1, 500);
	MockTransaction t;
	EXPECT_THROW(t.Make(a, b, 11), std::logic_error);
}
TEST(Transaction, Unaccessible_action)
{
	Account a(1, 500);
	Account b(1, 500);
	MockTransaction t;
	t.set_fee(80);
	EXPECT_FALSE(t.Make(a, b, 110));
}
TEST(Transaction, Successfull_make) 
{
	MockTransaction t;
	t.set_fee(1);
	MockAccount from(1, 10000);
	MockAccount to(2, 0);
	EXPECT_CALL(to, GetBalance()).WillOnce(testing::Return(10000));
	EXPECT_TRUE(t.Make(from, to, 100));
}
TEST(Transaction, Pay_denied)
{
	MockTransaction t;
	t.set_fee(1);
	MockAccount from(5, 0);
	MockAccount to(5, 0);
	EXPECT_CALL(to, GetBalance()).WillOnce(testing::Return(0));
	EXPECT_FALSE(t.Make(from,to,120));
}
