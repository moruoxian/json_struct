/*
 * Copyright © 2016 Jorgen Lind
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#include "json_struct.h"

#include "catch2/catch.hpp"
#include <unordered_map>

namespace json_struct_test
{

static const char json_data1[] = "{\n"
                                 "\"StringNode\": \"Some test data\",\n"
                                 "\"NumberNode\": 4676.4,\n"
                                 "\"BooleanTrue\": true,\n"
                                 "\"BooleanFalse\": false,\n"
                                 "\"TestStruct\": {\n"
                                 "\"SubString\": \"Some other string\",\n"
                                 "\"SubNumber\": 500,\n"
                                 "\"Array\": [\n"
                                 "5,\n"
                                 "6,\n"
                                 "3,\n"
                                 "6\n"
                                 "],\n"
                                 "\"optional_float\": 300,\n"
                                 "\"this_property_does_not_exist\": true\n"
                                 "},\n"
                                 "\"OptionalButWithData\": [ 17.5 ],\n"
                                 "\"subStruct2\": {\n"
                                 "\"Field1\": 4,\n"
                                 "\"Field2\": true\n"
                                 "},\n"
                                 "\"Skipped_sub_object\": {\n"
                                 "\"Field3\": 465\n"
                                 "}\n"
                                 "}\n";

struct SubStruct
{
  std::string SubString;
  int SubNumber;
  std::vector<double> Array;
  JS::OptionalChecked<float> optional_float;
  JS::OptionalChecked<double> optional_double;
  JS::Optional<double> optional_with_value = 4.5;
  JS_OBJECT(JS_MEMBER(SubString), JS_MEMBER(SubNumber), JS_MEMBER(Array), JS_MEMBER(optional_float),
            JS_MEMBER(optional_with_value));
};

static const char sub_struct3_data[] = "{\n"
                                       "\"Field1\": 4,\n"
                                       "\"Field2\": true,\n"
                                       "\"Field3\": \"432\"\n"
                                       "}\n";

struct SubStruct2
{
  float Field1;
  bool Field2;
  JS_OBJECT(JS_MEMBER(Field1), JS_MEMBER_ALIASES(Field2, "hello", "Foobar"));
};

struct SubStruct3 : public SubStruct2
{
  std::string Field3;
  int Field4;
  JS::Optional<std::string> Field5;
  JS_OBJECT_WITH_SUPER(JS_SUPER_CLASSES(JS_SUPER_CLASS(SubStruct2)), JS_MEMBER(Field3), JS_MEMBER(Field4),
                       JS_MEMBER(Field5));
};

struct JsonData1
{
  std::string StringNode;
  double NumberNode;
  bool BooleanTrue;
  /*!
   *very special comment for BooleanFalse
   *
   *\json
   *{
   *   json
   *}
   **/
  bool BooleanFalse;
  JS::Optional<int> OptionalInt;
  /// Test structur comment
  SubStruct TestStruct;
  JS::Optional<std::vector<double>> OptionalButWithData;
  float unassigned_value;
  std::unique_ptr<SubStruct2> subStruct2;

  int Field3 = 243;
  std::string NodeWithLiteral = "SOME STRING LITERAL!!!";

  JS_OBJECT(JS_MEMBER(StringNode), JS_MEMBER(NumberNode), JS_MEMBER(BooleanTrue), JS_MEMBER(BooleanFalse),
            JS_MEMBER(OptionalInt), JS_MEMBER(TestStruct), JS_MEMBER(OptionalButWithData), JS_MEMBER(unassigned_value),
            JS_MEMBER(subStruct2), JS_MEMBER(Field3), JS_MEMBER(NodeWithLiteral));
};

TEST_CASE("check_json_tree_nodes"
          "json_struct")
{
  JS::ParseContext context(json_data1);
  JsonData1 data;
  context.parseTo(data);

  for (double x : data.TestStruct.Array)

  data.TestStruct.optional_with_value = 5;
  REQUIRE(data.StringNode == "Some test data");
  REQUIRE(context.error == JS::Error::NoError);

  REQUIRE(data.Field3 == 243);

  std::string json = JS::serializeStruct(data);
}

