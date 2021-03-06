/*
Conor Byrne
D00185566
Charlie Duff
D00183790
*/
#include "GameServer.hpp"
#include "NetworkProtocol.hpp"
#include "Utility.hpp"
#include "Pickup.hpp"
#include "Aircraft.hpp"
#include <iostream>

#include <SFML/Network/Packet.hpp>
#include <SFML/System.hpp>
//CD - Added by me following instructions from screencast, values were edited to work with our game
GameServer::RemotePeer::RemotePeer()
	: ready(false)
	, timedOut(false)
{
	socket.setBlocking(false);
}

GameServer::GameServer(sf::Vector2f battlefieldSize)
	: mThread(&GameServer::executionThread, this)
	, mListeningState(false)
	, mClientTimeoutTime(sf::seconds(3.f))
	, mMaxConnectedPlayers(10)
	, mConnectedPlayers(0)
	, mWorldHeight(768.f)
	, mBattleFieldRect(0.f, mWorldHeight - battlefieldSize.y, battlefieldSize.x, battlefieldSize.y)
	, mBattleFieldScrollSpeed(-50.f)
	, mAircraftCount(0)
	, mPeers(1)
	, mAircraftIdentifierCounter(1)
	, mWaitingThreadEnd(false)
	, mLastSpawnTime(sf::Time::Zero)
	, mTimeForNextSpawn(sf::seconds(5.f))
{
	mListenerSocket.setBlocking(false);
	mPeers[0].reset(new RemotePeer());
	mThread.launch();
}

GameServer::~GameServer()
{
	mWaitingThreadEnd = true;
	mThread.wait();
}

void GameServer::notifyPlayerRealtimeChange(sf::Int32 aircraftIdentifier, sf::Int32 action, bool actionEnabled)
{
	for (std::size_t i = 0; i < mConnectedPlayers; ++i)
	{
		if (mPeers[i]->ready)
		{
			sf::Packet packet;
			packet << static_cast<sf::Int32>(Server::PacketType::PlayerRealtimeChange);
			packet << aircraftIdentifier;
			packet << action;
			packet << actionEnabled;

			mPeers[i]->socket.send(packet);
		}
	}
}

void GameServer::notifyPlayerEvent(sf::Int32 aircraftIdentifier, sf::Int32 action)
{
	for (std::size_t i = 0; i < mConnectedPlayers; ++i)
	{
		if (mPeers[i]->ready)
		{
			sf::Packet packet;
			packet << static_cast<sf::Int32>(Server::PacketType::PlayerEvent);
			packet << aircraftIdentifier;
			packet << action;

			mPeers[i]->socket.send(packet);
		}
	}
}

void GameServer::notifyPlayerSpawn(sf::Int32 aircraftIdentifier)
{
	for (std::size_t i = 0; i < mConnectedPlayers; ++i)
	{
		if (mPeers[i]->ready)
		{
			sf::Packet packet;
			packet << static_cast<sf::Int32>(Server::PacketType::PlayerConnect);
			packet << aircraftIdentifier << mAircraftInfo[aircraftIdentifier].position.x << mAircraftInfo[aircraftIdentifier].position.y;
			mPeers[i]->socket.send(packet);
		}
	}
}

void GameServer::setListening(bool enable)
{
	// Check if it isn't already listening
	if (enable)
	{
		if (!mListeningState)
			mListeningState = (mListenerSocket.listen(ServerPort) == sf::TcpListener::Done);
	}
	else
	{
		mListenerSocket.close();
		mListeningState = false;
	}
}

void GameServer::executionThread()
{
	setListening(true);

	sf::Time stepInterval = sf::seconds(1.f / 20);
	sf::Time stepTime = sf::Time::Zero;
	sf::Time tickInterval = sf::seconds(1.f / 30.f);
	sf::Time tickTime = sf::Time::Zero;
	sf::Clock stepClock, tickClock;

	while (!mWaitingThreadEnd)
	{
		handleIncomingPackets();
		handleIncomingConnections();

		stepTime += stepClock.getElapsedTime();
		stepClock.restart();

		tickTime += tickClock.getElapsedTime();
		tickClock.restart();

		// Fixed update step
		while (stepTime >= stepInterval)
		{
			mBattleFieldRect.left += mBattleFieldScrollSpeed * stepInterval.asSeconds();
			stepTime -= stepInterval;
		}

		// Fixed tick step
		while (tickTime >= tickInterval)
		{
			tick();
			tickTime -= tickInterval;
		}

		// Sleep to prevent server from consuming 100% CPU
		sf::sleep(sf::milliseconds(100));
	}
}

