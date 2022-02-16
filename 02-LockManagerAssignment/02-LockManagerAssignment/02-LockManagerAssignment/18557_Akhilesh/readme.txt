Name: G Sai Akhilesh
RegNo: 18557

In Terminal
lockmanager2.cpp is the program which contains the source code.

To compile: g++ lockmanager2.cpp -std=c++11

To run : ./a.out

In main function, if we need to lock a resource, call the function lock(resource_name, transaction_id, locktype) to lock.

For unlocking, call function unlock(resourcename, transaction_id)

For displaying the lock table, display_locktable() function is called.

The main feature of this program is populating and removing entries from the lock table. The lock table consists of resource names and a pointer to a list of pointers to lockable resources.

The resources will be locked in a FCFS fashion in the list. This I have implemented to avoid the starvation of a lockable resource waiting for an exclusive lock. The locking happens in such a way that if a transaction requested for a shared lock on a resource if there are no exclusive locks wiating in the list, then the lock is granted else it is waiting state.
