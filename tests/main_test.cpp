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

class TestClass
{
public:
    TestClass(const int value) : x(value) {}
    int x = 29;
    int TestMethod() const { return 42; }
};

TEST(MainTest, Objects)
{
    using namespace mildew;
    auto my_object = std::make_shared<ScriptObject>("test_object", nullptr, new cpp::Object(new TestClass(100)));
    ScriptAny foo = my_object;
    auto test_object = foo.ToValue<ScriptObject>();
    EXPECT_EQ(my_object, test_object) << test_object;
    EXPECT_NE(test_object, std::shared_ptr<ScriptObject>(nullptr)) << test_object;
    auto obj = test_object->native_object()->Cast<TestClass>();
    EXPECT_EQ(obj->x, 100);
    EXPECT_EQ(obj->TestMethod(), 42);
}