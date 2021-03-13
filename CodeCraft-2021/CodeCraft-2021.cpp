#include <iostream>
#include <vector>
#include <cassert>
#include <unordered_map>
using namespace std;
struct Server {
    string type;
    int cpuCores{}, memory{}, cost{}, dailyCost{};
};
struct ServerOwned {
    Server& server;
    int id;
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

unordered_map<string, Server> servers;
vector<VM> VMs;


struct Deployment {
    struct DeploymentItem { int physcialServerId; char node; };
    void print() {
        for (const auto &d:deployments) {
            if (d.node) printf("(%d, %c)\n", d.physcialServerId, d.node);
            else printf("(%d)\n", d.physcialServerId);
        }
    }
    void fulfill(ServerOwned server, char node = 0) {
        deployments.push_back(DeploymentItem{server.id, node});
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
    ServerOwned buy(string type) {
        static int id = 0;
        auto newServer = ServerOwned{servers[type], id++};
        items[type]+=1;
        return newServer;
    }
    ServerOwned buy(const Server& s) {
        return buy(s.type);
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

void readInput(unordered_map<string, Server>& servers, vector<VM>& VMs, vector<vector<Request>>& requests) {
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
        VMs.push_back(vm);
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
    freopen("../training-1.txt","r", stdin);

    vector<vector<Request>> requests;
    readInput(servers, VMs, requests);
    vector<ServerOwned> ownedServers;

    //vector<pair<Purchase, Migration>> solution;
    for(const auto& day: requests) {
        Purchase p;
        Migration m;
        Deployment d;
        for(const auto& req: day){
            ownedServers.push_back(p.buy(servers.begin()->second));
            if(req.requestType == Request::Type::ADD) {
                d.fulfill(*ownedServers.end());
            }
        }
        //solution.emplace_back(std::move(p),std::move(m));
        p.print();
        m.print();
        d.print();
    }
    return 0;
}
