/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2021, 🍀☀🌕🌥 🌊
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

/**
 * @file schema_tests.cpp
 * @brief Unit tests for container_schema constraint validators
 *
 * Tests cover:
 * - range() for integer and floating-point constraints
 * - length() for string/bytes length validation
 * - pattern() for regex matching
 * - one_of() for enum-style validation
 * - custom() for user-defined validators
 * - Nested schema validation
 */

#include <gtest/gtest.h>
#include "test_compat.h"
#include <container/core/container/schema.h>

using namespace container_module;

// ============================================================================
// Test Fixture
// ============================================================================

class SchemaTest : public ::testing::Test {
protected:
	std::unique_ptr<value_container> container;

	void SetUp() override {
		container = std::make_unique<value_container>();
	}

	void TearDown() override {
		container.reset();
	}
};

// ============================================================================
// Basic Schema Tests
// ============================================================================

TEST_F(SchemaTest, RequiredFieldValidation) {
	auto schema = container_schema()
		.require("name", value_types::string_value)
		.require("age", value_types::int_value);

	// Missing required fields
	auto errors = schema.validate_all(*container);
	EXPECT_EQ(errors.size(), 2);
	EXPECT_EQ(errors[0].code, validation_codes::missing_required);
	EXPECT_EQ(errors[1].code, validation_codes::missing_required);

	// Add required fields
	container->set("name", std::string("John"));
	container->set("age", 25);

	auto result = schema.validate(*container);
	EXPECT_FALSE(result.has_value());
}

TEST_F(SchemaTest, OptionalFieldValidation) {
	auto schema = container_schema()
		.require("name", value_types::string_value)
		.optional("email", value_types::string_value);

	container->set("name", std::string("John"));

	// Optional field missing is OK
	auto result = schema.validate(*container);
	EXPECT_FALSE(result.has_value());

	// Optional field with wrong type should fail
	container->set("email", 12345);

	result = schema.validate(*container);
	EXPECT_TRUE(result.has_value());
	EXPECT_EQ(result->code, validation_codes::type_mismatch);
}

TEST_F(SchemaTest, TypeMismatchValidation) {
	auto schema = container_schema()
		.require("age", value_types::int_value);

	container->set("age", std::string("not a number"));

	auto result = schema.validate(*container);
	EXPECT_TRUE(result.has_value());
	EXPECT_EQ(result->code, validation_codes::type_mismatch);
	EXPECT_NE(result->message.find("type mismatch"), std::string::npos);
}

// ============================================================================
// Integer Range Tests
// ============================================================================

TEST_F(SchemaTest, IntegerRangeValidation_InRange) {
	auto schema = container_schema()
		.require("age", value_types::int_value)
		.range("age", 0, 150);

	container->set("age", 25);

	auto result = schema.validate(*container);
	EXPECT_FALSE(result.has_value());
}

TEST_F(SchemaTest, IntegerRangeValidation_AtBoundary) {
	auto schema = container_schema()
		.require("age", value_types::int_value)
		.range("age", 0, 150);

	// Test minimum boundary
	container->set("age", 0);
	EXPECT_FALSE(schema.validate(*container).has_value());

	// Test maximum boundary
	container->set("age", 150);
	EXPECT_FALSE(schema.validate(*container).has_value());
}

TEST_F(SchemaTest, IntegerRangeValidation_BelowMinimum) {
	auto schema = container_schema()
		.require("age", value_types::int_value)
		.range("age", 0, 150);

	container->set("age", -1);

	auto result = schema.validate(*container);
	EXPECT_TRUE(result.has_value());
	EXPECT_EQ(result->code, validation_codes::out_of_range);
	EXPECT_NE(result->message.find("out of range"), std::string::npos);
}

TEST_F(SchemaTest, IntegerRangeValidation_AboveMaximum) {
	auto schema = container_schema()
		.require("age", value_types::int_value)
		.range("age", 0, 150);

	container->set("age", 200);

	auto result = schema.validate(*container);
	EXPECT_TRUE(result.has_value());
	EXPECT_EQ(result->code, validation_codes::out_of_range);
}

