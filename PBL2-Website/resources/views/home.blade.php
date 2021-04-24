<!DOCTYPE html>
<html lang="pt-br">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>PBL 2 - Controle de lâmpada</title>

    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.0.0-beta3/dist/css/bootstrap.min.css" rel="stylesheet" integrity="sha384-eOJMYsd53ii+scO/bJGFsiCZc+5NDVN2yr8+0RDqr0Ql0h+rP48ckxlpbzKgwra6" crossorigin="anonymous">
    <style> 
        .botao:active{
            position: relative;
            top: 5px;
            box-shadow: none;
        }
        h1.title{
                font-family: Georgia, serif;
                font-weight: 900;
                color: rgb(0, 0, 0); 
                font-size: 50px;
            }
        p.title{
            color: rgb(0, 0, 0);
            font-weight: 400;
            font-size: 15px;
            font-family: Georgia, serif;
        }
        .botao-agendar{
            position: relative;
        }
        .botao-agendar:active{
            position: relative;
            top: 5px;
            box-shadow: none;
        }
    </style>
</head>

<body id = "corpo">
    <center>
        <h1 class = "title">LIGUE OU DESLIGUE SUA LÂMPADA</h1>
        <p class = "title">Estado Atual: <span id="state">  </span></p>

        <button id="buttonState" class="btn btn-outline-primary botao" >ALTERAR ESTADO</button>

        <h1 class = "title" >AGENDE UM TEMPORIZADOR</h1>
        <p class = "title" style = "font-size: 20px;">Coloque em quantos minutos deseja ligar (minutos 1) vírgula (,) <br/>quantos minutos deseja deixar ligada (minuto 2).</p>
        <p class = "title" style = "font-size: 13px;">OBS: coloque zero antes da virgula para ligar imediatamente, e/ou zero depois da vírgula<br/>para não determinar tempo de desligamento</p>
        <input  id="userSchedule" type = "text" name = "tempo" placeholder="Minuto 1, Minuto 2" />
        <button id= "buttonSchedule" class="btn btn-outline-primary botao botao-agendar">AGENDAR</button>

        <input  id="userValor" type = "text" placeholder="Coloque o valor da taxa mensal" />
        <button id= "buttonValor" class="btn btn-outline-primary botao botao-agendar">OK</button>

        <div class = "container" style = "position: relative; top: 130px;" >
            <h1 class = "title">TABELA DE HISTÓRICO MENSAL</h1>
            <table class="table">
                <thead>
                    <tr>
                        <th scope="col">Dia</th>
                        <th scope="col">$ Gasto (KWh)</th>
                    </tr>
                </thead>
                <tbody>
                    <tr>
                        <th scope="row">1</th>
                        <td>x reais</td>
                    </tr>
                    <tr>
                        <th scope="row">2</th>
                        <td>x reais</td>
                    </tr>
                    <tr>
                        <th scope="row">3</th>
                        <td >x reais</td>
                    </tr>
                </tbody>
            </table>
        </div>
    </center>
    
    <script src="https://cdn.jsdelivr.net/npm/@popperjs/core@2.9.1/dist/umd/popper.min.js" integrity="sha384-SR1sx49pcuLnqZUnnPwx6FCym0wLsk5JZuNx2bPPENzswTNFaQU1RDvt3wT4gWFG" crossorigin="anonymous"></script>
    <script src="https://code.jquery.com/jquery-3.6.0.min.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.0.0-beta3/dist/js/bootstrap.min.js" integrity="sha384-j0CNLUeiqtyaRmlzUHCPZ+Gy5fQu0dQ6eZ/xAww941Ai1SxSY+0EQqNXNE6DZiVc" crossorigin="anonymous"></script>
    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.0.0-beta3/dist/js/bootstrap.bundle.min.js" integrity="sha384-JEW9xMcG8R+pH31jmWH6WWP0WintQrMb4s7ZOdauHnUtxwoG2vI5DkLtS3qm9Ekf" crossorigin="anonymous"></script>    
    <script src="https://cdnjs.cloudflare.com/ajax/libs/jquery.form/4.3.0/jquery.form.min.js"> </script>

    <script> 
        $('#buttonState').on('click', function(){
            $.ajax({
                url: '{{route('att')}}',
                type: 'get',
                success: function(response){
                    console.log(response['state']);
                    $('#state').html(response['state']);
                }
                
            });
        });

        $('#buttonSchedule').on('click', function(){
            let data =  {'cont2': $('#userSchedule').val()};
            console.log(data);
            $.ajax({
                url: '{{route('schedule')}}',
                type: 'get',
                data: {data},
                success: function(response){
                    console.log(response);
                }
                
            });
        });

    </script>
</body>
</html>