static const char json_data2[] = "{\n"
                                 "\"some_int\": 4,\n"
                                 "\"sub_object\": {\n"
                                 "\"more_data\": \"some text\",\n"
                                 "\"a_float\": 1.2,\n"
                                 "\"boolean_member\": false\n"
                                 "}\n"
                                 "}\n";

template <typename T>
struct OuterStruct
{
  int some_int;
  T sub_object;

  JS_OBJECT(JS_MEMBER(some_int), JS_MEMBER(sub_object));
};

struct SubObject
{
  std::string more_data;
  float a_float;
  bool boolean_member;

  JS_OBJECT(JS_MEMBER(more_data), JS_MEMBER(a_float), JS_MEMBER(boolean_member));
};

TEST_CASE("check_json_tree_template", "json_struct")
{
  JS::ParseContext context(json_data2);
  OuterStruct<SubObject> data;
  context.parseTo(data);
  REQUIRE(data.sub_object.more_data == "some text");
  std::string json = JS::serializeStruct(data);
};

TEST_CASE("check_json_tree_subclass", "json_struct")
{
  JS::ParseContext context(sub_struct3_data);
  SubStruct3 substruct3;
  context.parseTo(substruct3);
  REQUIRE(substruct3.Field3 == std::string("432"));
}

static const char json_data3[] = "{\n"
                                 "\"SuperSuper\": 5,\n"
                                 "\"Regular\": 42,\n"
                                 "\"Super\": \"This is in the Superclass\"\n"
                                 "}\n";

struct SuperSuperClass
{
  int SuperSuper;
  JS_OBJECT(JS_MEMBER(SuperSuper));
};

struct SuperClass : public SuperSuperClass
{
  std::string Super;
  JS_OBJECT_WITH_SUPER(JS_SUPER_CLASSES(JS_SUPER_CLASS(SuperSuperClass)), JS_MEMBER(Super));
};

struct RegularClass : public SuperClass
{
  int Regular;
  JS_OBJECT_WITH_SUPER(JS_SUPER_CLASSES(JS_SUPER_CLASS(SuperClass)), JS_MEMBER(Regular));
};

TEST_CASE("check_json_tree_deep_tree", "json_struct")
{
  JS::ParseContext context(json_data3);
  RegularClass regular;
  context.parseTo(regular);
  REQUIRE(regular.SuperSuper == 5);
  REQUIRE(regular.Super == "This is in the Superclass");
  REQUIRE(regular.Regular == 42);
}

static const char missing_object_def[] = R"json({
  "first": true,
  "second": "hello world",
    "third": {},
    "fourth": 33
})json";

struct MissingObjectDef
{
  bool first;
  std::string second;
  int fourth;

  JS_OBJECT(JS_MEMBER(first), JS_MEMBER(second), JS_MEMBER(fourth));
};

TEST_CASE("check_json_missing_object", "json_struct")
{
  JS::ParseContext context(missing_object_def);
  MissingObjectDef missing;
  context.parseTo(missing);

  REQUIRE(context.error == JS::Error::NoError);
  REQUIRE(missing.fourth == 33);
}

static const char error_in_sub[] = R"json({
  "first": {
    "ffirst": 4,
    "fsecond": {},
    "not_assigned": 555
  },
  "second": "hello world",
  "third": 33
})json";

struct ErrorInSubChild
{
  int ffirst;
  JS_OBJECT(JS_MEMBER(ffirst));
};

struct ErrorInSub
{
  ErrorInSubChild first;
  std::string second;
  int third;
  JS::Optional<int> not_assigned = 999;
  JS_OBJECT(JS_MEMBER(first), JS_MEMBER(second), JS_MEMBER(third));
};

TEST_CASE("check_json_error_in_sub", "json_struct")
{
  JS::ParseContext context(error_in_sub);
  ErrorInSub sub;
  context.parseTo(sub);
  REQUIRE(context.error == JS::Error::NoError);
  REQUIRE(sub.second == "hello world");
  REQUIRE(sub.third == 33);
  REQUIRE(sub.not_assigned.data == 999);
}

