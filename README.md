# FMFCP

FMFCP é um script desenvolvido com o objetivo de auxiliar na transferência rápida de arquivos utilizando compressão paralela antes do envio pela rede. Esse código é baseado no script do CCP desenvolvido pelo professor Fábio disponível no seguinte link: https://github.com/fabiogvb/ccp

Há 2 scripts disponibilizados: ccp_sshp e ccp_vnl. A diferença de funcionalidade entre eles é que o ccp_sshp requisita 2 entradas de senha, enquanto o ccp_vnl requisita 5.

## Pré-requisitos

As duas versões são compatíveis apenas com ambiente Linux.

A versão ccp_sshp necessita da instalação do comando sshpass, que pode ser obtido rodando "sudo apt install sshpass" no terminal.

## Utilização

A sintaxe é a mesma do CCP original. Escolha a versão que você prefere utilizar, compile-a gerando um executável com o nome "ccp" e rode o seguinte comando para utilizar:

./ccp user@SOURCE_HOST:path user@DESTINATION_HOST:path
