#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include <cassert>
#include <unordered_map>
#include <unordered_set>
using namespace std;

struct Server {
    string type;
    int cpuCores{}, memory{}, cost{}, dailyCost{};
};

struct VM {
    string type;
    int cpuCores{}, memory{};
    bool doubleNode{};
};

struct Request {
    enum Type {ADD,DEL};
    Type requestType;
    string vmType;
    int id{};
};

struct ServerOwned;
unordered_map<int, ServerOwned*> vmIdToServerOwned;
unordered_map<int, const VM*> vmIdToVMInfo;

struct ServerOwned {
    ServerOwned(Server& s, int i): server(s), id(i){}
    Server& server;
    unordered_set<int> nodeA, nodeB, doubleNode; // vm id in this server

    int id;
    int nodeACpuUsed = 0, nodeAMemoryUsed = 0, nodeBCpuUsed = 0, nodeBMemoryUsed = 0;
    int nodeACpuLeft() { return server.cpuCores / 2 - nodeACpuUsed; }
    int nodeBCpuLeft() { return server.cpuCores / 2 - nodeBCpuUsed; }
    int nodeAMemoryLeft() { return server.memory / 2 - nodeAMemoryUsed; }
    int nodeBMemoryLeft() { return server.memory / 2 - nodeBMemoryUsed; }
    int maxCpuLeft() { return max(nodeACpuLeft(), nodeBCpuLeft()); }
    int maxMemoryLeft() { return max(nodeAMemoryLeft(), nodeBMemoryLeft()); }
    void allocateForVM(const VM& vm, int vmId, char node)
    {
        if (node == 'A') {
            nodeACpuUsed += vm.cpuCores;
            nodeAMemoryUsed += vm.memory;
            nodeA.insert(vmId);
        }else if (node == 'B') {
            nodeBCpuUsed += vm.cpuCores;
            nodeBMemoryUsed += vm.memory;
            nodeB.insert(vmId);
        }else
        {
            nodeACpuUsed += vm.cpuCores / 2;
            nodeAMemoryUsed += vm.memory / 2;
            nodeBCpuUsed += vm.cpuCores / 2;
            nodeBMemoryUsed += vm.memory / 2;
            doubleNode.insert(vmId);
        }
    }
    void deallocateForVM(int vmId)
    {
        const VM& vm = *vmIdToVMInfo[vmId];
        if (findAndRemove(nodeA, vmId)) {
            nodeACpuUsed -= vm.cpuCores;
            nodeAMemoryUsed -= vm.memory;
        }
        else if (findAndRemove(nodeB, vmId)) {
            nodeBCpuUsed -= vm.cpuCores;
            nodeBMemoryUsed -= vm.memory;
        }
        else
        {
            assert(findAndRemove(doubleNode, vmId));
            nodeACpuUsed -= vm.cpuCores / 2;
            nodeAMemoryUsed -= vm.memory / 2;
            nodeBCpuUsed -= vm.cpuCores / 2;
            nodeBMemoryUsed -= vm.memory / 2;
        }
    }

    bool canAllocateForVM(const VM& vm, char node)
    {
        if (node == 'A') {
            if (nodeACpuUsed + vm.cpuCores > server.cpuCores / 2) return false;
            if (nodeAMemoryUsed + vm.memory > server.memory / 2) return false;
        }
        else if (node == 'B') {
            if (nodeBCpuUsed + vm.cpuCores > server.cpuCores / 2) return false;
            if (nodeBMemoryUsed + vm.memory > server.memory / 2) return false;
        }
        else
        {
            if (nodeACpuUsed + vm.cpuCores / 2 > server.cpuCores / 2) return false;
            if (nodeAMemoryUsed + vm.memory / 2 > server.memory / 2) return false;
            if (nodeBCpuUsed + vm.cpuCores / 2 > server.cpuCores / 2) return false;
            if (nodeBMemoryUsed + vm.memory / 2 > server.memory / 2) return false;
        }
        return true;
    }

private:
    static bool findAndRemove(unordered_set<int>& s, int v)
    {
        auto iter = s.find(v);
        bool found = iter != s.end();
        if (found) s.erase(iter);
        return found;
    }
    
};


unordered_map<string, Server> servers;

unordered_map<string, VM> VMs;

struct Deployment {
    struct DeploymentItem { int physcialServerId; char node; };
    void print() {
        for (const auto &d:deployments) {
            if (d.node) printf("(%d, %c)\n", d.physcialServerId, d.node);
            else printf("(%d)\n", d.physcialServerId);
        }
    }
    void fulfill(ServerOwned& server, Request req, char node = 0) {
        auto& vm = VMs[req.vmType];
        server.allocateForVM(vm, req.id, node);
        deployments.push_back(DeploymentItem{ server.id, node });
        vmIdToServerOwned[req.id] = &server;
        vmIdToVMInfo[req.id] = &vm;
        /*printf("Allocating VM core=%d mem=%d to server %d core=%d mem=%d [node=%c]\n", vm.cpuCores, vm.memory, server.id, server.server.cpuCores, server.server.memory, node == 0 ? 'D' : node);
        printf("Remaining: A: core=%d/%d,mem=%d/%d\tB: core=%d/%d,mem=%d/%d\n",
            server.nodeACpuLeft(), server.server.cpuCores / 2, server.nodeAMemoryLeft(), server.server.memory / 2,
            server.nodeBCpuLeft(), server.server.cpuCores / 2, server.nodeBMemoryLeft(), server.server.memory / 2
        );*/
    }
    vector<DeploymentItem> deployments;
};

