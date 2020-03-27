/*
Charlie Duff
D00183790
*/
#pragma once
//CD - Added from screen cast, no packetTypes added as no extra were needed
#include <SFML/Config.hpp>
#include <SFML/System/Vector2.hpp>

const unsigned short ServerPort = 50000;

namespace Server
{
	//Packets that originate from the server
	enum class PacketType 
	{
		BroadcastMessage, //This takes an std::string and is used to send a message to all clients, which they can show on the screen for some seconds
		SpawnSelf, //This takes an sf::Int32 value for the aircraft identifier, and two float values for its initial position. These are used to spawn the clients player one aircraft
		InitialState, //This takes two float values, the world height and the initial scrolling in it, then sends an sf::Int32 with the count of the aircraft in the world, then for each aircraft it sends sf::Int32 identifier and two float values with the position of the aircraft
		PlayerEvent, //This takes two sf::Int32, the aircraft identifier and the identifier as declared in ActionID.hpp
		PlayerRealtimeChange, //the same as PlayerEvent but for real time actions. this means we are changing an ongoing state so add a Boolean value to the parameters
		PlayerConnect, //This is SpawnSelf but indicates that an aircraft from a different machine is entering the world.
		PlayerDisconnect, //Sends one sf::Int32 with identifier for aircraft that has disconnected
		AcceptCoopPartner, //This is used to tell the client that it can spawn a second player. It needs sf::Int32 with id for the new plane and two float values with the position of the aircraft
		SpawnEnemy, //Takes sf::Int32 with the type od the enemy and two float values with the position 
		SpawnPickup, //sf::Int32 with type of pickup, 2 floats for position
		UpdateClientState, //This takes a float with the current scrolling distance on the server, an sf::Int32 with the aircraft count. For each aircraft send identifier and two float values for position
		MissionSuccess //This takes no arguments. It is just used to tell the client that the game is over
	};
}

namespace Client
{
	// Packets originated in the client
	enum class PacketType
	{
		PlayerEvent, //This takes two sf::Int32 variables, an aircraft identifier, and the event to be triggered as defined in ActionID. It is used to request the server to trigger an event on the requested aircraft
		PlayerRealtimeChange, //This is the same as Playerevent but additionally takes a boolean to state whether the action is active or not
		RequestCoopPartner, //Takes no parameters. It is sent when the user presses the Return key to request a local partner. Its counterpart is AcceptCoopPartner
		PositionUpdate, //Takes an sf::Int32 variable with the number of local aircraft, and for each aircraft it packs another sf::Int32 variable for the identifer and two float values for the position
		GameEvent, //This informs the server of a specific happening in the client's game logic, such as enemy explosions
		Quit //Takes no parameters - it just informs the server that the game state is closing so it can remove its aricraft
	};
}