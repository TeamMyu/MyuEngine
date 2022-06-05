#pragma once

template < typename T >
class Singleton
{
protected:
    Singleton(){}
    virtual ~Singleton(){}
 
public:
    static T* getInstance()
    {
        if (m_pInstance == nullptr)
            m_pInstance = new T;
        return m_pInstance;
    };
 
    static void destroyInstance()
    {
        if (m_pInstance)
        {
            delete m_pInstance;
            m_pInstance = nullptr;
        }
    };
 
private:
    inline static T* m_pInstance;
};
