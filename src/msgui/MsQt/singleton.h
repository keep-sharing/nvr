#ifndef SINGLETON_H
#define SINGLETON_H

#include <QObject>
#include <QMutex>

//使用方法：
//通过以下语句：Singleton<类名>::instance()，获取指向某类实例的指针。

template<class T>
class Singleton : public QObject
{
private:
    Singleton(QObject *parent) {}
    Singleton(const Singleton &) {}
    Singleton& operator= (const Singleton&) {}
    ~Singleton() {}

public:
	static T* instance()
	{
		static QMutex mutex;
		if (!self)
		{
			QMutexLocker locker(&mutex);
			if (!self)
			{
				self = new T(nullptr);
			}
		}
		return self;
	}
	static void deleteInstance()
	{
		if (self)
		{
			delete self;
			self = nullptr;
		}
	}

private:
	static T* self;
};
template<class T> T* Singleton<T>::self = nullptr;

#endif //SINGLETON_H
