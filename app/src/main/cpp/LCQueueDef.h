//
// Created by Administrator on 2021/11/9.
//

#ifndef LCPLAYER_LCQUEUEDEF_H
#define LCPLAYER_LCQUEUEDEF_H

#include <list>
#include <mutex>

using namespace std;

template <class T>
class LCMediaQueue : public list<T>
{
public:
    LCMediaQueue()
    {

    }

    ~LCMediaQueue()
    {

    }

    void enqueue(const T &t) {
        m_mutex.lock();
        list<T>::push_back(t);
        m_mutex.unlock();
    }

    T dequeue()
    {
        m_mutex.lock();
        T t = NULL;
        if ( !list<T>::empty() ){
            t = list<T>::front();
            list<T>::pop_front();
        }
        m_mutex.unlock();
        return t;
    }

    bool isEmpty()
    {
        m_mutex.lock();
        bool b = list<T>::empty();
        m_mutex.unlock();
        return b;
    }

private:
    mutex m_mutex;
};

#endif //LCPLAYER_LCQUEUEDEF_H
