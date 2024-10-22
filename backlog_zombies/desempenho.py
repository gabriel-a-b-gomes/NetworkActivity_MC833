import os
import subprocess
import time
import threading
import argparse
import matplotlib.pyplot as plt

def run_server(port, backlog):
    cmd = f"./server {port} {backlog}" 
    proc = subprocess.Popen(cmd, shell=True)
    return proc

def run_client(client_id, results, port):
    start = time.time()
    try:
        subprocess.run(f"./client 127.0.0.1 {port}", shell=True, timeout=5)
        end = time.time()
        results.append({ "client_id": client_id, "time": end - start, "sucesso": True })  # Sucesso na conexão
    except subprocess.TimeoutExpired:
        results.append({ "client_id": client_id, "time": None, "sucesso": False })  # Falha na conexão

def test_backlog(port, backlog, num_clients=30):
    server = run_server(port, backlog)
    time.sleep(2) 
    
    results = []
    
    threads = []
    for i in range(num_clients):
        t = threading.Thread(target=run_client, args=(i, results, port))
        threads.append(t)
        t.start()
    
    for t in threads:
        t.join()

    server.terminate() 
    
    time.sleep(5)
    
    return results

def evaluate_performance(port):
    backlog_range = range(0, 16)
    avg_response_times = []
    successful_connections = []
    
    for backlog in backlog_range:
        results = test_backlog(port, backlog)
        response_times = [r["time"] for r in results if r["sucesso"]]
        successful_conn = sum(1 for r in results if r["sucesso"])
        
        if response_times:
            avg_time = sum(response_times) / len(response_times)
        else:
            avg_time = float('inf')
        
        avg_response_times.append(avg_time)
        successful_connections.append(successful_conn)
        print(f"Backlog {backlog}: Tempo médio = {avg_time:.3f}s, Qtde. conn. estabelecidas = {successful_conn}/{len(results)}")

        port += 1

    plt.figure(figsize=(10, 5))
    
    plt.subplot(1, 2, 1)
    plt.plot(backlog_range, avg_response_times, marker='o', label='Tempo Médio de Resposta (s)')
    plt.xlabel('Backlog')
    plt.ylabel('Tempo (s)')
    plt.title('Tempo Médio de Resposta vs Backlog')
    plt.grid(True)
    
    plt.subplot(1, 2, 2)
    plt.plot(backlog_range, successful_connections, marker='o', color='green', label='Conexões Bem-sucedidas')
    plt.xlabel('Backlog')
    plt.ylabel('Conexões')
    plt.title('Conexões Bem-sucedidas vs Backlog')
    plt.grid(True)

    plt.tight_layout()
    plt.savefig('./charts/desempenho.png')
    print("Gráfico salvo em ./charts/desempenho.png")

def main():
    parser = argparse.ArgumentParser(description="Script para testar desempenho do servidor com diferentes valores de backlog.")
    parser.add_argument("port", type=int, help="Porta de execução do servidor")
    args = parser.parse_args()

    evaluate_performance(args.port)

main()