struct JsonObjectTester
{
  std::string field;
  JS::JsonObject obj;
  int number = 0;

  JS_OBJECT(JS_MEMBER(field), JS_MEMBER(obj), JS_MEMBER(number));
};

struct JsonObjectOrArrayObjectTester
{
  std::string field;
  JS::JsonObjectOrArray obj;
  int number = 0;

  JS_OBJECT(JS_MEMBER(field), JS_MEMBER(obj), JS_MEMBER(number));
};

struct JsonObjectRefTester
{
  std::string field;
  JS::JsonObjectRef obj;
  int number = 0;

  JS_OBJECT(JS_MEMBER(field), JS_MEMBER(obj), JS_MEMBER(number));
};

struct JsonObjectOrArrayObjectRefTester
{
  std::string field;
  JS::JsonObjectOrArrayRef obj;
  int number = 0;

  JS_OBJECT(JS_MEMBER(field), JS_MEMBER(obj), JS_MEMBER(number));
};

static const char jsonObjectTest[] = R"json({
  "field": "hello",
  "obj": {
    "some_sub_filed": 2,
    "some_sub_array": [ "a", "b", "c"],
    "some_sub_object": { "field": "not hello" }
  },
  "number": 43
})json";

TEST_CASE("check_json_object", "json_struct")
{
  JS::ParseContext context(jsonObjectTest);
  JsonObjectTester obj;
  context.parseTo(obj);
  REQUIRE(context.error == JS::Error::NoError);
  REQUIRE(obj.field == "hello");
  REQUIRE(obj.obj.data.size() > 0);
  REQUIRE(obj.number == 43);

  std::string out = JS::serializeStruct(obj);
  REQUIRE(out == jsonObjectTest);
}

TEST_CASE("check_json_object_or_array_object", "json_struct")
{
  JS::ParseContext context(jsonObjectTest);
  JsonObjectOrArrayObjectTester obj;
  context.parseTo(obj);
  REQUIRE(context.error == JS::Error::NoError);
  REQUIRE(obj.field == "hello");
  REQUIRE(obj.obj.data.size() > 0);
  REQUIRE(obj.number == 43);

  std::string out = JS::serializeStruct(obj);
  REQUIRE(out == jsonObjectTest);
}

TEST_CASE("check_json_object_ref", "json_struct")
{
  JS::ParseContext context(jsonObjectTest);
  JsonObjectRefTester obj;
  context.parseTo(obj);
  REQUIRE(context.error == JS::Error::NoError);
  REQUIRE(obj.field == "hello");
  REQUIRE(obj.obj.ref.size > 0);
  REQUIRE(obj.number == 43);

  std::string out = JS::serializeStruct(obj);
  REQUIRE(out == jsonObjectTest);
}

TEST_CASE("check_json_object_or_array_object_ref", "json_struct")
{
  JS::ParseContext context(jsonObjectTest);
  JsonObjectOrArrayObjectRefTester obj;
  context.parseTo(obj);
  REQUIRE(context.error == JS::Error::NoError);
  REQUIRE(obj.field == "hello");
  REQUIRE(obj.obj.ref.size > 0);
  REQUIRE(obj.number == 43);

  std::string out = JS::serializeStruct(obj);
  REQUIRE(out == jsonObjectTest);
}

struct JsonArrayTester
{
  std::string string;
  JS::JsonArray array;
  int number = 0;

  JS_OBJECT(JS_MEMBER(string), JS_MEMBER(array), JS_MEMBER(number));
};

struct JsonObjectOrArrayArrayTester
{
  std::string string;
  JS::JsonObjectOrArray array;
  int number = 0;

  JS_OBJECT(JS_MEMBER(string), JS_MEMBER(array), JS_MEMBER(number));
};

struct JsonArrayRefTester
{
  std::string string;
  JS::JsonArrayRef array;
  int number = 0;

