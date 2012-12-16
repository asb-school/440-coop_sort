/**
 * - Course: CPTR440 Operating Systems
 * - Institution: Andrews University
 * - Semester: Fall 2012
 * - Instructor: Dr. Villafane
 *
 * - Item: Cooperative sorting system
 *
 * - Environment: C++ with Posix threads
 *
 * - Student: Andrew Breja
 *
 * - compile |
 *   g++ -std=c++0x -pthread coop_sort.cpp -o coop_sort
 *
 * - execute |
 *   coop_sort numThreads numItems
 */

#include <iostream>
#include <algorithm>
#include <vector>
#include <queue>
#include <iterator>
#include <algorithm>

#include <cstdlib>
#include <pthread.h>
#include <semaphore.h>

//#include "conf.h"

using namespace std;

// Forward declarations
void *item_gen(void *p);
void *sorter(void *p);
void *merger(void *p);
int mainx(int argc, const char * argv[]);


template <typename T, typename TX>
bool is_sorted(T begin, T end)
{
    // later: iterator_traits<T>::value_type
    return adjacent_find(begin, end, greater<TX>()) == end;
}

/**
 * Random number generator wrapper for rand_r which can be used as a
 * generator function.
 */
class randt {
private:
	unsigned int s;
	//static unsigned int s_static;
public:
	randt(unsigned int seed):s(seed) {};
	randt() { s = rand(); /* s = rand_r(&s_static); */ };
	
	unsigned int operator()() {
		return rand_r(&s);
	}
};

//unsigned int randt::s_static = (unsigned int)time(NULL);

/**
 * Bounded buffer safe for use by multiple threads.
 */
class bounded_buffer {
public:
	queue< vector<int> > bucket_queue;
	sem_t mutex;
	sem_t empty;
	sem_t full;
	
	bounded_buffer(size_t sz)
	{
		sem_init(&mutex, 0, 1);
		sem_init(&empty, 0, (unsigned int)sz);
		sem_init(&full, 0, 0);
	}
	
	~bounded_buffer()
	{
		sem_destroy(&mutex);
		sem_destroy(&empty);
		sem_destroy(&full);
	}
	
	// Used by producer to add items
	void add(const vector<int>& bucket)
	{
		sem_wait(&empty);
		sem_wait(&mutex);
		
		bucket_queue.push(bucket);
		
		sem_post(&mutex);
		sem_post(&full);
	}
	
	// Used by consumer to extract items
	vector<int> remove()
	{
		sem_wait(&full);
		sem_wait(&mutex);
		
		vector<int> bucket;
		bucket = bucket_queue.front();
		bucket_queue.pop();
		
		sem_post(&mutex);
		sem_post(&empty);
		
		return bucket;
	}
};

/**
 * Information used by each item generator thread.
 */
class item_gen_tinfo
{
public:
	int id; // id number of item generator
	size_t num_items; // total number of items to generate
	size_t min_bucket_size;
	size_t max_bucket_size;
	bounded_buffer* bbuf;
	randt rng; // thread-safe random number generator
	
	item_gen_tinfo()
	{
		id = 0;
		num_items = 0;
		min_bucket_size = 0;
		max_bucket_size = 0;
		bbuf = NULL;
	}
	
	void print(ostream& os)
	{
		cout << "id: " << id << endl;
		cout << "num_items: " << num_items << endl;
		cout << "min_bucket_size: " << min_bucket_size << endl;
		cout << "max_bucket_size: " << max_bucket_size << endl;
		cout << "bbuf: " << bbuf << endl;
	}
};

/**
 * Information used by each sorting thread.
 */
class sorter_tinfo
{
public:
	int id; // id number of sorter
	
	// Incoming unsorted buckets
	bounded_buffer* bbuf_unsorted;
	// Outgoing sorted buckets
	bounded_buffer* bbuf_sorted;
	
	sorter_tinfo()
	{
		id = 0;
		bbuf_unsorted = NULL;
		bbuf_sorted = NULL;
	}
	
	void print(ostream& os)
	{
		cout << "id: " << id << endl;
		cout << "bbuf_unsorted: " << bbuf_unsorted << endl;
		cout << "bbuf_sorted: " << bbuf_sorted << endl;
	}
};

