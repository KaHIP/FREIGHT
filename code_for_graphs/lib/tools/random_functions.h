/******************************************************************************
 * random_functions.h 
 * *
 * Source of KaHIP -- Karlsruhe High Quality Partitioning.
 * Christian Schulz <christian.schulz.phone@gmail.com>
 *****************************************************************************/

#ifndef RANDOM_FUNCTIONS_RMEPKWYT
#define RANDOM_FUNCTIONS_RMEPKWYT

#include <iostream>
#include <random>
#include <vector>
/* #include <immintrin.h> */

#include "definitions.h"
#include "partition_config.h"

#define UNLIKELY(x) __builtin_expect((x), 0)
#define TAM 150000

typedef std::mt19937 MersenneTwister;


template<uint32_t N>
struct ConstRandArray {
	constexpr ConstRandArray() : arr() {
		for (uint32_t i = 0; i != N; ++i)
			arr[i] = crc32_const(i); 
	}
	uint32_t arr[N];
};

constexpr ConstRandArray rand_nums = ConstRandArray<TAM>();



class random_functions {
        public:
                random_functions();
                virtual ~random_functions();

                template<typename sometype>
                        static void circular_permutation(std::vector<sometype> & vec) {
                                if(vec.size() < 2) return;
                                for( unsigned int i = 0; i < vec.size(); i++) {
                                        vec[i] = i;
                                }                   

                                unsigned int size = vec.size();
                                std::uniform_int_distribution<unsigned int> A(0,size-1);
                                std::uniform_int_distribution<unsigned int> B(0,size-1);

                                for( unsigned int i = 0; i < size; i++) {
                                        unsigned int posA = A(m_mt);
                                        unsigned int posB = B(m_mt);

                                        while(posB == posA) {
                                                posB = B(m_mt);
                                        }

                                        if( posA != vec[posB] && posB != vec[posA]) {
                                                std::swap(vec[posA], vec[posB]); 
                                        }
                                } 

                         }

                template<typename sometype>
                        static void permutate_vector_fast(std::vector<sometype> & vec, bool init) {
                                if(init) {
                                        for( unsigned int i = 0; i < vec.size(); i++) {
                                                vec[i] = i;
                                        }                   
                                }
                                
                                if(vec.size() < 10) return;
                                        
                                int distance = 20; 
                                std::uniform_int_distribution<unsigned int> A(0, distance);
                                unsigned int size = vec.size()-4;
                                for( unsigned int i = 0; i < size; i++) {
                                        unsigned int posA = i;
                                        unsigned int posB = (posA + A(m_mt))%size;
                                        std::swap(vec[posA], vec[posB]);
                                        std::swap(vec[posA+1], vec[posB+1]); 
                                        std::swap(vec[posA+2], vec[posB+2]); 
                                        std::swap(vec[posA+3], vec[posB+3]); 
                                }               
                        }

                static void permutate_vector_good(std::vector<std::pair< NodeID, NodeID >> & vec) {

                        unsigned int size = vec.size();
                        if(size < 4) return;

                        std::uniform_int_distribution<unsigned int> A(0,size - 4);
                        std::uniform_int_distribution<unsigned int> B(0,size - 4);

                        for( unsigned int i = 0; i < size; i++) {
                                unsigned int posA = A(m_mt);
                                unsigned int posB = B(m_mt);
                                std::swap(vec[posA], vec[posB]); 
                                std::swap(vec[posA+1], vec[posB+1]); 
                                std::swap(vec[posA+2], vec[posB+2]); 
                                std::swap(vec[posA+3], vec[posB+3]); 

                        } 
                }

                template<typename sometype>
                        static void permutate_vector_good(std::vector<sometype> & vec, bool init) {
                                if(init) {
                                        for( unsigned int i = 0; i < vec.size(); i++) {
                                                vec[i] = (sometype)i;
                                        }                   
                                }

                                if(vec.size() < 10) { 
                                        permutate_vector_good_small(vec);
                                        return;
                                }
                                unsigned int size = vec.size();
                                std::uniform_int_distribution<unsigned int> A(0,size - 4);
                                std::uniform_int_distribution<unsigned int> B(0,size - 4);

                                for( unsigned int i = 0; i < size; i++) {
                                        unsigned int posA = A(m_mt);
                                        unsigned int posB = B(m_mt);
                                        std::swap(vec[posA], vec[posB]); 
                                        std::swap(vec[posA+1], vec[posB+1]); 
                                        std::swap(vec[posA+2], vec[posB+2]); 
                                        std::swap(vec[posA+3], vec[posB+3]); 

                                } 
                        }