TEST_F(SchemaTest, IntegerRangeValidation_NegativeRange) {
	auto schema = container_schema()
		.require("temperature", value_types::int_value)
		.range("temperature", -40, 50);

	container->set("temperature", -20);
	EXPECT_FALSE(schema.validate(*container).has_value());

	container->set("temperature", -50);
	EXPECT_TRUE(schema.validate(*container).has_value());
}

TEST_F(SchemaTest, IntegerRangeValidation_LongLongType) {
	auto schema = container_schema()
		.require("big_number", value_types::llong_value)
		.range("big_number", static_cast<int64_t>(-1000000000000LL),
		       static_cast<int64_t>(1000000000000LL));

	container->set("big_number", 500000000000LL);
	EXPECT_FALSE(schema.validate(*container).has_value());

	container->set("big_number", 2000000000000LL);
	EXPECT_TRUE(schema.validate(*container).has_value());
}

// ============================================================================
// Double Range Tests
// ============================================================================

TEST_F(SchemaTest, DoubleRangeValidation_InRange) {
	auto schema = container_schema()
		.require("price", value_types::double_value)
		.range("price", 0.0, 1000000.0);

	container->set("price", 99.99);

	auto result = schema.validate(*container);
	EXPECT_FALSE(result.has_value());
}

TEST_F(SchemaTest, DoubleRangeValidation_AtBoundary) {
	auto schema = container_schema()
		.require("price", value_types::double_value)
		.range("price", 0.01, 1000000.0);

	container->set("price", 0.01);
	EXPECT_FALSE(schema.validate(*container).has_value());

	container->set("price", 1000000.0);
	EXPECT_FALSE(schema.validate(*container).has_value());
}

TEST_F(SchemaTest, DoubleRangeValidation_BelowMinimum) {
	auto schema = container_schema()
		.require("price", value_types::double_value)
		.range("price", 0.01, 1000000.0);

	container->set("price", 0.001);

	auto result = schema.validate(*container);
	EXPECT_TRUE(result.has_value());
	EXPECT_EQ(result->code, validation_codes::out_of_range);
}

TEST_F(SchemaTest, DoubleRangeValidation_AboveMaximum) {
	auto schema = container_schema()
		.require("price", value_types::double_value)
		.range("price", 0.01, 1000000.0);

	container->set("price", 1000000.01);

	auto result = schema.validate(*container);
	EXPECT_TRUE(result.has_value());
	EXPECT_EQ(result->code, validation_codes::out_of_range);
}

TEST_F(SchemaTest, FloatRangeValidation) {
	auto schema = container_schema()
		.require("percentage", value_types::float_value)
		.range("percentage", 0.0F, 100.0F);

	container->set("percentage", 50.5F);
	EXPECT_FALSE(schema.validate(*container).has_value());

	container->set("percentage", -1.0F);
	EXPECT_TRUE(schema.validate(*container).has_value());
}

// ============================================================================
// String Length Tests
// ============================================================================

TEST_F(SchemaTest, StringLengthValidation_InRange) {
	auto schema = container_schema()
		.require("username", value_types::string_value)
		.length("username", 3, 20);

	container->set("username", std::string("john_doe"));

	auto result = schema.validate(*container);
	EXPECT_FALSE(result.has_value());
}

TEST_F(SchemaTest, StringLengthValidation_AtBoundary) {
	auto schema = container_schema()
		.require("username", value_types::string_value)
		.length("username", 3, 20);

	// Minimum length
	container->set("username", std::string("abc"));
	EXPECT_FALSE(schema.validate(*container).has_value());

	// Maximum length
	container->set("username", std::string("12345678901234567890"));
	EXPECT_FALSE(schema.validate(*container).has_value());
}

TEST_F(SchemaTest, StringLengthValidation_TooShort) {
	auto schema = container_schema()
		.require("username", value_types::string_value)
		.length("username", 3, 20);

	container->set("username", std::string("ab"));

	auto result = schema.validate(*container);
	EXPECT_TRUE(result.has_value());
	EXPECT_EQ(result->code, validation_codes::invalid_length);
	EXPECT_NE(result->message.find("length"), std::string::npos);
}

