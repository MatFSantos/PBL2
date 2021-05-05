
#  Sistema de lâmpada Smart - Manual do Usuário

Nesse manual é explicado como o usuário pode usar os recursos do ***Sistema de lâmpada Smart*** no *Website*. Ao fim desse manual o leitor saberá todas as funcionalidades e como o *Website* funciona, além de aprender como ligar a lâmpada e visualizar o seu consumo.

##  Primeiros passos

Antes de tudo, é necessário que sua placa *NodeMCU* e o *Website* já estejam devidamente configurados. Para isso, veja o manual do sistema.

Após toda as configurações feitas na placa e no site, conecte a placa à energia e espere com que ela pare de piscar a Led. A Led necessariamente ligará e desligará cinco vezes. Isso indica que as conexões foram todas feitas com sucesso. Após isso, hospede o site e abra-o. Caso tenha hospedado ele localmente pelo comando ``php artisan serve``, abra-o utilizando o link ``localhost:8000`` no navegador.
  
  Com isso já está tudo devidamente "settado" para uso ;).

## Funcionalidades do Website

O *Website* possui diversas funcionalidades: de temporizador à tabela de histórico de consumo mensal de energia por parte da Led. A seguir serão explicadas todas essas funcionalidades e como utilizá-las.


###  Botão "Atualizar"

Esse botão faz a atualização da página, mostrando o estado atual da Led e, caso tenha algum dado novo do histórico para ser acrescentado esse botão o fará.

###  Botão "Alterar estado"

Esse botão, quando pressionado, altera o estado atual da Led, mostrado logo acima.

  

###  Agendar

Essa funcionalidade é feita a partir da box de texto "Minuto 1, Minuto 2" e do botão "Agendar". Para fazer esse agendamento digite na box em quantos minutos deseja que a Led ligue a partir do horário que você está (Minuto 1), coloque uma vírgula (,) e após a vírgula coloque quantos minutos deseja que a Led permaneça ligada (Minuto 2). Clique em agendar. Pronto, seu temporizador foi agendado.

  

###  Tabela de Histórico de consumo

A tabela de histórico, que se encontra no fim da página do *Website*, possui a energia gasta pela Led na data informada em Wh e também o custo dessa energia. Esse preço já é definido no código da placa (vide manual do sistema).
> **OBS:** Para editar o preço, faça o processo presente no manual do sistema, alterando o código e carregando ele novamente.

## Outras funções

Além de ser possível acender e apagar a Led pelo site, também é possível fazer isso pela própria placa, manualmente a partir do botão *Flash*

## Adendo

Para cada vez que o Led apaga os valores na tabela já podem ser atualizados. Para a visualização desses novos valores utilize o botão "Atualizar" para atualizar a tabela. Utilize também esse botão quando não tem certeza sobre o estado atual da lâmpada, por motivos de desligamento/ligamento manual ou desligamento/ligamento por temporizador.
