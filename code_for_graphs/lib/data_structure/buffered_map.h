/******************************************************************************
 * buffered_map.h 
 * *
 * Source of KaHIP -- Karlsruhe High Quality Partitioning.
 * Marcelo Fonseca Faraj
 *****************************************************************************/

#ifndef BUFFERED_MAP_EFRXO4X2
#define BUFFERED_MAP_EFRXO4X2

#include <iostream>
#include <vector>
#include <unordered_map>
#include <bitset>
#include <utility>
#include <algorithm>
#include <set>

#include "definitions.h"
#include "data_structure/single_adj_list.h"




//////// Vector Version
class buffered_map {

	public:
		buffered_map(std::vector<PartitionID> *external_vector, bool vec_preassigned) {
			this->key_to_value = external_vector;
			this->vec_preassigned = vec_preassigned;
		}
		~buffered_map() {
		}

		bool has_key(LongNodeID key) {
			return (this->vec_preassigned) || (*this->key_to_value)[key] != INVALID_PARTITION;
		}

		// We make no test here. Programmer has to call has_key first
		LongNodeID const operator[](LongNodeID key) const {
			return (*this->key_to_value)[key];
		}

		// We make no test here. has_key has to return false before push_back and value == this->value_to_key.size()
		void push_back(LongNodeID key, LongNodeID value) {
			(*this->key_to_value)[key] = value;
			this->value_to_key.push_back(key);
		}

		void clear() {
			if(!this->vec_preassigned) {
				for (auto& key: this->value_to_key) {
					(*this->key_to_value)[key] = INVALID_PARTITION;
				}
			}
			this->value_to_key.clear();
		}

		bool vec_preassigned;
		std::vector<PartitionID> *key_to_value;
		std::vector<LongNodeID> value_to_key;
};

//macros  
#define forall_pairs_bufferedmap(obj,key,value) { for (LongNodeID value = 0; value < (obj).value_to_key.size(); value++) { LongNodeID key = (obj).value_to_key[value]; 
#define endfor_bufferedmap }}


template <typename U>
class minimal_min_heap {
	public:
		minimal_min_heap() {
			this->fp = NULL;
		}
		minimal_min_heap(LongNodeID input_id, std::vector<U>* v) {
			this->fp = NULL;
			this->initialize(input_id, v);
		}
		~minimal_min_heap() {
//			delete this->fp;
		}
		void clear() {
			if (this->fp != NULL) {
				delete this->fp;
				this->fp = NULL;
			}
			this->n = 0;
		}
		void initialize(LongNodeID input_id, std::vector<U>* v) {
			this->id = input_id;
			this->fp = v;
			this->n = v->size();
		}
		void redo(LongNodeID left, LongNodeID right) {
			LongNodeID j = left * 2;                                                   
			while (j <= right) {                                                
				if ((j < right) && ((*this->fp)[j-1] > (*this->fp)[j])) j++;
				if ((*this->fp)[left-1] <= (*this->fp)[j-1]) break;         
				std::swap((*this->fp)[left-1], (*this->fp)[j-1]);           
				left = j;                                                   
				j = left * 2;                                               
			}                                                                   
		}
		void build() const {
			LongNodeID left = n / 2 + 1;
			while (left > 1) {
				left--;
				this->redo(left, this->n);
			}
		}
		U& get_min() {
			return (*this->fp)[0];
		}
		U& del_min() {
			std::swap((*this->fp)[0], (*this->fp)[n-1]);           
			this->n--;
			this->redo(1, this->n);                 
			return (*this->fp)[this->n];            
		}
		void update_min() {
			this->redo(1, this->n);
		}
		LongNodeID size() const {
			return this->n;
		}
		LongNodeID get_id() const {
			return this->id;
		}
		bool operator<(const minimal_min_heap<U>& obj) { return (this->size()*obj.size()) ? (this->get_min() < obj.get_min()) : (false); }
		bool operator<=(const minimal_min_heap<U>& obj) { return (this->size()*obj.size()) ? (this->get_min() <= obj.get_min()) : (false); }
		bool operator>(const minimal_min_heap<U>& obj) { return (this->size()*obj.size()) ? (this->get_min() > obj.get_min()) : (false); }
		bool operator>=(const minimal_min_heap<U>& obj) { return (this->size()*obj.size()) ? (this->get_min() >= obj.get_min()) : (false); }
		bool operator==(const minimal_min_heap<U>& obj) { return (this->size()*obj.size()) ? (this->get_min() == obj.get_min()) : (false); }
		bool operator!=(const minimal_min_heap<U>& obj) { return (this->size()*obj.size()) ? (this->get_min() != obj.get_min()) : (false); }

	private:
		LongNodeID id, n;
		std::vector<U>* fp;
};



