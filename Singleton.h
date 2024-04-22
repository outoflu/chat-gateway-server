#pragma once
#ifndef SINGLETON_H
#define SINGLETON_H
#include <memory>
#include <mutex>
#include <iostream>
using std::shared_ptr;

template <typename T>
class Singleton {
    static shared_ptr<T> _instance;
protected:
    Singleton() = default;
    Singleton(const Singleton<T>&) = delete;
    Singleton& operator=(const Singleton<T>&) = delete;
public:
    static shared_ptr<T> GetInstance() {
        static std::once_flag s_flag;
        std::call_once(s_flag, []() {
            _instance = std::shared_ptr<T>(new T);
            });

        return _instance;
    }
    void PrintAddress()const {
        std::cout << _instance.get() << std::endl;
    }
    ~Singleton() {
        std::cout << "this is singleton destruct" << std::endl;
    }
};

template<typename T>
shared_ptr<T> Singleton<T>::_instance = nullptr;

#endif // SINGLETON_H
