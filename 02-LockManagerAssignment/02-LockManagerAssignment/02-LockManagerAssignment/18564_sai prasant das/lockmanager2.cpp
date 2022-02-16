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

uint8_t lock(std::string resource_name, 
          std::uint32_t txn_id,
          uint8_t lock_type);

uint8_t unlock(std::string resource_name,
              std::uint32_t txn_id);

unordered_map<std::string, list<lockable_resource *>*> lock_table;

int main()
{
  uint8_t ret = lock("AAA", 1234, lockType::EXCLUSIVE);
  cout<<"print status for resource AAA for txt id 1234 "<<lockStatus(ret)<<endl;
  ret = lock("AAA", 4567, lockType::EXCLUSIVE);
  cout<<"print status for resource AAA for txt id 4567 "<<lockStatus(ret)<<endl;
  ret = unlock("AAA", 1234);
  cout<<"print status for resource AAA for txt id 4567 after unlocking 1234 "<<lockStatus(ret)<<endl;

  return 0;

}

uint8_t lock(std::string resource_name,
          std::uint32_t txn_id,
          uint8_t lock_type)
{
  uint8_t retval = lockStatus::WAITING;
  // Check if lock exists. 
  //   No: Add to map, create new list and add lockable_resource to list
  if (lock_table.find(resource_name) == lock_table.end())
  {
    // lock table does not exist. 
    //  => locable resource has to be created. 
    //     list of lockable resources has to be created. 
    //     lock table should be updated with resource. 
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
  
  else 
  {
    //lock table does exist and lockable resources also exist
    lockable_resource *lr = new lockable_resource(
      txn_id,
      lock_type,
      lockStatus::GRANTED
    );
    int add=1;
    list<lockable_resource*> *lst = lock_table.find(resource_name)->second;
    
    for(auto& x : *(lst))
    {
      if(x->getTxnId() == lr->getTxnId())
      {
        if(lr->getLockType() == lockType::SHARED && (x->getLockType() == lockType::EXCLUSIVE || x->getLockType() == lockType::SHARED))
        {
          cout<<"LOCK already exist with status : "<<lr->getStatus()<<"and type : "<<lr->getLockType()<<endl;
          add=0;
          break;
        }
        else if (lr->getLockType() == lockType::EXCLUSIVE && x->getLockType() == lockType::EXCLUSIVE)
        {
          cout<<"LOCK already exist with status : "<<lr->getStatus()<<"and type : "<<lr->getLockType()<<endl;
          add=0;
          break;
        }
        //else case where lr->getLockType == lockType::EXCLUSIVE the txt has to join the list to get exclusive i.e making this implementation strictly first come first bases
        else
        {
          lr->setLockStatus(lockStatus::WAITING);
          break;
        }
      }
      if(x->getLockType() == lockType::EXCLUSIVE)
      { 
        lr->setLockStatus(lockStatus::WAITING);
        break;
      }
    }
    if (lr->getLockType() == lockType::EXCLUSIVE)
      lr->setLockStatus(lockStatus::WAITING);
    if (add == 1)
      lst->emplace_back(lr);
    retval=lr->getStatus();
  }
return(retval);
}
uint8_t unlock(std::string resource_name,
              std::uint32_t txn_id)
{
  list<lockable_resource*> *lst = lock_table.find(resource_name)->second;
  lockable_resource *x = lst->front(); 
  if (x->getTxnId() == txn_id)
    lst->remove(x);
  x = lst->front();
  if(x->getStatus() == lockStatus::GRANTED)
    return 1;
  else 
    if(x->getLockType() == lockType::SHARED)
    {
      for (auto& iter :*(lst))
      {
        if(iter->getLockType() == lockType::SHARED)
          iter->setLockStatus(lockStatus::GRANTED);
        else
          return 1;
      }
    }
    else 
      x->setLockStatus(lockStatus::GRANTED);
  for (auto& iter :*(lst))
  {
    if (iter->getTxnId() == txn_id)
    {
      lst->remove(iter);
      return 1;
    }
  }
  return 0;
}

/* Evaluation Comments 
1. Basic Compilation (g++ -std=c++11 lockmanager2.cpp):
   Code compiles without errors. 

2. Correctness of output:
    1. Unlocking the last resource causes segmentation fault. 
    2. Basic cases work. 
    3. The third lock request in the sequence below is put in the waiting state when in fact it can be granted:
       ret = lock("AAA",1,lockType::SHARED);
       ret = lock("BBB",1,lockType::SHARED);
       ret = lock("AAA",1,lockType::EXCLUSIVE);
   
2. Comments and programming style: 
    1. There are no comments in your implementation. Neither I nor you (after a few weeks) will be able to follow your design without comments. 
    2. Indentation is neat and consistent - good!
    3. The remove() method does not delete an item from an STL container. It is better to use erase(). 

3. Exception Handling:
   1. Unlock last lock on item (more items in map) - not handled (line 172 causes segmentation fault.) 
   2. Ulock last item in map - not handled (line 172 causes segmentation fault.). 
   3. Calling unlock on empty table - not handled (line 167 causes segmentation fault - SIGSEGV). 
   4. Check for memory allocation errors - not done. 

Score: 7/10
*/