TEST_F(SchemaTest, StringLengthValidation_TooLong) {
	auto schema = container_schema()
		.require("username", value_types::string_value)
		.length("username", 3, 20);

	container->set("username", std::string("this_username_is_way_too_long"));

	auto result = schema.validate(*container);
	EXPECT_TRUE(result.has_value());
	EXPECT_EQ(result->code, validation_codes::invalid_length);
}

TEST_F(SchemaTest, BytesLengthValidation) {
	auto schema = container_schema()
		.require("data", value_types::bytes_value)
		.length("data", 4, 1024);

	std::vector<uint8_t> valid_data = {0x01, 0x02, 0x03, 0x04, 0x05};
	container->set("data", valid_data);
	EXPECT_FALSE(schema.validate(*container).has_value());

	std::vector<uint8_t> short_data = {0x01, 0x02};
	container->set("data", short_data);
	EXPECT_TRUE(schema.validate(*container).has_value());
}

// ============================================================================
// Pattern Tests
// ============================================================================

TEST_F(SchemaTest, PatternValidation_Email) {
	auto schema = container_schema()
		.require("email", value_types::string_value)
		.pattern("email", R"(^[\w\.-]+@[\w\.-]+\.\w+$)");

	container->set("email", std::string("user@example.com"));
	EXPECT_FALSE(schema.validate(*container).has_value());

	container->set("email", std::string("user.name@subdomain.example.co.uk"));
	EXPECT_FALSE(schema.validate(*container).has_value());
}

TEST_F(SchemaTest, PatternValidation_InvalidEmail) {
	auto schema = container_schema()
		.require("email", value_types::string_value)
		.pattern("email", R"(^[\w\.-]+@[\w\.-]+\.\w+$)");

	container->set("email", std::string("not-an-email"));

	auto result = schema.validate(*container);
	EXPECT_TRUE(result.has_value());
	EXPECT_EQ(result->code, validation_codes::pattern_mismatch);
	EXPECT_NE(result->message.find("pattern"), std::string::npos);
}

TEST_F(SchemaTest, PatternValidation_PhoneNumber) {
	auto schema = container_schema()
		.require("phone", value_types::string_value)
		.pattern("phone", R"(^\+?[0-9]{10,15}$)");

	container->set("phone", std::string("+821012345678"));
	EXPECT_FALSE(schema.validate(*container).has_value());

	container->set("phone", std::string("1234567890"));
	EXPECT_FALSE(schema.validate(*container).has_value());

	container->set("phone", std::string("123"));
	EXPECT_TRUE(schema.validate(*container).has_value());
}

TEST_F(SchemaTest, PatternValidation_UUID) {
	auto schema = container_schema()
		.require("id", value_types::string_value)
		.pattern("id", R"(^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$)");

	container->set("id", std::string("550e8400-e29b-41d4-a716-446655440000"));
	EXPECT_FALSE(schema.validate(*container).has_value());

	container->set("id", std::string("not-a-uuid"));
	EXPECT_TRUE(schema.validate(*container).has_value());
}

// ============================================================================
// One-Of (Enum) Tests
// ============================================================================

TEST_F(SchemaTest, OneOfValidation_ValidValue) {
	auto schema = container_schema()
		.require("status", value_types::string_value)
		.one_of("status", {"active", "inactive", "pending"});

	container->set("status", std::string("active"));
	EXPECT_FALSE(schema.validate(*container).has_value());

	container->set("status", std::string("pending"));
	EXPECT_FALSE(schema.validate(*container).has_value());
}

TEST_F(SchemaTest, OneOfValidation_InvalidValue) {
	auto schema = container_schema()
		.require("status", value_types::string_value)
		.one_of("status", {"active", "inactive", "pending"});

	container->set("status", std::string("deleted"));

	auto result = schema.validate(*container);
	EXPECT_TRUE(result.has_value());
	EXPECT_EQ(result->code, validation_codes::not_in_allowed_values);
	EXPECT_NE(result->message.find("not in allowed"), std::string::npos);
}