void GameServer::tick()
{
	updateClientState();
	float planeLocation = 0.f;

	bool allAircraftsDone = false;      // Check for mission success = all planes with position.y < offset
	for (auto pair : mAircraftInfo)
	{

		std::cout << pair.second.score << std::endl;
		//std::cout << pair.second.score << std::endl;
		// As long as one player has not crossed the finish line yet, set variable to false
		if (pair.second.position.x > 5000.f) {
			allAircraftsDone = true;
		}
		else {
			allAircraftsDone = false;
		}
	}

	//std::cout << planeLocation << std::endl;
	if (allAircraftsDone)
	{
		//std::cout << "Success" << std::endl;
		sf::Packet missionSuccessPacket;
		missionSuccessPacket << static_cast<sf::Int32>(Server::PacketType::MissionSuccess);
		sendToAll(missionSuccessPacket);
	}

	// Remove IDs of aircraft that have been destroyed (relevant if a client has two, and loses one)
	for (auto itr = mAircraftInfo.begin(); itr != mAircraftInfo.end(); )
	{
		if (itr->second.hitpoints <= 0)
			mAircraftInfo.erase(itr++);
		else
			++itr;
	}

	// Check if its time to attempt to spawn enemies
	if (now() >= mTimeForNextSpawn + mLastSpawnTime)
	{
		// No more enemies are spawned near the end
		if (mBattleFieldRect.left < 1900.f)
		{
			std::size_t enemyCount = 2;
			float spawnCenter = static_cast<float>(300 + randomInt(400));

			// In case only one enemy is being spawned, it appears directly at the spawnCenter
			float planeDistance = 0.f;
			float nextSpawnPosition = spawnCenter;
			float xAxisSpawn = mAircraftInfo[1].position.x;

			// In case there are two enemies being spawned together, each is spawned at each side of the spawnCenter, with a minimum distance
			if (enemyCount > 1)
			{
				planeDistance = static_cast<float>(250);
				nextSpawnPosition = spawnCenter - planeDistance;
			}

			// Send the spawn orders to all clients
			for (std::size_t i = 0; i < enemyCount; ++i)
			{
				
				//CD - Second enemy type now spawns in multiplayer, only one of second enemy type will spawn while the enemy size can be extended to include more of the raptor enemy in one spawn
				xAxisSpawn += 50 + randomInt(50);
				sf::Packet packet;
				packet << static_cast<sf::Int32>(Server::PacketType::SpawnEnemy);
				if (i < 1) 
				{
					packet << static_cast<sf::Int32>(AircraftID::Avenger);
				}
				else
				{
					packet << static_cast<sf::Int32>(AircraftID::Raptor);
					
				}
				packet << xAxisSpawn;
				packet << nextSpawnPosition;

				nextSpawnPosition += planeDistance / 2.f;

				sendToAll(packet);
			}

			mLastSpawnTime = now();
			mTimeForNextSpawn = sf::milliseconds(2000 + randomInt(6000));
		}
	}
}

sf::Time GameServer::now() const
{
	return mClock.getElapsedTime();
}

void GameServer::handleIncomingPackets()
{
	bool detectedTimeout = false;

	for (PeerPtr & peer : mPeers)
	{
		if (peer->ready)
		{
			sf::Packet packet;
			while (peer->socket.receive(packet) == sf::Socket::Done)
			{
				// Interpret packet and react to it
				handleIncomingPacket(packet, *peer, detectedTimeout);

				// Packet was indeed received, update the ping timer
				peer->lastPacketTime = now();
				packet.clear();
			}

			if (now() >= peer->lastPacketTime + mClientTimeoutTime)
			{
				peer->timedOut = true;
				detectedTimeout = true;
			}
		}
	}

	if (detectedTimeout)
		handleDisconnections();
}

