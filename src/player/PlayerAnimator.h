#pragma once
class Player;
struct PlayerPose{
  float headPitch=0,torsoBob=0,torsoLean=0,leftArm=0,rightArm=0,leftArmOut=0,rightArmOut=0,leftLeg=0,rightLeg=0;
};
class PlayerAnimator{
public:void update(const Player&,float dt);const PlayerPose& pose()const{return m_current;}
private:PlayerPose m_current{},m_target{};float m_cycle=0,m_idleTime=0;void approach(float&,float,float);};

