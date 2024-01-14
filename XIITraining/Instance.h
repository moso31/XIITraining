#pragma once

template<class T>
class Instance
{
protected:
    Instance() {}
    virtual ~Instance() {}

    Instance(const Instance&) = delete;
    Instance& operator=(const Instance&) = delete;

public:
    static T* GetInstance()
    {
        static T pInstance;
        return &pInstance;
    }
};
