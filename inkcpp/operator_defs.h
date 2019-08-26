// DO NOT INCLUDE! Internal use only

template<typename T>
value evaluate(Command command, const T& left, const T& right)
{
	switch (command)
	{
	case binary::ADD:
		return left + right;
	case binary::IS_EQUAL:
		return left == right;
	}

	throw std::exception("FUCK");
}