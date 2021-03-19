#include <gtest/gtest.h>
#include <iostream>
#include <memory>

#include <mildew/lexer.h>
#include <mildew/nodes.h>
#include <mildew/types/any.h>
#include <mildew/types/object.h>

TEST(MainTest, TokenTest)
{
    using namespace mildew;
    Token token = {Token::Type::EQUALS, {1, 1}, "==", Token::LiteralFlag::NONE};
    EXPECT_EQ(token.type, Token::Type::EQUALS) << token;
}

TEST(MainTest, Objects)
{
    using namespace mildew;
    auto my_object = std::make_shared<ScriptObject>("test_object", nullptr);
    ScriptAny foo = my_object;
    auto test_object = foo.ToValue<ScriptObject>();
    EXPECT_EQ(my_object, test_object) << test_object;
    EXPECT_NE(test_object, std::shared_ptr<ScriptObject>(nullptr)) << test_object;
}