  JS_OBJECT(JS_MEMBER(string), JS_MEMBER(array), JS_MEMBER(number));
};

struct JsonObjectOrArrayArrayRefTester
{
  std::string string;
  JS::JsonObjectOrArrayRef array;
  int number = 0;

  JS_OBJECT(JS_MEMBER(string), JS_MEMBER(array), JS_MEMBER(number));
};

static const char jsonArrayTest[] = R"json({
  "string": "foo",
  "array": [
    ["a","b","c"],
    {
      "sub object": 44.50
    },
    12345
  ],
  "number": 43
})json";

TEST_CASE("check_json_array", "json_struct")
{
  JS::ParseContext context(jsonArrayTest);
  JsonArrayTester obj;
  context.parseTo(obj);
  REQUIRE(context.error == JS::Error::NoError);
  REQUIRE(obj.string == "foo");
  REQUIRE(obj.array.data.size() > 0);
  REQUIRE(obj.number == 43);

  std::string out = JS::serializeStruct(obj);
  REQUIRE(out == jsonArrayTest);
}

TEST_CASE("check_json_object_or_array_array", "json_struct")
{
  JS::ParseContext context(jsonArrayTest);
  JsonObjectOrArrayArrayTester obj;
  context.parseTo(obj);
  REQUIRE(context.error == JS::Error::NoError);
  REQUIRE(obj.string == "foo");
  REQUIRE(obj.array.data.size() > 0);
  REQUIRE(obj.number == 43);

  std::string out = JS::serializeStruct(obj);
  REQUIRE(out == jsonArrayTest);
}

TEST_CASE("check_json_array_ref", "json_struct")
{
  JS::ParseContext context(jsonArrayTest);
  JsonArrayRefTester obj;
  context.parseTo(obj);
  REQUIRE(context.error == JS::Error::NoError);
  REQUIRE(obj.string == "foo");
  REQUIRE(obj.array.ref.size > 0);
  REQUIRE(obj.number == 43);

  std::string out = JS::serializeStruct(obj);
  REQUIRE(out == jsonArrayTest);
}

TEST_CASE("check_json_object_or_array_array_ref", "json_struct")
{
  JS::ParseContext context(jsonArrayTest);
  JsonObjectOrArrayArrayRefTester obj;
  context.parseTo(obj);
  REQUIRE(context.error == JS::Error::NoError);
  REQUIRE(obj.string == "foo");
  REQUIRE(obj.array.ref.size > 0);
  REQUIRE(obj.number == 43);

  std::string out = JS::serializeStruct(obj);
  REQUIRE(out == jsonArrayTest);
}

struct JsonMapTest
{
  std::unordered_map<std::string, JS::JsonTokens> map;

  JS_OBJECT(JS_MEMBER(map));
};

static const char jsonMapTest[] = R"json({
  "map": {
    "hello": { "some object": 3 },
    "bye": [4]
  }
})json";

TEST_CASE("check_json_map", "json_struct")
{
#ifdef JS_UNORDERED_MAP_HANDLER
  JS::ParseContext context(jsonMapTest);
  JsonMapTest obj;
  context.parseTo(obj);
  REQUIRE(context.error == JS::Error::NoError);
#endif
}

struct TypeHandlerTypes
{
  double doubleN;
  float floatN;
  int intN;
  unsigned int uintN;
  int64_t int64N;
  uint64_t uint64N;
  int16_t int16N;
  uint16_t uint16N;
  uint8_t uint8N;
  int8_t int8N;
  char charN;
  signed char scharN;   // Normally same as int8_t
  unsigned char ucharN; // Normally same as uint8_t
  bool boolN;

  JS_OBJECT(JS_MEMBER(doubleN), JS_MEMBER(floatN), JS_MEMBER(intN), JS_MEMBER(uintN), JS_MEMBER(int64N),
            JS_MEMBER(uint64N), JS_MEMBER(int16N), JS_MEMBER(uint16N), JS_MEMBER(uint8N), JS_MEMBER(int8N),
            JS_MEMBER(charN), JS_MEMBER(scharN), JS_MEMBER(ucharN), JS_MEMBER(boolN));
};

