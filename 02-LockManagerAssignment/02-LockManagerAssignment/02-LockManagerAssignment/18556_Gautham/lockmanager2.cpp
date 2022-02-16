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
  uint32_t getTxnId()
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

uint8_t lock(std::string resource_name,std::uint32_t txn_id,uint8_t lock_type);

uint8_t unlock(std::string resource_name,std::uint32_t txn_id);

unordered_map<std::string, list<lockable_resource*>* > lock_table;

int main()
{
/* 
1. Lock a resource in either shared or exclusive mode. 
2. Unlock a resource held by a transaction. 
A resource will be identified by a 10 character string. 
*/
//lock mode either in shared or exclusive
  
  bool ret1,t22,t11,t3,t33,t1,t2,ret;// two sample return values
  
  ret = lock("AAA", 1234, lockType::SHARED);
  ret1=unlock("AAA",1234);
  t1 = lock("AAA", 4567, lockType::SHARED); 
  t11=unlock("AAA",4567);
  t2 = lock("AAA", 123456, lockType::EXCLUSIVE);
  t22=unlock("AAA",123456);
  t3=lock("BBB",1234567,lockType::SHARED);
  t33=unlock("BBB",1234567);

     if (ret==lockStatus::GRANTED)
     cout<<"Successfully locked the resource AAA for transaction for 1234"<<endl;
     else 
     cout<<"Successfully could not be locked the resource waiting for lock. ret"<<endl;
     if (ret1==lockStatus::GRANTED)
     cout<<"Successfully unlocked the resource."<<endl;
     else
     cout<<"Could not find the resource to unlock"<<endl;
     
     
     
     
     
     if (t1==lockStatus::GRANTED)
     cout<<"Successfully locked the resource AAA by 1234. t1"<<endl;
     else 
     cout<<"Successfully could not lock the resource AAA. t1"<<endl;
     if (t11==lockStatus::GRANTED)
     cout<<"Successfully unlocked the resource AAA from 1234."<<endl;
     else
     cout<<"Could not find the resource to unlock AAA"<<endl;
     if (t2==lockStatus::GRANTED)
     cout<<"Successfully locked the resource AAA by 4567.t2 "<<endl;
     else 
     cout<<"Successfully could not lock the resource. t2"<<endl;
     if (t3==lockStatus::GRANTED)
     cout<<"Successfully locked the resource.t3"<<endl;
     else 
     cout<<"Successfully could not lock the resource.t3"<<endl;


    //unlock the resources held by the transactions.
    
    
     if (t11==lockStatus::GRANTED)
     cout<<"Successfully unlocked the resource AAA."<<endl;
     else
     cout<<"Could not find the resource to unlock"<<endl;
     if (t22==lockStatus::GRANTED)
     cout<<"Successfully unlocked the resource AAA by 456."<<endl;
     else
     cout<<"Could not find the resource to unlock"<<endl;
     if (t33==lockStatus::GRANTED)
     cout<<"Successfully unlocked the resource BBB by 123456."<<endl;
     else
     cout<<"Could not find the resource to unlock"<<endl;
  return 0;
}

