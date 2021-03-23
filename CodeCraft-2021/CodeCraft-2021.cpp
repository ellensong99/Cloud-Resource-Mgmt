#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include <cassert>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <algorithm>
#include <set>
using namespace std;

int nDays;

struct Server {
    string type;
    int cpuCores{}, memory{}, cost{}, dailyCost{};
    unsigned long long weight = -1;
    void calcWeight()
    {
        weight = (cost + dailyCost * nDays / 2); //* 10000 / (cpuCores + memory);
    }
};

struct VM {
    string type;
    int cpuCores{}, memory{};
    bool doubleNode{};
};

struct ServerOwned;
unordered_map<int, ServerOwned*> vmIdToServerOwned;
unordered_map<int, const VM*> vmIdToVMInfo;

struct ServerOwned {
    ServerOwned(Server& s, int i): server(s), id(i){}
    Server& server;
    unordered_set<int> nodeA, nodeB, doubleNode; // vm id in this server

    // lower the better
    int weight()
    {
        bool emptyServer = nodeA.empty() && nodeB.empty() && doubleNode.empty();
        return maxCpuLeft() + maxMemoryLeft() + emptyServer * 1000;
    }

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
vector<Server> serverWeighted;
unordered_map<string, VM> VMs;


struct Request {
    enum Type { ADD, DEL };
    Type requestType;
    string vmType;
    int id{};
    int originalOrder{};
    int weight() const noexcept
    {
        assert(requestType == ADD);
        auto& vm = VMs[vmType];
        return vm.cpuCores + vm.memory;
    }
};

struct Deployment {
    struct DeploymentItem { int& physcialServerId; char node; int order; };
    void print() {
        for (const auto& d : deployments) {
            if (d.node) printf("(%d, %c)\n", d.physcialServerId, d.node);
            else printf("(%d)\n", d.physcialServerId);
        }
    }
    void fulfill(ServerOwned& server, Request req, char node = 0) {
        auto& vm = VMs[req.vmType];
        server.allocateForVM(vm, req.id, node);
        deployments.insert(DeploymentItem{ server.id, node, req.originalOrder });
        vmIdToServerOwned[req.id] = &server;
        vmIdToVMInfo[req.id] = &vm;
        // printf("Allocating VM %s core=%d mem=%d to server %d core=%d mem=%d [node=%c]\n",vm.type.c_str(), vm.cpuCores, vm.memory, server.id, server.server.cpuCores, server.server.memory, node == 0 ? 'D' : node);
        // printf("Remaining: A: core=%d/%d,mem=%d/%d\tB: core=%d/%d,mem=%d/%d\n",
        //     server.nodeACpuLeft(), server.server.cpuCores / 2, server.nodeAMemoryLeft(), server.server.memory / 2,
        //     server.nodeBCpuLeft(), server.server.cpuCores / 2, server.nodeBMemoryLeft(), server.server.memory / 2
        // );
    }
    struct cmp
    {
        bool operator()(const DeploymentItem& r1, const DeploymentItem& r2) const noexcept { return r1.order < r2.order; }
    };
    set<DeploymentItem, cmp> deployments;
};

struct Purchase {
    void print() {
        printf("(purchase, %lu)\n", items.size());
        for(const auto& item: items){
            printf("(%s, %d)\n", item.first.data(), item.second);
        }
    }
    shared_ptr<ServerOwned> buy(string type) {
        items[type] += 1;
        auto s = make_shared<ServerOwned>(servers[type], -1);
        newServers.push_back(s);
        return std::move(s);
    }
    shared_ptr<ServerOwned> buy(const Server& s) {
        return std::move(buy(s.type));
    }
    void assignId()
    {
        static int id = 0;
        for (const auto& item : items) {
            for (auto& server:newServers)
            {
                if (server->server.type == item.first) server->id = id++;
            }
        }
    }
    unordered_map<string, int> items;
    vector<shared_ptr<ServerOwned>> newServers;
};

struct Migration {
    struct MigrationAction {
        MigrationAction(int i, int i1, char i2=0):vmId(i),serverId(i1),targetNode(i2){}

        int vmId, serverId; char targetNode=0;
    };
    void print() {
        printf("(migration, %lu)\n", migrations.size());
        for(const auto& migration: migrations){
            if(!migration.targetNode)
                printf("(%d, %d)\n", migration.vmId, migration.serverId);
            else
                printf("(%d, %d, %c)\n", migration.vmId, migration.serverId, migration.targetNode);
        }
    }
    void migration(int vm, int serverId, char node) {
        migrations.emplace_back(vm, serverId, node);
    }
    vector<MigrationAction> migrations;
};

void readInput(unordered_map<string, Server>& servers, unordered_map<string, VM>& VMs, vector<vector<Request>>& requests) {
    int n;
    // read servers
    cin >> n;

    servers.reserve(n);
    serverWeighted.reserve(n);
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
        serverWeighted.push_back(s);
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
    nDays = n;
    requests.reserve(n);
    Request req;
    vector<Request> aDayRequest;
    string type;
    while (n--) {
        int j;
        cin >> j;
        aDayRequest.reserve(j);
        int order = 0;
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
            req.originalOrder = order++;
            aDayRequest.push_back(req);
        }
        requests.push_back(aDayRequest);
        aDayRequest.clear();
    }
}


