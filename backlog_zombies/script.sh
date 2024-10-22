#!/bin/bash

# Definir variáveis
PORTA=1234
IP=127.0.0.1
NUM_CLIENTES=12 # Número de clientes simultâneos
BACKLOG_INICIAL=0
BACKLOG_FINAL=12
DADOS_CONEXOES="dados_conexoes.txt"

# Compila o servidor e o cliente (certifique-se que ambos estão no mesmo diretório)
gcc server.c -o server
gcc client.c -o client

# Limpa o arquivo de dados das conexões anteriores
> $DADOS_CONEXOES

# Loop de backlog de 0 a 12
for ((backlog=$BACKLOG_INICIAL; backlog<=$BACKLOG_FINAL; backlog++)); do
    echo "Iniciando servidor com backlog $backlog"
    ./server $PORTA $backlog &
    SERVER_PID=$!

    # Esperar para garantir que o servidor está escutando
    sleep 1

    # Iniciar múltiplos clientes simultaneamente
    echo "Iniciando $NUM_CLIENTES clientes..."
    CLIENT_PIDS=()  # Array para armazenar os PIDs dos clientes
    for ((i=0; i<$NUM_CLIENTES; i++)); do
        ./client $IP $PORTA &
        CLIENT_PIDS+=($!)  # Adiciona o PID do cliente ao array
    done

    # Esperar um pouco para os clientes tentarem se conectar
    sleep 2

    # Usar netstat para verificar o número de conexões na porta especificada
    CONEXOES=$(netstat -taulpn | grep ":$PORTA" | grep ESTABLISHED | wc -l)
    echo "Conexões estabelecidas para backlog $backlog: $CONEXOES"

    # Salva o backlog e o número de conexões bem-sucedidas no arquivo
    echo "$backlog $CONEXOES" >> $DADOS_CONEXOES

    # Aguardar a finalização de todos os clientes antes de encerrar o servidor
    for pid in "${CLIENT_PIDS[@]}"; do
        kill "$pid"
        wait "$pid" 2>/dev/null
    done

    # Encerrar o servidor
    kill $SERVER_PID
    wait $SERVER_PID 2>/dev/null

    # Esperar um pouco antes de começar o próximo teste
    sleep 5
done

# Executa o script Python para gerar o gráfico
python3 gerar_grafico.py
