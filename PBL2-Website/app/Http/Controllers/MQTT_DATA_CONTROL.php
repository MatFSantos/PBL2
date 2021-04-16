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
    public function mqtt_function($state){
        $mqtt = MQTT::connection();
        $verify = "";
        $mqtt->subscribe('CONEXAO', function (string $topic, string $message) {
            echo sprintf('Received QoS level 0 message on topic [%s]: %s', $topic, $message);    
            //FALTA COISA

        }, 0);

        $mqtt->subscribe('STATE', function (string $topic, string $message) {
            echo sprintf('Received QoS level 0 message on topic [%s]: %s', $topic, $message);    
            //FALTA COISA
        }, 0);

        if($state == "att"){
            $mqtt->publish('ON_OFF', 'att', 0);
            return view('home', compact('state','verify'));
        }
        /*else($state == "temporizer"){


        } colocar uma caixa de texto, pegar o q tem nela (tempo ou horario)*/

    }

}
