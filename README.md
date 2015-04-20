# memPE
A persisted memcahced projecy

1.why I did this persisted memcached?
I just want to search a job in big data filed,I datermined to do sometings so that I can know how to process big data quickly.Memcached is a good project to study big data.

2.why is memcached?
I compared the projects between memcached and redis.I found a Chinese book which explained redis so detailed,but memcached with a little Chinese resource,So I think I should do a hard thing.Memcahed is hard beacuse it has a little Chinese documents.

3.the strategy about this project
  3.1 How to store the data in disk. 
	Selected libtdb as persisted database.Libtdb has been deployed in many embeded projects.You can google it for more infos.
  3.2 Which are the enabled memcached command in this project?
       you can use all of memcached command in this project.
  3.3 When the data will be stored to disk?
      Data will be stored disk when the data has been changed on memory( add/modify ).
  3.4 When the data will be deleted from disk?
      Data will be deleted from disk after it was moved from memory.
  3.5 When the data will be loaded from disk to memory?
      Data will be loaded to memory when memcached starting.
  
4.What are the new source files.
  mkvdb.c for database operating
  dbopt.c which wrapered the interface of libtdb.
  mkv_thread.c which is the worker thread.
  utility.c for utility function. 

4.How to build ?
  step 1. cd thirdlib/tdb-1.1.3 & ./configure & make
  step 2. cd .. & configure & make

5.Which is platform the project can work?
  In theory,the project can work on all of linux includes arm&x86.But I Just tested on ubuntu14(X64&x86).


connect me: songbin.sky#gmail.com or anticode#163.com and my skype id is 'songbin.sky'
