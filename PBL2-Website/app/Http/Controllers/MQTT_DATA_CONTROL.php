<?php

namespace App\Http\Controllers;
use PhpMqtt\Client\Facades\MQTT;
use Illuminate\Http\Request;

class MQTT_DATA_CONTROL extends Controller
{
    
    public function home(){
        
        $state = "NaN";
        $verify = "NaN";
        return view('home', compact('state','verify'));
    }
    public function mqtt_function(){
        $verify = "";
        
        $state = "att";
        $mqtt->subscribe('STATE', function (string $topic, string $message) {
            echo sprintf('Received QoS level 0 message on topic [%s]: %s', $topic, $message);    
            //FALTA COISA
        }, 0);

        if($state == "att"){
            $mqtt->publish('ON_OFF', 'att', 0);
            $stateSuccess['success'] = true;
            echo json_enconde($stateSuccess);
            return response()->json(['verify'=>$verify, 'state'=> $state]);
        }
        $stateSuccess['success'] = false;
        echo json_enconde($stateSuccess);
        return response()->json(['verify'=>$verify, 'state'=> $state]);
        /*else($state == "temporizer"){


        } colocar uma caixa de texto, pegar o q tem nela (tempo ou horario)*/

    }

}
