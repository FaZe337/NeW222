#pragma once
#include <vector>
#include "LocalPlayer.cpp"
#include "Player.cpp"
#include "Math.cpp"
#include "Level.cpp"
#include "X11Utils.cpp"

class Sense
{
public:
    {
        if (!m_level->isPlayable())
            return;
        for (int i = 0; i < m_players->size(); i++)
        {
            Player *player = m_players->at(i);
            if (!player->isValid())
                continue;
            if (player->getTeamNumber() == m_localPlayer->getTeamNumber())
                continue;

            //if (player->isVisible()){

                player->setCustomGlow();
                player->setCustomGlowWhite();

                if(player->getShieldValue() <= 10){
                    player->setCustomGlowGreen();
                } else

                if(player->getShieldValue() <= 50){
                    player->setCustomGlowWhite();
                } else

                if(player->getShieldValue() <= 75){
                    player->setCustomGlowBlue();
                } else

                if(player->getShieldValue() <= 100){
                    player->setCustomGlowPurple();
                } else

                if(player->getShieldValue() <= 125){
                    player->setCustomGlowRed();
                }

                
            /*}
            else
            {
                player->setCustomGlow();
                player->setCustomGlowRed();
            } */
        }
    }
};