static const char jsonTypeHandlerTypes[] = R"json({
  "doubleN": 44.50,
  "floatN": 33.40,
  "intN": -345,
  "uintN": 567,
  "int64N": -1234,
  "uint64N": 987,
  "int16N": -23,
  "uint16N": 45,
  "uint8N": 255,
  "int8N": -127,
  "charN": 123,
  "scharN": -123,
  "ucharN": 234,
  "boolN": true
})json";

TEST_CASE("check_json_type_handler_types", "json_struct")
{
  JS::ParseContext context(jsonTypeHandlerTypes);
  TypeHandlerTypes obj;
  context.parseTo(obj);
  REQUIRE(context.error == JS::Error::NoError);
}

struct TypeHandlerIntTypes
{
  int intN;
  unsigned int uintN;
  int64_t int64N;
  uint64_t uint64N;
  int16_t int16N;
  uint16_t uint16N;
  uint8_t uint8N;
  int8_t int8N;
  char charN;
  signed char scharN;   // Normally same as int8_t
  unsigned char ucharN; // Normally same as uint8_t
  bool boolN;

  JS_OBJECT(JS_MEMBER(intN), JS_MEMBER(uintN), JS_MEMBER(int64N), JS_MEMBER(uint64N), JS_MEMBER(int16N),
            JS_MEMBER(uint16N), JS_MEMBER(uint8N), JS_MEMBER(int8N), JS_MEMBER(charN), JS_MEMBER(scharN),
            JS_MEMBER(ucharN), JS_MEMBER(boolN));
};

static const char jsonTypeHandlerIntTypes[] = R"json({
  "intN": -345,
  "uintN": 567,
  "int64N": -1234,
  "uint64N": 987,
  "int16N": -23,
  "uint16N": 45,
  "uint8N": 255,
  "int8N": -127,
  "charN": 123,
  "scharN": -123,
  "ucharN": 234,
  "boolN": true
})json";

TEST_CASE("check_json_type_handler_integer_types", "json_struct")
{
  JS::ParseContext context(jsonTypeHandlerIntTypes);
  TypeHandlerIntTypes obj;
  context.parseTo(obj);
  REQUIRE(context.error == JS::Error::NoError);

  std::string serialized = JS::serializeStruct(obj);

  JS::ParseContext context2(serialized);
  TypeHandlerIntTypes obj2;
  context2.parseTo(obj2);
  REQUIRE(context2.error == JS::Error::NoError);

  REQUIRE(obj.intN == -345);
  REQUIRE(obj.uintN == 567);
  REQUIRE(obj.int64N == -1234);
  REQUIRE(obj.uint64N == 987);
  REQUIRE(obj.int16N == -23);
  REQUIRE(obj.uint16N == 45);
  REQUIRE(obj.uint8N == 255);
  REQUIRE(obj.int8N == -127);
  REQUIRE(obj.charN == 123);
  REQUIRE(obj.scharN == -123);
  REQUIRE(obj.ucharN == 234);
  REQUIRE(obj.boolN == true);

  REQUIRE(obj.intN == obj2.intN);
  REQUIRE(obj.uintN == obj2.uintN);
  REQUIRE(obj.int64N == obj2.int64N);
  REQUIRE(obj.uint64N == obj2.uint64N);
  REQUIRE(obj.int16N == obj2.int16N);
  REQUIRE(obj.int16N == obj2.int16N);
  REQUIRE(obj.uint16N == obj2.uint16N);
  REQUIRE(obj.uint8N == obj2.uint8N);
  REQUIRE(obj.int8N == obj2.int8N);
  REQUIRE(obj.charN == obj2.charN);
  REQUIRE(obj.scharN == obj2.scharN);
  REQUIRE(obj.ucharN == obj2.ucharN);
  REQUIRE(obj.boolN == obj2.boolN);
}

struct ArrayTest
{
  double data[3];

  JS_OBJECT(JS_MEMBER(data));
};

