#pragma once
#include <vector>
#include "LocalPlayer.cpp"
#include "Player.cpp"
#include "Math.cpp"
#include "Level.cpp"
#include "math.h"
#include "X11Utils.cpp"

class Aimbot
{
private:
    const int m_smoothing = 4;   // If you cross-hairs are not on target then this smoothness will be used.
    const int m_activationFOV = 3; // FOV for activation

public:
    void update(Level *level, LocalPlayer *localPlayer, std::vector<Player *> *players, X11Utils *x11Utils)
    {
        if (!x11Utils->triggerKeyDown())
            return;
        if (!level->isPlayable())
            return;
        if (localPlayer->isDead())
            return;
        if (localPlayer->isKnocked())
            return;
        double desiredViewAngleYaw = 0;
        if (level->isTrainingArea())
        {
            desiredViewAngleYaw = calculateDesiredYaw(m_localPlayer->getLocationX(),
                                                      m_localPlayer->getLocationY(),
                                                      31518,
                                                      -6712);
            desiredViewAnglePitch = calculateDesiredPitch(m_localPlayer->getLocationX(),
                                                          m_localPlayer->getLocationY(),
                                                          m_localPlayer->getLocationZ(),
                                                          31518,
                                                          -6712,
                                                          -29235);
        }
        else
        {
            if (m_lockedOnPlayer == nullptr)
                m_lockedOnPlayer = findClosestEnemy();
            if (m_lockedOnPlayer == nullptr)
                return;
            
            if(m_lockedOnPlayer != m_lockedOnPlayerBefore){
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }

            double distanceToTarget = math::calculateDistanceInMeters(m_localPlayer->getLocationX(),
                                                                      m_localPlayer->getLocationY(),
                                                                      m_localPlayer->getLocationZ(),
                                                                      m_lockedOnPlayer->getLocationX(),
                                                                      m_lockedOnPlayer->getLocationY(),
                                                                      m_lockedOnPlayer->getLocationZ());
            if (distanceToTarget > m_configLoader->getAimbotMaxRange())
                return;
            desiredViewAngleYaw = calculateDesiredYaw(m_localPlayer->getLocationX(),
                                                      m_localPlayer->getLocationY(),
                                                      m_lockedOnPlayer->getLocationX(),
                                                      m_lockedOnPlayer->getLocationY());
            desiredViewAnglePitch = calculateDesiredPitch(m_localPlayer->getLocationX(),
                                                          m_localPlayer->getLocationY(),
                                                          m_localPlayer->getLocationZ(),
                                                          m_lockedOnPlayer->getLocationX(),
                                                          m_lockedOnPlayer->getLocationY(),
                                                          m_lockedOnPlayer->getLocationZ());

            m_lockedOnPlayerBefore = m_lockedOnPlayer;
        }

        // Setup Pitch
        const double pitch = m_localPlayer->getPitch();
        const double pitchAngleDelta = calculatePitchAngleDelta(pitch, desiredViewAnglePitch);
        const double pitchAngleDeltaAbs = abs(pitchAngleDelta);
        if (pitchAngleDeltaAbs > (double)rand()/(RAND_MAX)+(rand()%1) + m_configLoader->getAimbotActivationFOV() / 2) 
            return;

        // Setup Yaw
        const double yaw = m_localPlayer->getYaw();
        const double angleDelta = calculateAngleDelta(yaw, desiredViewAngleYaw);
        const double angleDeltaAbs = abs(angleDelta);
        if (angleDeltaAbs > m_configLoader->getAimbotActivationFOV())
            return;
        double newYaw = flipYawIfNeeded(yaw + (angleDelta / m_configLoader->getAimbotSmoothing()));
        m_localPlayer->setYaw(newYaw);

        std::this_thread::sleep_for(std::chrono::milliseconds(m_configLoader->getAimbotSleepTime()));


    }
    double flipYawIfNeeded(double angle)
    {
        double myAngle = angle;
        if (myAngle > 180)
            myAngle = (360 - myAngle) * -1 + (double)rand()/(RAND_MAX)+(rand()%4);
        else if (myAngle < -180)
            myAngle = (360 + myAngle);
        return myAngle;
    }
    double calculatePitchAngleDelta(double oldAngle, double newAngle)
    {
        double wayA = newAngle - oldAngle;
        return wayA;
    }
    double calculateAngleDelta(double oldAngle, double newAngle)
    {
        double wayA = newAngle - oldAngle;
        double wayB = 360 - abs(wayA);
        if (wayA > 0 && wayB > 0)
            wayB *= -1 + (double)rand()/(RAND_MAX)+(rand()%4);
        if (abs(wayA) < abs(wayB))
            return wayA;
        return wayB;
    }
    double calculateDesiredYaw(
        double localPlayerLocationX,
        double localPlayerLocationY,
        double enemyPlayerLocationX,
        double enemyPlayerLocationY)
    {
        const double locationDeltaX = enemyPlayerLocationX - localPlayerLocationX;
        const double locationDeltaY = enemyPlayerLocationY - localPlayerLocationY;
        const double yawInRadians = atan2(locationDeltaY, locationDeltaX);
        const double yawInDegrees = yawInRadians * (180 / M_PI);
        return yawInDegrees;
    }
    double calculateDesiredPitch(
        double localPlayerLocationX,
        double localPlayerLocationY,
        double localPlayerLocationZ,
        double enemyPlayerLocationX,
        double enemyPlayerLocationY,
        double enemyPlayerLocationZ)
    {
        const double locationDeltaZ = enemyPlayerLocationZ - localPlayerLocationZ;
        const double distanceBetweenPlayers = math::calculateDistance2D(enemyPlayerLocationX, enemyPlayerLocationY, localPlayerLocationX, localPlayerLocationY);
        const double pitchInRadians = atan2(-locationDeltaZ, distanceBetweenPlayers);
        const double pitchInDegrees = pitchInRadians * (180 / M_PI);
        return pitchInDegrees;
    }
    Player *findClosestEnemy()
    {
        Player *closestPlayerSoFar = nullptr;
        double closestPlayerAngleSoFar;
        for (int i = 0; i < m_players->size(); i++)
        {
            Player *player = m_players->at(i);
            if (!player->isValid())
                continue;
            if (player->isKnocked())
                continue;
            if (player->getTeamNumber() == m_localPlayer->getTeamNumber())
                continue;
            if (!player->isVisible())
                continue;
            double desiredViewAngleYaw = calculateDesiredYaw(m_localPlayer->getLocationX(),
                                                             m_localPlayer->getLocationY(),
                                                             player->getLocationX(),
                                                             player->getLocationY());
            double angleDelta = calculateAngleDelta(m_localPlayer->getYaw(), desiredViewAngleYaw);
            if (closestPlayerSoFar == nullptr)
            {
                closestPlayerSoFar = player;
                closestPlayerAngleSoFar = abs(angleDelta);
            }
            else
            {

                if (abs(angleDelta) < closestPlayerAngleSoFar)
                {
                    closestPlayerSoFar = player;
                    closestPlayerAngleSoFar = abs(angleDelta);
                }
            }
        }
        return closestPlayerSoFar;
    }
};
