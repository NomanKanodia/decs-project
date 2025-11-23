#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <random>
#include <httplib.h>
#include <atomic>
#include <string>

using namespace std;

class LoadGenerator {
private:
    string host;
    int port;
    int num_threads;
    int duration_sec;
    string workload_type;
    atomic<long> total_requests{0};
    atomic<long> total_response_time{0};

public:
    LoadGenerator(const string& h, int p, int threads, int duration, const string& workload)
        : host(h), port(p), num_threads(threads), duration_sec(duration), workload_type(workload) {}

    void run_worker(int thread_id) {
        httplib::Client cli(host, port);
        auto start_time = chrono::steady_clock::now();
        auto end_time = start_time + chrono::seconds(duration_sec);
        
        random_device rd;
        mt19937 gen(rd() + thread_id); 
        
        uniform_int_distribution<> popular_key_dist(1, 10);   
        uniform_int_distribution<> unique_key_dist(1, 10000); 
        uniform_int_distribution<> op_dist(0, 2);             
        uniform_int_distribution<> mixed_op_dist(0, 100);     

        while (chrono::steady_clock::now() < end_time) {
            string key;
            int op;
            
            if (workload_type == "put_all") {
                op = 1; 
                key = "key_" + to_string(unique_key_dist(gen));
            }
            else if (workload_type == "get_all") {
                op = 0; 
                key = "key_" + to_string(unique_key_dist(gen));
            }
            else if (workload_type == "get_popular") {
                op = 0;
                key = "key_" + to_string(popular_key_dist(gen));
            }
            else if (workload_type == "get_put") {
                int rand_val = mixed_op_dist(gen);
                if (rand_val < 70) {
                    op = 0; 
                } else if (rand_val < 90) {
                    op = 1; 
                } else {
                    op = 2; 
                }
                key = "key_" + to_string(unique_key_dist(gen));
            }
            else {
                op = op_dist(gen);
                key = "key_" + to_string(unique_key_dist(gen));
            }
            
            auto req_start = chrono::steady_clock::now();
            bool success = false;
            
            switch (op) {
                case 0: { 
                    auto res = cli.Get(("/get/" + key).c_str());
                    success = res && res->status == 200;
                    break;
                }
                case 1: { 
                    auto res = cli.Put(("/set/" + key).c_str(), "value_data_" + key, "text/plain");
                    success = res && res->status == 200;
                    break;
                }
                case 2: { 
                    auto res = cli.Delete(("/delete/" + key).c_str());
                    success = res && (res->status == 200 || res->status == 404);
                    break;
                }
            }
            
            auto req_end = chrono::steady_clock::now();
            
            if (success) {
                auto duration = chrono::duration_cast<chrono::microseconds>(req_end - req_start);
                total_response_time += duration.count();
                total_requests++;
            }
        }
    }

    void run() {
        vector<thread> threads;
        
        cout << "Starting load test with workload: " << workload_type << "\n";
        cout << "Threads: " << num_threads << ", Duration: " << duration_sec << "s\n";
        
        auto start_time = chrono::steady_clock::now();
        
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back(&LoadGenerator::run_worker, this, i);
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        auto end_time = chrono::steady_clock::now();
        auto total_duration = chrono::duration_cast<chrono::seconds>(end_time - start_time).count();
        
        if (total_duration == 0) total_duration = 1;
        
        double avg_throughput = static_cast<double>(total_requests) / total_duration;
        double avg_response_time = total_requests > 0 ? 
            static_cast<double>(total_response_time) / total_requests / 1000.0 : 0.0;
        
        cout << workload_type << " Workload Results:\n";
        cout << "Total Requests: " << total_requests << "\n";
        cout << "Duration: " << total_duration << " seconds\n";
        cout << "Average Throughput: " << avg_throughput << " req/sec\n";
        cout << "Average Response Time: " << avg_response_time << " ms\n";
    }
};

int main(int argc, char* argv[]) {
    if (argc != 5) {
        cerr << "Usage: " << argv[0] << " <threads> <duration_sec> <port> <workload_type>\n";
        cerr << "Workload types:\n";
        cerr << "  put_all     - Only PUT operations (database-intensive)\n";
        cerr << "  get_all     - Only GET operations with unique keys (database-intensive)\n";
        cerr << "  get_popular - Only GET operations with popular keys (cache-friendly)\n";
        cerr << "  get_put     - Mixed operations (70% GET, 20% PUT, 10% DELETE)\n";
        return 1;
    }
    
    int threads = stoi(argv[1]);
    int duration = stoi(argv[2]);
    int port = stoi(argv[3]);
    string workload_type = argv[4];
    
    if (workload_type != "put_all" && workload_type != "get_all" && 
        workload_type != "get_popular" && workload_type != "get_put") {
        cerr << "Invalid workload type. Use: put_all, get_all, get_popular, or get_put\n";
        return 1;
    }
    
    LoadGenerator loadgen("localhost", port, threads, duration, workload_type);
    loadgen.run();
    
    return 0;
}