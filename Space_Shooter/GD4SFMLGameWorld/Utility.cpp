#include "Utility.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>

#include <random>
#include <cmath>
#include <ctime>
#include <cassert>

namespace
{
	std::default_random_engine createRandomEngine()
	{
		auto seed = static_cast<unsigned long>(std::time(nullptr));
		return std::default_random_engine(seed);
	}

	auto RandomEngine = createRandomEngine();
}



void centreOrigin(sf::Sprite& sprite)
{
	sf::FloatRect bounds = sprite.getLocalBounds();
	sprite.setOrigin(std::floor(bounds.left + bounds.width / 2.f), std::floor(bounds.top + bounds.height / 2.f));
}

void centreOrigin(sf::Text& text)
{
	sf::FloatRect bounds = text.getLocalBounds();
	text.setOrigin(std::floor(bounds.left + bounds.width / 2.f), std::floor(bounds.top + bounds.height / 2.f));
}

float toDegree(float radian)
{
	return 180.f / 3.141592653589793238462643383f * radian;
}

float toRadian(float degree)
{
	return 3.141592653589793238462643383f / 180.f * degree;
}

float length(sf::Vector2f vector)
{
	return std::sqrt(vector.x * vector.x + vector.y * vector.y);
}

sf::Vector2f unitVector(sf::Vector2f vector)
{
	assert(vector != sf::Vector2f(0.f, 0.f));
	return vector / length(vector);
}

int randomInt(int exclusiveMax)
{
	std::uniform_int_distribution<> distr(0, exclusiveMax - 1);
	return distr(RandomEngine);
}

void centreOrigin(Animation& animation)
{
	sf::FloatRect bounds = animation.getLocalBounds();
	animation.setOrigin(std::floor(bounds.left + bounds.width / 2.f), std::floor(bounds.top + bounds.height / 2.f));
}
//
//std::string toString(sf::Keyboard::Key key) {
//#define BOOK_KEYTOSTRING_CASE(KEY) case sf::Keyboard::KEY:
//	return #KEY;
//	switch (key) 
//	{
//		BOOK_KEYTOSTRING_CASE(Unknown)
//			BOOK_KEYTOSTRING_CASE(A)
//			BOOK_KEYTOSTRING_CASE(B)
//			BOOK_KEYTOSTRING_CASE(C)
//			BOOK_KEYTOSTRING_CASE(D)
//			BOOK_KEYTOSTRING_CASE(E)
//			BOOK_KEYTOSTRING_CASE(F)
//			BOOK_KEYTOSTRING_CASE(G)
//			BOOK_KEYTOSTRING_CASE(H)
//			BOOK_KEYTOSTRING_CASE(I)
//			BOOK_KEYTOSTRING_CASE(J)
//			BOOK_KEYTOSTRING_CASE(K)
//			BOOK_KEYTOSTRING_CASE(L)
//			BOOK_KEYTOSTRING_CASE(M)
//			BOOK_KEYTOSTRING_CASE(N)
//			BOOK_KEYTOSTRING_CASE(O)
//			BOOK_KEYTOSTRING_CASE(P)
//			BOOK_KEYTOSTRING_CASE(Q)
//			BOOK_KEYTOSTRING_CASE(R)
//			BOOK_KEYTOSTRING_CASE(S)
//			BOOK_KEYTOSTRING_CASE(T)
//			BOOK_KEYTOSTRING_CASE(U)
//			BOOK_KEYTOSTRING_CASE(V)
//			BOOK_KEYTOSTRING_CASE(W)
//			BOOK_KEYTOSTRING_CASE(X)
//			BOOK_KEYTOSTRING_CASE(Y)
//			BOOK_KEYTOSTRING_CASE(Z)
//			BOOK_KEYTOSTRING_CASE(Num0)
//			BOOK_KEYTOSTRING_CASE(Num1)
//			BOOK_KEYTOSTRING_CASE(Num2)
//			BOOK_KEYTOSTRING_CASE(Num3)
//			BOOK_KEYTOSTRING_CASE(Num4)
//			BOOK_KEYTOSTRING_CASE(Num5)
//			BOOK_KEYTOSTRING_CASE(Num6)
//			BOOK_KEYTOSTRING_CASE(Num7)
//			BOOK_KEYTOSTRING_CASE(Num8)
//			BOOK_KEYTOSTRING_CASE(Num9)
//			BOOK_KEYTOSTRING_CASE(Escape)
//			BOOK_KEYTOSTRING_CASE(LControl)
//			BOOK_KEYTOSTRING_CASE(LShift)
//			BOOK_KEYTOSTRING_CASE(LAlt)
//			BOOK_KEYTOSTRING_CASE(LSystem)
//			BOOK_KEYTOSTRING_CASE(RControl)
//			BOOK_KEYTOSTRING_CASE(F1)
//			BOOK_KEYTOSTRING_CASE(F2)
//			BOOK_KEYTOSTRING_CASE(F3)
//			BOOK_KEYTOSTRING_CASE(F4)
//			BOOK_KEYTOSTRING_CASE(F5)
//			BOOK_KEYTOSTRING_CASE(F6)
//			BOOK_KEYTOSTRING_CASE(F7)
//			BOOK_KEYTOSTRING_CASE(F8)
//			BOOK_KEYTOSTRING_CASE(F9)
//			BOOK_KEYTOSTRING_CASE(F10)
//			BOOK_KEYTOSTRING_CASE(F11)
//			BOOK_KEYTOSTRING_CASE(F12)
//			BOOK_KEYTOSTRING_CASE(Pause)
//	}
}

