#include <iostream>
#include <vector>
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

void readInput(vector<Server>& servers, vector<VM>& VMs, vector<vector<Request>>& requests) {
    int n;
    // read servers
    cin >> n;
    cerr << n << endl;

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
        servers.push_back(s);
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
    freopen("training-1.txt","r", stdin);

    vector<Server> servers;
    vector<VM> VMs;
    vector<vector<Request>> requests;
    readInput(servers, VMs, requests);

    return 0;
}
