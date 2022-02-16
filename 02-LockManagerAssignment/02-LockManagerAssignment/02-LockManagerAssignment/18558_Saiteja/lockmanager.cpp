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
#include <string>
#define stringify( name ) # name
using namespace std; 

// Abstraction of a resource that can be locked. 
// A resource is locked in a 'mode' by a 'transaction'. 
// The lock request may be granted or put on wait based
// on a lock compatibility matrix. 
//FOLLOWS FIRST COME FIRST SERVED PRINCIPLE 
enum lockType
{
  SHARED,
  EXCLUSIVE
};
enum lockStatus
{
  GRANTED,                //the txn has got the lock on the resource
  WAITING,                //the txn is waiting for the lock on the resource    
  UNLOCKED,               //the txn's lock  on the resource has been removed     
  NO_LOCK,                //}these are used as return values for       
  NO_RESOURCE             //}   unlock() function
};

const char* lockType_values[] =
{
  stringify(SHARED),
  stringify(EXCLUSIVE)

};
const char* lockStatus_values[] = 
  {
  stringify(GRANTED),
  stringify(WAITING),
  stringify(UNLOCKED),
  stringify(NO_LOCK),
  stringify(NO_RESOURCE)
  };
void print_lockType(lockType lock){
  cout<< lockType_values[lock] <<" ";
      
}
void print_lockStatus(lockStatus lock){
  cout<< lockStatus_values[lock] <<" ";
      
}

// The datatype for a request made by a txn for a lock

class lock_request{
  private:
    uint32_t txn_id_;
    uint8_t lock_type_; // SHARED, EXCLUSIVE
      uint8_t lock_status_; // GRANTED, WAITING
  public:
    lock_request(
      uint32_t txn_id,
      uint8_t lock_type,
      uint8_t lock_status):
      txn_id_(txn_id),
      lock_type_(lock_type),
      lock_status_(lock_status)
    {}
    uint8_t getLockType(){ 
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
   
    void print(){
      cout<<"Txn_id:  "<<getTxnId()<<"\nLockType  ";
      print_lockType((lockType)((int)(getLockType())));
      cout<<"\nLockStatus  ";
      print_lockStatus((lockStatus)((int)getStatus()));
      cout<<endl;
    }

};                              
// Datatype for The resource on which a lock is requested
class resource{
  string name_;
  uint8_t lock_type_;
  int Exc_count_;                                       // #Exclusive  locks(waiting or granted) on the resource
  list<lock_request*> requests;                         //list of lock requests
  list<lock_request*>:: iterator last_granted;          // the latest txn which has been granted a lock 

  public:
    resource(
      string name,
      uint8_t lock_type,
      lock_request* lr):
      name_(name),
      Exc_count_(lock_type == lockType::SHARED? 0:1)
      {
        insert_request(lr);
        last_granted = requests.begin();  
      }
    void insert_request(lock_request* request)  {
      requests.emplace_back(request);
    }

    uint8_t get_locktype(){
      return lock_type_;
    }
    
    uint8_t no_of_requests(){
      return requests.size();
    }

    void print_requests(){
      cout<<"\n--------------------Resource: "<<name_<<"---------------\n\n";
      list<lock_request*>:: iterator iter;
      for (iter = requests.begin();iter!=requests.end();++iter){
        (*iter)->print();
        cout<<endl;
        }
    }
    uint8_t request(uint32_t txn_id,uint8_t lock_type){
      list<lock_request*>::iterator iter; 
      uint8_t retval; 
      lock_request* lr;
        //iter = requests->begin();
        //lock_request* lc_rs = *iter;
        cout<<"# requests: ";
        cout<<requests.size()<<endl;

        if( requests.size() == 0 || (Exc_count_ == 0 && lock_type == lockType::SHARED)){
    //              cout<<"both are shared"<<endl;
                  lr = new lock_request(
                  txn_id,
                  lock_type,
                  lockStatus::GRANTED);

                  retval = lockStatus::GRANTED;
                  
          }
        else { 
                  lr = new lock_request(
                  txn_id,
                  lock_type,
                  lockStatus::WAITING);
                  retval = lockStatus::WAITING;

        }
        if(lock_type == lockType::EXCLUSIVE){
          Exc_count_++;
        }
        insert_request(lr);
        if( Exc_count_ == 0 ){
          last_granted = requests.end();
          last_granted--;
        }
        return(retval);



    }


