![](https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white)
---

# Disease-Monitor
A system that accepts, processes, records and answers questions about cases of viruses.

## 1st task
Directory: oneProcess_task1


In this work I implemented a program that accepts, processes, records and answers questions about cases of viruses. Specifically I implemented a set of structures (hash tables, linked lists, binary trees) that allow the entry and queries in a large volume of records patientRecord type. Although the exercise data will come from files, eventually all Records will only be stored in main memory.

## 2nd task
Directory:  multiProcess_pipeComm_task2 


Creating processes using system calls fork/exec, process communication through pipes, use of low-level I / O and creation bash scripts. As part of this work I implemented a distributed information processing tool that receives, processes, records and answeres questions about viruses. Specifically, I implemented the diseaseAggregator application which creates a series of Worker processes that, along with the application, answers user queries.

## 3rd task
Directory: multiThreaded_socketComm_task3  


Thread programming and network communication. As part of this work I implemented a distributed process system that will provide the same functionality with the diseaseAggregator application of the second task. Specifically, I implemented three programs:
1. a master program that will create a series of Worker processes (as did the parent process in the second task),
2. a multi-threaded whoServer that will collect over the network summary statistics from Worker processes and queries from clients, and
3. a multithreaded client whoClient program that will create a lot threads, where each thread plays the role of a client sending queries to whoServer.

---

© Konstantinos Nikoletos
