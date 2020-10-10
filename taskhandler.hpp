#pragma once

#include <ctime>
#include <queue>
#include <mutex>
#include <atomic>
#include <vector>
#include <string>
#include <chrono>
#include <memory>
#include <thread>
#include <iostream>
#include "socket/SC_Server.h"
#include "socket/DataFrame.h"

using namespace std;

//����ӿ�
class WorkItem
{
public:
	//�ӿڷ�������������ʵ��
	virtual void run() = 0;

public:
	//��������ӿ�
	virtual void clean()
	{
	}
	//�ж������Ƿ��ִ��(������ʱ����Ż�ִ��)
	virtual bool runnable()
	{
		return true;
	}
};

//��������
class SpinMutex
{
private:
	atomic_flag flag = ATOMIC_FLAG_INIT;

public:
	void lock()
	{
		while (flag.test_and_set(memory_order_acquire));
	}
	void unlock()
	{
		flag.clear(std::memory_order_release);
	}
};

//�������
class TaskQueue
{
private:
	size_t maxsz;
	size_t threads;
	mutable SpinMutex mtx;
	std::queue<shared_ptr<WorkItem>> que;

	TaskQueue()
	{
		this->maxsz = 0;
	}
	bool pop(shared_ptr<WorkItem>& item)
	{
		std::lock_guard<SpinMutex> lk(mtx);
		if (que.empty()) return false;
		item = que.front(); que.pop();
		return true;
	}
public:
	//ʵ�ֵ���ģʽ
	static TaskQueue* Instance()
	{
		static TaskQueue obj;
		return &obj;
	}


public:
	//��ֹ������
	void stop()
	{
		threads = 0;
	}
	//��ն���
	void clear()
	{
		std::lock_guard<SpinMutex> lk(mtx);
		while (que.size() > 0) que.pop();
	}
	//�ж϶����Ƿ�Ϊ��
	bool empty() const
	{
		std::lock_guard<SpinMutex> lk(mtx);
		return que.empty();
	}
	//��ȡ�������
	size_t size() const
	{
		std::lock_guard<SpinMutex> lk(mtx);
		return que.size();
	}
	//��ȡ�����߳���
	size_t getThreads() const
	{
		return threads;
	}
	//����������
	bool push(shared_ptr<WorkItem> item)
	{
		std::lock_guard<SpinMutex> lk(mtx);
		if (maxsz > 0 && que.size() >= maxsz) return false;
		que.push(item);
		return true;
	}
	//�����������(���������߳�)
	void start(size_t threads = 1, size_t maxsz = 10000)
	{
		this->threads = threads;
		this->maxsz = maxsz;

		for (size_t i = 0; i < threads; i++)
		{
			std::thread(std::bind(&TaskQueue::run, this)).detach();
		}
	}

public:
	//����������洦���������
	void run()
	{
		shared_ptr<WorkItem> item;

		while (threads > 0)
		{
			if (pop(item))
			{
				if (item->runnable())
				{
					item->run();
					item->clean();
				}
				else
				{
					std::lock_guard<SpinMutex> lk(mtx);
					que.push(item);
				}
			}
			else
			{
				std::chrono::milliseconds dura(1);
				std::this_thread::sleep_for(dura);
			}
		}
	}
};

enum TaskType 
{
	UPLOAD,
	REPLAY
};

/////////////////////////////////////////////////////////////////////////

//ʵ��һ������ӿ�
class Task : public QObject, public WorkItem
{
	Q_OBJECT
	TaskType type;
public:
	int id;
public:
	//�����������ʵ�־�������
	void run()
	{
		switch (this->type)
		{
		case TaskType::UPLOAD: 
		{
			Sleep(200);
			emit upload();
			break;
		}
		default:
			break;
		}
	}

signals:
	void upload();
public:
	Task(const int& id = -1, TaskType type = TaskType::UPLOAD)
	{
		this->id = id;
		this->type = type;
	}
};