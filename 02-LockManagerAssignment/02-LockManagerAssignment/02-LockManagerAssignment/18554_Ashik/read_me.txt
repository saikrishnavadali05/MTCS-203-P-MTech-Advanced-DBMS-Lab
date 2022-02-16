------------------------------------------------------------- NOTE ---------------------------------------------------------------------
There is no conflict if the duplicate requests is of same lock_type even if the lock is "GRANTED or WAITING" for the old transaction.
	i.e: Transaction 'A' has locked resource 'R' as "SHARED" and again asking for "SHARED" lock.

But if the duplicate request has different lock_type than the actual/old transaction then.....
Case 1: if the transaction 'A' has already requested the resource 'R' as "SHARED" and the "duplicate" request 'B' is "EXCLUSIVE"
	then change the "lock_type" of the old txn 'A' to "EXCLUSIVE" -> if the "status" of the old txn 'A' is "WAITING"
	if the "status" of old transaction is "GRANTED" then 
	CASE 1.a : if only one "SHARED" lock exists on resource 'R' -> change the lock_type of the actual txn 'A' to "EXCLUSIVE + GRANTED"
	CASE 1.b : if there are many "SHARED" lock on resource 'R'-> change the lock_type of the actual txn 'A' to "EXCLUSIVE + WAITING".

Case 2: if the transaction 'A' has already requested the resource 'R' as "EXCLUSIVE" and the "duplicate" 'B' request us "SHARED"
	then change the "lock_type" of the old_txn 'A' to "SHARED" -> if the "status" of the old txn 'A' is "WAITING"
	if the "status" of the old txn 'A' is "GRANTED" 
		then change the "lock_type" of the old_txn 'A' to "SHARED","GRANTED" 
			then change the "lock_status" of the txn who are waiting ,if any, for the SHARED key to "GRANTED"

	
-------------------------------------------------------------------------------------------------------------------------------------------
IN INSERT:
CASE 1: If no lock on the resource exists then lock the resource as GRENTED and add the entry in the lock_table

CASE 2: If the transcation asking the resource is duplicate transaction
	CASE 2.a: If the requested lock is EXCLUSIVE and the actual lock is SHARED
		....-> if there are any "SHARED+GRANTED" locks on the same resource
		....  	then we update the lock_type of the actual transaction to "WAITING + EXCLUSIVE"
		....-> if there are no "SHARED" lock on the same resource
		.....	then just change the lock type of the actual transaction to "EXCLUSIVE"
	CASE 2.b: If the requested lock_type is SHARED and the actual lock is EXCLUSIVE
          	....-> If there are any "SHARED+GRANTED" lock on the same resource 
          	....	then we update the lock_type of the actual transaction to "SHARED + EXCLUSIVE"
		NOTE:	ACTUALLY This may can cause STARVATION PROBLEM 
        		To solve this change the if statement of line 258: to
			if (lock_count[0] > 0 && lock_count[1]==0)
			So, we are saying that while inserting a transaction with SHARED locktype if there exists an EXCLUSIVE WAITING lock before 
			it then wait this SHARED lock until the EXCLUSIVE becomes GRANTED or else the EXCLUSIVE lock may never become GRANTED
		....-> If there are no "SHARED" locks on the same resource
          	....	then just change the lock_type of the actual transaction to "SHARED"
		....-> So, if we update the actual transaction to SHARED than other WAITING+SHARED should also becom GRANTED
		....	but again it may lead to starvation so we update only those transation to SHARED+GRANTED which are before
		....	any EXCLUSIVE locks

CASE 3: If the transactions are not duplicate
	 If the requested lock is SHARED 
         ... CASE 3.a: if there are already "SHARED+GRANTED" locks on the same resource then 
         ..........  add this new transaction as "SHARED+GRANTED"
         ... CASE 3.b: if there are no "SHARED+GRANTED" locks on the resource then
         ..........  add this new transaction as "SHARED+WAITING"
	 ... CASE 3.c:
         Since whenever a new transaction is asking an EXCLUSIVE lock to a resource which is already locked
         .... then irrespective of GRANTED lock_type on the resource 
         ..... we add the transaction as "EXCLUSIVE+WAITING"


IN UNLOCK:
CASE 1: First we are finding whether the resource exists in the lock_table or not
	If the resource is not found then raise ERROR and exit.
CASE 2: We find the transaction to be unlocked and remove it from the lock_table.
	We then search for the next WAITING lock and GRANTING the lock.
	CASE 2.a: If the lock released was "EXCLUSIVE+GRANTED" and if the next "WAITING" locks is
	....	CASE 2.a.I : SHARED -> then make all the "SHARED+WAITING" to GRANTED until an "EXCLUSIVE" lock is found (to handle starvation problem )
	.....	CASE 2.a.II: EXCLUSIVE -> then make the next "EXCLUSIVE" lock to "GRANTED".
	CASE 2.b: If the lock released was "EXCLUSIVE+WAITING" then just remove it.
	CASE 2.c: If the lock released was "SHARED+GRANTED" and the next "WAITING" locks will be EXCLUSIVE so make it GRANTED
	CASE 2.d: If the lock released was "SHARED+WAITING" then just remove it.




