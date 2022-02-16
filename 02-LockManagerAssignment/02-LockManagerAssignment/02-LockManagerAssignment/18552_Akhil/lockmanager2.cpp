/* 

Goal : Build a lock manager. 
The lock manager should support the following capabilities:
1. Lock a resource in either shared or exclusive mode. 
2. Unlock a resource held by a transaction. 
A resource will be identified by a 10 character string. 

 */

#include <iostream>
#include <unordered_map>
#include <list>

using namespace std;

// Abstraction of a resource that can be locked. 
// A resource is locked in a 'mode' by a 'transaction'. 
// The lock request may be granted or put on wait based
// on a lock compatibility matrix. 
class lockable_resource
{
	private:
		uint32_t txn_id_;
		uint8_t lock_type_; // SHARED, EXCLUSIVE
		uint8_t lock_status_; // GRANTED, WAITING
	public:
		lockable_resource(
				uint32_t txn_id,
				uint8_t lock_type,
				uint8_t lock_status):
			txn_id_(txn_id),
			lock_type_(lock_type),
			lock_status_(lock_status)
	{}
		uint8_t getLockType() 
		{
			return(lock_type_);
		}
		uint8_t getStatus()
		{
			return(lock_status_);
		}
		uint8_t getTxnId()
		{
			return(txn_id_);
		}
		bool setLockStatus(uint8_t st)
		{
			lock_status_ = st;
		}

};

enum lockType
{
	SHARED,
	EXCLUSIVE
};
enum lockStatus
{
	GRANTED,
	WAITING
};

// Resource
// type of lock
// txnid

bool lock(std::string resource_name, std::uint32_t txn_id, uint8_t lock_type);

bool unlock(std::string resource_name, std::uint32_t txn_id);

unordered_map<std::string, list<lockable_resource *>*> lock_table;

int main()
{
	uint8_t ret = lock("AAA", 1234, lockType::EXCLUSIVE);
	ret = lock("AAA", 4567, lockType::EXCLUSIVE);
	return 0;

}

bool lock(std::string resource_name, std::uint32_t txn_id, uint8_t lock_type)
{
	uint8_t retval = lockStatus::WAITING;
	// Check if lock exists. 
	//   No: Add to map, create new list and add lockable_resource to list
	if (lock_table.find(resource_name) == lock_table.end())
	{
		// lock table does not exist. 
		//  => locable resource has to be created. 
		//     list of lockable resources has to be created. 
		//     lock table should be updated wit resource. 
		lockable_resource *lr = new lockable_resource(
				txn_id,
				lock_type,
				lockStatus::GRANTED
				);
		retval = lockStatus::GRANTED;
		list<lockable_resource*> *lst = new list<lockable_resource*>;
		lst->emplace_back(lr);
		lock_table[resource_name] = lst;
	}
	return(retval);

}
bool unlock(std::string resource_name, std::uint32_t txn_id, uint8_t lock_type)
{
	if(lock_table.find(resource_name) == lock_table.end())
		return true;
	else
	{
		list<lockable_resource*> *lt = lock_table[resource_name];

		for(list<lockable_resource*>::iterator it = (*lt).begin(); it != (*lt).end(); it ++)
		{
			if((*it)->getTxnId() == txn_id && (*it)->getLockType() == lock_type && (*it)->getStatus() == lockStatus::GRANTED)
			{
				(*lt).erase(it);

				if((*lt).empty())
					lock_table.erase(resource_name);
				else if(( *((*lt).begin()) ) -> getLockType() == lockType::SHARED)
					( *((*lt).begin()) ) -> setLockStatus(lockStatus::GRANTED);
				else
				{
					uint8_t status = lockStatus::GRANTED;

					for(list<lockable_resource*>::iterator it = (*lt).begin(); it != (*lt).end(); it ++)
						if((*it)->getStatus() == lockStatus::GRANTED)
							status = lockStatus::WAITING;

					( *((*lt).begin()) ) -> setLockStatus(status);
				}


				return false;
			}
		}
	}
	return true;
}

/* Evaluation Comments 
1. Basic Compilation:
   1. When using the unlock method, how did this application compile for you? I am getting the following compilation error:
      undefined reference to `unlock(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, unsigned int)'
      collect2: error: ld returned 1 exit status

      The unlock method (when i attempt to use it) is forward declared on line 72 to have 3 arguments. You have
      implemented it with 3 arguments. Why is a lock type needed for unlock? 

2. Correctness of output:
   Did you try running your program via the debugger? It does not work correctly. 
   Why is the unlock method returning true when no work is done? (line 142 - last but one line in your program)?
   The unlock method did not remove the lock when I tested via the debugger. 
   The reason for the incorrect behaviour is in line 118 in this check - (*it)->getTxnId() == txn_id 
   The getTxnId() method returns the txn ID as a 1 byte integer. You are comparing it with a 4 byte integer. This fails. 
   Thhough the method was part of the boilerplate code given to you, you should have found this if you had tested your
   program. 

2. Comments and programming style: 
    1. There are no comments to help me understand your design. 
    2. You have used tabs instead of spaces for indentation. It is a good practice to always use spaces instead of
	   tabs for indentation.  You can set your IDE to do this automatically. All IDEs/editors support this feature.
	   Set it replace tabs with 2 spaces. 

3. Exception Handling:
   1. Unlock last lock on item (more items in map) - not handled. 
   2. Ulock last item in map - not handled. 
   3. Calling unlock on empty table - handled. 
   4. Check for memory allocation errors - not done. 
     

Score: 6/10.
*/

