#pragma once
#include "Arduino.h"


class Item {
public:
	Item();
	~Item();
	int Index = 0;
	int sort;
	Item * next;
	virtual int getSort();
	void * __collection;
};

class Collection
{
	Item * first;
	Item * last;
	int count;

public:
	Collection();
	~Collection();

	Item * getFirst();
	Item * getLast();
	Item * get(int index);
	Item * add(Item * item);
	Item * insert(int index, Item * item);
	void remove(Item * item);
	void removeAt(int index);
	void sort();
};

