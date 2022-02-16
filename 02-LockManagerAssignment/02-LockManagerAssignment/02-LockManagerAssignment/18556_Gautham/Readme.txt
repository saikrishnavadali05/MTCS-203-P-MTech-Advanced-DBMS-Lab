Sairam this folder consists of the following files:
	1. lockmanager2.cpp.
	2. lockmanager.
	3. Readme.txt.
	4. Headers related to the lockmanager.
In lockmanager2.cpp we have the main code and the related headers are present in the same folder. 

TO RUN THE FILE FIRST USE MAKE FOR CREATING EXECUTABLE FILE AND enter ./lockmanager to run the program
 		command  : ./lockmanager

The following situations are simulated as examples for the lockmanager program :
 		
  we have two resources AAA and BBB on which different processes ex: 1234 wants locks either shared or exclusive

  ret = lock("AAA", 1234, lockType::SHARED);    
  ret1=unlock("AAA",1234);
  t1 = lock("AAA", 4567, lockType::SHARED);
  t11=unlock("AAA",4567);
  t2 = lock("AAA", 123456, lockType::EXCLUSIVE);
  t22=unlock("AAA",123456);
  t3=lock("BBB",1234567,lockType::SHARED);
  t33=unlock("BBB",123456);

we get the result as following:

Successfully locked the resource AAA for transaction for 1234
Successfully unlocked the resource.
Successfully locked the resource AAA by 1234. t1
Successfully unlocked the resource AAA from 1234.
Successfully could not lock the resource. t2
Successfully locked the resource.t3
Successfully unlocked the resource AAA.
Successfully unlocked the resource AAA by 456.
Successfully unlocked the resource BBB by 123456.
