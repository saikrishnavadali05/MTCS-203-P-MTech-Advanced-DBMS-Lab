/* 

Goal : Build a lock manager. 
The lock manager should support the following capabilities:
1. Lock a resource in either shared or exclusive mode. 
2. Unlock a resource held by a transaction. 
A resource will be identified by a 10 character string. 

*/
//Author: Sateesh
#include <iostream>
#include <unordered_map>
#include <list>
#include <algorithm>      // for std::find
#include <iterator>       // for std::advance

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
  uint32_t getTxnId()
  {
    return(txn_id_);
  }
  bool setLockStatus(uint8_t st)
  {
    lock_status_ = st;
  }
  bool setLockType(uint8_t st)
  {
    lock_type_ = st;
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

bool lock(std::string resource_name, 
          std::uint32_t txn_id,
          uint8_t lock_type);

uint8_t unlock(std::string resource_name,
              std::uint32_t txn_id);

unordered_map<std::string, list<lockable_resource *>*> lock_table;

int main()
{
  uint8_t ret = lock("BBB", 442, lockType::SHARED);
  ret = lock("BBB", 442, lockType::SHARED);
  ret = lock("BBB", 554, lockType::SHARED);
  ret = lock("AAA", 124, lockType::EXCLUSIVE);
  ret = lock("AAA", 243, lockType::SHARED);
  ret= unlock("CCC",442);
 // ret= unlock("BBB",554);
  //ret= unlock("BBB",124);
  //ret= unlock("BBB",111);
  return 0;

} // end of main()

///////////////////////////////////////////////////////////////////////////////
uint8_t unlock(std::string resource_name,
               std::uint32_t txn_id/*,
               uint8_t lock_type*/)
{
uint8_t retVal = true;
  if(lock_table.find(resource_name) == lock_table.end()){
    cout<<"Resource "<<resource_name<<" can't be unlocked by "<<txn_id<<endl;
    retVal = true;
    return retVal;
  }
  else
  {
    std::list<lockable_resource*>::iterator it= lock_table[resource_name]->begin();
    for(; it != lock_table[resource_name]->end();++it){
        if((*it)->getTxnId() == txn_id){
          
            list<lockable_resource*>::iterator iter2;
            iter2=it;
            iter2++;
            lock_table[resource_name]->erase(it);
            cout<<(*it)->getTxnId()<<" has released lock on "<<resource_name<<endl;
            it = iter2;
            (*it)->setLockStatus(GRANTED);
            cout<<(*it)->getTxnId()<<" has acquired lock on "<<resource_name<<endl;
            if((*it)->getLockType() == SHARED){
              for(;it != lock_table[resource_name]->end() && (*it)->getLockType() == SHARED ; ++it){
                if((*it)->getStatus() == lockStatus::WAITING){
                  (*it)->setLockStatus(GRANTED);
                  cout<<(*it)->getTxnId()<<" has acquired lock on "<<resource_name<<endl;
                }            
              }
              
            }
          return retVal;
        }
      }
   }  
   cout<<"Resource "<<resource_name<<" can't be unlocked by "<<txn_id<<endl;
   return retVal;
}

/////////////////////////////////////////////////////////////////////////////
bool lock(std::string resource_name,
          std::uint32_t txn_id,
          uint8_t lock_type)
{
  uint8_t retval = lockStatus::WAITING;
  lockable_resource *lr;
  // Check if lock exists. 
  //   No: Add to map, create new list and add lockable_resource to list
      list<lockable_resource*> *lst = new list<lockable_resource*>;

  if (lock_table.find(resource_name) == lock_table.end())
  {
    // lock table does not exist. 
    //  => locable resource has to be created. 
    //     list of lockable resources has to be created. 
    //     lock table should be updated wit resource. 
   // cout<<"Yaah Granted!!!"<<txn_id<<endl;
    lockable_resource *lr = new lockable_resource(
      txn_id,
      lock_type,
      lockStatus::GRANTED
    );
     if(lock_type == 0)
        cout<<txn_id<<" has Shared on "<<" resource "<<resource_name<<endl;
     else
        cout<<txn_id<<" has Exclusive on "<<" resource "<<resource_name<<endl;      
    retval = lockStatus::GRANTED;
    lst->emplace_back(lr);
    lock_table[resource_name] = lst;
  }
  else
  {
     
     std::list<lockable_resource*> l= *(lock_table[resource_name]);
     std::list<lockable_resource*>::iterator it= lock_table[resource_name]->begin();
     std::list<lockable_resource*>::iterator it2= lock_table[resource_name]->begin();
     std::list<lockable_resource*>::iterator it3= lock_table[resource_name]->begin();

      if(lock_type == 0)
          cout<<"Asking Shared Lock By: "<<txn_id<<endl;
      else
      {
          cout<<"Asking Exclusive Lock By: "<<txn_id<<endl;

      }
        if((*it)->getTxnId() == txn_id && (*it)->getLockType() == lock_type){
          cout<<"Duplicate Locking is not allowed"<<"(by "<<txn_id<<")"<<" on "<<resource_name<<endl;
          return retval;
        }
        if((*it)->getTxnId() == txn_id){
          if((*it)->getLockType() == EXCLUSIVE && lock_type == lockType::SHARED){
            cout<<txn_id<<" has changed its locktype on "<<resource_name<<" from Exclusive to Shared"<<endl;
            (*it)->setLockType(SHARED);
            return retval;
          }
        }
        if((*it)->getLockType() == EXCLUSIVE){
            //insert();
          //  cout<<"at 1"<<endl;
            lockable_resource *lr1 = new lockable_resource(
             txn_id,
            lock_type,
            lockStatus::WAITING
            );
            lock_table[resource_name]->emplace_back(lr1);
            cout<<"But in Waiting"<<endl;
            
            
            //return retval;
        }
        if((*it)->getLockType() == SHARED && lock_type == lockType::SHARED){
           for(;it3 != lock_table[resource_name]->end();it3++){
             if((*it3)->getStatus() == WAITING){
               lockable_resource *lr2 = new lockable_resource(
               txn_id,
               lock_type,
               lockStatus::WAITING
                );
                cout<<"In Waiting State by: "<< (*lr2).getTxnId() << endl;
                lock_table[resource_name]->emplace_back(lr2);
                return retval;
              }
           }
           lockable_resource *lr2 = new lockable_resource(
             txn_id,
            lock_type,
            lockStatus::GRANTED
            );
            //l.emplace_back(lr);
            cout<<"Yaaah Granted!!! to "<< (*lr2).getTxnId() << endl;
            lock_table[resource_name]->emplace_back(lr2);
           /* for(;it2 != lock_table[resource_name]->end();++it2){
             cout<<(*it2)->getTxnId()<<endl;
            }*/
            return retval;
        }
        if((*it)->getLockType() == SHARED && lock_type == lockType::EXCLUSIVE){
             lockable_resource *lr3 = new lockable_resource(
             txn_id,
            lock_type,
            lockStatus::WAITING
            );
            cout<<"In Waiting State by: "<< (*lr3).getTxnId() << endl;
            lock_table[resource_name]->emplace_back(lr3);
            /*for(;it2 != lock_table[resource_name]->end();++it2){
             cout<<(*it2)->getTxnId() << endl;
           }*/
        } 
      return retval;
  }
}

/* Evaluation Comments 
1. Basic Compilation (g++ -std=c++11 lockmanager2.cpp):
   Code compiles without errors. 

2. Correctness of output:
    Line 123 - (*it)->setLockStatus(GRANTED); will cause a segmentation fault (SIGSEGV) 
    if you have just unlocked the last lock in the lock_table. 
    You should not blindly set the lockState to granted after unlocking. What if you have just
    unlocked a shared lock and the next item on the list has the lock granted already. 
   
2. Comments and programming style: 
    1. There are no comments (apart from the boilerplate comments) that help me follow your design. 
    2. Use consistent indentation.  You have used 2 spaces in most places and 4 in a few. 
       (ex: line 166 vs line 185)

3. Exception Handling:
   1. Last lock on item (more items in map) - not handled. 
   2. Last item in map - not handled. 
   3. Calling unlock on empty table - handled. 
   4. Check for memory allocation errors - not done. 
     
Score: 8/10

*/