TEST_F(SchemaTest, OneOfValidation_CaseSensitive) {
	auto schema = container_schema()
		.require("role", value_types::string_value)
		.one_of("role", {"admin", "user", "guest"});

	container->set("role", std::string("Admin"));
	EXPECT_TRUE(schema.validate(*container).has_value());

	container->set("role", std::string("admin"));
	EXPECT_FALSE(schema.validate(*container).has_value());
}

TEST_F(SchemaTest, OneOfValidation_Currency) {
	auto schema = container_schema()
		.require("currency", value_types::string_value)
		.one_of("currency", {"USD", "EUR", "GBP", "JPY", "KRW"});

	container->set("currency", std::string("KRW"));
	EXPECT_FALSE(schema.validate(*container).has_value());

	container->set("currency", std::string("BTC"));
	EXPECT_TRUE(schema.validate(*container).has_value());
}

// ============================================================================
// Custom Validator Tests
// ============================================================================

TEST_F(SchemaTest, CustomValidation_Success) {
	auto schema = container_schema()
		.require("age", value_types::int_value)
		.custom("age", [](const optimized_value& val) -> std::optional<std::string> {
			if (const auto* p = std::get_if<int>(&val.data)) {
				if (*p % 2 == 0) {
					return std::nullopt; // even numbers are valid
				}
				return "Age must be an even number";
			}
			return "Invalid type";
		});

	container->set("age", 20);
	EXPECT_FALSE(schema.validate(*container).has_value());
}

TEST_F(SchemaTest, CustomValidation_Failure) {
	auto schema = container_schema()
		.require("age", value_types::int_value)
		.custom("age", [](const optimized_value& val) -> std::optional<std::string> {
			if (const auto* p = std::get_if<int>(&val.data)) {
				if (*p % 2 == 0) {
					return std::nullopt;
				}
				return "Age must be an even number";
			}
			return "Invalid type";
		});

	container->set("age", 25);

	auto result = schema.validate(*container);
	EXPECT_TRUE(result.has_value());
	EXPECT_EQ(result->code, validation_codes::custom_validation_failed);
}

TEST_F(SchemaTest, CustomValidation_MultipleValidators) {
	auto schema = container_schema()
		.require("password", value_types::string_value)
		.length("password", 8, 100)
		.custom("password", [](const optimized_value& val) -> std::optional<std::string> {
			if (const auto* p = std::get_if<std::string>(&val.data)) {
				if (p->find_first_of("0123456789") == std::string::npos) {
					return "Password must contain at least one digit";
				}
				return std::nullopt;
			}
			return "Invalid type";
		})
		.custom("password", [](const optimized_value& val) -> std::optional<std::string> {
			if (const auto* p = std::get_if<std::string>(&val.data)) {
				if (p->find_first_of("!@#$%^&*") == std::string::npos) {
					return "Password must contain at least one special character";
				}
				return std::nullopt;
			}
			return "Invalid type";
		});

	container->set("password", std::string("SecurePass1!"));
	EXPECT_FALSE(schema.validate(*container).has_value());

	container->set("password", std::string("NoDigits!"));
	EXPECT_TRUE(schema.validate(*container).has_value());
}

// ============================================================================
// Nested Schema Tests
// ============================================================================

TEST_F(SchemaTest, NestedSchemaValidation_Success) {
	auto address_schema = container_schema()
		.require("street", value_types::string_value)
		.require("city", value_types::string_value)
		.require("zip", value_types::string_value)
		.pattern("zip", R"(^\d{5}(-\d{4})?$)");

	auto user_schema = container_schema()
		.require("name", value_types::string_value)
		.require("address", value_types::container_value, address_schema);

	auto address_container = std::make_shared<value_container>();
	address_container->set("street", std::string("123 Main St"));
	address_container->set("city", std::string("Springfield"));
	address_container->set("zip", std::string("12345"));

	container->set("name", std::string("John Doe"));
	container->set("address", address_container);

	auto result = user_schema.validate(*container);
	EXPECT_FALSE(result.has_value());
}