class buffered_input {
	public:
		buffered_input(std::vector<std::string>* input_lines) {
			this->lines = input_lines;
			this->row = 0;
			this->n_buffer_lines = 1;
			this->column = 0;
			this->basic_instanciation = true;
		}
		buffered_input(std::vector<std::string>* input_lines, LongNodeID input_cursor) {
			this->lines = input_lines;
			this->row = input_cursor;
			this->n_buffer_lines = 1;
			this->column = 0;
			this->basic_instanciation = true;
		}
		buffered_input(std::vector<std::string>* input_lines, LongNodeID input_cursor, LongNodeID n_buffer_lines, LongNodeID ub) {
			this->lines = input_lines;
			this->row = input_cursor;
			this->n_buffer_lines = n_buffer_lines;
			this->vecvec_below_ub = new std::vector<std::vector<LongNodeID>>(n_buffer_lines);
			this->ub = ub;
			this->column = 0;
			this->reading_row = 0;
			this->basic_instanciation = false;
		}
		~buffered_input() {
			if (!this->basic_instanciation) {
				delete this->vecvec_below_ub;
			}
		}
		void convert_lines() {
			std::vector<minimal_min_heap<LongNodeID>>* vecheap_above_ub = new std::vector<minimal_min_heap<LongNodeID>>(this->n_buffer_lines);
			for (LongNodeID i=0; i< this->n_buffer_lines; i++, this->row++) {
				this->scan_curr_line(i, (*this->vecvec_below_ub)[i], (*vecheap_above_ub)[i]);
			}
			this->heapheap_above_ub.initialize(UNDEFINED_LONGNODE,vecheap_above_ub);
			this->heapheap_above_ub.build();
		}
		bool next_smallest_above_ub(LongNodeID& id, LongNodeID& item) {
			while(this->heapheap_above_ub.size() > 0) {
				minimal_min_heap<LongNodeID>& smallest_heap = this->heapheap_above_ub.get_min();
				if (smallest_heap.size() > 0) {
					id   = smallest_heap.get_id();
					item = smallest_heap.del_min();
					this->heapheap_above_ub.update_min();
					return true;
				} else {
					this->heapheap_above_ub.del_min();
					smallest_heap.clear();
				}
			}
			if (this->heapheap_above_ub.size()==0) {
				heapheap_above_ub.clear();
			}
			return false;
		}
		void set_reading_row(LongNodeID out_row) {
			this->reading_row = out_row;
		}
		bool read_column(LongNodeID col, LongNodeID& val) {
			if (col < (*this->vecvec_below_ub)[this->reading_row].size()) {
				val = (*this->vecvec_below_ub)[this->reading_row][col];
				return true;
			}
			return false;

		}
		inline void simple_scan_line(std::vector<LongNodeID>& vec) {
			vec.clear();
			this->column = 0;
			LongNodeID item = 0;
			while (this->next_int(item)) {
				vec.push_back(item);
			}
			this->row++;
		}
		void simple_scan_adjlist(single_adj_list* my_adj_list, bool read_nw, LongNodeID LB) {
			EdgeWeight weight;
			if (read_nw) {
				this->column = 0;
				LongNodeID item = 0;
				while (this->next_int(item)) {
					this->next_int(weight);
					if(item < LB) { // edge to a previous node
						my_adj_list->add_neighbor(item-1, weight);
					}
				}
			} else {
				this->column = 0;
				LongNodeID item = 0;
				while (this->next_int(item)) {
					if(item < LB) { // edge to a previous node
						my_adj_list->add_neighbor(item-1, 1);
					}
				}
			}
			this->row++;
		}
	private:
		void scan_curr_line(LongNodeID id, std::vector<LongNodeID>& vec_below_ub, minimal_min_heap<LongNodeID>& heap_above_ub) {
			std::vector<LongNodeID> *vec_above_ub = new std::vector<LongNodeID>();
			vec_below_ub.clear();
			this->column = 0;
			LongNodeID item = 0;
			while (this->next_int(item)) {
				this->store_item(item, vec_below_ub, *vec_above_ub);
			}
			heap_above_ub.initialize(id, vec_above_ub);
			heap_above_ub.build();
		}
		template <typename T>
		inline bool next_int(T& item) {
			bool valid_number = false;
			item = 0;
			while (this->column < (*this->lines)[this->row].size()) {
				const char c = (*this->lines)[this->row][this->column];
				switch(c) {
					case '0': case '1': case '2': case '3': case '4':
					case '5': case '6': case '7': case '8': case '9':
						item = 10*item + (T) (c - '0');                                     
						valid_number = true; 
						break;
					default:
						if (valid_number) return true;
						break;
				}
				this->column++; 
			}
			return valid_number;
		}
		void store_item(LongNodeID item, std::vector<LongNodeID>& vec_below_ub, std::vector<LongNodeID>& vec_above_ub) {
			if(item <= this->ub) { 
				vec_below_ub.push_back(item);
			} else {
				vec_above_ub.push_back(item);
			}
		}
		std::vector<std::string>* lines;
		LongNodeID row, column, n_buffer_lines, ub;
		LongNodeID reading_row;
		minimal_min_heap<minimal_min_heap<LongNodeID>> heapheap_above_ub;
		std::vector<std::vector<LongNodeID>>* vecvec_below_ub;
		bool basic_instanciation;
};



#endif /* end of include guard: BUFFERED_MAP_EFRXO4X2 */
//macros  