void GameServer::handleIncomingPacket(sf::Packet& packet, RemotePeer& receivingPeer, bool& detectedTimeout)
{
	sf::Int32 packetType;
	packet >> packetType;

	switch (packetType)
	{
		case static_cast<int>(Client::PacketType::Quit) :
		{
			receivingPeer.timedOut = true;
			detectedTimeout = true;
		} break;

			case static_cast<int>(Client::PacketType::PlayerEvent) :
			{
				sf::Int32 aircraftIdentifier;
				sf::Int32 action;
				packet >> aircraftIdentifier >> action;

				notifyPlayerEvent(aircraftIdentifier, action);
			} break;

				case static_cast<int>(Client::PacketType::PlayerRealtimeChange) :
				{
					sf::Int32 aircraftIdentifier;
					sf::Int32 action;
					bool actionEnabled;
					packet >> aircraftIdentifier >> action >> actionEnabled;
					mAircraftInfo[aircraftIdentifier].realtimeActions[action] = actionEnabled;
					notifyPlayerRealtimeChange(aircraftIdentifier, action, actionEnabled);
				} break;

					case static_cast<int>(Client::PacketType::RequestCoopPartner) :
					{
						receivingPeer.aircraftIdentifiers.push_back(mAircraftIdentifierCounter);
						mAircraftInfo[mAircraftIdentifierCounter].position = sf::Vector2f(mBattleFieldRect.width / 2, mBattleFieldRect.top + mBattleFieldRect.height / 2);
						mAircraftInfo[mAircraftIdentifierCounter].hitpoints = 100;
						mAircraftInfo[mAircraftIdentifierCounter].missileAmmo = 2;
						mAircraftInfo[mAircraftIdentifierCounter].score = 0;

						sf::Packet requestPacket;
						requestPacket << static_cast<sf::Int32>(Server::PacketType::AcceptCoopPartner);
						requestPacket << mAircraftIdentifierCounter;
						requestPacket << mAircraftInfo[mAircraftIdentifierCounter].position.x;
						requestPacket << mAircraftInfo[mAircraftIdentifierCounter].position.y;

						receivingPeer.socket.send(requestPacket);
						mAircraftCount++;

						// Inform every other peer about this new plane
						for (PeerPtr & peer : mPeers)
						{
							if (peer.get() != &receivingPeer && peer->ready)
							{
								sf::Packet notifyPacket;
								notifyPacket << static_cast<sf::Int32>(Server::PacketType::PlayerConnect);
								notifyPacket << mAircraftIdentifierCounter;
								notifyPacket << mAircraftInfo[mAircraftIdentifierCounter].position.x;
								notifyPacket << mAircraftInfo[mAircraftIdentifierCounter].position.y;
								peer->socket.send(notifyPacket);
							}
						}
						mAircraftIdentifierCounter++;
					} break;

						case static_cast<int>(Client::PacketType::PositionUpdate) :
						{
							sf::Int32 numAircrafts;
							packet >> numAircrafts;

							for (sf::Int32 i = 0; i < numAircrafts; ++i)
							{
								sf::Int32 aircraftIdentifier;
								sf::Int32 aircraftHitpoints;
								sf::Int32 missileAmmo;
								sf::Vector2f aircraftPosition;
								sf::Int32 score;
								packet >> aircraftIdentifier >> aircraftPosition.x >> aircraftPosition.y >> aircraftHitpoints >> missileAmmo >> score;
								mAircraftInfo[aircraftIdentifier].position = aircraftPosition;
								mAircraftInfo[aircraftIdentifier].hitpoints = aircraftHitpoints;
								mAircraftInfo[aircraftIdentifier].missileAmmo = missileAmmo;
								mAircraftInfo[aircraftIdentifier].score = score;
							}
						} break;

							case static_cast<int>(Client::PacketType::GameEvent) :
							{
								sf::Int32 action;
								float x;
								float y;

								packet >> action;
								packet >> x;
								packet >> y;

								// Enemy explodes: With certain probability, drop pickup
								// To avoid multiple messages spawning multiple pickups, only listen to first peer (host)
								if (action == static_cast<int>(GameActionID::EnemyExplode) && randomInt(3) == 0 && &receivingPeer == mPeers[0].get())
								{
									sf::Packet packet;
									packet << static_cast<sf::Int32>(Server::PacketType::SpawnPickup);
									packet << static_cast<sf::Int32>(randomInt(static_cast<int>(PickupID::TypeCount)));
									packet << x;
									packet << y;

									sendToAll(packet);
								}
							}
	}
}

void GameServer::updateClientState()
{
	sf::Packet updateClientStatePacket;
	updateClientStatePacket << static_cast<sf::Int32>(Server::PacketType::UpdateClientState);
	updateClientStatePacket << static_cast<float>(mBattleFieldRect.top + mBattleFieldRect.height);
	updateClientStatePacket << static_cast<sf::Int32>(mAircraftInfo.size());

	for (auto aircraft : mAircraftInfo)
		updateClientStatePacket << aircraft.first << aircraft.second.position.x << aircraft.second.position.y;

	sendToAll(updateClientStatePacket);
}

