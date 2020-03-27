/*
Conor Byrne
D00185566
Charlie Duff
D00183790
*/
template<typename T>

std::string toString(const T& value)
{
	std::stringstream stream;
	stream << value;
	return stream.str();
}
