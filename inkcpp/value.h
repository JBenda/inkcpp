#pragma once

namespace binary
{
	namespace runtime
	{
		enum ValueType
		{
			VALUE_TYPES_START,

			INTEGER = VALUE_TYPES_START,
			FLOAT,
			STRING,
			DIVERT,

			NUM_VALUE_TYPES
		};

		class value
		{
		public:
			// == value constructors == 
			value() : _intValue(0), _type(INTEGER) { }
			value(int val) : _intValue(val), _type(INTEGER) { }
			value(float val) : _floatValue(val), _type(FLOAT) { }
			value(double val) : _floatValue((float)val), _type(FLOAT) { }
			value(const std::string& str) : _stringValue(str), _type(STRING) { }
			value(uint32_t divert_value) : _divertValue(divert_value), _type(ValueType::DIVERT) { }

			// Casts to another value type
			value cast(ValueType type) const;

			// Type accessor
			ValueType type() const { return _type; }

			// == accessors ==
			float as_float() const { return _floatValue; }
			int as_int() const { return _intValue; }
			const std::string& as_string() const { return _stringValue; }
			uint32_t as_divert() const { return _divertValue; }

			template<typename T>
			T get() const;

			template<>
			int get<int>() const { return as_int(); }

			template<>
			float get<float>() const { return as_float(); }

			template<>
			std::string get<std::string>() const { return as_string(); }

		private:
			// Type
			ValueType _type;

			// Data
			union
			{
				float _floatValue;
				int _intValue;
				uint32_t _divertValue;
			};
			std::string _stringValue;
		};

		std::ostream& operator << (std::ostream& out, const value& v);
	}
}
