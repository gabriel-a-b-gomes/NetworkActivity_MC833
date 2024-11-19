# Exercício 7

## Serviço de Bate-Papo com Notificações de Status

Seguem abaixo as instruções de compilação e execução do server.c e do client.c

## Compilação

Para o servidor: 

```.sh
gcc server.c -Wall -o server
```

Para o cliente:

```.sh
gcc client.c -Wall -o client
```

## Execução

Para executar o chat e utilizá-lo é simples. 

1. Servidor

Você deve fornecer a Porta do chat TCP e a Porta do serviço de Notificação UDP.

```.sh
./server <PortaTCP> <PortaUDP>
```

Após executar o servidor, serão exibidas as informações dos sockets TCP e UDP.

Exemplo:

```.sh
./server 1234 4321
Chat em execução (0.0.0.0, 1234)
Serviço de Notificação em execução (0.0.0.0, 4321)
```

2. Cliente

Para o cliente você deve fornecer o IP do servidor onde está localizado o chat (e.g 127.0.0.1) e as portas do serviço TCP e UDP nessa ordem.

```.sh
./client <IPServidor> <PortaTCP> <PortaUDP>
```

Após isso, o programa irá solicitar um nome para utilizar no chat. Basta digitá-lo e, assim, o chat estará pronto para ser usado.

Exemplo:

```
 ./client 127.0.0.1 1234 4321
Insira seu nome: MC833
Bem vindo ao chat! (127.0.0.1, 1234)
Você é o único no chat!     
Teste de Mensagem
[Tue Nov 19 19:53:54 2024] [MC833] Teste de Mensagem
```

Para sair do chat, basta apertar ctrl+C