struct Purchase {
    void print() {
        printf("(purchase, %d)\n", items.size());
        for(const auto& item: items){
            printf("(%s, %d)\n", item.first.data(), item.second);
        }
    }
    shared_ptr<ServerOwned> buy(string type) {
        static int id = 0;
        items[type] += 1;
        return std::move(make_shared<ServerOwned>(servers[type], id++));
    }
    shared_ptr<ServerOwned> buy(const Server& s) {
        return std::move(buy(s.type));
    }
    unordered_map<string, int> items;
};

struct Migration {
    struct MigrationAction {
        MigrationAction(int i, int i1, char i2=0):vmId(i),serverId(i1),targetNode(i2){}

        int vmId, serverId; char targetNode=0;
    };
    void print() {
        printf("(migration, %d)\n", migrations.size());
        for(const auto& migration: migrations){
            if(migration.targetNode)
                printf("(%d, %d)\n", migration.vmId, migration.serverId);
            else
                printf("(%d, %d, %c)\n", migration.vmId, migration.serverId, migration.targetNode);
        }
    }
    void migration(int vm, int serverId) {
        migrations.push_back(MigrationAction{vm, serverId, '\0'});
    }
    vector<MigrationAction> migrations;
};

void readInput(unordered_map<string, Server>& servers, unordered_map<string, VM>& VMs, vector<vector<Request>>& requests) {
    int n;
    // read servers
    cin >> n;

    servers.reserve(n);
    Server s;
    while (n--) {
        cin.ignore(2); // \n and (
        cin >> s.type;
        cin.ignore(1); // ,
        cin >> s.cpuCores;
        cin.ignore(1);
        cin >> s.memory;
        cin.ignore(1);
        cin >> s.cost;
        cin.ignore(1);
        cin >> s.dailyCost;
        cin.ignore(1); // )
        s.type.pop_back();
        servers[s.type] = s;
    }
    // read vms
    cin >> n;
    VMs.reserve(n);
    VM vm;
    while (n--) {
        cin.ignore(2); // \n and (
        cin >> vm.type;
        cin.ignore(1); // ,
        cin >> vm.cpuCores;
        cin.ignore(1);
        cin >> vm.memory;
        cin.ignore(1);
        cin >> vm.doubleNode;
        cin.ignore(1);
        vm.type.pop_back();
        VMs[vm.type] = vm;
    }
    // read requests
    cin >> n;
    requests.reserve(n);
    Request req;
    vector<Request> aDayRequest;
    string type;
    while (n--) {
        int j;
        cin >> j;
        aDayRequest.reserve(j);
        while (j--) {
            cin.ignore(2);
            cin >> type;
            if (type[0] == 'a') { //add
                req.requestType = Request::Type::ADD;
                cin >> req.vmType;
                cin.ignore(1);
                cin >> req.id;
                cin.ignore(1);
                req.vmType.pop_back();
            } else {
                req.requestType = Request::Type::DEL;
                cin >> req.id;
                cin.ignore(1);
                req.vmType.clear();
            }
            aDayRequest.push_back(req);
        }
        requests.push_back(aDayRequest);
        aDayRequest.clear();
    }
}

int main()
{
    // debugging only, remember to remove when submitting
    freopen("../CodeCraft-2021/training-1.txt","r", stdin);

    vector<vector<Request>> requests;
    readInput(servers, VMs, requests);
    vector<shared_ptr<ServerOwned>> ownedServers;

    for(const auto& day: requests) {
        Purchase p;
        Migration m;
        Deployment d;
        for(const auto& req: day){
            if(req.requestType == Request::Type::ADD) {
                auto& vm = VMs[req.vmType];
                bool fulfilled = false;
                if(vm.doubleNode) // need to deploy on two nodes of server
                {
                    for (auto& ownedServer : ownedServers)
                    {
                        if(ownedServer->canAllocateForVM(vm, 0))
                        {
                            d.fulfill(*ownedServer, req, 0);
                            fulfilled = true;
                            break;
                        }
                    }
                } else // can be deployed to one node of the server.
                {
                    for (auto& ownedServer : ownedServers)
                    {
                        char node = 'A';
                        if (ownedServer->canAllocateForVM(vm, node))
                        {
                            d.fulfill(*ownedServer, req, node);
                            fulfilled = true;
                            break;
                        }
                        node = 'B';
                        if (ownedServer->canAllocateForVM(vm, node))
                        {
                            d.fulfill(*ownedServer, req, node);
                            fulfilled = true;
                            break;
                        }
                    }
                }
                if(!fulfilled) // none of our current servers can host this vm.
                {
                    for(auto& specPair: servers)
                    {
                        auto spec = specPair.second;
                        if( (vm.doubleNode && spec.cpuCores>=vm.cpuCores && spec.memory>=vm.memory) ||
                            (!vm.doubleNode&& spec.cpuCores/2 >= vm.cpuCores && spec.memory/2 >= vm.memory))
                        {
                            auto newServer = p.buy(specPair.second);
                            ownedServers.push_back(newServer);
                            d.fulfill(*newServer, req, 'A');
                            break;
                        }
                    }
                }
            }else
            {
                vmIdToServerOwned[req.id]->deallocateForVM(req.id);
            }
        }
        p.print();
        m.print();
        d.print();
    }
    return 0;
}
