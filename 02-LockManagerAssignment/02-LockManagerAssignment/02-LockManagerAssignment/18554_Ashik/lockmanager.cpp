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
  lockable_resource(){}
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
    return true;
  }
  bool setLockType(uint8_t lt){
    lock_type_= lt;
    return true;
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
void print_locktable();

bool unlock(std::string resource_name,
              std::uint32_t txn_id);

unordered_map<std::string, list<lockable_resource *>*> lock_table;

int main()
{
  uint8_t ret;
  ret = lock("AAA", 1234, lockType::EXCLUSIVE);
  ret = lock("AAA", 12345, lockType::SHARED);
  ret = lock("BAAA", 123444, lockType::SHARED);
  ret = lock("BAAA", 12344, lockType::EXCLUSIVE);
  ret = lock("BAAA", 12344, lockType::SHARED);
  ret = lock("AAA", 12321, lockType::SHARED);
  int opt;
  string resource_name;
  uint32_t txnid;
  
  while(true){
    print_locktable();
    cout <<"\nLOCK\t: (1)\nUNLOCK\t: (2)\nEXIT\t: (3)\nOption\t: ";
    cin >> opt;
    
    switch (opt)
    {
      case 1:
        cout <<""<<endl;
        cout <<"Resource Name\t:";
        cin>>resource_name;
        cout <<"Transaction ID\t:";
        cin >>txnid;
        cout <<"EXCLUSIVE\t(1)\nSHARED\t\t(2)\nOption\t: ";
        int lock_type;
        cin >>lock_type;
        lockType lt;
        if (lock_type == 1)
          lt= lockType::EXCLUSIVE;
        else
          lt= lockType::SHARED;
        
        lock(resource_name,txnid,lt);
        break;
      case 2:
        cout <<"Unlock Module\n";cout <<""<<endl;
        cout <<"Resource Name\t:";
        cin>>resource_name;
        cout <<"Transaction ID\t:";
        cin >>txnid;
        unlock(resource_name,txnid);
        break;
      case 3:
        return 0;
      default:
        cerr <<"ERROR: Option not found.";
        break;
    }
  }
  print_locktable();
  unlock("AAA",1234);
  print_locktable();
  return 0;

}

int* count_lock(string resource_name, int (& count)[2])
{
  list<lockable_resource *>::iterator l_itr= lock_table[resource_name]->begin();
  for (; l_itr != lock_table[resource_name]->end(); l_itr++){
    lockable_resource resource= **l_itr;
    if (resource.getLockType() == lockType::SHARED && resource.getStatus() == lockStatus::GRANTED)
      count[0]++;
    else if (resource.getLockType() == lockType::EXCLUSIVE && resource.getStatus() == lockStatus::GRANTED)
      count[1]++;
    
  }
  return count;
}

bool lock(std::string resource_name,std::uint32_t txn_id,uint8_t lock_type)
{
  // lock_count is like bookepping
  // ... It stores on a resource 'R' how many 'GRANTED' 'SHARED' locks are there and how many 'EXCLUSIVE' locks are there
  // .... It is used to take care of starving problem
  int lock_count[2]= {0,0};
  
  uint8_t retval = lockStatus::WAITING;

  // Check if lock exists. 
  // if No then Add to lock_table, create new list and add lockable_resource to list
  unordered_map<std::string, list<lockable_resource *>*>::const_iterator lock_table_iterator = lock_table.find(resource_name);
  if (lock_table_iterator == lock_table.end())
  {
    // In lock table the entry does not exist. 
    //  => locable resource has to be created. 
    //     list of lockable resources has to be created. 
    //     lock table should be updated with resource. 

    // Creating a lockable_resource
    lockable_resource *lr = new lockable_resource(txn_id,lock_type,lockStatus::GRANTED);
    // setting its lockstatus as granted
    retval = lockStatus::GRANTED;
    // Creating a list for the resource
    list<lockable_resource*> *lst = new list<lockable_resource*>;
    // adding the lockable_resource in the list
    lst->emplace_back(lr);
    // Adding the list and resource locked in the lock_table
    lock_table[resource_name] = lst;
  }
  else{
    // Here we are counting how many shared (GRANTED) and exclusive (both) locks are there in the resource 
    count_lock(resource_name,lock_count);

    // Already some txn has locked the resource before.
    // ... So, going to start of the list that containes the detail of all txns that has requested for that resource
    list<lockable_resource *>::iterator lr_iter= lock_table[resource_name]->begin();

    // This flag is used to indicate whether the transaction requesting lock is duplicate or not.
    //  true: for duplicate transaction
    //  false: for new transaction
    bool flag= false;
    lockable_resource *resource;
    // searching if the resource is held by the same transaction that has requested
    // ..... i.e: Searching for duplicate request.
    for(; lr_iter != lock_table[resource_name]->end(); lr_iter++){
      resource= *lr_iter;
      // If the duplicate transaction is found then raise flag true.
      if (resource->getTxnId() == txn_id){
        flag= true;
        break;
      }
    }
    
    // if flag is true: then a duplicate request is made.
    // There is no conflict if the duplicate requests is of same lock_type even if the lock is "GRANTED or WAITING" for the old transaction.
    // ..... i.e: Transaction 'A' has locked resource 'R' as "SHARED" and again asking for "SHARED" lock.
    //
    // But if the duplicate request has different lock_type than the actual/old transaction then.....
    //
    // Case1: if the transaction 'A' has already requested the resource 'R' as "SHARED" and the "duplicate" request 'B' is "EXCLUSIVE"
    //        ..... then change the "lock_type" of the old txn 'A' to "EXCLUSIVE" -> if the "status" of the old txn 'A' is "WAITING"
    //        ..... if the "status" of old transaction is "GRANTED" then 
    //        ...... CASE 1.a: if only one "SHARED" lock is there on the resource 'R' then change the lock_type of the actual/old txn 'A' to "EXCLUSIVE" and "GRANTED"
    //        ...... CASE 1.b: if there are many "SHARED" lock on the resource 'R' then change the lock_type of the actual/old txn 'A' to "EXCLUSIVE" and "WAITING".
    //
    // Case 2: if the transaction 'A' has already requested the resource 'R' as "EXCLUSIVE" and the "duplicate" 'B' request us "SHARED"
    //         ..... then change the "lock_type" of the old_txn 'A' to "SHARED" -> if the "status" of the old txn 'A' is "WAITING"
    //         ..... if the "status" of the old txn 'A' is "GRANTED" 
    //         ......... then change the "lock_type" of the old_txn 'A' to "SHARED","GRANTED" 
    //         ............... then change the "lock_status" of the txn who are waiting ,if any, for the SHARED key to "GRANTED"
    

    if (flag==true){
      // If the transaction is duplicate
      // When the lock held by the actual transaction is different than the lock asked by the duplicate transaction      
      if (lock_type != resource->getLockType() ){
        if (lock_type == lockType::EXCLUSIVE){
          // The requested lock is EXCLUSIVE and the actual lock is SHARED
          // .... So, if there are any "SHARED+GRANTED" locks on the same resource
          //  ...... then we update the lock_type of the actual transaction to "WAITING + EXCLUSIVE"
          if (lock_count[0] > 1){
            resource->setLockStatus(lockStatus::WAITING);
            retval= lockStatus::WAITING;
          }else
            retval= lockStatus::GRANTED;
          
          // ..... if there are no "SHARED" lock on the same resource
          // ......... then just change the lock type of the actual transaction to "EXCLUSIVE"
          resource->setLockType(lockType::EXCLUSIVE);
        }else
        {
          // The requested lock_type is SHARED and the actual lock is EXCLUSIVE
          // ...... If there are any "SHARED+GRANTED" lock on the same resource 
          // ......... then we update the lock_type of the actual transaction to "SHARED + EXCLUSIVE"
          // NOTE:  ACTUALLY This may can cause STARVATION PROBLEM 
          //............ To solve this change the below if statement to
          // if (lock_count[0] > 0 && lock_count[1]==0)
          // ... So, we are saying that while inserting a transaction with SHARED locktype if there exists an EXCLUSIVE WAITING lock before 
          // .... it then wait this SHARED lock until the EXCLUSIVE becomes GRANTED or else the EXCLUSIVE lock may never become GRANTED
          if (lock_count[0] > 0){
            resource->setLockStatus(lockStatus::GRANTED);
            retval= lockStatus::GRANTED;
          }else retval= lockStatus::WAITING;
          // ..... if there are no "SHARED" locks on the same resource
          // .......... then just change the lock_type of the actual transaction to "SHARED"
          resource->setLockType(lockType::SHARED);
          // So, if we update the actual transaction to SHARED than other WAITING+SHARED should also becom GRANTED
          // .... but again it may lead to starvation so we update only those transation to SHARED+GRANTED which are before
          // .... any EXCLUSIVE locks
          lr_iter= lock_table[resource_name]->begin();
          for(; lr_iter != lock_table[resource_name]->end(); lr_iter++){
            resource= *(lr_iter);
            if (resource->getLockType() == lockType::SHARED && resource->getStatus() == lockStatus::WAITING){
              resource->setLockStatus(lockStatus::GRANTED);
              retval= lockStatus::GRANTED;
            }
            else{
              if (resource->getLockType()==lockType::EXCLUSIVE)
                break;
            }
          }
        }
        
      }
    }else{
      // If the transaction is not duplicate
      list <lockable_resource*> * resourceP= lock_table[resource_name];
      if (lock_type == lockType::SHARED){

        // If the requested lock is SHARED 
        // ... CASE1: if there are already "SHARED+GRANTED" locks on the same resource then 
        // ..........  add this new transaction as "SHARED+GRANTED"
        // ... CASE2: if there are no "SHARED+GRANTED" locks on the resource then
        // ..........  add this new transaction as "SHARED+WAITING"
        if (lock_count[0] > 0){
          resourceP->emplace_back(new lockable_resource(txn_id,lock_type,lockStatus::GRANTED));
          retval= lockStatus::GRANTED;
        }else{
          resourceP->emplace_back(new lockable_resource(txn_id,lock_type,lockStatus::WAITING));
          retval= lockStatus::WAITING;
        }
      }else
      {
        // Since whenever a new transaction is asking an EXCLUSIVE lock to a resource which is already locked
        // .... then irrespective of GRANTED lock_type on the resource 
        // ..... we add the transaction as "EXCLUSIVE+WAITING"
        resourceP->emplace_back(new lockable_resource(txn_id,lock_type,lockStatus::WAITING));
        retval= lockStatus::WAITING;
      }
      
    }

  }

  return(retval);

}


bool unlock(std::string resource_name,std::uint32_t txn_id)
{

    // Here we are counting how many shared (GRANTED) and exclusive (both) locks are there in the resource 
  int lock_count[2]= {0,0};
  // Here we are unlocking the resource_name held by transaction txn_id
  // First we are finding wheter the resource exists in the lock_table or not
  unordered_map<string, list<lockable_resource *>*>::const_iterator lock_table_iterator = lock_table.find(resource_name);

  // If the resource is not found then raise ERROR and return
  if (lock_table_iterator==lock_table.end())
  {
    cerr<<"ERROR: no such resource is found."<<endl;
    return false;
  }
  // It tells we whether the unlocked resource has lock SHARED or EXLCUSIVE
  // true for SHARED
  // false for EXCLUSIVE
  bool flag= NULL;
  // If resource is found then get the correspoding list of txn that has the resource or are waiting for the resource
  list<lockable_resource *>::iterator l_itr= lock_table[resource_name]->begin();
  list<lockable_resource *>* resource_list= lock_table[resource_name];

  for (; l_itr != lock_table[resource_name]->end(); l_itr++)
  {
    lockable_resource resource= *(*l_itr);
    
    // When the transaction which is unlocking the resource is found
    if (txn_id == resource.getTxnId()){
      // remove that record from the lock_table of from the list
      resource_list->erase(l_itr);
      if(resource.getLockType() == lockType::EXCLUSIVE)
        flag= false;
      else
        flag= true;      
      break;
    }
  }

  count_lock(resource_name,lock_count);
  // Here, in this loop we are finding the first transaction whose lock_type on the resource is WAITING
  // .... then we grant the lock to that transaction as set the lock_status to GRANTED
  
  // In this loop we are finding the next WAITING lock and GRANTING the lock
  // CASE 1: If the lock released was "EXCLUSIVE+GRANTED" and the next "WAITING" locks is
  // ..... CASE 1.a: SHARED -> then make all the "SHARED+WAITING" to GRANTED until an "EXCLUSIVE" lock is found (to handle starvation problem )
  // ..... CASE 1.b: EXCLUSIVE -> then make the next "EXCLUSIVE" lock to "GRANTED".
  // CASE 2: If the lock released was "EXCLUSIVE+WAITING" then just remove it.
  // CASE 3: If the lock released was "SHARED+GRANTED" and the next "WAITING" locks will be EXCLUSIVE so make it GRANTED
  // CASE 4: If the lock released was "SHARED+WAITING" then just remove it.
  for (l_itr= lock_table[resource_name]->begin(); l_itr != lock_table[resource_name]->end(); l_itr++)
  {
    lockable_resource resource= *(*l_itr);
    if (resource.getStatus() == lockStatus::WAITING && resource.getLockType() == lockType::EXCLUSIVE && lock_count[0] == 0){
      if (flag==true){
        (*l_itr)->setLockStatus(lockStatus::GRANTED);
        break;
      }else flag= true;
    }else if (resource.getStatus() == lockStatus::WAITING && resource.getLockType() == lockType::SHARED && lock_count[1] == 0)
      if (flag==false)
        (*l_itr)->setLockStatus(lockStatus::GRANTED);
      else flag=true;
  }
  cout<<"Successfully unlocked the resource "<<resource_name<<endl;
  return true;

}

void print_locktable(){
  cout <<"\nprinting locktable"<<endl;
  unordered_map<std::string, list<lockable_resource *>*>::iterator lock_table_itr;
  for (lock_table_itr=lock_table.begin(); lock_table_itr!=lock_table.end(); lock_table_itr++)
  {
    cout <<"RESOURCE: "<<lock_table_itr->first <<endl;
    list<lockable_resource *>::iterator lt_itr= lock_table[lock_table_itr->first]->begin();
    for(;lt_itr != lock_table[lock_table_itr->first]->end(); lt_itr++)
    {
      lockable_resource resource= *(*(lt_itr));
      cout<<"\t transaction_Id: "<<resource.getTxnId()<<"\t";
      if (resource.getLockType() == lockType::EXCLUSIVE)
        cout<<"LockType: Exclusive";
      else
        cout<<"LockType: Shared";
      cout <<"\t";
      if(resource.getStatus()==lockStatus::WAITING)
        cout<<"Status: Waiting";
      else
        cout <<"Status: Granted";
      cout<<endl;
      

    }
  }
}

/* Evaluation Comments 
1. Basic Compilation (g++ -std=c++11 lockmanager2.cpp):
   Code compiles without errors. 

2. Correctness of output:
    Correct output for inputs I tested with. 
    During unlock, it is not enough to erase the resource from the lock_table. The table is a list of pointers. 
    The pointer should also be deallocated. 
   
2. Comments and programming style: 
    I am glad you added comments to your code. They were useful. 
    The menu driven approach is excellent.  Printing the lock table was also a fantastic idea!
    It is not a good practice to use basic types without a size specifier. You have used int(4 bytes)
    in many places where an uint8_t (one byte) would have been enough. 

3. Exception Handling:
   1. Unlock last lock on item (more items in map) -  handled. 
   2. Ulock last item in map - handled. 
   3. Calling unlock on empty table - handled. 
   4. Check for memory allocation errors - not done. 
   The program gets into an infinite loop of printing the lock table if I enter a string instead of a number for the menu option. 

Score: 9.5/10.
*/