#pragma once
template <typename Type> class Vector { 
private:
	Type * vector;
	int size;
	int currentSize;
public:
	
	Vector ();
	void push (Type data);
	void push ( Type data , int index );
	Type get ( int index );
	void pop ();
	void pop ( int index );
	int getSize ();
	int getCurrentSize ();
	~Vector ();
};