static const char arrayTestJson[] = R"json({
  "data": [4, 5, 6]
})json";

TEST_CASE("check_json_array_test", "json_struct")
{
  JS::ParseContext context(arrayTestJson);
  ArrayTest obj;
  context.parseTo(obj);
  REQUIRE(context.error == JS::Error::NoError);
}

struct SkipTestBase
{
  std::string name;
  int id;

  JS_OBJECT(JS_MEMBER(name), JS_MEMBER(id));
};

struct SkipTestInternalContainer
{
  std::vector<float> items;

  JS_OBJECT(JS_MEMBER(items));
};

struct SkipTestNameContainer
{
  std::string name;
  int id;
  SkipTestInternalContainer container;

  JS_OBJECT(JS_MEMBER(name), JS_MEMBER(id), JS_MEMBER(container));
};

struct SkipTestSubClass : public SkipTestBase
{
  float value;
  std::vector<SkipTestNameContainer> skip_test_list_01;
  std::vector<SkipTestNameContainer> skip_test_list_02;

  JS_OBJECT_WITH_SUPER(JS_SUPER_CLASSES(JS_SUPER_CLASS(SkipTestBase)), JS_MEMBER(value), JS_MEMBER(skip_test_list_01),
                       JS_MEMBER(skip_test_list_02));
};

static const char jsonSkipTest[] = R"json(
{
  "skip_test_list_01": [
    {
      "id": 1,
      "container": {
        "items": []
      },
      "skip_me": [],
      "name": "list01"
    },
    {
      "name": "list02",
      "skip_me": [],
      "container": {
        "items": [1.1, 2.2, 3.3]
      },
      "id": 2
    },
    {
      "skip_me": [],
      "name": "list03",
      "id": 3,
      "container": {
        "items": [0, 1, 2]
      }
    }
  ],
  "skip_test_list_02": [
    {
      "name": "list01",
      "id": 1,
      "container": {
        "items": []
      },
      "skip_me": []
    },
    {
      "name": "list02",
      "skip_me": [],
      "container": {
        "items": []
      },
      "id": 2
    },
    {
      "container": {
        "items": []
      },
      "skip_me": [],
      "name": "list03",
      "id": 3
    }
  ],
  "value": 3.14,
  "name": "base_name",
  "id": 444
}
)json";

TEST_CASE("check_json_skip_test", "json_struct")
{
  JS::ParseContext base_context(jsonSkipTest);
  SkipTestBase base;
  base_context.parseTo(base);
  REQUIRE(base_context.error == JS::Error::NoError);
  REQUIRE(base.name == "base_name");
  REQUIRE(base.id == 444);

  JS::ParseContext sub_context(jsonSkipTest);
  SkipTestSubClass sub;
  sub_context.parseTo(sub);
  REQUIRE(sub_context.error == JS::Error::NoError);
  REQUIRE(sub.skip_test_list_01[2].name == "list03");
  REQUIRE(sub.skip_test_list_02[1].name == "list02");
  REQUIRE(sub.name == "base_name");
  REQUIRE(sub.id == 444);
}

static const char multi_top_level_json[] = R"json({ a: 1}{a: 2}{a:3})json";
struct MultiTopLevel
{
  int a;
  JS_OBJECT(JS_MEMBER(a));
};

TEST_CASE("check_multi_top_level_json", "json_struct")
{
  JS::ParseContext pc(multi_top_level_json);
  pc.tokenizer.allowAsciiType(true);
  MultiTopLevel t;
  for (int i = 0; i < 3; i++)
  {
    REQUIRE(pc.tokenizer.currentPosition() < multi_top_level_json + sizeof(multi_top_level_json) - 1);
    pc.parseTo(t);
    REQUIRE(t.a == i + 1);
  }
  REQUIRE(pc.tokenizer.currentPosition() == multi_top_level_json + sizeof(multi_top_level_json) - 1);
}

static const char escapedJson[] = R"json({
  "some_text": "more\"_te\\xt",
  "sub_object": {
    "more_data": "so\\me \"text",
    "a_float": 1.2,
    "boolean_member": false
  }
})json";

