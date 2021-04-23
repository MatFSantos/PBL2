<!DOCTYPE html>
<html lang="pt-br">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Controle da Placa NodeMCU8266</title>

    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.0.0-beta3/dist/css/bootstrap.min.css" rel="stylesheet" integrity="sha384-eOJMYsd53ii+scO/bJGFsiCZc+5NDVN2yr8+0RDqr0Ql0h+rP48ckxlpbzKgwra6" crossorigin="anonymous">

</head>
<body>

<!--<button type="button" href="{{url('/home/0')}}" class="btn btn-dark">Dark</button>
<button type="button" href="{{url('/home/1')}}" class="btn btn-light">Light</button>
-->

<p> Estado atual da Lâmpada: <strong> {{$verify}} </strong></p>
<!-- Colocar o que vem do callback pelo MQTT no topico STATE. -->
<p>  </p>

<a id="buttonState" type="button" class="btn btn-dark"> Mudar estado </a>
    <div class="form-floating">
        <input id="userSchedule"  type="text" placeholder="Escreva quando deseja agendar" size="28"> </input>
        <a id="buttonSchedule" type="button" class="btn btn-dark"> Agendar! </a>
    </div>

<script src="https://cdn.jsdelivr.net/npm/@popperjs/core@2.9.1/dist/umd/popper.min.js" integrity="sha384-SR1sx49pcuLnqZUnnPwx6FCym0wLsk5JZuNx2bPPENzswTNFaQU1RDvt3wT4gWFG" crossorigin="anonymous"></script>
<script src="https://code.jquery.com/jquery-3.6.0.min.js"></script>
<script src="https://cdn.jsdelivr.net/npm/bootstrap@5.0.0-beta3/dist/js/bootstrap.min.js" integrity="sha384-j0CNLUeiqtyaRmlzUHCPZ+Gy5fQu0dQ6eZ/xAww941Ai1SxSY+0EQqNXNE6DZiVc" crossorigin="anonymous"></script>
<script src="https://cdn.jsdelivr.net/npm/bootstrap@5.0.0-beta3/dist/js/bootstrap.bundle.min.js" integrity="sha384-JEW9xMcG8R+pH31jmWH6WWP0WintQrMb4s7ZOdauHnUtxwoG2vI5DkLtS3qm9Ekf" crossorigin="anonymous"></script>    
<script src="https://cdnjs.cloudflare.com/ajax/libs/jquery.form/4.3.0/jquery.form.min.js"> </script>
<script> 
    $('#buttonState').on('click', function(){
        console.log("Tá funcionando!");
        $.ajax({
            url: '{{route('att')}}',
            type: 'get',
            success: function(response){
                console.log(response);
            }
            
        });
    });

    $('#buttonSchedule').on('click', function(){
        console.log("Tá funcionando!");
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
