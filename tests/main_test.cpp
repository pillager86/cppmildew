#include <gtest/gtest.h>
#include <iostream>
#include <memory>

#include <cppd/array.hpp>
#include <mildew/lexer.hpp>
#include <mildew/nodes.hpp>
#include <mildew/types/any.hpp>
#include <mildew/types/array.hpp>
#include <mildew/types/object.hpp>

TEST(MainTest, ArrayTest)
{
    using namespace cppd;
    auto test_array = Array{1, 100, 200, 69};
    for(auto item : test_array)
        EXPECT_NE(item, 0) << item;
}

TEST(MainTest, TokenTest)
{
    using namespace mildew;
    Token token = {Token::Type::EQUALS, {1, 1}, "==", Token::LiteralFlag::NONE};
    EXPECT_EQ(token.type, Token::Type::EQUALS) << token;
}

TEST(MainTest, AnyTest)
{
    using namespace mildew;
    ScriptAny foo = 29;
    ScriptAny bar = 9.8;
    foo = bar;
    EXPECT_EQ(foo, bar);
    foo = 99;
    bar = 99;
    EXPECT_EQ(foo, bar);
    foo = true;
    bar = 1;
    EXPECT_EQ(foo, bar);
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
    auto my_object = std::make_shared<ScriptObject>("test_object", nullptr, new cppd::Object(new TestClass(100)));
    ScriptAny foo = my_object;
    auto test_object = foo.ToValue<ScriptObject>();
    EXPECT_EQ(my_object, test_object) << test_object;
    EXPECT_NE(test_object, std::shared_ptr<ScriptObject>(nullptr)) << test_object;
    auto obj = test_object->native_object()->Cast<TestClass>();
    EXPECT_EQ(obj->x, 100);
    EXPECT_EQ(obj->TestMethod(), 42);
}