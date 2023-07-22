/******************************************************************************
 * self_sorting_monotonic_vector.h
 * *
 * Marcelo Fonseca Faraj <marcelofaraj@gmail.com>
 * *
 * This template class implements a self sorting vector where a fixed number of
 * keys [0, ..., n_elements] start all with an initial value initial_value
 * and the value associated with each key can only get incremented 1 by 1.
 * The elements are always sorted by value in ascending order. The class allows 
 * cache-efficient O(1) random access to the keys using the [] operator.
 * Incrementing the value associated with a key is also O(1).
 *****************************************************************************/

#ifndef SELF_SORTING_MONOTONIC_VECTOR_EM8YJPA9
#define SELF_SORTING_MONOTONIC_VECTOR_EM8YJPA9

#include <iterator> // For std::forward_iterator_tag
#include <cstddef>  // For std::ptrdiff_t
#include <limits>
#include <algorithm>
#include <utility>
#include <vector>
#include <list>
#include "definitions.h"

template <typename K, typename V>
struct BUCKET{
	K size;
	V value;
	K pos_begin;
};

template <typename K, typename V>
struct ELEMENT{
	K key;
	typename std::list<BUCKET<K,V> >::iterator buck;
};

template <typename K, typename V>
class self_sorting_monotonic_vector {
        public:
                self_sorting_monotonic_vector(){}
                self_sorting_monotonic_vector(K n_elements, V initial_value);
                ~self_sorting_monotonic_vector() {}
                void initialize(K n_elements, V initial_value);
		void increment(K key);
		K operator[](K position) const;
		K size() const;
		// For Later: include input iterator
		/* struct Iterator */ 
		/* { */
		/* 	using iterator_category = std::input_iterator_tag; */
		/* 	using difference_type   = std::ptrdiff_t; */
		/* 	using value_type        = int; */
		/* 	using pointer           = int*;  // or also value_type* */
		/* 	using reference         = int&;  // or also value_type& */
		/* }; */

	private:
		bool next_bucket_matches_value(K key, V new_value);
		V get_value(K key);
		void remove_from_bucket(K key);
		void decrement_bucket_size(typename std::list<BUCKET<K,V>>::iterator buck);
		void assign_to_new_bucket(K key, V value);
		void assign_to_next_bucket(K key);
                K n_elements;
		typename std::list<BUCKET<K,V>> bucket_list;
		typename std::vector<ELEMENT<K,V>> ordered_list;
		typename std::vector<K> pos;
};

template <typename K, typename V>
self_sorting_monotonic_vector<K,V>::self_sorting_monotonic_vector(K n_elements, V initial_value) {
	this->n_elements = n_elements;
	BUCKET<K,V> my_bucket = {this->n_elements, initial_value, this->n_elements - 1};
        this->bucket_list.push_front(my_bucket);
        this->ordered_list.resize(this->n_elements);
        this->pos.resize(this->n_elements);
	for (K i=(K)0; i<this->n_elements; i++) {
		this->ordered_list[i].key  = i;
		this->ordered_list[i].buck = this->bucket_list.begin();
		this->pos[i] = i;
	}
}

template <typename K, typename V>
inline void self_sorting_monotonic_vector<K,V>::initialize(K n_elements, V initial_value) {
	this->n_elements = n_elements;
	BUCKET<K,V> my_bucket = {this->n_elements, initial_value, this->n_elements - 1};
        this->bucket_list.push_front(my_bucket);
        this->ordered_list.resize(this->n_elements);
        this->pos.resize(this->n_elements);
	for (K i=(K)0; i<this->n_elements; i++) {
		this->ordered_list[i].key  = i;
		this->ordered_list[i].buck = this->bucket_list.begin();
		this->pos[i] = i;
	}
}

template <typename K, typename V>
inline void self_sorting_monotonic_vector<K,V>::increment(K key) {
	V new_value = get_value(key) + 1;
	remove_from_bucket(key);
	if (next_bucket_matches_value(key, new_value)) {
		assign_to_next_bucket(key);
	} else {
		assign_to_new_bucket(key, new_value);
	}
}

template <typename K, typename V>
inline bool self_sorting_monotonic_vector<K,V>::next_bucket_matches_value(K key, V new_value) {
	K next_pos = this->pos[key] + 1;
	return ( (next_pos < this->n_elements) && this->ordered_list[next_pos].buck->value == new_value );
}

template <typename K, typename V>
inline V self_sorting_monotonic_vector<K,V>::get_value(K key) {
	K my_pos = this->pos[key];
        ELEMENT<K,V> ele = this->ordered_list[my_pos];
	return ele.buck->value;
}

template <typename K, typename V>
inline void self_sorting_monotonic_vector<K,V>::remove_from_bucket(K key) {
	K my_pos = this->pos[key];
        ELEMENT<K,V> ele = this->ordered_list[my_pos];
	K new_pos = ele.buck->pos_begin;
	K tmp_key = this->ordered_list[new_pos].key;
	std::swap(this->ordered_list[my_pos], this->ordered_list[new_pos]);
	std::swap(this->pos[key]	    , this->pos[tmp_key]);
	decrement_bucket_size(ele.buck);
	ele.buck = this->bucket_list.end();
}

template <typename K, typename V>
inline void self_sorting_monotonic_vector<K,V>::decrement_bucket_size(typename std::list<BUCKET<K,V>>::iterator buck) {
	buck->pos_begin--; 
	buck->size--; 
	if (buck->size == 0) this->bucket_list.erase(buck);
}

template <typename K, typename V>
inline void self_sorting_monotonic_vector<K,V>::assign_to_new_bucket(K key, V value) {
	K my_pos = this->pos[key];
	BUCKET<K,V> my_buck = {1, value, my_pos};
        this->bucket_list.push_front(my_buck);
        this->ordered_list[my_pos].buck = this->bucket_list.begin();
}

template <typename K, typename V>
inline void self_sorting_monotonic_vector<K,V>::assign_to_next_bucket(K key) {
	K my_pos = this->pos[key];
	typename std::list<BUCKET<K,V>>::iterator my_buck = this->ordered_list[(my_pos+1)].buck;
        this->ordered_list[my_pos].buck = my_buck;
	my_buck->size++;
}

template <typename K, typename V>
inline K self_sorting_monotonic_vector<K,V>::operator[](K position) const {
	return this->ordered_list[position].key;
}

template <typename K, typename V>
inline K self_sorting_monotonic_vector<K,V>::size() const {
	return this->ordered_list.size();
}


#endif /* end of include guard: SELF_SORTING_MONOTONIC_VECTOR_EM8YJPA9 */