uint8_t lock(std::string resource_name,std::uint32_t txn_id,uint8_t lock_type)
{
  uint8_t retval = lockStatus::WAITING;
  // Check if lock exists for the resource. 
  //   If not the add the resource to map, create new list and add lockable_resource to list
   if (lock_table.find(resource_name) == lock_table.end())
  {
    // lock table does not exist. 
    //  => locable resource has to be created. 
    //     list of lockable resources has to be created. 
    //     lock table should be updated with this resource. 
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
//else if resource is already present in the lock table then iterate through the list and give the desired locks on the resource.
else 
  {
    list<lockable_resource*>:: iterator iter = lock_table[resource_name]->begin();
  //we got to the list of the transactions and their locks.
  // we see the type of locks taken by the transactions and give accordingly the lock on the resource.
//first see if there are any exclusive locks present in the list
  for (;iter!=lock_table[resource_name]->end();iter++)
          {
            if ((*iter)->getLockType() == lockType::EXCLUSIVE  )
            {
               (*iter)->setLockStatus(lockStatus::WAITING);
               retval = lockStatus::WAITING ;
                return(retval);
            }
          }
    for (iter=lock_table[resource_name]->begin();iter!=lock_table[resource_name]->end();iter++)      
            {
              if ((*iter)->getLockType() == lockType::SHARED  )
            {
              if(lock_type == lockType::SHARED)
                 {           
          lockable_resource *lr = new lockable_resource(txn_id,lock_type,lockStatus::GRANTED);     
          lock_table[resource_name]->emplace_back(lr);
          //cout<< (lockStatus)((*iter)->getStatus()) <<endl;
               (*iter)->setLockStatus(lockStatus::GRANTED);
          retval = lockStatus::GRANTED;
          return(retval);
                 }
            }
            else //if ((*iter)->getLockType() == lockType::EXCLUSIVE ) 
            {
                (*iter)->setLockStatus(lockStatus::WAITING);
               retval = lockStatus::WAITING; 
               return(retval);
            }
           cout<<((*iter)->getTxnId()) <<endl;
          }
 
 /* for (;iter!=lock_table[resource_name]->end();iter++)
          {
           cout<<"Locks are held by \n: "<<endl;
           cout<<((*iter)->getTxnId()) <<endl;

            if ((*iter)->getLockType() == lockType::SHARED  )
            {
              if(lock_type == lockType::SHARED)
                 {           
          lockable_resource *lr = new lockable_resource(txn_id,lock_type,lockStatus::GRANTED);     
          lock_table[resource_name]->emplace_back(lr);
          //cout<< (lockStatus)((*iter)->getStatus()) <<endl;
          retval = lockStatus::GRANTED;
                 }
            }
            else //if ((*iter)->getLockType() == lockType::EXCLUSIVE ) 
            {
               return(lockStatus::WAITING); 
            }
          }

*/
    //lockable_resource *lr = new lockable_resource(txn_id,lock_type,lockStatus::GRANTED);     
    //lock_table[resource_name]->emplace_back(lr);
          //cout<< (lockStatus)((*iter)->getStatus()) <<endl;
 //retval= lockStatus::GRANTED;
 } 
return(retval);
}


//method to unlock the lock on the resource
uint8_t unlock(std::string resource_name,std::uint32_t txn_id)
{
uint8_t retval = lockStatus::WAITING;
   list<lockable_resource*>:: iterator iter = lock_table[resource_name]->begin();
  if (lock_table.find(resource_name) == lock_table.end())
  {
     cout<<"No resource with resource name "<< resource_name << " found to be locked "<<endl;
    retval = lockStatus::WAITING; // return waiting if not found. 
  
  return(retval);
  }
  else // see in the locktable and get the list and unlock the resource.
  {
     for (;iter!=lock_table[resource_name]->end();iter++)
          {
            if ((*iter)->getTxnId() == txn_id )
            {
               (*iter)->setLockStatus(GRANTED); 
               retval = lockStatus::GRANTED ;
                return(retval);
            }
          }
  }






}



 /*
      for (auto x : lock_table[resource_name]) 
{
      list<lockable_resource*> p=*(x.second);
      cout << x.first << p.front()<< endl; 
}
*/

/* Evaluation Comments 
1. Basic Compilation (g++ -std=c++11 lockmanager2.cpp):
   Code compiles without errors. 

2. Correctness of output:
    1. Line 244 - Unlock method should never return value lockState::WAITING. 
       Unlock should always succed. If resource is not found, it is not necessarily an error.
    2. Unlock method is wrong even for cases where resource is found. The resource should be removed
       from the list.   
    3. Your cout statements printing the outcome of the lock and unlock requests are misleading. 
       For example, the code on lines 100 and 101 will state that the unlock suceeded when in fact, the lock 
       is not removed from the lock table.   
    4. The lock method is incorrect. If transaction B requests a lock held by transaction A in an incompatible
       mode, your lock method changes the grant state of transaction A. This is done in line 175:
       (*iter)->setLockStatus(lockStatus::WAITING); 
    
   
2. Comments and programming style: 
    1. Your method of capturing all the return codes and a large block of cout statements is confusing. 
       It would have been better to print the outcome after each operation. 
    2. Indentation and braces are not consistent (esp in unlock method). 
    3. Always delete commented out code before submitting. Otherwise, it creates clutter.

3. Exception Handling:
   1. Unlock last lock on item (more items in map) - not handled. 
   2. Ulock last item in map - not handled. 
   3. Calling unlock on empty table - handled. 
   4. Check for memory allocation errors - not done. 

Score: 6/10
*/