TEST_F(SchemaTest, NestedSchemaValidation_Failure) {
	auto address_schema = container_schema()
		.require("street", value_types::string_value)
		.require("city", value_types::string_value)
		.require("zip", value_types::string_value)
		.pattern("zip", R"(^\d{5}(-\d{4})?$)");

	auto user_schema = container_schema()
		.require("name", value_types::string_value)
		.require("address", value_types::container_value, address_schema);

	auto address_container = std::make_shared<value_container>();
	address_container->set("street", std::string("123 Main St"));
	address_container->set("city", std::string("Springfield"));
	address_container->set("zip", std::string("invalid-zip"));

	container->set("name", std::string("John Doe"));
	container->set("address", address_container);

	auto errors = user_schema.validate_all(*container);
	EXPECT_GT(errors.size(), 0);

	// Check that nested field path is included
	bool found_nested_error = false;
	for (const auto& err : errors) {
		if (err.field.find("address.zip") != std::string::npos) {
			found_nested_error = true;
			break;
		}
	}
	EXPECT_TRUE(found_nested_error);
}

// ============================================================================
// Combined Constraint Tests
// ============================================================================

TEST_F(SchemaTest, CombinedConstraints) {
	auto schema = container_schema()
		.require("username", value_types::string_value)
		.length("username", 3, 20)
		.pattern("username", R"(^[a-z][a-z0-9_]*$)")
		.require("age", value_types::int_value)
		.range("age", 18, 120)
		.require("status", value_types::string_value)
		.one_of("status", {"active", "inactive"});

	container->set("username", std::string("john_doe123"));
	container->set("age", 25);
	container->set("status", std::string("active"));

	EXPECT_FALSE(schema.validate(*container).has_value());
}

TEST_F(SchemaTest, CombinedConstraints_MultipleErrors) {
	auto schema = container_schema()
		.require("username", value_types::string_value)
		.length("username", 3, 20)
		.require("age", value_types::int_value)
		.range("age", 18, 120);

	container->set("username", std::string("ab"));  // too short
	container->set("age", 15);  // below minimum

	auto errors = schema.validate_all(*container);
	EXPECT_EQ(errors.size(), 2);
}

// ============================================================================
// Schema Utility Method Tests
// ============================================================================

TEST_F(SchemaTest, SchemaFieldCount) {
	auto schema = container_schema()
		.require("field1", value_types::string_value)
		.require("field2", value_types::int_value)
		.optional("field3", value_types::bool_value);

	EXPECT_EQ(schema.field_count(), 3);
}

TEST_F(SchemaTest, SchemaHasField) {
	auto schema = container_schema()
		.require("name", value_types::string_value)
		.optional("email", value_types::string_value);

	EXPECT_TRUE(schema.has_field("name"));
	EXPECT_TRUE(schema.has_field("email"));
	EXPECT_FALSE(schema.has_field("phone"));
}

