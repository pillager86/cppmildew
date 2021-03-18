#include <gtest/gtest.h>
#include <iostream>
#include <mildew/lexer.h>

TEST(MainTest, TokenTest)
{
    using namespace mildew;
    Token token = {Token::Type::EQUALS, {1, 1}, "==", Token::LiteralFlag::NONE};
    EXPECT_EQ(token.type, Token::Type::EQUALS);
}