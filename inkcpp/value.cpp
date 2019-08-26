#include "pch.h"
#include "value.h"

namespace binary 
{
	namespace runtime 
	{
		value value::cast(ValueType type) const
		{
			// No need to cast when we're of the same type
			if (type == _type)
				return *this;

			switch (_type)
			{
			case INTEGER:
				switch (type)
				{
				case FLOAT:
					return (float)_intValue;
				case STRING:
					std::to_string(_intValue);
				}
			case FLOAT:
				switch (type)
				{
				case INTEGER:
					return (int)_floatValue;
				case STRING:
					return std::to_string(_floatValue);
				}
				break;
			case STRING:
				switch (type)
				{
				case INTEGER:
					return std::stoi(_stringValue);
				case FLOAT:
					return std::stof(_stringValue);
				}
				break;
			}

			throw new std::exception("Bad cast"); // TODO
		}
	}
}