# HuaWei CodeCraft-2021 Cloud Resource Management
In Cloud Computing scenarios, proper resource allocation is an important step towards effective deployment and optimized VM planning. The algorithm finds the scheduling plan to minimize the costs to deploy virtual machines to servers.

## Sample Input
  - Line 1: integer n, number of available server types
  - Next n lines give information about the server in the form of (Server Type, CPU, RAM, Hardware Cost, Daily Cost)
  - Line n+2: integer m, number of available VM types
  - Next m lines give information about the VM in the form of (Instance Type, CPU, RAM, 1-Node/2-Node Deployment)
  - Line m+n+3: integer t, number of days the customer sends requests
  - Line m+n+4: integer t<sub>1</sub>, number of request the customer sends in Day 1
  - Next t<sub>1</sub> lines give information about request details in the form of (add/del, Instance Type, Instance ID)
  - ....
```bash
2 
(NV603, 92, 324, 53800, 500) 
(NV604, 128, 512, 87800, 800) 
2
(c3.large.4, 2, 8, 0) 
(c3.8xlarge.2, 32, 64, 1)
3
2
(add, c3.large.4, 5) 
(add, c3.large.4, 0)
2
(del, 0)
(add, c3.8xlarge.2, 1) 
3
(add, c3.large.4, 2) 
(del, 1)
(del, 2)
```

## Sample Output
  - Line 1: (purchase, Q), where Q is an integer indicating how many types of servers you need to expand and purchase.
  - Next Q lines, each in the format: (server type, purchase number). For example, (NV603, 2) means to purchase two NV603 servers for expansion. Each purchased server will be assigned a number, starting from zero.
  - Line Q+2: (migration, W), where W is an integer representing the number of virtual machines to be migrated.
  - Next W lines, each in the format: (VM ID, destination server ID) or (VM ID, destination server ID, destination server node). 
  - Then based on the input VM request on this day, output the server ID it is deployed to, in the format: (server ID) or (server ID, node ID(A or B))
  - ...
```bash
(purchase, 2)
(NV603, 1)
(NV604, 1)
(migration, 0)
(0, A)
(0, B)
(purchase, 0)
(migration, 0)
(1)
(purchase, 0)
(migration, 0)
(1, B)
```
