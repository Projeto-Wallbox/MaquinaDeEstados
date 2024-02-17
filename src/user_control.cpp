#include <user_control.h>

void UserControl::setUserCurrent(int current){
    m_userCurrent = current;
}

void UserControl::setStartCharging(bool status){
    m_startCharging = status;
}

void UserControl::setStopCharging(bool status){
    m_stopCharging = status;
}

int UserControl::getUserCurrent() const{
    return m_userCurrent;
}

bool UserControl::getStartCharging() const{
    return m_startCharging;
}

bool UserControl::getStopCharging() const{
    return m_stopCharging;
}