template <typename T>
struct EscapedOuterStruct
{
  std::string some_text;
  T sub_object;

  JS_OBJECT(JS_MEMBER(some_text), JS_MEMBER(sub_object));
};

struct EscapedSubObject
{
  std::string more_data;
  float a_float;
  bool boolean_member;

  JS_OBJECT(JS_MEMBER(more_data), JS_MEMBER(a_float), JS_MEMBER(boolean_member));
};

TEST_CASE("check_json_escaped", "json_struct")
{
  JS::ParseContext context(escapedJson);
  EscapedOuterStruct<EscapedSubObject> data;
  context.parseTo(data);
  REQUIRE(context.error == JS::Error::NoError);
  std::string equals("more\"_te\\xt");
  REQUIRE(data.some_text == equals);
  std::string json = JS::serializeStruct(data);
};

struct OutsideMeta
{
  std::string data;
  float a;
};

} // namespace json_struct_test

JS_OBJECT_EXTERNAL(json_struct_test::OutsideMeta, JS_MEMBER(data), JS_MEMBER(a));

namespace json_struct_test
{

static const char outside_json[] = R"json({
  "data": "this is some text",
  "a": 44.5
})json";

TEST_CASE("check_json_meta_outside", "json_struct")
{
  JS::ParseContext context(outside_json);
  OutsideMeta data;
  context.parseTo(data);
  REQUIRE(context.error == JS::Error::NoError);
  REQUIRE(data.data == "this is some text");
  REQUIRE(data.a == 44.5);
}

struct FloatingPointAtTheEnd
{
  bool enabled;
  float value;

  JS_OBJECT(JS_MEMBER(enabled), JS_MEMBER(value));
};

static const char short_floating_at_the_end[] = R"json({
  "enabled": true,
  "value": 0.5
})json";

TEST_CASE("check_short_floating_point", "json_struct")
{
  JS::ParseContext context(short_floating_at_the_end);
  FloatingPointAtTheEnd data;
  context.parseTo(data);
  REQUIRE(context.error == JS::Error::NoError);
}

struct InvalidFloating
{
  bool enabled;
  float value;
  JS_OBJECT(JS_MEMBER(enabled), JS_MEMBER(value));
};

static const char invalid_floating[] = R"json({
  "enabled": true,
  "value": 0.5.e..-.E.5
})json";

TEST_CASE("check_invalid_floating_point", "json_struct")
{
  JS::ParseContext context(invalid_floating);
  FloatingPointAtTheEnd data;
  context.parseTo(data);
  REQUIRE(context.error == JS::Error::FailedToParseFloat);
}

static const char moreEscapedJsonAtEnd[] = R"json({
  "some_text": "more\n",
  "some_other": "tests\"",
  "pure_escape": "\n",
  "strange_escape": "foo\s",
  "pure_strange_escape": "\k",
  "empty_string": ""
  }
})json";

struct MoreEscapedStruct
{
  std::string some_text;
  std::string some_other;
  std::string pure_escape;
  std::string strange_escape;
  std::string pure_strange_escape;
  std::string empty_string;
  JS_OBJECT(JS_MEMBER(some_text), JS_MEMBER(some_other), JS_MEMBER(pure_escape), JS_MEMBER(strange_escape),
            JS_MEMBER(pure_strange_escape), JS_MEMBER(empty_string));
};

TEST_CASE("check_json_escaped_end", "json_struct")
{
  JS::ParseContext context(moreEscapedJsonAtEnd);
  MoreEscapedStruct data;
  context.parseTo(data);
  REQUIRE(context.error == JS::Error::NoError);
  REQUIRE(data.some_text == std::string("more\n"));
  REQUIRE(data.some_other == std::string("tests\""));
  REQUIRE(data.pure_escape == std::string("\n"));
  REQUIRE(data.strange_escape == std::string("foo\\s"));
  REQUIRE(data.pure_strange_escape == std::string("\\k"));
  std::string json = JS::serializeStruct(data);
};

} // namespace json_struct_test