/**
 * Information used by merger thread.
 */
class merger_tinfo
{
public:
	// Incoming sorted buckets
	bounded_buffer* bbuf_sorted;
	// Outgoing single collection of sorted items
	vector<int>* sorted_items;
	
	merger_tinfo()
	{
		bbuf_sorted = NULL;
		sorted_items = NULL;
	}
};

//pthread_mutex_t mut;
//sem_t sem;
//pthread_barrier_t barr;

/**
 * Item generator creates buckets of unsorted items.
 */
void* item_gen(void *p)
{
    // wait for all threads to start together using a barrier
    //pthread_barrier_wait(&barr);
	
    item_gen_tinfo& ti(*(static_cast<item_gen_tinfo*>(p)));
    //ti.print(cout);
	
    size_t items_remaining;
    items_remaining = ti.num_items;
    while(items_remaining > 0)
    {
        size_t bucket_size;
        bucket_size = min(ti.min_bucket_size + ( ti.rng() %
												(1 + ti.max_bucket_size - ti.min_bucket_size)),
						  items_remaining);
        vector<int> bucket(bucket_size);
        generate(bucket.begin(), bucket.end(), ti.rng);
		
        cout << "Item gen " << ti.id << " adding " << bucket_size << " items" << endl;
        //cout << "Items: ";
        //copy(bucket.begin(), bucket.end(), ostream_iterator<int>(cout, " "));
        //cout << endl;
		
        ti.bbuf->add(bucket);
        items_remaining = items_remaining - bucket_size;
        //cout << "Items remaining: " << items_remaining << endl;
    }
	
    // wait for all threads to finish together using a barrier
    //cout << "Done with thread number" << tinfoi->thread_id << endl;
    //pthread_barrier_wait(&barr);
	
    return NULL;
}

/**
 * Sorter performs a sort on each individual bucket of unsorted items.
 * When an incoming bucket of size 0 is received, the sorter exits.
 */
void* sorter(void *p)
{
    return NULL;
}

/**
 * Merger merges buckets of sorted items into a single collection. When an
 * incoming bucket of size 0 is received, the merger exits.
 */
void* merger(void *p)
{
    return NULL;
}