                template<typename sometype>
                        static void permutate_vector_good_small(std::vector<sometype> & vec) {
                                if(vec.size() < 2) return;
                                unsigned int size = vec.size();
                                std::uniform_int_distribution<unsigned int> A(0,size-1);
                                std::uniform_int_distribution<unsigned int> B(0,size-1);

                                for( unsigned int i = 0; i < size; i++) {
                                        unsigned int posA = A(m_mt);
                                        unsigned int posB = B(m_mt);
                                        std::swap(vec[posA], vec[posB]); 
                                } 
                        }

                template<typename sometype>
                        static void permutate_entries(const PartitionConfig & partition_config, 
                                                      std::vector<sometype> & vec, 
                                                      bool init) {
                                if(init) {
                                        for( unsigned int i = 0; i < vec.size(); i++) {
                                                vec[i] = i;
                                        }                   
                                }

                                switch(partition_config.permutation_quality) {
                                        case PERMUTATION_QUALITY_NONE: break;
                                        case PERMUTATION_QUALITY_FAST: permutate_vector_fast(vec, false); break;
                                        case PERMUTATION_QUALITY_GOOD: permutate_vector_good(vec, false); break;
                                }      

                        }

                static bool nextBool() {
                        std::uniform_int_distribution<unsigned int> A(0,1);
                        return (bool) A(m_mt); 
                }

		template <typename U = uint64_t> class fastRandBool {
			public:
				bool nextBool() {
					if (UNLIKELY(1 == m_rand)) {
						m_rand = std::uniform_int_distribution<U>{}(m_mt) | s_mask_left1;
					}
					bool const ret = m_rand & 1;
					m_rand >>= 1;
					return ret;
				}

			private:
				static constexpr const U s_mask_left1 = U(1) << (sizeof(U) * 8 - 1);
				U m_rand = 1;
		};

		static double approx_invsqrt( double number ) {                                                             
			double y = number;                                                                    
			double x2 = y * 0.5;                                                                  
			std::int64_t i = *(std::int64_t *) &y;                                                
			// The magic number is for doubles is from https://cs.uwaterloo.ca/~m32rober/rsqrt.pdf
			i = 0x5fe6eb50c7b537a9 - (i >> 1);                                                    
			y = *(double *) &i;                                                                   
			y = y * (1.5 - (x2 * y * y));   // 1st iteration                                      
			//      y  = y * ( 1.5 - ( x2 * y * y ) );   // 2nd iteration, this can be removed    
			return y;                                                                             
		}                                                                                             

		static double approx_sqrt( double number ) {                                                             
			return 1/approx_invsqrt(number);
		}                                                                                             

                //including lb and rb
                static unsigned nextInt(unsigned int lb, unsigned int rb) {
                        std::uniform_int_distribution<unsigned int> A(lb,rb);
                        return A(m_mt); 
                }

		// excluding rb
                static uint32_t nextIntHashing(uint64_t rb) {
			/* return (crc32(rand_counter++) % rb); */
			rand_counter = (rand_counter+1) % TAM;
			return rand_nums.arr[rand_counter] % rb;
                }

		/* static unsigned fastNextInt(unsigned int rb) { */
		/* 	unsigned int next_num; */
		/* 	char counter; */
		/* 	while(!_rdrand32_step(&next_num) && counter++ < 5); */
		/* 	return next_num % rb; */
		/* } */

                static double nextDouble(double lb, double rb) {
                        double rnbr   = (double) rand() / (double) RAND_MAX; // rnd in 0,1
                        double length = rb - lb;
                        rnbr         *= length;
                        rnbr         += lb;

                        return rnbr; 
                }

                static void setSeed(int seed) {
                        m_seed = seed;
                        srand(seed);
                        m_mt.seed(m_seed);
			uint64_t m_rand = 1;
			rand_counter = crc32(seed);
                }

        private:
                static int m_seed;
                static MersenneTwister m_mt;
		static uint32_t rand_counter;
};

#endif /* end of include guard: RANDOM_FUNCTIONS_RMEPKWYT */