TEST_F(SchemaTest, SchemaIsRequired) {
	auto schema = container_schema()
		.require("name", value_types::string_value)
		.optional("email", value_types::string_value);

	EXPECT_TRUE(schema.is_required("name"));
	EXPECT_FALSE(schema.is_required("email"));
	EXPECT_FALSE(schema.is_required("nonexistent"));
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(SchemaTest, EmptySchema) {
	auto schema = container_schema();

	container->set("anything", std::string("value"));

	auto result = schema.validate(*container);
	EXPECT_FALSE(result.has_value());
}

TEST_F(SchemaTest, EmptyContainer) {
	auto schema = container_schema()
		.optional("field", value_types::string_value);

	auto result = schema.validate(*container);
	EXPECT_FALSE(result.has_value());
}

TEST_F(SchemaTest, ConstraintOnUndefinedField) {
	auto schema = container_schema()
		.range("undefined_field", 0, 100);

	// range() on undefined field should be silently ignored
	EXPECT_EQ(schema.field_count(), 0);
}

TEST_F(SchemaTest, InvalidRegexPattern) {
	auto schema = container_schema()
		.require("field", value_types::string_value)
		.pattern("field", "[invalid(regex");

	container->set("field", std::string("test"));

	// Invalid regex pattern is silently ignored (compiled_pattern is nullopt)
	// This allows schema to validate successfully even with invalid patterns
	auto result = schema.validate(*container);
	EXPECT_FALSE(result.has_value());
}

// ============================================================================
// Schema Copy/Move Tests
// ============================================================================

TEST_F(SchemaTest, SchemaCopyConstruction) {
	auto original = container_schema()
		.require("name", value_types::string_value)
		.range("age", 0, 150);

	const auto copy = original;

	EXPECT_EQ(copy.field_count(), original.field_count());
	EXPECT_TRUE(copy.has_field("name"));
}

TEST_F(SchemaTest, SchemaMoveConstruction) {
	auto original = container_schema()
		.require("name", value_types::string_value);

	auto moved = std::move(original);

	EXPECT_EQ(moved.field_count(), 1);
	EXPECT_TRUE(moved.has_field("name"));
}

// ============================================================================
// Schema-Validated Deserialization Tests (Issue #249)
// ============================================================================

TEST_F(SchemaTest, DeserializeWithSchema_ValidData) {
	auto schema = container_schema()
		.require("name", value_types::string_value)
		.require("age", value_types::int_value)
		.range("age", 0, 150);

	// Create a valid container and serialize it
	auto source = std::make_unique<value_container>();
	source->set("name", std::string("Alice"));
	source->set("age", 30);
	auto serialize_result = source->serialize_string(value_container::serialization_format::binary);
	ASSERT_TRUE(kcenon::common::is_ok(serialize_result));
	auto serialized_data = kcenon::common::get_value(serialize_result);

	// Deserialize with schema validation
	auto target = std::make_unique<value_container>();
	auto result = target->deserialize_result(serialized_data, schema, false);

	EXPECT_TRUE(kcenon::common::is_ok(result));
	EXPECT_TRUE(target->get_validation_errors().empty());

	auto name_opt = target->get_value("name");
	EXPECT_TRUE(name_opt.has_value());
}

TEST_F(SchemaTest, DeserializeWithSchema_MissingRequiredField) {
	auto schema = container_schema()
		.require("name", value_types::string_value)
		.require("age", value_types::int_value);

	// Create a container missing required field
	auto source = std::make_unique<value_container>();
	source->set("name", std::string("Alice"));
	// Missing 'age' field
	auto serialize_result = source->serialize_string(value_container::serialization_format::binary);
	ASSERT_TRUE(kcenon::common::is_ok(serialize_result));
	auto serialized_data = kcenon::common::get_value(serialize_result);

	auto target = std::make_unique<value_container>();
	auto result = target->deserialize_result(serialized_data, schema, false);

	EXPECT_FALSE(kcenon::common::is_ok(result));
	EXPECT_FALSE(target->get_validation_errors().empty());
	EXPECT_EQ(target->get_validation_errors()[0].code, validation_codes::missing_required);
}

TEST_F(SchemaTest, DeserializeWithSchema_InvalidRange) {
	auto schema = container_schema()
		.require("age", value_types::int_value)
		.range("age", 0, 150);

	auto source = std::make_unique<value_container>();
	source->set("age", 200);  // Out of range
	auto serialize_result = source->serialize_string(value_container::serialization_format::binary);
	ASSERT_TRUE(kcenon::common::is_ok(serialize_result));
	auto serialized_data = kcenon::common::get_value(serialize_result);

	auto target = std::make_unique<value_container>();
	auto result = target->deserialize_result(serialized_data, schema, false);

	EXPECT_FALSE(kcenon::common::is_ok(result));
	EXPECT_FALSE(target->get_validation_errors().empty());
	EXPECT_EQ(target->get_validation_errors()[0].code, validation_codes::out_of_range);
}

TEST_F(SchemaTest, DeserializeWithSchema_ByteArray) {
	auto schema = container_schema()
		.require("name", value_types::string_value);

	auto source = std::make_unique<value_container>();
	source->set("name", std::string("Bob"));
	auto serialize_result = source->serialize(value_container::serialization_format::binary);
	ASSERT_TRUE(kcenon::common::is_ok(serialize_result));
	auto serialized_bytes = kcenon::common::get_value(serialize_result);

	auto target = std::make_unique<value_container>();
	auto result = target->deserialize_result(serialized_bytes, schema, false);

	EXPECT_TRUE(kcenon::common::is_ok(result));
	EXPECT_TRUE(target->get_validation_errors().empty());
}

TEST_F(SchemaTest, GetValidationErrors_Empty) {
	// New container should have no validation errors
	EXPECT_TRUE(container->get_validation_errors().empty());
}

TEST_F(SchemaTest, ClearValidationErrors) {
	auto schema = container_schema()
		.require("missing", value_types::string_value);

	auto serialize_result = container->serialize_string(value_container::serialization_format::binary);
	ASSERT_TRUE(kcenon::common::is_ok(serialize_result));
	auto serialized_data = kcenon::common::get_value(serialize_result);

	auto target = std::make_unique<value_container>();
	(void)target->deserialize_result(serialized_data, schema, false);
	EXPECT_FALSE(target->get_validation_errors().empty());

	target->clear_validation_errors();
	EXPECT_TRUE(target->get_validation_errors().empty());
}

TEST_F(SchemaTest, DeserializeWithSchema_CollectsAllErrors) {
	auto schema = container_schema()
		.require("name", value_types::string_value)
		.require("age", value_types::int_value)
		.require("email", value_types::string_value);

	// Empty container - all required fields missing
	auto serialize_result = container->serialize_string(value_container::serialization_format::binary);
	ASSERT_TRUE(kcenon::common::is_ok(serialize_result));
	auto serialized_data = kcenon::common::get_value(serialize_result);

	auto target = std::make_unique<value_container>();
	(void)target->deserialize_result(serialized_data, schema, false);

	// Should have 3 validation errors (one for each missing field)
	EXPECT_EQ(target->get_validation_errors().size(), 3);
}

TEST_F(SchemaTest, DeserializeWithSchema_PatternValidation) {
	auto schema = container_schema()
		.require("email", value_types::string_value)
		.pattern("email", R"(^[\w\.-]+@[\w\.-]+\.\w+$)");

	auto source = std::make_unique<value_container>();
	source->set("email", std::string("invalid-email"));  // No @ symbol
	auto serialize_result = source->serialize_string(value_container::serialization_format::binary);
	ASSERT_TRUE(kcenon::common::is_ok(serialize_result));
	auto serialized_data = kcenon::common::get_value(serialize_result);

	auto target = std::make_unique<value_container>();
	auto result = target->deserialize_result(serialized_data, schema, false);

	EXPECT_FALSE(kcenon::common::is_ok(result));
	EXPECT_EQ(target->get_validation_errors()[0].code, validation_codes::pattern_mismatch);
}

TEST_F(SchemaTest, DeserializeWithSchema_OneOfValidation) {
	auto schema = container_schema()
		.require("status", value_types::string_value)
		.one_of("status", {"active", "inactive", "pending"});

	auto source = std::make_unique<value_container>();
	source->set("status", std::string("unknown"));
	auto serialize_result = source->serialize_string(value_container::serialization_format::binary);
	ASSERT_TRUE(kcenon::common::is_ok(serialize_result));
	auto serialized_data = kcenon::common::get_value(serialize_result);

	auto target = std::make_unique<value_container>();
	auto result = target->deserialize_result(serialized_data, schema, false);

	EXPECT_FALSE(kcenon::common::is_ok(result));
	EXPECT_EQ(target->get_validation_errors()[0].code, validation_codes::not_in_allowed_values);
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