unordered_map<int, int> weights;

vector<shared_ptr<ServerOwned>> ownedServers;

pair<shared_ptr<ServerOwned>, char> findAvailableServer(const VM& vm)
{
    if (vm.doubleNode) // need to deploy on two nodes of server
    {
        for (auto& ownedServer : ownedServers)
            if (ownedServer->canAllocateForVM(vm, 0))
                return make_pair(ownedServer, 0);
    }
    else // can be deployed to one node of the server.
    {
        for (auto& ownedServer : ownedServers)
        {
            if (ownedServer->canAllocateForVM(vm, 'A'))
                return make_pair(ownedServer, 'A');
            if (ownedServer->canAllocateForVM(vm, 'B'))
                return make_pair(ownedServer, 'B');
        }
    }
    return make_pair(nullptr, 0); // not found
}

bool tryMigrate(int vmId, shared_ptr<ServerOwned> so, Migration& m)
{
    auto& vm = *vmIdToVMInfo[vmId];
    auto res = findAvailableServer(vm);
    if(res.first && res.first.get() != so.get())
    {
        res.first->allocateForVM(vm, vmId, res.second);
        so->deallocateForVM(vmId);
        vmIdToServerOwned[vmId] = res.first.get();
        m.migration(vmId, res.first->id, res.second);
        return true;
    }
    return false;
}
int main()
{
    // debugging only, remember to remove when submitting
    // freopen("../CodeCraft-2021/training-2.txt","r", stdin);
    //freopen("../judger/out", "w", stdout);

    vector<vector<Request>> requests;
    readInput(servers, VMs, requests);

    for (auto& s : serverWeighted) s.calcWeight();

    sort(serverWeighted.begin(), serverWeighted.end(), [](Server& s1, Server& s2)-> bool
        {
            return s1.weight < s2.weight;
        });
    
    for (auto& day : requests) {
        //for (auto& os : ownedServers) weights[os->id] = os->weight();
        //sort(ownedServers.begin(), ownedServers.end(), [](shared_ptr<ServerOwned> s1, shared_ptr<ServerOwned> s2)-> bool
        //    {   return weights[s1->id] < weights[s2->id]; });

        Migration m;
        for (auto& so : ownedServers) {
            if (m.migrations.size() + 1 >= 5 * ownedServers.size() / 1000) break;

            if (so->nodeA.size() + so->nodeB.size() + so->doubleNode.size() == 1) {
                int vmId = 0;
                if (!so->nodeA.empty()) vmId = *so->nodeA.begin();
                else if (!so->nodeB.empty()) vmId = *so->nodeB.begin();
                else if (!so->doubleNode.empty()) vmId = *so->doubleNode.begin();
                else continue;

                tryMigrate(vmId, so, m);
            }
        }
        Purchase p;
        Deployment d;

        // partition and sort
        auto startIter = day.begin();
        auto startIterInvalid = startIter->requestType == Request::DEL;
        auto cmp = [](const Request& r1, const Request& r2)-> bool {
            return r1.weight() > r2.weight(); // allocate larger vm first.
        };
        for(auto iter = day.begin(); iter != day.end(); iter++)
        {
            if(iter->requestType==Request::DEL){
                if (!startIterInvalid) {
                    sort(startIter, iter, cmp);
                }
                startIterInvalid = true;
            }else if (startIterInvalid)
            {
                startIterInvalid = false;
                startIter = iter;
            }
        }
        if (!startIterInvalid && day[day.size()-1].requestType==Request::ADD) {
            sort(startIter, day.end(), cmp);
        }

        for (const auto& req : day) {
            if (req.requestType == Request::Type::ADD) {
                auto& vm = VMs[req.vmType];
                auto res = findAvailableServer(vm);
                if(res.first)
                {
                    d.fulfill(*res.first, req, res.second);
                }
                else // none of our current servers can host this vm.
                {
                    bool fulfilled = false;
                    for (auto& spec : serverWeighted)
                    {
                        if ((vm.doubleNode && spec.cpuCores >= vm.cpuCores && spec.memory >= vm.memory) ||
                            (!vm.doubleNode && spec.cpuCores / 2 >= vm.cpuCores && spec.memory / 2 >= vm.memory))
                        {
                            auto newServer = p.buy(spec);
                            ownedServers.push_back(newServer);
                            d.fulfill(*newServer, req, vm.doubleNode ? 0 : 'A');
                            fulfilled = true;
                            break;
                        }
                    }
                    if (!fulfilled)
                    {
                        cerr << "Failed to fulfill VM " << vm.type << endl;
                        exit(-1);
                    }
                }
            }
            else
            {
                auto serverOwned = vmIdToServerOwned[req.id];
                serverOwned->deallocateForVM(req.id);
            }
        }
        p.assignId();
        p.print();
        m.print();
        d.print();
    }
    return 0;
}