void GameServer::handleIncomingConnections()
{
	if (!mListeningState)
		return;

	if (mListenerSocket.accept(mPeers[mConnectedPlayers]->socket) == sf::TcpListener::Done)
	{
		// order the new client to spawn its own plane ( player 1 )
		mAircraftInfo[mAircraftIdentifierCounter].position = sf::Vector2f(mBattleFieldRect.width / 2, mBattleFieldRect.top + mBattleFieldRect.height / 2);
		mAircraftInfo[mAircraftIdentifierCounter].hitpoints = 100;
		mAircraftInfo[mAircraftIdentifierCounter].missileAmmo = 2;
		mAircraftInfo[mAircraftIdentifierCounter].score = 0;

		sf::Packet packet;
		packet << static_cast<sf::Int32>(Server::PacketType::SpawnSelf);
		packet << mAircraftIdentifierCounter;
		packet << mAircraftInfo[mAircraftIdentifierCounter].position.x;
		packet << mAircraftInfo[mAircraftIdentifierCounter].position.y;

		mPeers[mConnectedPlayers]->aircraftIdentifiers.push_back(mAircraftIdentifierCounter);

		broadcastMessage("New player!");
		informWorldState(mPeers[mConnectedPlayers]->socket);
		notifyPlayerSpawn(mAircraftIdentifierCounter++);

		mPeers[mConnectedPlayers]->socket.send(packet);
		mPeers[mConnectedPlayers]->ready = true;
		mPeers[mConnectedPlayers]->lastPacketTime = now(); // prevent initial timeouts
		mAircraftCount++;
		mConnectedPlayers++;

		if (mConnectedPlayers >= mMaxConnectedPlayers)
			setListening(false);
		else // Add a new waiting peer
			mPeers.push_back(PeerPtr(new RemotePeer()));
	}
}

void GameServer::handleDisconnections()
{
	for (auto itr = mPeers.begin(); itr != mPeers.end(); )
	{
		if ((*itr)->timedOut)
		{
			// Inform everyone of the disconnection, erase 
			for (sf::Int32 identifier : (*itr)->aircraftIdentifiers)
			{
				sendToAll(sf::Packet() << static_cast<sf::Int32>(Server::PacketType::PlayerDisconnect) << identifier);

				mAircraftInfo.erase(identifier);
			}

			mConnectedPlayers--;
			mAircraftCount -= (*itr)->aircraftIdentifiers.size();

			itr = mPeers.erase(itr);

			// Go back to a listening state if needed
			if (mConnectedPlayers < mMaxConnectedPlayers)
			{
				mPeers.push_back(PeerPtr(new RemotePeer()));
				setListening(true);
			}

			broadcastMessage("An ally has disconnected.");
		}
		else
		{
			++itr;
		}
	}
}

// Tell the newly connected peer about how the world is currently
void GameServer::informWorldState(sf::TcpSocket& socket)
{
	sf::Packet packet;
	packet << static_cast<sf::Int32>(Server::PacketType::InitialState);
	packet << mWorldHeight << mBattleFieldRect.top + mBattleFieldRect.height;
	packet << static_cast<sf::Int32>(mAircraftCount);

	for (std::size_t i = 0; i < mConnectedPlayers; ++i)
	{
		if (mPeers[i]->ready)
		{
			for (sf::Int32 identifier : mPeers[i]->aircraftIdentifiers)
				packet << identifier << mAircraftInfo[identifier].position.x << mAircraftInfo[identifier].position.y << mAircraftInfo[identifier].hitpoints << mAircraftInfo[identifier].missileAmmo << mAircraftInfo[identifier].score;
		}
	}

	socket.send(packet);
}

void GameServer::broadcastMessage(const std::string& message)
{
	for (std::size_t i = 0; i < mConnectedPlayers; ++i)
	{
		if (mPeers[i]->ready)
		{
			sf::Packet packet;
			packet << static_cast<sf::Int32>(Server::PacketType::BroadcastMessage);
			packet << message;

			mPeers[i]->socket.send(packet);
		}
	}
}

void GameServer::sendToAll(sf::Packet& packet)
{
	for (PeerPtr & peer : mPeers)
	{
		if (peer->ready)
			peer->socket.send(packet);
	}
}