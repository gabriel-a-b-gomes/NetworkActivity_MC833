import matplotlib.pyplot as plt

arquivo_dados = "dados_conexoes.txt"

backlog_values = []
successful_connections = []

# Ler o arquivo de dados
with open(arquivo_dados, "r") as f:
    for line in f:
        backlog, conexoes = map(int, line.split())
        backlog_values.append(backlog)
        successful_connections.append(conexoes)

# Plotando o gráfico
plt.plot(backlog_values, successful_connections, marker='o')
plt.xlabel("Valor do Backlog")
plt.ylabel("Número de Conexões Bem-Sucedidas")
plt.title("Impacto do Backlog no Número de Conexões Bem-Sucedidas")
plt.grid()
plt.savefig('./charts/resultado_backlog.png')
print("Gráfico salvo em ./charts/resultado_backlog.png")
