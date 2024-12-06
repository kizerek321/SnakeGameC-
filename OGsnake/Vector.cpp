#include "Vector.h"

template <typename Type>
Vector<Type>::Vector() {
   vector = new Type[1];
   size = 1;
   currentSize = 0;
}

template <typename Type>
void Vector<Type>::push(Type data) {
   if (currentSize == size) {
       Type* temp = new Type[2 * size];
       for (int i = 0; i < size; i++) {
           temp[i] = vector[i];
       }
       delete[] vector;
       size *= 2;
       vector = temp;
   }
   vector[currentSize] = data;
   currentSize++;
}

template <typename Type>
void Vector<Type>::push(Type data, int index) {
   if (index == size) {
       push(data);
   } else {
       vector[index] = data;
   }
}

template <typename Type>
Type Vector<Type>::get(int index) {
   if (index < currentSize) {
       return vector[index];
   }
   return -1;
}

template <typename Type>
void Vector<Type>::pop() {
   currentSize--;
}

template <typename Type>
void Vector<Type>::pop(int index) {
   if (index < currentSize) {
       for (int i = index; i < currentSize - 1; i++) {
           vector[i] = vector[i + 1];
       }
       currentSize--;
   }
}

template <typename Type>
int Vector<Type>::getSize() {
   return size;
}

template <typename Type>
int Vector<Type>::getCurrentSize() {
   return currentSize;
}

template <typename Type>
Vector<Type>::~Vector() {
   delete[] vector;
}

template class Vector<int>;