#include "any.h"

namespace mildew
{
    std::ostream& operator<<(std::ostream& os, const ScriptAny& any)
    {
        switch(any.type())
        {
            case ScriptAny::Type::UNDEFINED:
                os << "undefined";
                break;
            case ScriptAny::Type::NULL_:
                os << "null";
                break;
            case ScriptAny::Type::BOOLEAN:
                os << (any.as_boolean_ ? "true" : "false");
                break;
            case ScriptAny::Type::INTEGER:
                os << any.as_integer_;
                break;
            case ScriptAny::Type::DOUBLE:
                os << any.as_double_;
                break;
            default:
                os << "<Unknown ScriptAny type>";
                break;
        }
        return os;
    }
}