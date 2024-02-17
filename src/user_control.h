class UserControl{
private:
    int m_userCurrent;
    bool m_startCharging;
    bool m_stopCharging;
public:
    UserControl()
        : m_userCurrent(-1), m_startCharging(false), m_stopCharging(false)
    {

    }  

    void setUserCurrent(int current);
    void setStartCharging(bool status);
    void setStopCharging(bool status);

    int getUserCurrent() const;
    bool getStartCharging() const;
    bool getStopCharging() const;

};