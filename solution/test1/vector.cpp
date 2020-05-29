#include <iostream>
#include <cstdlib>
#include <cstring>
using namespace std;
class Element {
private:
	int number;
public:
	Element() :number(0) {
		cout << "ctor" << endl;
	}
	Element(int num) :number(num) {
		cout << "ctor" << endl;
	}
	Element(const Element& e) :number(e.number) {
		cout << "copy ctor" << endl;
	}
	Element(Element&& e) :number(e.number) {  //右值引用
		cout << "right value ctor" << endl;
	}
	~Element() {
		cout << "dtor" << endl;
	}
	void operator=(const Element& item) {
		number = item.number;
	}
	bool operator==(const Element& item) {
		return (number == item.number);
	}
	void operator()() {
		cout << number;
	}
	int GetNumber() {
		return number;
	}
};
template<typename T>
class Vector {
private:
	T* items;
	int count;
	int cap;
public:
	Vector() :count{ 0 }, items{ nullptr } {
		cap = 0;
	}
	Vector(const Vector& vector) :count{ vector.count } {
		items = static_cast<T*>(malloc(sizeof(T) * count));
		memcpy(items, vector.items, sizeof(T) * count);
		cap = count;
	}
	Vector(Vector&& vector) :count{ vector.count }, items{ vector.items } {
		items = static_cast<T*>(malloc(sizeof(T) * count));
		for (int i = 0; i < count; i++) {
			new(&(items[i])) T(move(vector.items[i]));
		}
		vector.~Vector();
		cap = count;
	}
	~Vector() {
		Clear();
	}
	T& operator[](int index) {
		if (index < 0 || index >= count) {
			cout << "invalid index" << endl;
			return items[0];
		}
		return items[index];
	}
	int returnCount() {
		return count;
	}
	void Clear() {
		for (int i = 0; i < count; i++) {
			items[i].~T();
		}
		free(items);
		items = nullptr;
		count = 0;
		cap = 0;
	}

	void Add(const T& item) {
		int newCnt = count + 1;
		if (newCnt > cap) {
			T* newitems;
			if(count == 0)
				newitems = (T*)malloc(sizeof(T));
			else
				newitems = (T*)malloc(sizeof(T) * 2 * count);
			for (int i = 0; i < count; i++) {
				new(newitems + i) T(move(items[i]));
			}
			free(items);
			items = newitems;
			cap = 2 * count;
		}
		T* addr = items + count;
		new(addr) T(move(item));
		count++;
	}
	bool Insert(const T& item, int index) {
		if (index > count || index< 0) return false;
		Add(item);  // no use,  add啥都行
		for (int i = count-1; i > index; i--) {   //count-1 存放的是item，但是现在我们直接覆盖它
			items[i] = std::move(items[i - 1]);
		}
		items[index] = item;
		return true;
	}
	bool Remove(int index) {
		if (index > count || index < 0) return false;
		for (int i = index; i < count; i++) {
			items[i] = std::move(items[i + 1]);
		}
		count--;
		return true;
	}
	int Contains(const T& item) {
		for (int i = 0; i < count; i++) {
			if (items[i] == item)
				return i;
		}
		return -1;
	}
};
template<typename T>
void PrintVector(Vector<T>& v) {
	int count = v.returnCount();
	for (int i = 0; i < count; i++)
	{
		v[i]();
		cout << " ";
	}
	cout << endl;
}
int main() {
	Vector<Element>v;
	for (int i = 0; i < 4; i++) {
		Element e(i);
		v.Add(e);
	}
	PrintVector(v);
	Element e2(4);
	if (!v.Insert(e2, 10))
	{
		v.Insert(e2, 2);
	}
	PrintVector(v);
	if (!v.Remove(10))
	{
		v.Remove(2);
	}
	PrintVector(v);
	Element e3(1), e4(10);
	cout << v.Contains(e3) << endl;
	cout << v.Contains(e4) << endl;
	Vector<Element>v2(v);
	Vector<Element>v3(move(v2));
	PrintVector(v3);
	v2.Add(e3);
	PrintVector(v2);
	return 0;
}