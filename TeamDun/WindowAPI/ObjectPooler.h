#pragma once

#include <stack>
#include <mutex>

typedef std::lock_guard<std::recursive_mutex> MutexLocker;

template<typename T>
class objectPool
{

	// ������Ʈ Ǯ���� ���� �ֱٿ� ������ �׳� ���ٰ� ���� ���⶧����, ���Լ��� [STACK]�� ����Ѵ�.

private:
	std::recursive_mutex	_mt;
	std::stack<T*>			_objects;	// ������Ʈ���� �̸� ������ �����صδ� ������Ʈ STACK
	int						_maxSize;	// �ִ� �迭 ũ��

public:
	objectPool();
	~objectPool();
	T* PopObject();
	void SetSize(int size);
	void Expand();
	void ReturnObject(T* object);
	int ReturnSize();
};

template<typename T>
objectPool<T>::objectPool() {}

// ������ƮǮ���� ����. (�������� RELEASE)
template<typename T>
objectPool<T>::~objectPool()
{
	MutexLocker locker(_mt);
	while (!_objects.empty()) {
		T* object = _objects.top();
		_objects.pop();
		delete object;
	}

	_maxSize = 0;
}

// ������ƮǮ���� ����� �����Ѵ�
template<typename T>
void objectPool<T>::SetSize(int size) {
	_maxSize = size;

	for (int i = 0; i < _maxSize; ++i) { // _maxSize��ŭ ������Ʈ�� �����д�
		T* newObject = new T();
		_objects.push(newObject);
	}
}

// ������Ʈ�� ������
template<typename T>
T* objectPool<T>::PopObject()
{
	MutexLocker locker(_mt);

	// ������Ʈ�� ���ٸ� Ȯ���Ѵ�
	if (_objects.empty()) {
		Expand();
	}

	T* retVal = _objects.top();
	_objects.pop();
	return retVal;
}

// ���� ũ�⸸ŭ ���ο� ������Ʈ�� �־� Ȯ���Ѵ�
template<typename T>
void objectPool<T>::Expand() {
	MutexLocker locker(_mt);

	for (int i = 0; i < _maxSize; ++i)
	{
		T* newObject = new T();
		_objects.push(newObject); // ���ο� ������Ʈ�� ����� ������Ʈ Ǯ���� �ִ´�
	}

	_maxSize += _maxSize;
}

// ������Ʈ�� �����Ѵ�.
template<typename T>
void objectPool<T>::ReturnObject(T* object)
{
	MutexLocker locker(_mt);
	_objects.push(object); // �޾ƿ� ������Ʈ�� �ٽ� ������Ʈ Ǯ���� �־��ش�.
}

template<typename T>
int objectPool<T>::ReturnSize()
{
	return _objects.size();
}