int mainx(int argc, const char * argv[])
{
    // Seed the random number generator with the system time in seconds
    srand((unsigned int)time(NULL));
	
    //====================================================================
    // Variable definitions
    //====================================================================
	
    int num_threads;
    int num_items;
    int num_item_gen_tasks = 4;
    int item_gen_min_bucket_size = 10;
    int item_gen_max_bucket_size = 25;
    int bounded_buffer_max_size = 100;
	
    // Collection of all sorted items
    vector<int> items;
	
    pthread_attr_t pattr;
    pthread_attr_init(&pattr);
	
    //====================================================================
    // Get command line parameters
    //====================================================================
	
    if(argc != 3)
    {
        cout << "Usage: coop_sort numThreads numItems" << endl;
        exit(0);
    }
	
    num_threads = atoi(argv[1]);
    num_items = atoi(argv[2]);
	
    cout << "Number of threads: " << num_threads << endl;
    cout << "Number of items: " << num_items << endl;
	
    //====================================================================
    // More variable definitions
    //====================================================================
	
	
    //====================================================================
    // Create bucket queues
    //====================================================================
	
    bounded_buffer unsorted_q(bounded_buffer_max_size);
    bounded_buffer sorted_q(bounded_buffer_max_size);
	
    //====================================================================
    // Create item generator threads
    //====================================================================
	
    vector<pthread_t> item_gen_threads(num_item_gen_tasks);
    vector<item_gen_tinfo> item_gen_tinfos(num_item_gen_tasks);
	
    {
        int tid = 0;
        for(auto it = item_gen_tinfos.begin(); it != item_gen_tinfos.end(); it++)
        {
            item_gen_tinfo& ti = *it;
            ti.id = tid;
            ti.num_items = (num_items / num_item_gen_tasks) +
			((tid < num_items % num_item_gen_tasks) ? 1 : 0);
            ti.min_bucket_size = item_gen_min_bucket_size;
            ti.max_bucket_size = item_gen_max_bucket_size;
            ti.bbuf = &unsorted_q;
            cout << "Item generator " << tid << " will generate "
			<< ti.num_items << " items" << endl;
            tid++;
        }
    }
	
    {
        int tid = 0;
        for(auto it = item_gen_threads.begin(); it != item_gen_threads.end(); it++)
        {
            pthread_t* t = &(*it);
            pthread_create(t, &pattr, item_gen, &item_gen_tinfos[tid]);
            tid++;
        }
    }
	
    //====================================================================
    // Create sorter threads
    //====================================================================
    vector<pthread_t> sorter_threads(num_threads);
    vector<sorter_tinfo> sorter_tinfos(num_item_gen_tasks);
	
    {
        int tid = 0;
        for(auto it = sorter_tinfos.begin(); it != sorter_tinfos.end(); it++)
        {
            sorter_tinfo& ti = *it;
            ti.id = tid;
            tid++;
        }
    }
    {
        int tid = 0;
        for(auto it = sorter_threads.begin(); it != sorter_threads.end(); it++)
        {
            pthread_t* t = &(*it);
            pthread_create(t, &pattr, sorter, &sorter_tinfos[tid]);
            tid++;
        }
    }
	
    //====================================================================
    // Create merger thread
    //====================================================================
	
    pthread_t merger_thread;
    merger_tinfo merger_tinfo_x;
	
    merger_tinfo_x.sorted_items = &items;
    pthread_create(&merger_thread, &pattr, merger, &merger_tinfo_x);
	
    //====================================================================
    // Wait for all threads to finish
    //====================================================================
	
    //cout << "Before thread joins" << endl;
	
    // Wait for item generator threads to finish
    for(auto it = item_gen_threads.begin(); it != item_gen_threads.end(); it++)
    {
        pthread_t* t = &(*it);
        pthread_join(*t, NULL);
    }
	
    // Wait for sorter threads to finish. Each one must receive a
    // bucket of size 0 to know it is time to exit.
    //unsorted_q x
    for(int i = 0; i < num_threads; i++)
    {
        //unsorted_q.add(vector<int>());
    }
    for(auto it = sorter_threads.begin(); it != sorter_threads.end(); it++)
    {
        pthread_t* t = &(*it);
        pthread_join(*t, NULL);
    }
	
    // Wait for merger thread to finish. It must receive a
    // bucket of size 0 to know it is time to exit.
    //sorted_q.add(vector<int>());
    pthread_join(merger_thread, NULL);
	
    //cout << "After thread joins" << endl;
	
    //====================================================================
    // Process results
    //====================================================================
	
    //if(is_sorted<vector<int>::const_iterator,int>(items.begin(), items.end()))
    //{
    //    cout << "Sorting success" << endl;
    //}
    //else
    //{
    //    cout << "Sorting failure" << endl;
    //}
	
    // Temporary: print contents of both queues
    {
        int bucket_num = 0;
        cout << "Unsorted queue numbers are below" << endl;
        while(!unsorted_q.bucket_queue.empty())
        {
            vector<int> bucket = unsorted_q.bucket_queue.front();
            unsorted_q.bucket_queue.pop();
            cout << "bucket " << bucket_num << endl;
            copy(bucket.begin(), bucket.end(), ostream_iterator<int>(cout, "\n"));
            bucket_num++;
        }
    }
    {
        int bucket_num = 0;
        cout << "Sorted queue numbers are below" << endl;
        while(!sorted_q.bucket_queue.empty())
        {
            vector<int> bucket = sorted_q.bucket_queue.front();
            sorted_q.bucket_queue.pop();
            cout << "bucket " << bucket_num << endl;
            copy(bucket.begin(), bucket.end(), ostream_iterator<int>(cout, "\n"));
            bucket_num++;
        }
    }
	
    // Print the sorted numbers
    cout << "Sorted numbers are below" << endl;
    copy(items.begin(), items.end(), ostream_iterator<int>(cout, "\n"));
	
    return 0;
}


//#if defined(BUILD_MAKE) && (defined(OS_LIN) || defined(OS_OSX))

// Linux and OSX built from the command line
int main(int argc, const char * argv[])
{
    return mainx(argc, argv);
}

//#elif defined(OS_OSX)

// OSX builds using Xcode

//#else

//#error "Operating system not supported"

//#endif
