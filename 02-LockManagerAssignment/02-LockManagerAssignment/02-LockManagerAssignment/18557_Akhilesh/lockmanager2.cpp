/* 

Goal : Build a lock manager. 
The lock manager should support the following capabilities:
1. Lock a resource in either shared or exclusive mode. 
2. Unlock a resource held by a transaction. 
A resource will be identified by a 10 character string. 

compilation : g++ lockmanager2.cpp -std=c++11
run: ./a.out
Name: G Sai Akhilesh
regno: 18557 
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
      //std::cout << "SAIRAM " << lock_type_ << std::endl;
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
  SHARED, //0
  EXCLUSIVE //1
};
enum lockStatus
{
  GRANTED, //0
  WAITING  //1
};

// Resource
// type of lock
// txnid

bool lock(std::string resource_name, 
          std::uint32_t txn_id,
          uint8_t lock_type);

//uint8_t unlock(std::string resource_name,std::uint32_t txn_id);
bool unlock(std::string resource_name,std::uint32_t txn_id);

void display_locktable();

unordered_map<std::string, list<lockable_resource *>*> lock_table;

int main()
{
  uint8_t ret = lock("AAA", 1234, lockType::SHARED);
  display_locktable();
  ret = lock("AAA", 1234, lockType::EXCLUSIVE);
  display_locktable();
  ret = lock("BBB", 5678, lockType::SHARED);
  display_locktable();
  ret = lock("BBB", 1008, lockType::SHARED);
  display_locktable();
  ret = lock("BBB", 108, lockType::EXCLUSIVE);
  display_locktable();
  ret = lock("BBB",10,lockType::SHARED);
  display_locktable();
  ret = lock("BBB",9,lockType::SHARED);
  display_locktable();
  ret = lock("BBB",8,lockType::EXCLUSIVE);
  display_locktable();
  ret = lock("BBB",3,lockType::SHARED);
  display_locktable();
  ret = lock("BBB",33,lockType::SHARED);
  display_locktable();

  bool r= unlock("BBB",5678);
  display_locktable();
  r= unlock("BBB",1008);
  display_locktable();
  r= unlock("BBB",108);
  display_locktable();
  r= unlock("BBB",10);
  display_locktable();
  r= unlock("BBB",9);
  display_locktable();
  r= unlock("BBB",8);
  display_locktable();  
  return 0;
}

bool lock(std::string resource_name,
          std::uint32_t txn_id,
          uint8_t lock_type)
{
  //cout<<"lock("<<resource_name<<","<<txn_id<<","<<unsigned(lock_type)<<")";
  cout<<"LOCK Resource = "<<resource_name<<" TransactionID = "<<txn_id<<" LOCK TYPE : "<<unsigned(lock_type)<<"\t{0: SHARED 1: EXCLUSIVE}"<<endl;
  uint8_t retval = lockStatus::WAITING;
  // Check if lock exists. 
  //   No: Add to map, create new list and add lockable_resource to list
  if (lock_table.find(resource_name) == lock_table.end())
  {
    // lock table does not exist. 
    //  => locable resource has to be created. 
    //     list of lockable resources has to be created. 
    //     lock table should be updated wit resource. 
    //cout<<"Resource does not exist in the lock table"<<endl;
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
  else //resource is present in the lock table
  {
    //cout<<"The resource is already in the lock table"<<endl;
    auto it = lock_table.find(resource_name);
    std::list<lockable_resource*> lst= *(it->second); //we get the list of lockable resources for the lockable resources
    list<lockable_resource*>::iterator i= lst.begin();
    //std::cout<<"Inside the list"<<std::endl;
    if((*i)->getLockType() == lockType::EXCLUSIVE) //resource is locked by a EL
    {
      //cout << "Already Resource is Exclusively locked!" << endl;
      //cout<<(*i)->getTxnId()<<endl;
      //cout<<txn_id<<endl;
      if((*i)->getTxnId() == txn_id) //on same txn id lock has happened
      {
        //cout<<"on same txn id lock has happened"<<endl;
        if((*i)->getLockType() == lock_type) //duplicate request
        {
          cout<<"DUPLICATE REQUEST Lock"<<endl;
        }
        else
        {
          retval= lockStatus::WAITING;
          lockable_resource *lr = new lockable_resource(txn_id,lock_type,lockStatus::WAITING);
          lst.emplace_back(lr); //add this lockable resource to the list
          //lock_table[resource_name]= lst; //update this list in the lock table
          *(it->second)= lst; //update this list in the lock table
          //cout<<"ADDED"<<txn_id<<endl;
        }
      }
      else //EL on diff t_id
      {
        //cout<<"Lock is on diff tid"<<endl;
        for(list<lockable_resource*>::iterator i1= lst.begin(); i1 != lst.end(); i1++) //checking if tid is in the list
        {
          //cout<<"Inside loop"<<endl;
          //display_locktable();
          //cout<<"SAIRAM: " << (*i1)->getTxnId()<<endl;
          //cout<<"In the table : "<<(*i)->getTxnId();
          //cout<<"\tRequested tr: "<<txn_id<<endl;
          if((*i1)->getTxnId() == txn_id) //t_id is in the list
          {
            //cout<<"Found the transaction"<<endl;
            display_locktable();
            if((*i1)->getLockType() == lock_type) //request is for same lock type
            {
              cout<<"DUPLICATE REQUEST SAIRAM"<<endl;
              //cout<<(*i1)->getTxnId()<<endl;
              //break;
              return false;
            }
            else //rewuest is for different lock type
            {
              lockable_resource *lr = new lockable_resource(txn_id,lock_type,lockStatus::WAITING); //create a new lockable resource
              lst.emplace_back(lr); //add this lockable resource to the list
              //lock_table[resource_name]= lst; //update this list in the lock table
              *(it->second)= lst; //update this list in the lock table
              //cout<<"New lockable resource added"<<endl;
            }
          }
        } //end of loop
        //cout<<"t_id is not in the list"<<endl;
        lockable_resource *lr = new lockable_resource(txn_id,lock_type,lockStatus::WAITING);
        lst.emplace_back(lr);
        //lock_table[resource_name]= lst; //update this list in the lock table
        *(it->second)= lst; //update this list in the lock table
        //cout<<"transaction added to the list"<<endl;
        //break;
      }
    }
    else  //resource is locked by a SL
    {
      //std::cout << "Already Resource is SHARED locked!" << std::endl;
      if((*i)->getTxnId() == txn_id) //on same txn id lock has happened
      {
        if((*i)->getLockType() == lock_type) //duplicate request
        {
          //cout<<"same type of lock is already granted by the same txnID DUPLICATE REQUEST"<<endl;
        }
        else //requested lock is EXCLUSIVE
        {
          lockable_resource *lr = new lockable_resource(txn_id,lock_type,lockStatus::WAITING); //create a new lockable resource
          lst.emplace_back(lr); //add this lockable resource
          //lock_table[resource_name]= lst; //update this list in the lock table
          *(it->second)= lst; //update this list in the lock table
        }
      }
      else //SL on diff t_id
      {
        for(list<lockable_resource*>::iterator i1= lst.begin(); i1 != lst.end(); i1++) //checking if tid is in the list
        {
          if((*i1)->getTxnId() == txn_id) //t_id is in the list
          {
            if((*i1)->getLockType() == lock_type) //request is for same lock type
            {
              //cout<<"DUPLICATE REQUEST for SL"<<endl;
              break;
            }
            else //request is for different lock type
            {
              lockable_resource *lr = new lockable_resource(txn_id,lock_type,lockStatus::WAITING); //create a lockable resource
              lst.emplace_back(lr); //add this to the list
              //lock_table[resource_name]= lst; //update this list in the lock table
              *(it->second)= lst; //update this list in the lock table
              //break;
            }
          }
          else //t_id is not in the list
          {
         
            if(lock_type == lockType::SHARED) //requested lock is shared
            {
              
              //we'll check if there are any EXCLUSIVE Locks requested and waiting in the list
              int flag= 1;
              for(list<lockable_resource*>::iterator i2= lst.begin(); i2 != lst.end(); i2++) //checking if tid is in the list
              {
                if((*i2)->getLockType() == lockType::EXCLUSIVE)
                {
                  flag= 0;
                  break;
                }
              }
              if(flag == 1)
              {
                retval= lockStatus::GRANTED;
                lockable_resource *lr = new lockable_resource(txn_id,lock_type,lockStatus::GRANTED); //create a lockable resource
                lst.emplace_back(lr); //add to the list
                *(it->second)= lst; //update this list in the lock table
              }
              else
              {
                retval= lockStatus::WAITING;
                lockable_resource *lr = new lockable_resource(txn_id,lock_type,lockStatus::WAITING); //create a lockable resource
                lst.emplace_back(lr); //add to the list
                *(it->second)= lst; //update this list in the lock table
              }
            }
            else //requested lock is EXCLUSIVE
            {
              lockable_resource *lr = new lockable_resource(txn_id,lock_type,lockStatus::WAITING); //create a lockable resource
              lst.emplace_back(lr); //add to the list
              *(it->second)= lst; //update this list in the lock table
            }
            break;
          }
        } //end of loop
      }
    }
  }
  return(retval);
} //end of lock method.

bool unlock(std::string resource_name,std::uint32_t txn_id)
{
  cout<<"\t UNLOCK Resource : "<<resource_name<<" Transaction : "<<txn_id<<endl;
  uint8_t retval = false;
  if (lock_table.find(resource_name) == lock_table.end())
  {
    cout<<"Resource not found in the lock table. Invalid unlock request"<<endl; //1
    return false;
  }
  else
  {
    //cout<<"The resource is available in the lock table"<<endl;
    //Now we need to check for the transaction id in the list
    auto it = lock_table.find(resource_name);
    std::list<lockable_resource*> lst= *(it->second);
    list<lockable_resource*>::iterator i= lst.begin();
    for(;i != lst.end();i++)
    {
      if((*i)->getTxnId() == txn_id) //transaction is found in the list
      {
        //cout<<"Transaction found in the list"<<endl;
        if((*i)->getStatus() == lockStatus::WAITING)
        {
          //cout<<"The resource is not yet locked. How can we unlock??"<<endl; //3
          //break;
          return false;
        }
        //remove the lockable resource from the list
        //cout<<"Let's remove the transaction from the list"<<endl;
        lst.remove((*i));
        //cout<<"Removed object"<<endl;
        (*(lst.begin()))->setLockStatus(lockStatus::GRANTED);
        //cout<<"Updated the acquired lock for the first resource"<<endl;
        //now check if there are some shared locks in the list
        if((*(lst.begin()))->getLockType() == lockType::EXCLUSIVE)
        {
          *(it->second)= lst;
          //cout<<"List updated successfully"<<endl;
          return true;
        }
        else //the first lockable resource has acquired a shared lock
        {
          for(list<lockable_resource*>::iterator i7= lst.begin(); i7 != lst.end(); i7++)
          {
            if((*i7)->getLockType() == lockType::EXCLUSIVE)
            {
              break;
            }
            else
            {
              (*i7)->setLockStatus(0);
            }
          } //end of loop
        }
        *(it->second)= lst;
        //cout<<"List updated successfully"<<endl;
        return true;
      }
      cout<<"This partcular transaction doesn't exist in the lock table. Invalid UNLOCK Request"<<endl; //2
      return false;
    } //end of loop
  }
} //end of unlock

void display_locktable()
{
  cout<<"###################"<<endl;
  cout<<"#   LOCK TABLE    #"<<endl;
  cout<<"###################"<<endl;
  
  auto at= lock_table.begin();
  for(;at != lock_table.end();at++)
  {
    cout<<"RESOURCE : "<<at->first<<endl;
    std::list<lockable_resource*> lt= *(at->second);
    list<lockable_resource*>::iterator j= lt.begin();
    for(;j != lt.end();j++)
    {
      cout<<"\tTxnId : "<<(*j)->getTxnId()<<" LOCK TYPE : "<<unsigned((*j)->getLockType())<<" LOCK STATUS : "<<unsigned((*j)->getStatus())<<endl;
    }
  } //end of loop
  cout<<"-------------------------------------------------------------------------------------"<<endl;
  cout<<"| LOCK STATUS --> 0 = GRANTED; 1 = WAITING; LOCK TYPE --> 0 = SHARED; 1 = EXCLUSIVE |"<<endl;
  cout<<"-------------------------------------------------------------------------------------"<<endl;
  cout<<"###################"<<endl;
  cout<<"#END OF LOCK TABLE#"<<endl;
  cout<<"###################"<<endl;
}

/* Evaluation Comments 
1. Basic Compilation (g++ -std=c++11 lockmanager2.cpp):
   Code compiles without errors. 

2. Correctness of output:
    1. Line 323 - An unlock request should succeed even if the lock request is in waiting state. This can happen
       when a txn is being rolled back as part of deadlock recovery. 
    2. Line 333 - (*(lst.begin()))->setLockStatus(lockStatus::GRANTED);
       You are assuming that there are other requesters waiting for the same lock when unlocking. This line
       causes a segmentation fault if you have just unlocked the last lock on a resource. 
    
   
2. Comments and programming style: 
    1. You have included some useful comments. 
    2. The indentation style is consistent. Good!
    3. You could have removed the commented out code lines before submission. In the future,you can include
       trace only or debug only code within #ifdef blocks. 
    4. The remove() method does not delete an item from an STL container. You have to use erase(). 

3. Exception Handling:
   1. Unlock last lock on item (more items in map) - not handled. 
   2. Ulock last item in map - not handled. 
   3. Calling unlock on empty table - handled. 
   4. Check for memory allocation errors - not done. 

Score: 7/10
*/