    uint8_t find(uint32_t txn_id){
      list<lock_request*>::iterator iter,iter2;
      uint8_t retval; 

      for(iter = requests.begin();iter!=requests.end();iter++){
              // cout<<"Txn_id ";
              // cout<<(*iter)->getTxnId();

        if((*iter)->getTxnId() == txn_id){
          break;
        }
      
      }


      if(iter == requests.end()){
         retval = lockStatus::NO_LOCK;
 //        cout<<"Lock NOT present"<<endl;
        
      }
      else{
        if((*iter)->getLockType() == lockType::EXCLUSIVE){
            Exc_count_--; 
     //       cout<<"Exclusive\n" ;    
        }
        cout<<"LAST GRANTED: "<<(*last_granted)->getTxnId()<<endl;
        if(iter == requests.begin()){
          iter2 = iter;
          iter2++;
          requests.erase(iter);
          cout<<"\nErased "<<(*iter)->getTxnId()<<endl<<"\n";
          
          if((*iter2)->getStatus() != lockStatus::GRANTED){
     //       cout<<"iter2 NOT Granted\n";
            (*iter2)->setLockStatus(lockStatus::GRANTED);
            last_granted = iter2;
            if((*iter2)->getLockType() == lockType::SHARED){
              iter2 = last_granted;
            //            cout<<(*last_granted)->getTxnId()<<"last_granted"<<endl;

              if(last_granted != requests.end()){
                iter2++;
              for(;iter2 != requests.end() && (*iter2)->getLockType() != lockType::EXCLUSIVE;iter2++){
                (*iter2)->setLockStatus(lockStatus::GRANTED);
          //      cout<<(*iter2)->getTxnId()<<"  Lock GraNTED\n"<<endl;
              }
              }
              last_granted = --iter2;
            }  
        }
        }

        else if((*iter)->getLockType() == lockType::EXCLUSIVE){
    //      cout<<"Entered EXC_count = 0"<<endl<<endl; 
          requests.erase(iter);
          cout<<"\nErased "<<(*iter)->getTxnId()<<endl<<"\n";
         // cout<<(*last_granted)->getTxnId()<<"last_granted"<<endl;
          iter2 = last_granted;
          if(iter2 != requests.end()){
          for(++iter2;iter2 != requests.end();iter2++){
            (*iter2)->setLockStatus(lockStatus::GRANTED);
           // cout<<(*iter2)->getTxnId()<<"  Lock GraNTED\n"<<endl;
          }
          last_granted = --iter2;
          }
        }

        else{
          requests.erase(iter);
        }
        //cout<<"Lock  present"<<endl;
        retval = lockStatus::UNLOCKED;
        

      } 
      this->print_requests(); 
      return retval;


    }

};



// Resource
// type of lock
// txnid

uint8_t lock(std::string resource_name, 
          std::uint32_t txn_id,
          uint8_t lock_type);

uint8_t unlock(std::string resource_name,
              std::uint32_t txn_id);

unordered_map<std::string, resource*> lock_table;






uint8_t lock(std::string resource_name,
          std::uint32_t txn_id,
          uint8_t lock_type)
{
  uint8_t retval = lockStatus::WAITING;
  // Check if lock exists. 
  //   No: Add to map, create new list and add lock_request to list
  resource* rsc;
  if (lock_table.find(resource_name) == lock_table.end())
  {
    // lock table does not exist. 
    //  => locable resource has to be created. 
    //     list of lockable resources has to be created. 
    //     lock table should be updated wit resource.
 
  
    lock_request *lr = new lock_request(
      txn_id,
      lock_type,
      lockStatus::GRANTED
    );
    
    retval = lockStatus::GRANTED;
    //list<lock_request*> *lst = new list<lock_request*>;
    rsc = new resource(resource_name,lock_type,lr);
    lock_table[resource_name] = rsc;

    cout<<"Inititally"<<endl;     
  }
  else{

    cout<<"Resource already Pressent\n"<<endl;
    auto iter = lock_table.find(resource_name);
    //cout << iter->first<<endl;
    rsc = iter->second;  
    /*cout<<"list size ";
    cout<<lst->size()<<endl;
    auto iter2 = lst->begin();
    cout << (*iter2)->lock_request::getTxnId()<<endl;
    list<lock_request*> lst1 = *(iter->second);
    
    for ( iter3= lst1.begin();iter3 != lst1.end();iter3++){
      cout<<*iter3<<endl;
    }
    */
    retval = (*rsc).request(txn_id,lock_type);   
  }
  
  rsc->print_requests();

  return(retval);

}

uint8_t unlock(std::string resource_name,std::uint32_t txn_id){
  if(lock_table.find(resource_name) != lock_table.end()){
    auto iter = lock_table.find(resource_name);
    resource* rsc = iter->second;
    cout<<"Resource present"<<endl;
    return(rsc->find(txn_id));


  }
  else{
    cout<<"Resource absent"<<endl;
    return(lockStatus :: NO_RESOURCE); 
  }


}

int main()
{
  string resource_name;
  int txn_id;
  int lock_type;
  int option;
/*
  uint8_t ret = lock("AAA", 1234, lockType::SHARED);
  cout<<"---------------------------------------------------------------------"<<endl;
  uint8_t ret2 = lock("AAA", 4567, lockType::SHARED);
  cout<<"----------------------------------------------------------------------"<<endl;
  uint8_t ret5 = lock("AAA", 4569, lockType::EXCLUSIVE);
  cout<<"----------------------------------------------------------------------"<<endl;
  uint8_t ret4 = lock("AAA", 4568, lockType::SHARED);
  cout<<"----------------------------------------------------------------------"<<endl;
  uint8_t ret7 = lock("AAA", 4520, lockType::EXCLUSIVE);
  cout<<"----------------------------------------------------------------------"<<endl;
  uint8_t ret3 = unlock("AAA",1234);
  cout<<"----------------------------------------------------------------------"<<endl;
  uint8_t ret6 = unlock("AAA",4569    );
  cout<<"----------------------------------------------------------------------"<<endl;
  */
  do {
    cout<<"\n---------------------------------------------------------------------------------------\n\n";
    cout<<"You can do following operations on your lock manager\n\n"
          "1.Lock\n"
          "2.Unlock\n"
          "0:Exit\n\n";

    cout<<"Enter the required option:  ";
    cin>>option;
    switch (option)
    {
      case 1:
        cout<<"Enter the details\n"
               "1.Resource name: ";
        cin>>resource_name;
        cout<<"2.Transcation ID: ";
        cin>>txn_id;
        cout<<"3.Lock Type:(0 - for SHARED, 1 - for EXCLUSIVE):  ";
        cin>>lock_type;
        if(lock_type == 0){
          lock(resource_name,txn_id,lockType::SHARED);

        }
        else{
        lock(resource_name,txn_id,lockType::EXCLUSIVE);
        } 
        break;
      case 2:
        cout<<"Enter the details\n"
               "1.Resource name: ";
        cin>>resource_name;
        cout<<"2.Transcation ID: ";
        cin>>txn_id;
        unlock(resource_name,txn_id);
        break;   
    
      case 0:
        cout<<"Thank you for using our Service\n------------------------------------------------------------------\n";
        exit(0);
        break;
      default:
        cout<<"\nEnter a valid option\n";
        break;
    } 
  }while(true);
  

  return 0;

  }

/* Evaluation Comments 
1. Basic Compilation (g++ -std=c++11 lockmanager2.cpp):
   Code compiles without errors. 

2. Correctness of output:
    1. Line 345 - Unlocking the last lock on a resource causes a segmentation fault (SIGSEGV). 
    2. Your implementation of unlock is not even 10% complete. The method does not unlock!
    3. The menu driven option is a nice addition. 
    4. Lock upgrade - not implementd. If a resource is locked in shared mode by txn A and the same txn now requests for an
       exclusive lock, it should be granted if there are no other shared locks granted or exclusive lock waiters. 
   
2. Comments and programming style: 
    1. Indentation is neat and consistent. Good! 
    2. For basic types like int, it is a good practice to only use types with storage specifiers like uint8_t, uint16_t etc. 
       You have used int in some places - An int will occupy 4 bytes. A smaller size would have sufficed. 

3. Exception Handling:
   1. Unlock last lock on item (more items in map) - not handled. 
   2. Ulock last item in map - not handled. 
   3. Calling unlock on empty table - handled. 
   4. Check for memory allocation errors - not done. 

Score: 7/10
*/