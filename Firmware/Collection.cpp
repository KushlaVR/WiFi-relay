#include "Collection.h"

static int _sort;



Collection::Collection()
{
	first = nullptr;
}

Collection::~Collection()
{
}

Item * Collection::getFirst()
{
	return first;
}

Item * Collection::getLast()
{
	return last;
}

Item * Collection::get(int index)
{
	if (index >= count) return nullptr;
	if (index == 0) return first;
	Item * itm = first;
	int i = 0;
	while (itm != nullptr) {
		if (index == i) return itm;
		itm = itm->next;
		i++;
	}
	return nullptr;
}

Item * Collection::add(Item * item)
{
	if (first == nullptr) {
		first = item;
		last = item;
		item->Index = 0;
		count = 1;
	}
	else {
		last->next = item;
		last = item;
		item->Index = count;
		count++;
	}
	item->__collection = this;
	return item;
}

Item * Collection::insert(int index, Item * item)
{
	if (index <= 0) index = 0;
	if (index < count) {
		if (index == 0) {
			item->next = first;
			first = item;
		}
		else {
			Item * itm = get(index);
			Item * prev = get(index - 1);
			prev->next = item;
			item->next = itm;
		}
		count++;
	}
	else {
		return add(item);
	}

	Item * itm = first;
	for (int i = 0; i < count; i++) {
		itm->Index = i;
		itm = itm->next;
	}
	item->__collection = this;
	return item;
}

void Collection::remove(Item * item)
{
	Item * itm = first;
	Item * prev = nullptr;
	while (itm != nullptr) {
		if (itm == item) {
			if (prev != nullptr) {
				prev->next = itm->next;
				itm->next = nullptr;
			}
			else
			{
				first = itm->next;
				itm->next = nullptr;
			}
			count--;
			return;
		}
		prev = itm;
		itm = itm->next;
	}
}

void Collection::removeAt(int index)
{
	Item * itm = first;
	Item * prev = nullptr;
	int i = 0;
	while (itm != nullptr) {
		if (i == index) {
			if (prev != nullptr) {
				prev->next = itm->next;
				itm->next = nullptr;
			}
			else
			{
				first = itm->next;
				itm->next = nullptr;
			}
			count--;
			return;
		}
		prev = itm;
		itm = itm->next;
	}
}

void Collection::sort()
{
	Item * t = nullptr;
	Item * n = nullptr;//next
	Item * p = nullptr;//prev
	bool was_swap = true;//були випадки перем≥ни м≥сц€ми елемент≥в колекц≥њ
	while (true) {
		if (t == nullptr) {//ѕочинаЇмо з першого елемента
			if (was_swap) {
				t = first;
				p = nullptr;
				was_swap = false;
			}
			else {
				//за попередн≥ ≥терац≥њ елементи не переставл€лись, значить масив посортовано
				return;
			}
		}
		if (t != nullptr) {
			n = t->next;
			if (n != nullptr) {//Ї наступний елемент, значить ми ще не дос€гли к≥нц€
				if (t->getSort() > n->getSort()) {//t => груба бульбашка н≥ж n, вона виринаЇ над n
					was_swap = true;//м≥н€Їмо м≥сц€ми два елемента
					t->next = n->next;
					n->next = t;
					if (p != nullptr)
						p->next = n;
					else
						first = n;
					p = n;
				}
				else {
					p = t;
					t = t->next;
				}
			}
			else {
				t = nullptr;// ≥нець масиву, починаЇмо з початку
			}
		}
	}
	

}



Item::Item()
{
	_sort++;
	sort = _sort;
	next = nullptr;
	__collection = nullptr;
}

Item::~Item()
{
}

int Item::getSort()
{
	return sort;
}
