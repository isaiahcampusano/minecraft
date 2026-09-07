#pragma once

class SprintState{
public:
  static constexpr double DOUBLE_TAP_SECONDS=.3;
  bool wanted()const{return m_wanted;}
  bool active()const{return m_active;}
  float movementSpeed()const{return m_active?10.f:5.f;}
  const char* hudText()const{return m_active?"ON":"OFF";}
  void toggle(){m_wanted=!m_wanted;if(!m_wanted)m_active=false;}
  void request(){m_wanted=true;}
  void cancel(){m_wanted=false;m_active=false;}
  bool forwardPressed(double now){const bool doubled=m_lastForwardPress>=0&&now-m_lastForwardPress<=DOUBLE_TAP_SECONDS;m_lastForwardPress=now;if(doubled)request();return doubled;}
  void evaluate(bool forward,bool allowed){m_active=m_wanted&&forward&&allowed;}
private:
  bool m_wanted=false,m_active=false;double m_lastForwardPress=-1;
};
